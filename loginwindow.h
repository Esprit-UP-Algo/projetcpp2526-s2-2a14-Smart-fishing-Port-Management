#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QProcess>

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLogin();
    void onFaceIDLogin();
    void onForgotPassword();
    void handleFaceIDOutput();
    void startFaceIDPrewarm();

private:
    void setupUi();
    QFrame* createLoginCard();
    QFrame* createInputField(const QString& icon, const QString& placeholder, bool isPassword = false);

    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QFrame* loginCard;

    // Pre-warmed Face ID process
    QProcess* faceIdProcess = nullptr;
    bool faceIdReady = false;
    QPushButton* faceIdBtn = nullptr;
    QString faceIdOutputBuffer;
};

#endif // LOGINWINDOW_H
