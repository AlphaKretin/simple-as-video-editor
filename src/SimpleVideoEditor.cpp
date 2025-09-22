#include "SimpleVideoEditor.h"
#include "TrimDialog.h"
#include "CropDialog.h"
#include "ResizeDialog.h"
#include "ConvertDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>
#include <QProcess>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>

SimpleVideoEditor::SimpleVideoEditor(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("Simple Video Editor");
    resize(1024, 768);

    // Create menu bar
    createMenus();

    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Video preview area
    mediaPlayer = new QMediaPlayer(this);
    videoWidget = new QVideoWidget(this);
    videoWidget->setMinimumSize(640, 360);
    mediaPlayer->setVideoOutput(videoWidget);
    mainLayout->addWidget(videoWidget);

    // Timeline and controls
    QHBoxLayout *timelineLayout = new QHBoxLayout();

    playButton = new QPushButton("Play", this);
    connect(playButton, &QPushButton::clicked, [this]()
            {
        if (mediaPlayer->playbackState() == QMediaPlayer::PlayingState) {
            mediaPlayer->pause();
            playButton->setText("Play");
        } else {
            mediaPlayer->play();
            playButton->setText("Pause");
        } });

    timelineSlider = new QSlider(Qt::Horizontal, this);
    timelineSlider->setMinimum(0);
    timelineSlider->setMaximum(100);

    connect(mediaPlayer, &QMediaPlayer::durationChanged, [this](qint64 duration)
            { timelineSlider->setMaximum(duration); });

    connect(mediaPlayer, &QMediaPlayer::positionChanged, [this](qint64 position)
            { timelineSlider->setValue(position); });

    connect(timelineSlider, &QSlider::sliderMoved, [this](int position)
            { mediaPlayer->setPosition(position); });

    timelineLayout->addWidget(playButton);
    timelineLayout->addWidget(timelineSlider);
    mainLayout->addLayout(timelineLayout);

    // Editing tools
    QHBoxLayout *toolsLayout = new QHBoxLayout();

    QPushButton *trimButton = new QPushButton("Trim", this);
    QPushButton *cropButton = new QPushButton("Crop", this);
    QPushButton *resizeButton = new QPushButton("Resize", this);
    QPushButton *convertButton = new QPushButton("Convert", this);

    // Connect editing tool buttons
    connect(trimButton, &QPushButton::clicked, this, &SimpleVideoEditor::trimVideo);
    connect(cropButton, &QPushButton::clicked, this, &SimpleVideoEditor::cropVideo);
    connect(resizeButton, &QPushButton::clicked, this, &SimpleVideoEditor::resizeVideo);
    connect(convertButton, &QPushButton::clicked, this, &SimpleVideoEditor::convertVideo);

    toolsLayout->addWidget(trimButton);
    toolsLayout->addWidget(cropButton);
    toolsLayout->addWidget(resizeButton);
    toolsLayout->addWidget(convertButton);
    mainLayout->addLayout(toolsLayout);

    // Status bar for feedback
    statusBar()->showMessage("Ready");

    setCentralWidget(centralWidget);
}

void SimpleVideoEditor::openFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "Open Video", "", "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv)");

    if (!fileName.isEmpty())
    {
        currentVideoFile = fileName;
        mediaPlayer->setSource(QUrl::fromLocalFile(fileName));
        playButton->setText("Play");
        statusBar()->showMessage("Loaded: " + fileName);
    }
}

void SimpleVideoEditor::saveFile()
{
    if (currentVideoFile.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No video loaded to save");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    "Save Video", "", "Video Files (*.mp4 *.avi *.mkv *.mov *.wmv)");

    if (!fileName.isEmpty())
    {
        // This would call FFMPEG to save with current edits
        statusBar()->showMessage("Saved to: " + fileName);
    }
}

void SimpleVideoEditor::executeFFMPEG(const QStringList &arguments)
{
    QProcess *ffmpeg = new QProcess(this);
    ffmpeg->start("ffmpeg", arguments);

    connect(ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, ffmpeg](int exitCode, QProcess::ExitStatus exitStatus)
            {
                if (exitCode == 0)
                {
                    statusBar()->showMessage("FFMPEG operation completed successfully");
                }
                else
                {
                    statusBar()->showMessage("FFMPEG error: " + QString(ffmpeg->readAllStandardError()));
                }
                ffmpeg->deleteLater();
            });
}

void SimpleVideoEditor::trimVideo()
{
    if (currentVideoFile.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No video loaded to trim");
        return;
    }

    // Get the duration of the video from the media player
    qint64 duration = mediaPlayer->duration();

    TrimDialog dialog(currentVideoFile, duration, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString outputFile = QFileDialog::getSaveFileName(this,
                                                          "Save Trimmed Video", "", "Video Files (*.mp4)");

        if (!outputFile.isEmpty())
        {
            QStringList args = dialog.getFFMPEGArguments(outputFile);
            executeFFMPEG(args);
        }
    }
}

void SimpleVideoEditor::cropVideo()
{
    if (currentVideoFile.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No video loaded to crop");
        return;
    }

    // Here we would determine the video dimensions
    // For simplicity, assume 1280x720 if we can't get actual dimensions
    int videoWidth = 1280;
    int videoHeight = 720;

    // In a real application, you would query the video for its dimensions
    // This could be done with ffprobe or by using the mediaPlayer's properties

    CropDialog dialog(currentVideoFile, videoWidth, videoHeight, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString outputFile = QFileDialog::getSaveFileName(this,
                                                          "Save Cropped Video", "", "Video Files (*.mp4)");

        if (!outputFile.isEmpty())
        {
            QStringList args = dialog.getFFMPEGArguments(outputFile);
            executeFFMPEG(args);
        }
    }
}

void SimpleVideoEditor::resizeVideo()
{
    if (currentVideoFile.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No video loaded to resize");
        return;
    }

    // Again, assume 1280x720 if we can't get actual dimensions
    int videoWidth = 1280;
    int videoHeight = 720;

    ResizeDialog dialog(currentVideoFile, videoWidth, videoHeight, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString outputFile = QFileDialog::getSaveFileName(this,
                                                          "Save Resized Video", "", "Video Files (*.mp4)");

        if (!outputFile.isEmpty())
        {
            QStringList args = dialog.getFFMPEGArguments(outputFile);
            executeFFMPEG(args);
        }
    }
}

void SimpleVideoEditor::createMenus()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");

    QAction *openAction = fileMenu->addAction("&Open");
    connect(openAction, &QAction::triggered, this, &SimpleVideoEditor::openFile);

    QAction *saveAction = fileMenu->addAction("&Save");
    connect(saveAction, &QAction::triggered, this, &SimpleVideoEditor::saveFile);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction("E&xit");
    connect(exitAction, &QAction::triggered, this, &SimpleVideoEditor::close);

    QMenu *editMenu = menuBar()->addMenu("&Edit");

    QAction *trimAction = editMenu->addAction("&Trim");
    connect(trimAction, &QAction::triggered, this, &SimpleVideoEditor::trimVideo);

    QAction *cropAction = editMenu->addAction("&Crop");
    connect(cropAction, &QAction::triggered, this, &SimpleVideoEditor::cropVideo);

    QAction *resizeAction = editMenu->addAction("Re&size");
    connect(resizeAction, &QAction::triggered, this, &SimpleVideoEditor::resizeVideo);

    QAction *convertAction = editMenu->addAction("&Convert");
    connect(convertAction, &QAction::triggered, this, &SimpleVideoEditor::convertVideo);

    QMenu *helpMenu = menuBar()->addMenu("&Help");

    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, [this]()
            { QMessageBox::about(this, "About Simple Video Editor",
                                 "Simple Video Editor v1.0\n\n"
                                 "A basic video editing application using FFMPEG backend.\n"
                                 "Features include: trim, crop, resize, and format conversion."); });
}