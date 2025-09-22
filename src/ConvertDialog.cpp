#include "ConvertDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QFileInfo>

ConvertDialog::ConvertDialog(const QString &videoFile, QWidget *parent)
    : QDialog(parent), videoFile(videoFile)
{
    setWindowTitle("Convert Video Format");
    setMinimumWidth(400);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Display current format
    QFileInfo fileInfo(videoFile);
    QString currentExt = fileInfo.suffix().toLower();
    QLabel *currentFormatLabel = new QLabel(QString("Current format: %1").arg(currentExt), this);
    mainLayout->addWidget(currentFormatLabel);
    
    // Video format settings
    QGroupBox *formatGroupBox = new QGroupBox("Format Settings", this);
    QFormLayout *formatLayout = new QFormLayout(formatGroupBox);
    
    // Format selection
    formatCombo = new QComboBox(this);
    formatCombo->addItem("MP4 (H.264)", "mp4");
    formatCombo->addItem("WebM (VP9)", "webm");
    formatCombo->addItem("MKV (H.264)", "mkv");
    formatCombo->addItem("AVI (MJPEG)", "avi");
    formatCombo->addItem("MOV (H.264)", "mov");
    formatCombo->addItem("GIF (animated)", "gif");
    formatLayout->addRow("Target Format:", formatCombo);
    
    // Video bitrate
    videoBitrateInput = new QSpinBox(this);
    videoBitrateInput->setRange(500, 20000);
    videoBitrateInput->setValue(2000);
    videoBitrateInput->setSuffix(" kbps");
    formatLayout->addRow("Video Bitrate:", videoBitrateInput);
    
    mainLayout->addWidget(formatGroupBox);
    
    // Audio settings
    QGroupBox *audioGroupBox = new QGroupBox("Audio Settings", this);
    QFormLayout *audioLayout = new QFormLayout(audioGroupBox);
    
    // Convert audio checkbox
    convertAudioCheckbox = new QCheckBox("Include audio", this);
    convertAudioCheckbox->setChecked(true);
    audioLayout->addRow("", convertAudioCheckbox);
    
    // Audio codec
    audioCodecCombo = new QComboBox(this);
    audioCodecCombo->addItem("AAC", "aac");
    audioCodecCombo->addItem("MP3", "libmp3lame");
    audioCodecCombo->addItem("Opus", "libopus");
    audioCodecCombo->addItem("FLAC", "flac");
    audioLayout->addRow("Audio Codec:", audioCodecCombo);
    
    // Audio bitrate
    audioBitrateInput = new QSpinBox(this);
    audioBitrateInput->setRange(32, 320);
    audioBitrateInput->setValue(128);
    audioBitrateInput->setSuffix(" kbps");
    audioLayout->addRow("Audio Bitrate:", audioBitrateInput);
    
    mainLayout->addWidget(audioGroupBox);
    
    // Connect signals
    connect(convertAudioCheckbox, &QCheckBox::toggled,
            this, &ConvertDialog::updateAudioOptions);
    connect(formatCombo, &QComboBox::currentTextChanged,
            this, &ConvertDialog::updateFormatSettings);
    
    // Add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    // Initialize UI
    updateAudioOptions(convertAudioCheckbox->isChecked());
    updateFormatSettings(formatCombo->currentText());
}

QString ConvertDialog::getOutputFormat() const
{
    return formatCombo->currentData().toString();
}

int ConvertDialog::getVideoBitrate() const
{
    return videoBitrateInput->value();
}

int ConvertDialog::getAudioBitrate() const
{
    return audioBitrateInput->value();
}

bool ConvertDialog::getConvertAudio() const
{
    return convertAudioCheckbox->isChecked();
}

QString ConvertDialog::getAudioCodec() const
{
    return audioCodecCombo->currentData().toString();
}

QStringList ConvertDialog::getFFMPEGArguments(const QString &outputFile) const
{
    QStringList args;
    
    args << "-y"                      // Overwrite output files without asking
         << "-i" << videoFile;        // Input file
    
    // Handle audio settings
    if (!getConvertAudio())
    {
        args << "-an";                // No audio
    }
    else
    {
        args << "-c:a" << getAudioCodec()  // Audio codec
             << "-b:a" << QString::number(getAudioBitrate()) + "k";  // Audio bitrate
    }
    
    // Format-specific settings
    QString format = getOutputFormat();
    if (format == "webm")
    {
        args << "-c:v" << "libvpx-vp9"
             << "-b:v" << QString::number(getVideoBitrate()) + "k";
    }
    else if (format == "gif")
    {
        // For GIF, we need a specific filter chain
        args << "-vf" << "fps=10,scale=320:-1:flags=lanczos,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse"
             << "-loop" << "0"; // Loop forever
    }
    else if (format == "mp4" || format == "mov")
    {
        args << "-c:v" << "libx264"
             << "-b:v" << QString::number(getVideoBitrate()) + "k";
    }
    else if (format == "mkv")
    {
        args << "-c:v" << "libx264"
             << "-b:v" << QString::number(getVideoBitrate()) + "k";
    }
    else if (format == "avi")
    {
        args << "-c:v" << "mjpeg"
             << "-b:v" << QString::number(getVideoBitrate()) + "k";
    }
    
    // Add output file
    args << outputFile;
    
    return args;
}

void ConvertDialog::updateAudioOptions(bool enabled)
{
    audioCodecCombo->setEnabled(enabled);
    audioBitrateInput->setEnabled(enabled);
}

void ConvertDialog::updateFormatSettings(const QString &format)
{
    // Adjust recommended settings based on selected format
    if (format.contains("WebM"))
    {
        audioCodecCombo->setCurrentText("Opus");
        
        // GIFs don't support audio
        if (format.contains("GIF"))
        {
            convertAudioCheckbox->setChecked(false);
            convertAudioCheckbox->setEnabled(false);
        }
        else
        {
            convertAudioCheckbox->setEnabled(true);
        }
    }
    else if (format.contains("MP4") || format.contains("MOV"))
    {
        audioCodecCombo->setCurrentText("AAC");
        convertAudioCheckbox->setEnabled(true);
    }
    else if (format.contains("AVI"))
    {
        // AVI works better with PCM audio
        audioCodecCombo->setCurrentText("FLAC");
        convertAudioCheckbox->setEnabled(true);
    }
}