#include "arduino.h"

QSerialPort* Arduino::s_active_serial = nullptr;

Arduino::Arduino()
{
    data = "";
    arduino_port_name = "";
    arduino_is_available = false;
    serial = new QSerialPort();
}

bool Arduino::writeSoundCommand(char command, const char* debugLabel)
{
    if (!s_active_serial || !s_active_serial->isOpen() || !s_active_serial->isWritable()) {
        qDebug() << "Arduino sound skipped for" << debugLabel << ": serial port unavailable.";
        return false;
    }

    s_active_serial->write(QByteArray(1, command));
    s_active_serial->flush();
    return true;
}

bool Arduino::sendCommand(const QByteArray& command)
{
    if (!s_active_serial || !s_active_serial->isOpen() || !s_active_serial->isWritable()) {
        qDebug() << "Arduino command skipped: serial port unavailable.";
        return false;
    }

    s_active_serial->write(command);
    s_active_serial->flush();
    return true;
}

bool Arduino::triggerSingleBeep()
{
    return writeSoundCommand('B', "single beep");
}

bool Arduino::triggerSoundEvent(SoundEvent event)
{
    switch (event) {
    case SoundEvent::Arrival:
        return writeSoundCommand('A', "boat arrival");
    case SoundEvent::Departure:
        return writeSoundCommand('L', "boat departure");
    case SoundEvent::Success:
        return writeSoundCommand('S', "docking success");
    case SoundEvent::Error:
        return writeSoundCommand('E', "error alert");
    }

    return false;
}

Arduino::~Arduino()
{
    close_arduino();
    delete serial;
}

QString Arduino::getarduino_port_name()
{
    return arduino_port_name;
}

QSerialPort *Arduino::getserial()
{
    return serial;
}

int Arduino::connect_arduino()
{
    // Search for the port where Arduino is connected
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &serial_port_info : serialPortInfos) {
        if (serial_port_info.hasVendorIdentifier() && serial_port_info.hasProductIdentifier()) {
            if ((serial_port_info.vendorIdentifier() == arduino_uno_vendor_id && 
                 serial_port_info.productIdentifier() == arduino_uno_product_id) ||
                (serial_port_info.vendorIdentifier() == arduino_ch340_vendor_id && 
                 serial_port_info.productIdentifier() == arduino_ch340_product_id)) {
                arduino_is_available = true;
                arduino_port_name = serial_port_info.portName();
                break;
            }
        }
    }

    qDebug() << "arduino_port_name is :" << arduino_port_name;

    if (arduino_is_available) {
        serial->setPortName(arduino_port_name);
        if (serial->open(QSerialPort::ReadWrite)) {
            serial->setBaudRate(QSerialPort::Baud9600);
            serial->setDataBits(QSerialPort::Data8);
            serial->setParity(QSerialPort::NoParity);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);
            s_active_serial = serial;
            return 0; // Success
        }
        return 1; // Could not open port
    }
    return -1; // Arduino not found
}

int Arduino::close_arduino()
{
    if (serial->isOpen()) {
        serial->close();
        if (s_active_serial == serial)
            s_active_serial = nullptr;
        return 0;
    }
    return 1;
}

QByteArray Arduino::read_from_arduino()
{
    if (serial->isOpen() && serial->isReadable()) {
        if (serial->bytesAvailable() > 0) {
            data = serial->readAll();
            return data;
        }
    }
    return QByteArray();
}

void Arduino::write_to_arduino(QByteArray d)
{
    if (serial->isWritable()) {
        serial->write(d);
        serial->flush(); // Force l'envoi immédiat
    } else {
        qDebug() << "Couldn't write to serial!";
    }
}
