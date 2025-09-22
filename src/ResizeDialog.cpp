#include "ResizeDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QProcess>

ResizeDialog::ResizeDialog(const QString &videoFile, int videoWidth, int videoHeight, QWidget *parent)
    : QDialog(parent), videoFile(videoFile), originalWidth(videoWidth), originalHeight(videoHeight),
      aspectRatio((double)videoWidth / videoHeight), updatingControls(false)
{
    setWindowTitle("Resize Video");
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Original dimensions display
    QLabel *originalSizeLabel = new QLabel(QString("Original size: %1 x %2")
                                               .arg(originalWidth)
                                               .arg(originalHeight),
                                           this);
    mainLayout->addWidget(originalSizeLabel);

    // Preview label
    previewLabel = new QLabel(this);
    previewLabel->setMinimumSize(320, 240);
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    mainLayout->addWidget(previewLabel);

    // Form for input fields
    QFormLayout *formLayout = new QFormLayout();

    // Width input
    widthInput = new QSpinBox(this);
    widthInput->setRange(10, 7680); // Up to 8K
    widthInput->setValue(originalWidth);
    widthInput->setSuffix(" px");
    formLayout->addRow("Width:", widthInput);

    // Height input
    heightInput = new QSpinBox(this);
    heightInput->setRange(10, 4320); // Up to 8K
    heightInput->setValue(originalHeight);
    heightInput->setSuffix(" px");
    formLayout->addRow("Height:", heightInput);

    // Maintain aspect ratio option
    aspectRatioCheckbox = new QCheckBox("Maintain aspect ratio", this);
    aspectRatioCheckbox->setChecked(true);
    formLayout->addRow("", aspectRatioCheckbox);

    // Preset sizes dropdown
    QHBoxLayout *presetLayout = new QHBoxLayout();
    QLabel *presetLabel = new QLabel("Preset sizes:", this);
    presetCombo = new QComboBox(this);
    presetCombo->addItem("Custom");
    presetCombo->addItem("HD (1280x720)");
    presetCombo->addItem("Full HD (1920x1080)");
    presetCombo->addItem("2K (2560x1440)");
    presetCombo->addItem("4K (3840x2160)");
    presetCombo->addItem("50%");
    presetCombo->addItem("25%");

    presetLayout->addWidget(presetLabel);
    presetLayout->addWidget(presetCombo);
    formLayout->addRow(presetLayout);

    // Scaling algorithm
    algorithmCombo = new QComboBox(this);
    algorithmCombo->addItem("Bilinear", "bilinear");
    algorithmCombo->addItem("Bicubic", "bicubic");
    algorithmCombo->addItem("Lanczos", "lanczos");
    algorithmCombo->addItem("Spline", "spline");
    formLayout->addRow("Scaling Algorithm:", algorithmCombo);

    mainLayout->addLayout(formLayout);

    // Connect signals
    connect(widthInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResizeDialog::updateHeight);
    connect(heightInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResizeDialog::updateWidth);
    connect(aspectRatioCheckbox, &QCheckBox::toggled,
            this, &ResizeDialog::aspectRatioToggled);
    connect(presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ResizeDialog::onPresetChanged);

    // Update preview when values change
    connect(widthInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResizeDialog::updatePreview);
    connect(heightInput, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ResizeDialog::updatePreview);

    // Add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    // Initialize preview
    updatePreview();
}

int ResizeDialog::getWidth() const
{
    return widthInput->value();
}

int ResizeDialog::getHeight() const
{
    return heightInput->value();
}

bool ResizeDialog::getMaintainAspectRatio() const
{
    return aspectRatioCheckbox->isChecked();
}

QString ResizeDialog::getScalingAlgorithm() const
{
    return algorithmCombo->currentData().toString();
}

QStringList ResizeDialog::getFFMPEGArguments(const QString &outputFile) const
{
    QStringList args;
    QString scaleFilter = QString("scale=%1:%2:flags=%3")
                              .arg(getWidth())
                              .arg(getHeight())
                              .arg(getScalingAlgorithm());

    args << "-y"                 // Overwrite output files without asking
         << "-i" << videoFile    // Input file
         << "-vf" << scaleFilter // Scale video filter
         << "-c:a" << "copy"     // Copy audio codec (no re-encode)
         << outputFile;          // Output file

    return args;
}

void ResizeDialog::updateHeight()
{
    if (updatingControls || !getMaintainAspectRatio())
        return;

    updatingControls = true;

    int newHeight = qRound(widthInput->value() / aspectRatio);
    heightInput->setValue(newHeight);

    updatingControls = false;
}

void ResizeDialog::updateWidth()
{
    if (updatingControls || !getMaintainAspectRatio())
        return;

    updatingControls = true;

    int newWidth = qRound(heightInput->value() * aspectRatio);
    widthInput->setValue(newWidth);

    updatingControls = false;
}

void ResizeDialog::aspectRatioToggled(bool checked)
{
    if (checked)
    {
        // Update height based on current width
        updateHeight();
    }

    // Set combo to Custom when toggling aspect ratio
    presetCombo->setCurrentIndex(0);
}

void ResizeDialog::onPresetChanged(int index)
{
    if (index == 0)
    {
        // Custom - do nothing
        return;
    }

    // Block signals to prevent recursive updates
    updatingControls = true;
    widthInput->blockSignals(true);
    heightInput->blockSignals(true);

    switch (index)
    {
    case 1: // HD
        widthInput->setValue(1280);
        heightInput->setValue(720);
        break;
    case 2: // Full HD
        widthInput->setValue(1920);
        heightInput->setValue(1080);
        break;
    case 3: // 2K
        widthInput->setValue(2560);
        heightInput->setValue(1440);
        break;
    case 4: // 4K
        widthInput->setValue(3840);
        heightInput->setValue(2160);
        break;
    case 5: // 50%
        widthInput->setValue(originalWidth / 2);
        heightInput->setValue(originalHeight / 2);
        break;
    case 6: // 25%
        widthInput->setValue(originalWidth / 4);
        heightInput->setValue(originalHeight / 4);
        break;
    }

    widthInput->blockSignals(false);
    heightInput->blockSignals(false);
    updatingControls = false;

    updatePreview();
}

void ResizeDialog::updatePreview()
{
    // Create a simple representation of the resized video
    QPixmap pixmap(320, 240);
    pixmap.fill(Qt::black);

    QPainter painter(&pixmap);
    painter.setPen(Qt::white);

    // Calculate scaling to fit in preview
    double previewWidth = 320;
    double previewHeight = 240;

    double widthRatio = previewWidth / getWidth();
    double heightRatio = previewHeight / getHeight();
    double scale = qMin(widthRatio, heightRatio);

    int scaledWidth = qRound(getWidth() * scale);
    int scaledHeight = qRound(getHeight() * scale);

    // Center the representation
    int xOffset = (320 - scaledWidth) / 2;
    int yOffset = (240 - scaledHeight) / 2;

    // Draw preview representation
    painter.fillRect(xOffset, yOffset, scaledWidth, scaledHeight, Qt::darkGray);
    painter.drawRect(xOffset, yOffset, scaledWidth, scaledHeight);

    // Draw original dimensions text
    QString originalText = QString("%1 x %2").arg(originalWidth).arg(originalHeight);
    painter.drawText(10, 20, originalText);

    // Draw new dimensions text
    QString newText = QString("%1 x %2").arg(getWidth()).arg(getHeight());
    painter.drawText(10, 220, newText);

    // Set the preview label
    previewLabel->setPixmap(pixmap);
}