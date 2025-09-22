#ifndef CONVERTDIALOG_H
#define CONVERTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QString>
#include <QStringList>

class ConvertDialog : public QDialog
{
    Q_OBJECT

public:
    ConvertDialog(const QString &videoFile, QWidget *parent = nullptr);

    QString getOutputFormat() const;
    int getVideoBitrate() const;
    int getAudioBitrate() const;
    bool getConvertAudio() const;
    QString getAudioCodec() const;
    QStringList getFFMPEGArguments(const QString &outputFile) const;

private slots:
    void updateAudioOptions(bool enabled);
    void updateFormatSettings(const QString &format);

private:
    QString videoFile;

    QComboBox *formatCombo;
    QSpinBox *videoBitrateInput;
    QCheckBox *convertAudioCheckbox;
    QComboBox *audioCodecCombo;
    QSpinBox *audioBitrateInput;
};

#endif // CONVERTDIALOG_H