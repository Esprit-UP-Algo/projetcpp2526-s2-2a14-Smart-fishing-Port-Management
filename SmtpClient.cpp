#include "SmtpClient.h"
#include <QEventLoop>
#include <QTimer>

SmtpClient::SmtpClient(const QString &user, const QString &pass, const QString &host, int port)
    : user(user), pass(pass), host(host), port(port)
{
    socket = new QSslSocket(this);
    socket->setProtocol(QSsl::TlsV1_2OrLater);
    socket->setPeerVerifyMode(QSslSocket::VerifyNone);
}

SmtpClient::~SmtpClient()
{
    if (socket->isOpen()) socket->close();
}

bool SmtpClient::sendMail(const QString &from, const QString &to,
                          const QString &subject, const QString &body)
{
    // ── 1. Connect with SSL (port 465 = implicit SSL) ──────────────
    socket->connectToHostEncrypted(host, port);
    if (!socket->waitForEncrypted(10000)) {
        qDebug() << "SMTP: SSL handshake failed:" << socket->errorString();
        return false;
    }
    qDebug() << "SMTP: Connected and encrypted.";

    // ── 2. Read server greeting (220) ──────────────────────────────
    if (!readResponse(220)) { qDebug() << "SMTP: No greeting"; return false; }

    // ── 3. EHLO ────────────────────────────────────────────────────
    sendCommand("EHLO localhost");
    if (!readResponse(250)) { qDebug() << "SMTP: EHLO failed"; return false; }

    // ── 4. AUTH LOGIN ──────────────────────────────────────────────
    sendCommand("AUTH LOGIN");
    if (!readResponse(334)) { qDebug() << "SMTP: AUTH LOGIN failed"; return false; }

    sendCommand(user.toUtf8().toBase64());
    if (!readResponse(334)) { qDebug() << "SMTP: Username rejected"; return false; }

    sendCommand(pass.remove(' ').toUtf8().toBase64());
    if (!readResponse(235)) { qDebug() << "SMTP: Password rejected"; return false; }

    // ── 5. Envelope ────────────────────────────────────────────────
    sendCommand("MAIL FROM:<" + from + ">");
    if (!readResponse(250)) { qDebug() << "SMTP: MAIL FROM failed"; return false; }

    sendCommand("RCPT TO:<" + to + ">");
    if (!readResponse(250)) { qDebug() << "SMTP: RCPT TO failed"; return false; }

    // ── 6. Body ────────────────────────────────────────────────────
    sendCommand("DATA");
    if (!readResponse(354)) { qDebug() << "SMTP: DATA failed"; return false; }

    // Headers + body (encode subject for UTF-8)
    QString message;
    message += "From: PortFlow Alert <" + from + ">\r\n";
    message += "To: " + to + "\r\n";
    message += "Subject: =?UTF-8?B?" +
               QString::fromLatin1(subject.toUtf8().toBase64()) + "?=\r\n";
    message += "MIME-Version: 1.0\r\n";
    message += "Content-Type: text/html; charset=UTF-8\r\n";
    message += "Content-Transfer-Encoding: 8bit\r\n";
    message += "\r\n";
    message += body + "\r\n";
    message += ".";
    sendCommand(message);

    if (!readResponse(250)) { qDebug() << "SMTP: Message rejected"; return false; }

    sendCommand("QUIT");
    socket->waitForDisconnected(3000);
    qDebug() << "SMTP: Email sent successfully!";
    return true;
}

void SmtpClient::sendCommand(const QString &cmd)
{
    qDebug() << "SMTP >>:" << cmd.left(40); // truncate long base64
    socket->write((cmd + "\r\n").toUtf8());
    socket->waitForBytesWritten(3000);
}

bool SmtpClient::readResponse(int expectedCode)
{
    QString response;
    // Gmail may send multi-line responses; keep reading until last line
    while (true) {
        if (!socket->waitForReadyRead(8000)) {
            qDebug() << "SMTP: Timeout waiting for response (expected" << expectedCode << ")";
            return false;
        }
        response += QString::fromUtf8(socket->readAll());
        qDebug() << "SMTP <<:" << response.trimmed();

        // Multi-line SMTP response: lines end with "XXX-", last ends with "XXX "
        // Check if we have a complete final line
        QStringList lines = response.split("\r\n", Qt::SkipEmptyParts);
        if (lines.isEmpty()) continue;
        QString lastLine = lines.last();
        // Final response line: starts with 3-digit code followed by space (not dash)
        if (lastLine.length() >= 4 && lastLine[3] == ' ') {
            int code = lastLine.left(3).toInt();
            return code == expectedCode;
        }
        // If the last line ends with a dash (e.g. "250-..."), keep reading
    }
}
