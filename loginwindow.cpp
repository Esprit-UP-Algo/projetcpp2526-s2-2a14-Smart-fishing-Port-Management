#include "loginwindow.h"
#include "mainwindow.h"
#include "employeewindow.h"
#include "Frigowindow.h"
#include <QFont>
#include <QApplication>
#include <QPalette>
#include <QBrush>
#include <QPixmap>
#include <QDebug>
#include <QProcess>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDir>
#include <QCoreApplication>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    // Start pre-warming the Face ID process in the background immediately
    // so by the time the user clicks the button, model is already loaded
    startFaceIDPrewarm();
}

LoginWindow::~LoginWindow()
{
    if (faceIdProcess) {
        faceIdProcess->kill();
        faceIdProcess->deleteLater();
    }
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

QString backgroundImagePath = "C:/Users/Anas/Downloads/login.png";
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

    QString logoPath = ":/images/images/logo.png";  // Loaded from resources

    // ============================================
    // LOGO DISPLAY OPTIONS
    // ============================================
    // Set this to true if your logo already includes the "PortFlow" text
    // Set this to false if your logo is just an icon and needs text below
    bool logoHasText = true;  // ← CHANGED TO TRUE: logo includes text

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
        logoLabel->setText("🚢");
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
    QFrame* usernameContainer = createInputField("👤", "Email (ex: nom@gmail.com)");
    layout->addWidget(usernameContainer);
    usernameInput = usernameContainer->findChild<QLineEdit*>();

    // Password input
    QFrame* passwordContainer = createInputField("🔒", "CIN (Mot de passe)", true);
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

    // Face ID button — stored as member so prewarm can update it
    faceIdBtn = new QPushButton("Login with Face ID  ⏳");
    faceIdBtn->setFont(btnFont);
    faceIdBtn->setCursor(Qt::PointingHandCursor);
    faceIdBtn->setFixedHeight(50);
    faceIdBtn->setEnabled(false);  // Disabled until model is ready
    faceIdBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #95A5A6;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 12px;
        }
        QPushButton:hover {
            background-color: #7F8C8D;
        }
    )");
    connect(faceIdBtn, &QPushButton::clicked, this, &LoginWindow::onFaceIDLogin);
    layout->addWidget(faceIdBtn);

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
    QString email    = usernameInput->text().trimmed();
    QString password = passwordInput->text().trimmed();

    if (email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Champs requis", "Veuillez saisir votre email et votre mot de passe.");
        return;
    }

    // Search the employee by EMAIL (identifier) and CIN (password)
    // Try both table names used in the project: EMPLOYEES and EMPLOYEE
    QSqlQuery query;
    bool found = false;
    QString employeeName;
    QString employeeRole;

    auto tryQuery = [&](const QString& tableName) {
        query.prepare(QString(
            "SELECT PRENOM, NOM, \"POSITION\" FROM %1 "
            "WHERE LOWER(EMAIL) = LOWER(:email) AND CAST(CIN AS TEXT) = :cin"
        ).arg(tableName));
        query.bindValue(":email", email);
        query.bindValue(":cin",   password);
        if (query.exec() && query.next()) {
            employeeName = query.value(0).toString() + " " + query.value(1).toString();
            employeeRole = query.value(2).toString();
            return true;
        }
        return false;
    };

    found = tryQuery("EMPLOYEES");
    if (!found) found = tryQuery("EMPLOYEE");

    if (found) {
        MainWindow* mainWin = new MainWindow(employeeName, employeeRole);
        mainWin->show();
        mainWin->raise();
        mainWin->activateWindow();
        QMessageBox::information(mainWin, "Connexion réussie",
            "Bienvenue, " + employeeName + " !\nPoste : " + employeeRole);
        this->close();
    } else {
        // Show a shake-like effect by briefly styling the inputs red
        QString errorStyle = R"(
            QLineEdit {
                background: #FEE2E2;
                border: 2px solid #EF4444;
                border-radius: 8px;
                padding: 5px;
                color: #991B1B;
            }
        )";
        usernameInput->setStyleSheet(errorStyle);
        passwordInput->setStyleSheet(errorStyle);

        // Reset style after 1.5 seconds
        QTimer::singleShot(1500, this, [this]() {
            usernameInput->setStyleSheet("");
            passwordInput->setStyleSheet("");
        });

        QMessageBox::warning(this, "Accès refusé",
            "Email ou mot de passe incorrect.\n"
            "L'identifiant est votre email, le mot de passe est votre CIN.");
    }
}

void LoginWindow::onFaceIDLogin()
{
    if (!faceIdProcess || !faceIdReady) {
        QMessageBox::information(this, "Please wait", "Face ID is still initializing. Please try again in a moment.");
        return;
    }

    faceIdBtn->setText("Scanning... \xF0\x9F\x91\x81");
    faceIdBtn->setEnabled(false);
    faceIdOutputBuffer.clear();

    // Just send GO — Python is already warmed up with camera + model loaded
    faceIdProcess->write("GO\n");
}

void LoginWindow::handleFaceIDOutput()
{
    if (!faceIdProcess) return;
    faceIdOutputBuffer += faceIdProcess->readAllStandardOutput();
    qDebug() << "Face ID output:" << faceIdOutputBuffer;

    // Check for READY signal (pre-warm complete)
    if (!faceIdReady && faceIdOutputBuffer.contains("READY")) {
        faceIdReady = true;
        faceIdOutputBuffer.clear();
        qDebug() << "Face ID model loaded and ready!";

        if (faceIdBtn) {
            faceIdBtn->setText("Login with Face ID");
            faceIdBtn->setEnabled(true);
            faceIdBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #2ECC71;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 12px;
                }
                QPushButton:hover {
                    background-color: #27AE60;
                }
            )");
        }
        return;
    }

    if (!faceIdReady) return;

    // Parse JSON result from scanning
    int jsonStart = faceIdOutputBuffer.indexOf("{");
    int jsonEnd = faceIdOutputBuffer.lastIndexOf("}");
    if (jsonStart == -1 || jsonEnd == -1 || jsonEnd <= jsonStart) return;

    QString jsonStr = faceIdOutputBuffer.mid(jsonStart, jsonEnd - jsonStart + 1);
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
    QJsonObject obj = doc.object();
    if (obj.isEmpty()) return;

    faceIdOutputBuffer.clear();

    if (obj["status"].toString() == "success") {
        QString name = obj["name"].toString();
        QString role = obj["role"].toString();
        name.replace("_", " ");

        faceIdProcess->disconnect();
        faceIdProcess->kill();
        faceIdProcess = nullptr; // Parent (this) owns the process and will delete it

        // Create and show MainWindow FIRST so the app has a live window
        MainWindow* mainWin = new MainWindow(name, role);
        mainWin->show();
        mainWin->raise();
        mainWin->activateWindow();

        // Show greeting parented to mainWin (not this) so it appears above the dashboard
        QMessageBox::information(mainWin, "Welcome", "Hello " + name + "!");

        // Finally close the login window — this triggers ~LoginWindow which
        // safely deletes faceIdProcess as a child (already nullptr, no danger)
        this->close();

    } else {
        QString errorMsg = obj["message"].toString();
        if (errorMsg.isEmpty()) errorMsg = "Face not recognized. Please try again.";
        QMessageBox::warning(this, "Authentication Failed", errorMsg);

        faceIdReady = false;
        // Kill the process — do NOT call deleteLater() (child of this, causes double-free)
        // startFaceIDPrewarm() below will kill & recreate it cleanly
        if (faceIdProcess) {
            faceIdProcess->disconnect();
            faceIdProcess->kill();
            faceIdProcess->waitForFinished(300);
            faceIdProcess = nullptr;
        }

        if (faceIdBtn) {
            faceIdBtn->setText("Login with Face ID  \xe2\x8f\xb3");
            faceIdBtn->setEnabled(false);
            faceIdBtn->setStyleSheet(R"(
                QPushButton {
                    background-color: #95A5A6;
                    color: white;
                    border: none;
                    border-radius: 8px;
                    padding: 12px;
                }
            )");
        }
        startFaceIDPrewarm();
    }
}

void LoginWindow::startFaceIDPrewarm()
{
    if (faceIdProcess) {
        faceIdProcess->disconnect();
        faceIdProcess->kill();
        faceIdProcess->waitForFinished(300);
        // Explicit delete is safe here — we're replacing it with a new instance below.
        // Do NOT use deleteLater() on a child object we're about to re-create.
        delete faceIdProcess;
        faceIdProcess = nullptr;
    }

    faceIdReady = false;
    faceIdOutputBuffer.clear();

    faceIdProcess = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PYTHONIOENCODING", "utf-8");
    faceIdProcess->setProcessEnvironment(env);

    QString scriptPath = "face_id/face_auth.py";
    QDir dir(QCoreApplication::applicationDirPath());
    while (!dir.isRoot()) {
        if (dir.exists(scriptPath)) {
            scriptPath = dir.absoluteFilePath(scriptPath);
            break;
        }
        dir.cdUp();
    }
    if (!QFileInfo(scriptPath).exists()) {
        scriptPath = "C:/Users/manne/Downloads/projetcpp2526-s2-2a14-Smart-fishing-Port-Management-gestion-des-peches-v2/projetcpp2526-s2-2a14-Smart-fishing-Port-Management-gestion-des-peches-v2/face_id/face_auth.py";
    }

    qDebug() << "Pre-warming Face ID from:" << scriptPath;
    connect(faceIdProcess, &QProcess::readyReadStandardOutput, this, &LoginWindow::handleFaceIDOutput);

    QStringList args;
    args << scriptPath << "verify";

    faceIdProcess->start("python", args);
    if (!faceIdProcess->waitForStarted(500)) {
        faceIdProcess->start("py", args);
        if (!faceIdProcess->waitForStarted(500)) {
            QString fallback = QDir::homePath() + "/AppData/Local/Programs/Python/Python312/python.exe";
            faceIdProcess->start(fallback, args);
            if (!faceIdProcess->waitForStarted(500)) {
                qDebug() << "Could not start Python for Face ID pre-warm.";
                delete faceIdProcess;
                faceIdProcess = nullptr;
            }
        }
    }
}

void LoginWindow::onForgotPassword()
{
    qDebug() << "Forgot password clicked";
    // Add your password recovery logic here
}

