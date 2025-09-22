#include "SimpleVideoEditor.h"
#include "ConvertDialog.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QStatusBar>

void SimpleVideoEditor::convertVideo()
{
    if (currentVideoFile.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "No video loaded to convert");
        return;
    }

    ConvertDialog dialog(currentVideoFile, this);
    if (dialog.exec() == QDialog::Accepted)
    {
        QString format = dialog.getOutputFormat();
        QString extension = "." + format;

        QString outputFile = QFileDialog::getSaveFileName(this,
                                                          "Save Converted Video", "",
                                                          "Video Files (*" + extension + ")");

        if (!outputFile.isEmpty())
        {
            QStringList arguments = dialog.getFFMPEGArguments(outputFile);

            // Execute FFMPEG with progress feedback
            QProcess *ffmpeg = new QProcess(this);
            statusBar()->showMessage("Converting video...");

            connect(ffmpeg, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
                    [this, ffmpeg, outputFile](int exitCode, QProcess::ExitStatus exitStatus)
                    {
                        if (exitCode == 0)
                        {
                            statusBar()->showMessage("Video converted successfully");

                            // Ask if user wants to load the new video
                            QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                                                      "Conversion Complete",
                                                                                      "Would you like to load the converted video?",
                                                                                      QMessageBox::Yes | QMessageBox::No);

                            if (reply == QMessageBox::Yes)
                            {
                                currentVideoFile = outputFile;
                                mediaPlayer->setSource(QUrl::fromLocalFile(outputFile));
                                playButton->setText("Play");
                            }
                        }
                        else
                        {
                            QMessageBox::critical(this, "Error",
                                                  "FFMPEG error: " + QString(ffmpeg->readAllStandardError()));
                        }
                        ffmpeg->deleteLater();
                    });

            ffmpeg->start("ffmpeg", arguments);
        }
    }
}