#include "loginwindow.h"
#include "employeewindow.h"
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QBrush>
#include <QPixmap>
#include <QDebug>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUi()
{
    setWindowTitle("PortFlow - Connexion");
    setFixedSize(1200, 800);

    // ============================================
    // CHANGE THIS PATH TO YOUR BACKGROUND IMAGE
    // ============================================
    // Examples:
    // Windows: "C:/Users/YourName/Pictures/background.jpg"
    // Linux: "/home/username/Pictures/background.jpg"
    // Mac: "/Users/username/Pictures/background.jpg"
    // Relative: "./images/background.jpg"

     QString backgroundImagePath = "C:/Users/manne/Downloads/login.png"; // ← CHANGE THIS

    // Try to load the background image
    QPixmap backgroundPixmap(backgroundImagePath);

    if (!backgroundPixmap.isNull()) {
        // Image loaded successfully - use it as background
        QPalette palette;
        backgroundPixmap = backgroundPixmap.scaled(this->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        palette.setBrush(QPalette::Window, QBrush(backgroundPixmap));
        this->setPalette(palette);

        qDebug() << "Background image loaded successfully from:" << backgroundImagePath;
    } else {
        // Image failed to load - use gradient fallback
        setStyleSheet(R"(
            QMainWindow {
                background: qlineargradient(
                    x1:0, y1:0, x2:1, y2:1,
                    stop:0 #5D9CEC,
                    stop:1 #7DB3F5
                );
            }
        )");

        qDebug() << "Warning: Could not load background image from:" << backgroundImagePath;
        qDebug() << "Using gradient fallback instead.";
    }

    // Create central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Main layout
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setAlignment(Qt::AlignCenter);

    // Login card
    loginCard = createLoginCard();
    mainLayout->addWidget(loginCard);
}

QFrame* LoginWindow::createLoginCard()
{
    QFrame* card = new QFrame();
    card->setFixedSize(520, 650);  // Increased from 480x580 to accommodate logo with text
    card->setStyleSheet(R"(
        QFrame {
            background-color: white;
            border-radius: 20px;
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setSpacing(25);  // Adjusted spacing
    layout->setContentsMargins(50, 40, 50, 40);  // Adjusted margins

    // Logo and title section
    QVBoxLayout* logoLayout = new QVBoxLayout();
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setSpacing(10);  // Reduced spacing

    // Logo
    QLabel* logoLabel = new QLabel();

    // ============================================
    // CHANGE THIS PATH TO YOUR LOGO IMAGE
    // ============================================
    // Examples:
    // Windows: "C:/Users/YourName/Pictures/logo.png"
    // Linux: "/home/username/Pictures/logo.png"
    // Mac: "/Users/username/Pictures/logo.png"
    // Relative: "./images/logo.png"

   QString logoPath = "C:/Users/manne/OneDrive/Documents/logo3.png";  // ← CHANGE THIS PATH

    // ============================================
    // LOGO DISPLAY OPTIONS
    // ============================================
    // Set this to true if your logo already includes the "PortFlow" text
    // Set this to false if your logo is just an icon and needs text below
    bool logoHasText = false;  // ← CHANGED TO FALSE: logo is icon only, add text below

    QPixmap logoPix(logoPath);
    if (!logoPix.isNull()) {
        // Logo loaded successfully
        if (logoHasText) {
            // Logo includes text - make it larger and don't add text below
            logoLabel->setPixmap(logoPix.scaled(400, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        } else {
            // Logo is icon only - make it smaller, will add text below
            logoLabel->setPixmap(logoPix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Logo loaded successfully from:" << logoPath;
    } else {
        // Logo failed to load - use emoji fallback
        logoLabel->setText("");
        logoLabel->setStyleSheet(R"(
            QLabel {
                font-size: 80px;
                color: #2B5EA6;
            }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
        logoHasText = false;  // Show text with emoji
        qDebug() << "Warning: Could not load logo from:" << logoPath;
        qDebug() << "Using emoji fallback instead.";
    }
    logoLayout->addWidget(logoLabel);

    // Only add app title and subtitle if logo doesn't already have them
    if (!logoHasText) {
        // App title
        QLabel* titleLabel = new QLabel("PortFlow");
        QFont titleFont("Segoe UI", 36, QFont::Bold);
        titleLabel->setFont(titleFont);
        titleLabel->setStyleSheet(R"(
            QLabel {
                color: #2B5EA6;
            }
        )");
        titleLabel->setAlignment(Qt::AlignCenter);
        logoLayout->addWidget(titleLabel);

        // Subtitle
        QLabel* subtitleLabel = new QLabel("Smart Fishing Port Management");
        QFont subtitleFont("Segoe UI", 13);
        subtitleLabel->setFont(subtitleFont);
        subtitleLabel->setStyleSheet(R"(
            QLabel {
                color: #7F8C8D;
            }
        )");
        subtitleLabel->setAlignment(Qt::AlignCenter);
        logoLayout->addWidget(subtitleLabel);
    }

    layout->addLayout(logoLayout);
    layout->addSpacing(20);

    // Username input
    QFrame* usernameContainer = createInputField(" ", "Identifiant");
    layout->addWidget(usernameContainer);
    usernameInput = usernameContainer->findChild<QLineEdit*>();

    // Password input
    QFrame* passwordContainer = createInputField(" ", "Mot de passe", true);
    layout->addWidget(passwordContainer);
    passwordInput = passwordContainer->findChild<QLineEdit*>();

    // Login button
    QPushButton* loginBtn = new QPushButton("Connexion");
    QFont btnFont("Segoe UI", 12, QFont::Medium);
    loginBtn->setFont(btnFont);
    loginBtn->setCursor(Qt::PointingHandCursor);
    loginBtn->setFixedHeight(50);
    loginBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #5D9CEC;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px;
        }
        QPushButton:hover {
            background-color: #4A89DC;
        }
        QPushButton:pressed {
            background-color: #3B77C4;
        }
    )");
    connect(loginBtn, &QPushButton::clicked, this, &LoginWindow::onLogin);
    layout->addWidget(loginBtn);

    // Forgot password link
    QPushButton* forgotPassword = new QPushButton("Mot de passe oublié?");
    QFont forgotFont("Segoe UI", 10);
    forgotPassword->setFont(forgotFont);
    forgotPassword->setCursor(Qt::PointingHandCursor);
    forgotPassword->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #5D9CEC;
            border: none;
            text-decoration: underline;
        }
        QPushButton:hover {
            color: #4A89DC;
        }
    )");
    connect(forgotPassword, &QPushButton::clicked, this, &LoginWindow::onForgotPassword);
    layout->addWidget(forgotPassword, 0, Qt::AlignCenter);

    layout->addStretch();

    // Version
    QLabel* versionLabel = new QLabel("Version 1.0");
    QFont versionFont("Segoe UI", 9);
    versionLabel->setFont(versionFont);
    versionLabel->setStyleSheet(R"(
        QLabel {
            color: #95A5A6;
        }
    )");
    versionLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(versionLabel);

    return card;
}

QFrame* LoginWindow::createInputField(const QString& icon, const QString& placeholder, bool isPassword)
{
    QFrame* container = new QFrame();
    container->setStyleSheet(R"(
        QFrame {
            background-color: #F8F9FA;
            border: 1px solid #E1E8ED;
            border-radius: 8px;
        }
    )");

    QHBoxLayout* layout = new QHBoxLayout(container);
    layout->setContentsMargins(15, 10, 15, 10);
    layout->setSpacing(10);

    // Icon
    QLabel* iconLabel = new QLabel(icon);
    iconLabel->setStyleSheet(R"(
        QLabel {
            font-size: 20px;
            color: #5D9CEC;
        }
    )");
    layout->addWidget(iconLabel);

    // Input field
    QLineEdit* inputField = new QLineEdit();
    inputField->setPlaceholderText(placeholder);
    QFont inputFont("Segoe UI", 11);
    inputField->setFont(inputFont);
    inputField->setStyleSheet(R"(
        QLineEdit {
            background: transparent;
            border: none;
            color: #2C3E50;
            padding: 5px;
        }
        QLineEdit::placeholder {
            color: #95A5A6;
        }
    )");

    if (isPassword) {
        inputField->setEchoMode(QLineEdit::Password);
    }

    layout->addWidget(inputField, 1);

    return container;
}

void LoginWindow::onLogin()
{
    QString username = usernameInput->text();
    QString password = passwordInput->text();

    if (!username.isEmpty() && !password.isEmpty()) {
        qDebug() << "Login attempt:" << username;

        // Open employee window
        EmployeeWindow* empWindow = new EmployeeWindow();
        empWindow->show();
        this->close();
    } else {
        qDebug() << "Please fill in all fields";
    }
}

void LoginWindow::onForgotPassword()
{
    qDebug() << "Forgot password clicked";
    // Add your password recovery logic here
}
