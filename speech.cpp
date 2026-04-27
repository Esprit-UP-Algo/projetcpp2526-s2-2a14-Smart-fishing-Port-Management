#include "speech.h"

#include "arduino.h"

#include <QProcess>
#include <QStringList>

void speakBoat(const QString& boatName)
{
    Arduino::triggerSingleBeep();

    QString safeBoatName = boatName;
    safeBoatName.replace("'", "''");

    const QString script =
        "Add-Type -AssemblyName System.Speech; "
        "$speaker = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$speaker.Rate = 0; "
        "$speaker.Volume = 100; "
        "$speaker.Speak('Announcement. Boat " + safeBoatName + " is leaving the port and is now at sea.');";

    QProcess::startDetached("powershell.exe",
                            QStringList() << "-NoProfile"
                                          << "-ExecutionPolicy" << "Bypass"
                                          << "-Command" << script);
}
