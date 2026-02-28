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

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void onLogin();
    void onForgotPassword();

private:
    void setupUi();
    QFrame* createLoginCard();
    QFrame* createInputField(const QString& icon, const QString& placeholder, bool isPassword = false);

    QLineEdit* usernameInput;
    QLineEdit* passwordInput;
    QFrame* loginCard;
};

#endif // LOGINWINDOW_H
