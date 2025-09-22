#include "CropDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QFormLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>

CropSelectionWidget::CropSelectionWidget(QWidget *parent) : QWidget(parent)
{
    setMinimumSize(640, 360);
    rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

    // Set default selection to full frame
    selectedRect = rect();
    rubberBand->setGeometry(selectedRect);
    rubberBand->show();

    // Track if we're currently selecting
    isSelecting = false;
}

void CropSelectionWidget::setVideoFrame(const QPixmap &frame)
{
    videoFrame = frame;

    // Calculate frame size to maintain aspect ratio
    if (!videoFrame.isNull())
    {
        float widgetRatio = (float)width() / height();
        float frameRatio = (float)videoFrame.width() / videoFrame.height();

        if (frameRatio > widgetRatio)
        {
            // Width limited
            frameRect.setWidth(width());
            frameRect.setHeight(width() / frameRatio);
            frameRect.moveTop((height() - frameRect.height()) / 2);
            frameRect.moveLeft(0);
        }
        else
        {
            // Height limited
            frameRect.setHeight(height());
            frameRect.setWidth(height() * frameRatio);
            frameRect.moveLeft((width() - frameRect.width()) / 2);
            frameRect.moveTop(0);
        }

        // Reset selection to full frame
        selectedRect = frameRect;
        rubberBand->setGeometry(selectedRect);
    }

    update();
}

QRect CropSelectionWidget::getSelectedRect() const
{
    if (videoFrame.isNull() || frameRect.isEmpty())
        return QRect();

    // Convert from widget coordinates to video frame coordinates
    float xScale = (float)videoFrame.width() / frameRect.width();
    float yScale = (float)videoFrame.height() / frameRect.height();

    QRect videoRect;
    videoRect.setLeft((selectedRect.left() - frameRect.left()) * xScale);
    videoRect.setTop((selectedRect.top() - frameRect.top()) * yScale);
    videoRect.setWidth(selectedRect.width() * xScale);
    videoRect.setHeight(selectedRect.height() * yScale);

    // Ensure coordinates are valid
    videoRect = videoRect.intersected(QRect(0, 0, videoFrame.width(), videoFrame.height()));

    return videoRect;
}

void CropSelectionWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    // Fill background
    painter.fillRect(rect(), Qt::black);

    // Draw video frame
    if (!videoFrame.isNull())
    {
        painter.drawPixmap(frameRect, videoFrame);
    }
}

void CropSelectionWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        isSelecting = true;
        origin = event->pos();

        // Begin new rubber band
        if (!rubberBand)
            rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

        rubberBand->setGeometry(QRect(origin, QSize()));
        rubberBand->show();
    }
}

void CropSelectionWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isSelecting)
    {
        // Update rubber band
        QRect rect = QRect(origin, event->pos()).normalized();

        // Constrain to video frame
        rect = rect.intersected(frameRect);

        rubberBand->setGeometry(rect);
    }
}

void CropSelectionWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelecting)
    {
        isSelecting = false;

        // Finalize selection
        selectedRect = QRect(origin, event->pos()).normalized();

        // Constrain to video frame
        selectedRect = selectedRect.intersected(frameRect);

        // Minimum size check
        if (selectedRect.width() < 10 || selectedRect.height() < 10)
        {
            selectedRect = frameRect;
        }

        rubberBand->setGeometry(selectedRect);
        emit selectionChanged();
    }
}

CropDialog::CropDialog(const QString &videoFile, int videoWidth, int videoHeight, QWidget *parent)
    : QDialog(parent), videoFile(videoFile), originalWidth(videoWidth), originalHeight(videoHeight)
{
    setWindowTitle("Crop Video");
    resize(800, 600);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Visual selection area
    selectionWidget = new CropSelectionWidget(this);
    mainLayout->addWidget(selectionWidget);

    // Manual input area
    QFormLayout *formLayout = new QFormLayout();

    xInput = new QSpinBox(this);
    xInput->setRange(0, videoWidth);
    xInput->setSuffix(" px");
    formLayout->addRow("X:", xInput);

    yInput = new QSpinBox(this);
    yInput->setRange(0, videoHeight);
    yInput->setSuffix(" px");
    formLayout->addRow("Y:", yInput);

    widthInput = new QSpinBox(this);
    widthInput->setRange(10, videoWidth);
    widthInput->setValue(videoWidth);
    widthInput->setSuffix(" px");
    formLayout->addRow("Width:", widthInput);

    heightInput = new QSpinBox(this);
    heightInput->setRange(10, videoHeight);
    heightInput->setValue(videoHeight);
    heightInput->setSuffix(" px");
    formLayout->addRow("Height:", heightInput);

    mainLayout->addLayout(formLayout);

    // Load video frame for preview
    QLabel *loadingLabel = new QLabel("Loading video frame...", this);
    mainLayout->addWidget(loadingLabel);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Update numeric values when selection changes
    connect(selectionWidget, &CropSelectionWidget::selectionChanged,
            this, &CropDialog::updateInputsFromSelection);

    // Update selection when numeric values change
    connect(xInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CropDialog::validateDimensions);
    connect(yInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CropDialog::validateDimensions);
    connect(widthInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CropDialog::validateDimensions);
    connect(heightInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CropDialog::validateDimensions);

    // Extract frame for preview using FFMPEG
    QProcess *ffmpeg = new QProcess(this);

    connect(ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, ffmpeg, loadingLabel](int exitCode, QProcess::ExitStatus exitStatus)
            {
                if (exitCode == 0)
                {
                    // Load the extracted frame
                    QPixmap frame;
                    frame.load("temp_frame.jpg");
                    if (!frame.isNull())
                    {
                        selectionWidget->setVideoFrame(frame);
                        loadingLabel->hide();
                    }
                    else
                    {
                        loadingLabel->setText("Failed to load video frame");
                    }
                }
                else
                {
                    loadingLabel->setText("Error extracting frame: " +
                                          QString(ffmpeg->readAllStandardError()));
                }
                ffmpeg->deleteLater();
            });

    // Extract a frame from the middle of the video
    QStringList args;
    args << "-y" << "-i" << videoFile
         << "-ss" << "00:00:01" // 1 second in
         << "-frames:v" << "1"
         << "-q:v" << "2"
         << "temp_frame.jpg";

    ffmpeg->start("ffmpeg", args);
}

CropDialog::~CropDialog()
{
    // Clean up temp file
    QFile::remove("temp_frame.jpg");
}

int CropDialog::getX() const
{
    return xInput->value();
}

int CropDialog::getY() const
{
    return yInput->value();
}

int CropDialog::getWidth() const
{
    return widthInput->value();
}

int CropDialog::getHeight() const
{
    return heightInput->value();
}

QStringList CropDialog::getFFMPEGArguments(const QString &outputFile) const
{
    QStringList args;
    QString cropFilter = QString("crop=%1:%2:%3:%4")
                             .arg(getWidth())
                             .arg(getHeight())
                             .arg(getX())
                             .arg(getY());

    args << "-y"                // Overwrite output files without asking
         << "-i" << videoFile   // Input file
         << "-vf" << cropFilter // Crop video filter
         << "-c:a" << "copy"    // Copy audio codec (no re-encode)
         << outputFile;         // Output file

    return args;
}

void CropDialog::validateDimensions()
{
    // Ensure crop area stays within video boundaries
    if (xInput->value() + widthInput->value() > originalWidth)
    {
        widthInput->setValue(originalWidth - xInput->value());
    }

    if (yInput->value() + heightInput->value() > originalHeight)
    {
        heightInput->setValue(originalHeight - yInput->value());
    }
}

void CropDialog::updateInputsFromSelection()
{
    QRect rect = selectionWidget->getSelectedRect();

    // Block signals to prevent recursive updates
    xInput->blockSignals(true);
    yInput->blockSignals(true);
    widthInput->blockSignals(true);
    heightInput->blockSignals(true);

    xInput->setValue(rect.x());
    yInput->setValue(rect.y());
    widthInput->setValue(rect.width());
    heightInput->setValue(rect.height());

    xInput->blockSignals(false);
    yInput->blockSignals(false);
    widthInput->blockSignals(false);
    heightInput->blockSignals(false);
}