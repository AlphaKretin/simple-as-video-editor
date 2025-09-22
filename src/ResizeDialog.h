#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QString>
#include <QStringList>
#include <QLabel>

class ResizeDialog : public QDialog
{
    Q_OBJECT

public:
    ResizeDialog(const QString &videoFile, int videoWidth, int videoHeight, QWidget *parent = nullptr);

    int getWidth() const;
    int getHeight() const;
    bool getMaintainAspectRatio() const;
    QString getScalingAlgorithm() const;
    QStringList getFFMPEGArguments(const QString &outputFile) const;

private slots:
    void updateHeight();
    void updateWidth();
    void aspectRatioToggled(bool checked);
    void onPresetChanged(int index);
    void updatePreview();

private:
    QString videoFile;
    int originalWidth;
    int originalHeight;
    double aspectRatio;

    QSpinBox *widthInput;
    QSpinBox *heightInput;
    QCheckBox *aspectRatioCheckbox;
    QComboBox *presetCombo;
    QComboBox *algorithmCombo;
    QLabel *previewLabel;

    bool updatingControls; // Flag to prevent recursive updates
};

#endif // RESIZEDIALOG_H