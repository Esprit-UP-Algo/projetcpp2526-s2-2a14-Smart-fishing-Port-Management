#ifndef ARDUINO_H
#define ARDUINO_H

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QByteArray>

class Arduino
{
public:
    enum class SoundEvent {
        Arrival,
        Departure,
        Success,
        Error
    };

    Arduino();
    ~Arduino();
    int connect_arduino();
    int close_arduino();
    void write_to_arduino(QByteArray d);
    QByteArray read_from_arduino();
    QSerialPort* getserial();
    QString getarduino_port_name();
    static bool sendCommand(const QByteArray& command);
    static bool triggerSingleBeep();
    static bool triggerSoundEvent(SoundEvent event);

private:
    static bool writeSoundCommand(char command, const char* debugLabel);
    QSerialPort *serial;
    static const quint16 arduino_uno_vendor_id = 9025;
    static const quint16 arduino_uno_product_id = 67;
    static const quint16 arduino_ch340_vendor_id = 6790;
    static const quint16 arduino_ch340_product_id = 29987;
    static QSerialPort* s_active_serial;
    QString arduino_port_name;
    bool arduino_is_available;
    QByteArray data;
};

#endif // ARDUINO_H
