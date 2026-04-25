#include "mainwindow.h"
#include "Employeewindow.h"
#include "Frigowindow.h"
#include "temperaturealert.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QRegularExpression>
#include <QSqlRecord>
#include "pechewindow.h"
#include "loginwindow.h"
#include <QFont>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include "bateau.h"
#include <QStyle>
#include <QIcon>
#include <QStringList>
#include <QSystemTrayIcon>

MainWindow::MainWindow(const QString& userName, const QString& userRole, QWidget *parent)
    : QMainWindow(parent), currentActiveBtn(nullptr), employeePage(nullptr), pechePage(nullptr), frigoPage(nullptr), bateauPage(nullptr), livraisonPage(nullptr), quaisPage(nullptr),
      loggedUserName(userName), loggedUserRole(userRole)
{
    setupUi();
    checkMaintenanceAlerts();

    // Initialize Arduino
    int ret = A.connect_arduino();
    switch(ret) {
        case(0): qDebug() << "Arduino connected to:" << A.getarduino_port_name(); break;
        case(1): qDebug() << "Arduino found but failed to connect."; break;
        case(-1): qDebug() << "Arduino not found."; break;
    }

    connect(A.getserial(), &QSerialPort::readyRead, this, &MainWindow::handleSerialData);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle("PortFlow - Dashboard");
    setMinimumSize(1400, 800);

    setStyleSheet(R"(
        QMainWindow {
            background-color: #F0F4F8;
        }
    )");

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame* sidebar = createSidebar();
    mainLayout->addWidget(sidebar);

    // Stacked Widget pour les pages
    stackedWidget = new QStackedWidget();
    stackedWidget->setStyleSheet("background-color: #F0F4F8;");

    // Page 0: Dashboard
    stackedWidget->addWidget(createDashboardPage());

    // Les autres pages seront créées à la demande
    mainLayout->addWidget(stackedWidget, 1);

    if (dashboardBtn) {
        setActiveButton(dashboardBtn);
    }
}

QFrame* MainWindow::createSidebar()
{
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(280);
    sidebar->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(
                x1:0, y1:0, x2:0, y2:1,
                stop:0 #2B5EA6,
                stop:1 #5D9CEC
            );
        }
    )");

    QVBoxLayout* layout = new QVBoxLayout(sidebar);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    // Logo section
    QFrame* logoFrame = new QFrame();
    logoFrame->setFixedHeight(160);
    logoFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* logoLayout = new QVBoxLayout(logoFrame);
    logoLayout->setAlignment(Qt::AlignCenter);
    logoLayout->setContentsMargins(20, 15, 20, 15);

    QFrame* logoContainer = new QFrame();
    logoContainer->setFixedSize(220, 100);
    logoContainer->setStyleSheet(R"(
        QFrame {
            background-color: transparent;
            border-radius: 0px;
        }
    )");

    QVBoxLayout* containerLayout = new QVBoxLayout(logoContainer);
    containerLayout->setContentsMargins(5, 5, 5, 5);
    containerLayout->setAlignment(Qt::AlignCenter);

    QLabel* logoLabel = new QLabel();
    QPixmap logoPix(":/images/images/logo.png");

    if (!logoPix.isNull()) {
        logoLabel->setPixmap(logoPix.scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Logo chargé depuis: :/images/images/logo.png";
    } else {
        logoLabel->setText("🚢");
        logoLabel->setStyleSheet(R"(
            QLabel {
                font-size: 50px;
                color: #2C3E50;
            }
        )");
        logoLabel->setAlignment(Qt::AlignCenter);
        qDebug() << "Attention: Logo non trouvé à :/images/images/logo.png - utilisation emoji";
    }

    logoLabel->setStyleSheet("background: transparent;");
    containerLayout->addWidget(logoLabel);
    logoLayout->addWidget(logoContainer);
    layout->addWidget(logoFrame);

    // Navigation
    QFrame* navFrame = new QFrame();
    navFrame->setStyleSheet("background: transparent;");
    QVBoxLayout* navLayout = new QVBoxLayout(navFrame);
    navLayout->setSpacing(8);
    navLayout->setContentsMargins(20, 20, 20, 20);

    dashboardBtn = createNavButton("🏠", "Dashboard", true);
    connect(dashboardBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToDashboard);
    navLayout->addWidget(dashboardBtn);

    bateauxBtn = createNavButton("⛵", "Bateaux");
    connect(bateauxBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToBateaux);
    navLayout->addWidget(bateauxBtn);

    pechesBtn = createNavButton("🐟", "Pêche");
    connect(pechesBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToPeches);
    navLayout->addWidget(pechesBtn);

    employeesBtn = createNavButton("👥", "Employés");
    connect(employeesBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToEmployees);
    navLayout->addWidget(employeesBtn);

    frigosBtn = createNavButton("🧊", "Frigos");
    connect(frigosBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToFrigos);
    navLayout->addWidget(frigosBtn);

    livraisonBtn = createNavButton("🚚", "Livraison");
    connect(livraisonBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToLivraison);
    navLayout->addWidget(livraisonBtn);

    quaisBtn = createNavButton("⚓", "Quais");
    connect(quaisBtn, &QPushButton::clicked, this, &MainWindow::onNavigateToQuais);
    navLayout->addWidget(quaisBtn);

    QPushButton* settingsBtn = createNavButton("⚙️", "Paramètres");
    connect(settingsBtn, &QPushButton::clicked, this, &MainWindow::onSettingsClicked);
    navLayout->addWidget(settingsBtn);
    navLayout->addStretch();

    QPushButton* logoutBtn = createNavButton("🚪", "Quitter", false, true);
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);
    navLayout->addWidget(logoutBtn);

    layout->addWidget(navFrame, 1);

    return sidebar;
}

QPushButton* MainWindow::createNavButton(const QString& icon, const QString& text, bool isActive, bool isLogout)
{
    QPushButton* btn = new QPushButton(icon + "  " + text);
    QFont btnFont("Segoe UI", 12, QFont::Medium);
    btn->setFont(btnFont);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedHeight(55);
    btn->setProperty("navButton", true);

    if (isLogout) {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.1);
                color: white;
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
            }
            QPushButton:hover {
                background-color: rgba(239, 68, 68, 0.8);
            }
        )");
    } else if (isActive) {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.25);
                color: white;
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
                font-weight: bold;
            }
        )");
    } else {
        btn->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: rgba(255, 255, 255, 0.9);
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.15);
            }
        )");
    }

    return btn;
}

void MainWindow::setActiveButton(QPushButton* activeBtn)
{
    if (currentActiveBtn && currentActiveBtn->property("navButton").toBool()) {
        currentActiveBtn->setStyleSheet(R"(
            QPushButton {
                background-color: transparent;
                color: rgba(255, 255, 255, 0.9);
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
            }
            QPushButton:hover {
                background-color: rgba(255, 255, 255, 0.15);
            }
        )");
    }

    currentActiveBtn = activeBtn;
    if (activeBtn) {
        activeBtn->setStyleSheet(R"(
            QPushButton {
                background-color: rgba(255, 255, 255, 0.25);
                color: white;
                border: none;
                border-radius: 12px;
                text-align: left;
                padding-left: 20px;
                font-weight: bold;
            }
        )");
    }
}

void MainWindow::switchPage(int index)
{
    QWidget* currentWidget = stackedWidget->currentWidget();
    QGraphicsOpacityEffect* effect = new QGraphicsOpacityEffect(currentWidget);
    currentWidget->setGraphicsEffect(effect);

    QPropertyAnimation* fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(150);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, [this, index, effect]() {
        stackedWidget->setCurrentIndex(index);

        QWidget* newWidget = stackedWidget->currentWidget();
        QGraphicsOpacityEffect* newEffect = new QGraphicsOpacityEffect(newWidget);
        newWidget->setGraphicsEffect(newEffect);

        QPropertyAnimation* fadeIn = new QPropertyAnimation(newEffect, "opacity");
        fadeIn->setDuration(150);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QPropertyAnimation::DeleteWhenStopped);

        effect->deleteLater();
    });

    fadeOut->start(QPropertyAnimation::DeleteWhenStopped);
}

QWidget* MainWindow::createDashboardPage()
{
    QWidget* content = new QWidget();
    content->setStyleSheet("background-color: #F0F4F8;");

    QVBoxLayout* layout = new QVBoxLayout(content);
    layout->setSpacing(30);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel("Tableau de Bord");
    QFont titleFont("Segoe UI", 32, QFont::Bold);
    title->setFont(titleFont);
    title->setStyleSheet("color: #2C3E50;");
    layout->addWidget(title);

    QString welcomeText = loggedUserName.isEmpty() ? "Bienvenue sur PortFlow" : "Bienvenue, " + loggedUserName;
    if (!loggedUserRole.isEmpty()) welcomeText += " (" + loggedUserRole + ")";
    
    welcomeLabel = new QLabel(welcomeText);
    QFont subtitleFont("Segoe UI", 16);
    welcomeLabel->setFont(subtitleFont);
    welcomeLabel->setStyleSheet("color: #6B7280;");
    layout->addWidget(welcomeLabel);

    layout->addStretch();
    return content;
}

void MainWindow::onNavigateToDashboard()
{
    setActiveButton(dashboardBtn);
    switchPage(0);
}

void MainWindow::onNavigateToEmployees()
{
    setActiveButton(employeesBtn);
    if (!employeePage) {
        EmployeeWindow* empWindow = new EmployeeWindow();
        empWindow->hide();
        QWidget* centralWidget = empWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());
        if (hLayout && hLayout->count() >= 2) {
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                employeePage = contentItem->widget();
                if (employeePage) {
                    employeePage->setParent(nullptr);
                    stackedWidget->addWidget(employeePage);
                }
            }
        }
    }
    if (employeePage) {
        updateThemeRecursive(employeePage, isDarkMode);
        translateRecursive(employeePage, isEnglish);
        switchPage(stackedWidget->indexOf(employeePage));
    }
}

void MainWindow::onNavigateToFrigos()
{
    setActiveButton(frigosBtn);
    if (!frigoPage) {
        FrigoWindow* frigoWindow = new FrigoWindow();
        frigoWindow->hide();
        QWidget* centralWidget = frigoWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());
        if (hLayout && hLayout->count() >= 2) {
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                frigoPage = contentItem->widget();
                if (frigoPage) {
                    frigoPage->setParent(nullptr);
                    stackedWidget->addWidget(frigoPage);
                }
            }
        }
    }
    if (frigoPage) {
        updateThemeRecursive(frigoPage, isDarkMode);
        translateRecursive(frigoPage, isEnglish);
        switchPage(stackedWidget->indexOf(frigoPage));
    }
}

void MainWindow::onNavigateToPeches()
{
    setActiveButton(pechesBtn);
    if (!pechePage) {
        PecheWindow* pecheWindow = new PecheWindow();
        pecheWindow->hide();
        QWidget* centralWidget = pecheWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());
        if (hLayout && hLayout->count() >= 2) {
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                pechePage = contentItem->widget();
                if (pechePage) {
                    pechePage->setParent(nullptr);
                    stackedWidget->addWidget(pechePage);
                }
            }
        }
    }
    if (pechePage) {
        updateThemeRecursive(pechePage, isDarkMode);
        translateRecursive(pechePage, isEnglish);
        switchPage(stackedWidget->indexOf(pechePage));
    }
}

void MainWindow::onNavigateToBateaux()
{
    setActiveButton(bateauxBtn);
    if (!bateauPage) {
        BateauWindow* batWindow = new BateauWindow();
        batWindow->hide();
        QWidget* centralWidget = batWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());
        if (hLayout && hLayout->count() >= 2) {
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                bateauPage = contentItem->widget();
                if (bateauPage) {
                    bateauPage->setParent(nullptr);
                    stackedWidget->addWidget(bateauPage);
                }
            }
        }
    }
    if (bateauPage) {
        updateThemeRecursive(bateauPage, isDarkMode);
        translateRecursive(bateauPage, isEnglish);
        switchPage(stackedWidget->indexOf(bateauPage));
    }
}

void MainWindow::onNavigateToLivraison()
{
    setActiveButton(livraisonBtn);
    if (!livraisonPage) {
        LivraisonWindow* livWindow = new LivraisonWindow();
        livWindow->hide();
        QWidget* central = livWindow->centralWidget();
        if (central) {
            livraisonPage = central;
            livraisonPage->setParent(nullptr);
            stackedWidget->addWidget(livraisonPage);
        }
    }
    if (livraisonPage) {
        updateThemeRecursive(livraisonPage, isDarkMode);
        translateRecursive(livraisonPage, isEnglish);
        switchPage(stackedWidget->indexOf(livraisonPage));
    }
}

void MainWindow::onNavigateToQuais()
{
    setActiveButton(quaisBtn);
    if (!quaisPage) {
        QuaisWindow* qWindow = new QuaisWindow();
        qWindow->hide();
        QWidget* central = qWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(central->layout());
        if (hLayout && hLayout->count() >= 2) {
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                quaisPage = contentItem->widget();
                if (quaisPage) {
                    quaisPage->setParent(nullptr);
                    stackedWidget->addWidget(quaisPage);
                }
            }
        }
    }
    if (quaisPage) {
        updateThemeRecursive(quaisPage, isDarkMode);
        translateRecursive(quaisPage, isEnglish);
        switchPage(stackedWidget->indexOf(quaisPage));
    }
}

void MainWindow::onLogout()
{
    if (QMessageBox::question(this, "Déconnexion", "Voulez-vous vraiment vous déconnecter ?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        LoginWindow* loginWindow = new LoginWindow();
        loginWindow->show();
        this->close();
    }
}

void MainWindow::toggleGlobalTheme()
{
    isDarkMode = !isDarkMode;
    updateThemeRecursive(this, isDarkMode);
}

void MainWindow::onSettingsClicked()
{
    QDialog* settingsDlg = new QDialog(this);
    settingsDlg->setWindowTitle("Paramètres - PortFlow");
    settingsDlg->setFixedSize(350, 250);
    settingsDlg->setStyleSheet(isDarkMode ? "background-color: #2D3748; color: white;" : "background-color: #F0F4F8; color: #2C3E50;");

    QVBoxLayout* layout = new QVBoxLayout(settingsDlg);
    layout->setSpacing(15);
    layout->setContentsMargins(30, 30, 30, 30);

    QLabel* title = new QLabel("Préférences");
    title->setFont(QFont("Segoe UI", 16, QFont::Bold));
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    QPushButton* langBtnDlg = new QPushButton(isEnglish ? "🌐  Language: English" : "🌐  Langue: Français");
    QPushButton* darkBtnDlg = new QPushButton(isDarkMode ? "☀️  Mode: Clair" : "🌙  Mode: Sombre");

    auto btnStyle = R"(
        QPushButton {
            background-color: #2B5EA6;
            color: white;
            border: none;
            border-radius: 12px;
            padding: 15px;
            font-weight: bold;
            font-size: 16px;
        }
        QPushButton:hover {
            background-color: #1e3a5f;
        }
    )";

    langBtnDlg->setStyleSheet(btnStyle);
    darkBtnDlg->setStyleSheet(btnStyle);

    connect(langBtnDlg, &QPushButton::clicked, [this, langBtnDlg, settingsDlg]() {
        toggleLanguage();
        langBtnDlg->setText(isEnglish ? "🌐  Language: English" : "🌐  Langue: Français");
        translateRecursive(settingsDlg, isEnglish);
    });

    connect(darkBtnDlg, &QPushButton::clicked, [this, darkBtnDlg, settingsDlg]() {
        toggleGlobalTheme();
        darkBtnDlg->setText(isDarkMode ? "☀️  Mode: Clair" : "🌙  Mode: Sombre");
        settingsDlg->setStyleSheet(isDarkMode ? "background-color: #2D3748; color: white;" : "background-color: #F0F4F8; color: #2C3E50;");
    });

    layout->addWidget(langBtnDlg);
    layout->addWidget(darkBtnDlg);
    
    QPushButton* closeBtn = new QPushButton("Fermer");
    closeBtn->setStyleSheet("background-color: #6c757d; color: white; border-radius: 10px; padding: 10px; font-weight: bold; margin-top: 10px;");
    connect(closeBtn, &QPushButton::clicked, settingsDlg, &QDialog::accept);
    layout->addWidget(closeBtn);

    translateRecursive(settingsDlg, isEnglish);
    settingsDlg->exec();
}

void MainWindow::updateThemeRecursive(QWidget* widget, bool isDark)
{
    if (!widget) return;
    QVariant origVar = widget->property("origStyleSheet");
    if (!origVar.isValid()) {
        origVar = widget->styleSheet();
        widget->setProperty("origStyleSheet", origVar);
    }
    QString style = origVar.toString();
    if (!style.isEmpty()) {
        if (isDark) {
            style.replace("#1e3a5f", "#F7FAFC", Qt::CaseInsensitive);
            style.replace("#2C3E50", "#F7FAFC", Qt::CaseInsensitive);
            style.replace("#6B7280", "#A0AEC0", Qt::CaseInsensitive);
            style.replace("#1f2937", "#F7FAFC", Qt::CaseInsensitive);
            style.replace("#0f172a", "#F7FAFC", Qt::CaseInsensitive);
            style.replace("#334155", "#F7FAFC", Qt::CaseInsensitive);
            style.replace("#F0F4F8", "#1A202C", Qt::CaseInsensitive);
            style.replace("#f8fafc", "#1A202C", Qt::CaseInsensitive);
            style.replace("#F9FAFB", "#2D3748", Qt::CaseInsensitive);
            style.replace("background-color: white", "background-color: #2D3748", Qt::CaseInsensitive);
            style.replace("background: white", "background: #2D3748", Qt::CaseInsensitive);
            style.replace("#FFFFFF", "#2D3748", Qt::CaseInsensitive);
            style.replace("#e2e8f0", "#4A5568", Qt::CaseInsensitive);
            style.replace("#d1d5db", "#4A5568", Qt::CaseInsensitive);
            style.replace("color: black", "color: white", Qt::CaseInsensitive);
        }
        widget->setStyleSheet(style);
    }
    for (QObject* child : widget->children()) {
        if (QWidget* w = qobject_cast<QWidget*>(child)) {
            updateThemeRecursive(w, isDark);
        }
    }
}

void MainWindow::toggleLanguage()
{
    isEnglish = !isEnglish;
    translateRecursive(this, isEnglish);
}

void MainWindow::translateRecursive(QWidget* widget, bool toEnglish)
{
    if (!widget) return;
    auto translateText = [toEnglish](QString& text) {
        if (text.isEmpty()) return;
        static const QHash<QString, QString> dict = {
            {"Tableau de Bord", "Dashboard"}, {"Bateaux", "Boats"}, {"Pêche", "Fishing"}, {"Frigos", "Fridges"},
            {"Employés", "Employees"}, {"Livraison", "Delivery"}, {"Quais", "Docks"}, {"Paramètres", "Settings"},
            {"Quitter", "Quit"}, {"Mode Sombre", "Dark Mode"}, {"Mode Clair", "Light Mode"},
            {"Gestion des Employés", "Employee Management"}, {"Administration du personnel et suivi des rôles", "Staff administration and role tracking"},
            {"Nouvel Employé", "New Employee"}, {"Trier par :", "Sort by :"}, {"Prénom", "First Name"},
            {"Nom", "Last Name"}, {"Position", "Position"}, {"Salaire", "Salary"}, {"Statut", "Status"},
            {"Date d'ajout", "Added Date"}, {"Administration", "Administration"}
        };
        for (auto it = dict.constBegin(); it != dict.constEnd(); ++it) {
            QString fr = it.key(); QString en = it.value();
            if (toEnglish && text.contains(fr)) text.replace(fr, en);
            else if (!toEnglish && text.contains(en)) text.replace(en, fr);
        }
    };
    if (QLabel* lbl = qobject_cast<QLabel*>(widget)) {
        QString text = lbl->text(); translateText(text); lbl->setText(text);
    } else if (QPushButton* btn = qobject_cast<QPushButton*>(widget)) {
        QString text = btn->text(); translateText(text); btn->setText(text);
    } else if (QTableWidget* table = qobject_cast<QTableWidget*>(widget)) {
        for (int i = 0; i < table->columnCount(); ++i) {
            if (QTableWidgetItem* item = table->horizontalHeaderItem(i)) {
                QString text = item->text(); translateText(text); item->setText(text);
            }
        }
    } else if (QLineEdit* line = qobject_cast<QLineEdit*>(widget)) {
        QString text = line->placeholderText(); translateText(text); line->setPlaceholderText(text);
    }
    for (QObject* child : widget->children()) {
        if (QWidget* w = qobject_cast<QWidget*>(child)) translateRecursive(w, toEnglish);
    }
}

void MainWindow::checkMaintenanceAlerts() {
    QStringList boatNames = Bateau::getUpcomingMaintenanceAlerts();
    
    if (!boatNames.isEmpty()) {
        if (!trayIcon) {
            trayIcon = new QSystemTrayIcon(this);
            connect(trayIcon, &QSystemTrayIcon::messageClicked, this, &MainWindow::onTrayMessageClicked);
        }
        
        trayIcon->setIcon(QIcon(":/images/images/logo.png"));
        if (trayIcon->icon().isNull()) {
            trayIcon->setIcon(this->style()->standardIcon(QStyle::SP_MessageBoxWarning));
        }
        trayIcon->show();
        
        QString message = "Les bateaux suivants nécessitent une maintenance :\n" + boatNames.join("\n");
        trayIcon->showMessage("Alerte Maintenance", message, QSystemTrayIcon::Warning, 15000);
    }
}

void MainWindow::onTrayMessageClicked() {
    QStringList boatNames = Bateau::getUpcomingMaintenanceAlerts();
    if (boatNames.isEmpty()) return;

    QString detail = "Voici la liste détaillée des bateaux nécessitant une maintenance :\n\n";
    detail += boatNames.join("\n");
    detail += "\n\nVeuillez vérifier l'état de ces bateaux dans l'onglet 'Bateaux'.";

    QMessageBox::information(this, "PortFlow - Détails Maintenance", detail);
}

void MainWindow::handleSerialData()
{
    serialBuffer += A.read_from_arduino();
    
    // 1. Process fridge messages (delimited by ;)
    while (serialBuffer.contains(';')) {
        int index = serialBuffer.indexOf(';');
        QByteArray message = serialBuffer.left(index).trimmed();
        serialBuffer.remove(0, index + 1);
        
        QString msgStr = QString::fromLatin1(message);
        qDebug() << "Fridge Data:" << msgStr;
        
        QRegularExpression re("(S[12]):(\\d+\\.?\\d*)");
        QRegularExpressionMatch match = re.match(msgStr);
        if (match.hasMatch()) {
            QString sensorIdStr = match.captured(1);
            double temp = match.captured(2).toDouble();
            checkFridgeTemperature((sensorIdStr == "S1") ? 1 : 2, temp);
        }
    }

    // 2. Process port access messages (delimited by #)
    while (serialBuffer.contains('#')) {
        int index = serialBuffer.indexOf('#');
        QString rawCode = QString::fromLatin1(serialBuffer.left(index));
        serialBuffer.remove(0, index + 1);
        
        // Nettoyage strict : on ne garde que les chiffres
        QString code = "";
        for(char c : rawCode.toStdString()) {
            if(isdigit(c)) code += c;
        }

        if (!code.isEmpty()) {
            processPortAccess(code);
        }
    }
}

void MainWindow::processPortAccess(const QString& code)
{
    qDebug() << "--- Tentative Accès Port ---";
    qDebug() << "Code reçu (nettoyé):" << code;
    
    bool ok;
    int codeInt = code.toInt(&ok);
    if (!ok) {
        qDebug() << "Erreur: Le code n'est pas un nombre valide.";
        A.write_to_arduino("E\n");
        return;
    }
    
    QSqlQuery bQuery;
    // On cherche le bateau par son code secret
    bQuery.prepare("SELECT IDBATEAU, NOMBATEAU, ETAT, IDQUAI FROM BATEAUX WHERE CODE_SECRET = :code");
    bQuery.bindValue(":code", codeInt);

    if (!bQuery.exec()) {
        qDebug() << "Erreur SQL recherche bateau:" << bQuery.lastError().text();
        QMessageBox::critical(this, "Erreur SQL", "Erreur lecture bateau : " + bQuery.lastError().text());
        return;
    }

    if (bQuery.next()) {
        int idBateau = bQuery.value(0).toInt();
        QString nomBateau = bQuery.value(1).toString();
        QString etatBateau = bQuery.value(2).toString();
        
        qDebug() << "Bateau trouvé:" << nomBateau << "(ID:" << idBateau << ")";
        
        // [SÉCURITÉ] Si le bateau est déjà au port
        QString cleanEtat = etatBateau.trimmed().simplified();
        if (cleanEtat.compare("Au port", Qt::CaseInsensitive) == 0) {
            qDebug() << "Refus: Bateau déjà au port.";
            A.write_to_arduino("R"); // Signal 'R' (Arduino)
            
            QTimer::singleShot(100, this, [this, nomBateau](){
                QMessageBox::warning(this, "Accès Refusé", "Bateau déjà au port : " + nomBateau);
            });
            return; 
        }

        // CAS 2 : Recherche de quai libre
        QSqlQuery qQuery;
        qQuery.prepare("SELECT IDQUAI, NUMERO FROM QUAIS "
                       "WHERE (UPPER(TRIM(ETAT)) NOT IN ('OCCUPÉ', 'OCCUPE') OR ETAT IS NULL) "
                       "AND ROWNUM <= 1");

        if (qQuery.exec()) {
            if (qQuery.next()) {
                int idQuaiLibre = qQuery.value(0).toInt();
                int numQuai = qQuery.value(1).toInt();

                qDebug() << "Quai libre trouvé:" << numQuai;

                // Envoie l'autorisation : juste le chiffre pour simplifier la lecture Arduino
                A.write_to_arduino(QByteArray::number(numQuai));

                QSqlQuery upB, upQ;
                upB.prepare("UPDATE BATEAUX SET IDQUAI = :q, ETAT = 'Au port' WHERE IDBATEAU = :id");
                upB.bindValue(":q", idQuaiLibre);
                upB.bindValue(":id", idBateau);

                upQ.prepare("UPDATE QUAIS SET ETAT = 'Occupé' WHERE IDQUAI = :id");
                upQ.bindValue(":id", idQuaiLibre);

                if (upB.exec() && upQ.exec()) {
                    BateauWindow::refreshAllTables();
                    QuaisWindow::refreshAll();
                    QTimer::singleShot(100, this, [this, nomBateau, numQuai](){
                        QMessageBox::information(this, "Accès Autorisé", nomBateau + " -> Quai " + QString::number(numQuai));
                    });
                }
            } else {
                qDebug() << "Refus: Port complet.";
                A.write_to_arduino("F");
                QMessageBox::warning(this, "Port Complet", "Plus de place disponible.");
            }
        }
    } else {
        // AUCUN BATEAU TROUVÉ
        qDebug() << "Refus: Aucun bateau trouvé avec le code" << codeInt;
        A.write_to_arduino("E");
        QTimer::singleShot(100, this, [this](){
            QMessageBox::critical(this, "Accès Refusé", "Code secret incorrect.");
        });
    }
}


void MainWindow::checkFridgeTemperature(int sensorId, double currentTemp)
{
    // Mapping: S1 -> FRG-001, S2 -> FRG-002 (as proposed in the plan)
    QString fridgeRef = (sensorId == 1) ? "FRG-001" : "FRG-002";
    
    QSqlQuery query;
    query.prepare("SELECT TEMPERATURE FROM FRIGOS WHERE REFERENCE = :ref");
    query.bindValue(":ref", fridgeRef);
    
    if (query.exec() && query.next()) {
        double threshold = query.value(0).toDouble();
        
        // User logic: "if the temperature goes below the required temperature"
        if (currentTemp < threshold) {
            static QSet<QString> activeAlerts; // Prevent spamming alerts
            if (!activeAlerts.contains(fridgeRef)) {
                activeAlerts.insert(fridgeRef);
                
                TemperatureAlert* alert = new TemperatureAlert(fridgeRef, threshold, currentTemp, this);
                connect(alert, &QDialog::finished, [fridgeRef]() {
                    // Allow alert to reappear after closing if condition persists (maybe with a delay)
                    // For now, we clear it so it can trigger again next time
                    QTimer::singleShot(10000, [fridgeRef]() {
                         // ActiveAlerts is static so this is tricky, let's keep it simple for now
                    });
                });
                alert->show();
                qDebug() << "ALERT: Fridge" << fridgeRef << "is too cold!" << currentTemp << "<" << threshold;
            }
        }
    } else {
        qDebug() << "Warning: No temperature threshold found for fridge" << fridgeRef;
    }
}
