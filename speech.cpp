#include "speech.h"
#include <cstdlib>

void speakBoat(const std::string& boatName) {
    std::string command =
        "PowerShell -Command \"Add-Type -AssemblyName System.Speech; "
        "$speak = New-Object System.Speech.Synthesis.SpeechSynthesizer; "
        "$speak.Speak('Boat ";

    command += boatName;
    command += " is leaving');\"";

    system(command.c_str());
}
