#ifndef SIMPLEVIDEOEDITOR_H
#define SIMPLEVIDEOEDITOR_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QSlider>
#include <QPushButton>
#include <QString>

class SimpleVideoEditor : public QMainWindow
{
    Q_OBJECT

public:
    SimpleVideoEditor(QWidget *parent = nullptr);

private slots:
    void openFile();
    void saveFile();
    void trimVideo();
    void cropVideo();
    void resizeVideo();
    void convertVideo();
    void executeFFMPEG(const QStringList &arguments);

private:
    void createMenus();

    QMediaPlayer *mediaPlayer;
    QVideoWidget *videoWidget;
    QSlider *timelineSlider;
    QPushButton *playButton;
    QString currentVideoFile;
};

#endif // SIMPLEVIDEOEDITOR_H