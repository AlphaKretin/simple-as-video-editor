#ifndef CROPDIALOG_H
#define CROPDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QLabel>
#include <QRubberBand>
#include <QRect>
#include <QPoint>
#include <QPixmap>
#include <QWidget>

// Custom widget for visual crop selection
class CropSelectionWidget : public QWidget
{
    Q_OBJECT

public:
    CropSelectionWidget(QWidget *parent = nullptr);

    void setVideoFrame(const QPixmap &frame);
    QRect getSelectedRect() const;

signals:
    void selectionChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QPixmap videoFrame;
    QRect frameRect;    // Rect where the frame is drawn
    QRect selectedRect; // Selected area in widget coordinates
    QRubberBand *rubberBand;
    QPoint origin;
    bool isSelecting;
};

class CropDialog : public QDialog
{
    Q_OBJECT

public:
    CropDialog(const QString &videoFile, int videoWidth, int videoHeight, QWidget *parent = nullptr);
    ~CropDialog();

    int getX() const;
    int getY() const;
    int getWidth() const;
    int getHeight() const;
    QStringList getFFMPEGArguments(const QString &outputFile) const;

private slots:
    void validateDimensions();
    void updatePreview();
    void updateInputsFromSelection();

private:
    QString videoFile;
    int originalWidth;
    int originalHeight;

    CropSelectionWidget *selectionWidget;
    QSpinBox *xInput;
    QSpinBox *yInput;
    QSpinBox *widthInput;
    QSpinBox *heightInput;
    QLabel *previewLabel;
};

#endif // CROPDIALOG_H