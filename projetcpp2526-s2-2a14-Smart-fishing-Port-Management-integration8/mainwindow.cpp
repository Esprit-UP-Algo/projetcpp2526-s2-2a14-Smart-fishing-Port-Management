#include "mainwindow.h"
#include "Employeewindow.h"
#include "Frigowindow.h"
#include "pechewindow.h"
#include "loginwindow.h"
#include <QFont>
#include <QPixmap>
#include <QDebug>
#include <QMessageBox>
#include <QGraphicsOpacityEffect>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentActiveBtn(nullptr), employeePage(nullptr), pechePage(nullptr), frigoPage(nullptr), bateauPage(nullptr), livraisonPage(nullptr), quaisPage(nullptr)
{

    setupUi();
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

    // Définir le bouton dashboard comme actif au démarrage
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

    navLayout->addWidget(createNavButton("⚙️", "Paramètres"));
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
    // Réinitialiser l'ancien bouton actif
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

    // Activer le nouveau bouton
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
    // Animation de fade
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

    QLabel* subtitle = new QLabel("Bienvenue sur PortFlow");
    QFont subtitleFont("Segoe UI", 14);
    subtitle->setFont(subtitleFont);
    subtitle->setStyleSheet("color: #6B7280;");
    layout->addWidget(subtitle);

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

    // Créer la page Employés si elle n'existe pas
    if (!employeePage) {
        EmployeeWindow* empWindow = new EmployeeWindow();
        empWindow->hide(); // Important : ne pas afficher la fenêtre complète

        // Extraire SEULEMENT le widget de contenu (partie droite)
        // On cherche le QWidget qui contient le tableau
        QWidget* centralWidget = empWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());

        if (hLayout && hLayout->count() >= 2) {
            // Le deuxième élément est le contenu (après la sidebar)
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                employeePage = contentItem->widget();
                if (employeePage) {
                    employeePage->setParent(nullptr); // Détacher du layout original
                    stackedWidget->addWidget(employeePage);
                }
            }
        }
    }

    if (employeePage) {
        switchPage(stackedWidget->indexOf(employeePage));
    }
}

void MainWindow::onNavigateToFrigos()
{
    setActiveButton(frigosBtn);

    // Créer la page Frigos si elle n'existe pas
    if (!frigoPage) {
        FrigoWindow* frigoWindow = new FrigoWindow();
        frigoWindow->hide(); // Important : ne pas afficher la fenêtre complète

        // Extraire SEULEMENT le widget de contenu (partie droite)
        QWidget* centralWidget = frigoWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());

        if (hLayout && hLayout->count() >= 2) {
            // Le deuxième élément est le contenu (après la sidebar)
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                frigoPage = contentItem->widget();
                if (frigoPage) {
                    frigoPage->setParent(nullptr); // Détacher du layout original
                    stackedWidget->addWidget(frigoPage);
                }
            }
        }
    }

    if (frigoPage) {
        switchPage(stackedWidget->indexOf(frigoPage));
    }
}

void MainWindow::onNavigateToPeches()
{
    setActiveButton(pechesBtn);

    // Créer la page Pêches si elle n'existe pas
    if (!pechePage) {
        PecheWindow* pecheWindow = new PecheWindow();
        pecheWindow->hide(); // Important : ne pas afficher la fenêtre complète

        // Extraire SEULEMENT le widget de contenu (partie droite)
        QWidget* centralWidget = pecheWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());

        if (hLayout && hLayout->count() >= 2) {
            // Le deuxième élément est le contenu (après la sidebar)
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                pechePage = contentItem->widget();
                if (pechePage) {
                    pechePage->setParent(nullptr); // Détacher du layout original
                    stackedWidget->addWidget(pechePage);
                }
            }
        }
    }

    if (pechePage) {
        switchPage(stackedWidget->indexOf(pechePage));
    }
}

void MainWindow::onNavigateToBateaux()
{
    setActiveButton(bateauxBtn);

    // Créer la page Bateaux si elle n'existe pas
    if (!bateauPage) {
        BateauWindow* batWindow = new BateauWindow();
        batWindow->hide(); // Important : ne pas afficher la fenêtre complète

        // Extraire SEULEMENT le widget de contenu (partie droite)
        QWidget* centralWidget = batWindow->centralWidget();
        QHBoxLayout* hLayout = qobject_cast<QHBoxLayout*>(centralWidget->layout());

        if (hLayout && hLayout->count() >= 2) {
            // Le deuxième élément est le contenu (après la sidebar)
            QLayoutItem* contentItem = hLayout->itemAt(1);
            if (contentItem) {
                bateauPage = contentItem->widget();
                if (bateauPage) {
                    bateauPage->setParent(nullptr); // Détacher du layout original
                    stackedWidget->addWidget(bateauPage);
                }
            }
        }
        // Nettoyer la fenêtre temporaire
        // batWindow->deleteLater(); // Attention: cela pourrait supprimer le widget enfant si mal géré
        // Dans ce cas, comme on a reparenté le widget, on peut supprimer la fenêtre conteneur vide
        // Mais par sécurité on peut le garder en mémoire ou le supprimer après
    }

    if (bateauPage) {
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
            // Index 0 is sidebar, index 1 is content
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
        switchPage(stackedWidget->indexOf(quaisPage));
    }
}


void MainWindow::onLogout()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Déconnexion",
                                  "Voulez-vous vraiment vous déconnecter ?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        LoginWindow* loginWindow = new LoginWindow();
        loginWindow->show();
        this->close();
    }
}
