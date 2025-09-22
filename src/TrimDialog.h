#ifndef TRIMDIALOG_H
#define TRIMDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QString>
#include <QStringList>

class TrimDialog : public QDialog
{
    Q_OBJECT

public:
    TrimDialog(const QString &videoFile, qint64 duration, QWidget *parent = nullptr);

    double getStartTime() const;
    double getEndTime() const;
    QStringList getFFMPEGArguments(const QString &outputFile) const;

private slots:
    void validateTimes();

private:
    QString videoFile;
    qint64 videoDuration;
    QDoubleSpinBox *startTimeInput;
    QDoubleSpinBox *endTimeInput;
};

#endif // TRIMDIALOG_H