#include "TrimDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>

TrimDialog::TrimDialog(const QString &videoFile, qint64 duration, QWidget *parent)
    : QDialog(parent), videoFile(videoFile), videoDuration(duration)
{

    setWindowTitle("Trim Video");

    QFormLayout *layout = new QFormLayout(this);

    // Convert duration from milliseconds to seconds
    double durationInSeconds = duration / 1000.0;

    // Start time input
    startTimeInput = new QDoubleSpinBox(this);
    startTimeInput->setDecimals(2);
    startTimeInput->setRange(0.0, durationInSeconds);
    startTimeInput->setSuffix(" sec");
    layout->addRow("Start Time:", startTimeInput);

    // End time input
    endTimeInput = new QDoubleSpinBox(this);
    endTimeInput->setDecimals(2);
    endTimeInput->setRange(0.0, durationInSeconds);
    endTimeInput->setValue(durationInSeconds);
    endTimeInput->setSuffix(" sec");
    layout->addRow("End Time:", endTimeInput);

    // Connect signals to validate that start < end
    connect(startTimeInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TrimDialog::validateTimes);
    connect(endTimeInput, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &TrimDialog::validateTimes);

    // Add buttons
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addRow(buttonBox);
}

double TrimDialog::getStartTime() const
{
    return startTimeInput->value();
}

double TrimDialog::getEndTime() const
{
    return endTimeInput->value();
}

QStringList TrimDialog::getFFMPEGArguments(const QString &outputFile) const
{
    QStringList args;
    args << "-y"                                     // Overwrite output files without asking
         << "-i" << videoFile                        // Input file
         << "-ss" << QString::number(getStartTime()) // Start time
         << "-to" << QString::number(getEndTime())   // End time
         << "-c" << "copy"                           // Copy codec (fast, no re-encode)
         << outputFile;                              // Output file

    return args;
}

void TrimDialog::validateTimes()
{
    // Ensure start time is less than end time
    if (startTimeInput->value() >= endTimeInput->value())
    {
        endTimeInput->setValue(startTimeInput->value() + 0.1);
    }
}