#ifndef SMTPCLIENT_H
#define SMTPCLIENT_H

#include <QObject>
#include <QString>
#include <QSslSocket>
#include <QDebug>

class SmtpClient : public QObject
{
    Q_OBJECT

public:
    SmtpClient(const QString &user, const QString &pass,
               const QString &host = "smtp.gmail.com", int port = 465);
    ~SmtpClient();

    bool sendMail(const QString &from, const QString &to,
                  const QString &subject, const QString &body);

private:
    QString user;
    QString pass;
    QString host;
    int port;
    QSslSocket *socket;

    void sendCommand(const QString &cmd);
    bool readResponse(int expectedCode);
};

#endif // SMTPCLIENT_H
