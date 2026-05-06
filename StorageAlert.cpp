#include "StorageAlert.h"
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>
#include <QDateTime>

StorageAlert::StorageAlert(const QString& fridgeRef, const QString& fishType, int days, QWidget *parent)
    : QDialog(parent)
{
    setupUi(fridgeRef, fishType, days);
    
    // Smooth entry animation
    setWindowOpacity(0);
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

StorageAlert::~StorageAlert() {}

void StorageAlert::setupUi(const QString& ref, const QString& type, int days)
{
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(500, 520);

    // Main Card with Glassmorphism / Modern Dark Theme
    QFrame* card = new QFrame(this);
    card->setObjectName("MainCard");
    card->setFixedSize(500, 520);
    card->setStyleSheet(R"(
        #MainCard {
            background: qlineargradient(
                x1:0, y1:0, x2:1, y2:1,
                stop:0 #1E293B,
                stop:1 #0F172A
            );
            border-radius: 30px;
            border: 1.5px solid rgba(255, 255, 255, 0.1);
        }
    )");

    // Shadow Effect (Deeper)
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(40);
    shadow->setXOffset(0);
    shadow->setYOffset(15);
    shadow->setColor(QColor(0, 0, 0, 180));
    card->setGraphicsEffect(shadow);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(15);

    // Header Decoration
    QHBoxLayout* headerDeco = new QHBoxLayout();
    QFrame* lineLeft = new QFrame();
    lineLeft->setFixedHeight(2);
    lineLeft->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 transparent, stop:1 #FACC15);");
    QFrame* lineRight = new QFrame();
    lineRight->setFixedHeight(2);
    lineRight->setStyleSheet("background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #FACC15, stop:1 transparent);");
    
    iconLabel = new QLabel("⚠️");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 48px;");
    
    headerDeco->addWidget(lineLeft, 1);
    headerDeco->addWidget(iconLabel);
    headerDeco->addWidget(lineRight, 1);
    layout->addLayout(headerDeco);

    // Animation for the icon (Pulse)
    QPropertyAnimation* pulse = new QPropertyAnimation(iconLabel, "geometry"); // Overriding geometry is hard, better use opacity or scale if possible
    // Simplified: Just use a static but high-quality title
    
    // Title
    titleLabel = new QLabel("ALERTE CONSERVATION");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Outfit", 18, QFont::ExtraBold)); // Using Outfit if available, else Arial/Segoe
    titleLabel->setStyleSheet("color: #FACC15; letter-spacing: 3px; margin-top: 10px;");
    layout->addWidget(titleLabel);

    // Divider
    QFrame* divider = new QFrame();
    divider->setFixedHeight(1);
    divider->setStyleSheet("background-color: rgba(255, 255, 255, 0.1);");
    layout->addWidget(divider);

    // Message context
    QLabel* ctxLabel = new QLabel("Dépassement du seuil critique de sécurité alimentaire.");
    ctxLabel->setAlignment(Qt::AlignCenter);
    ctxLabel->setStyleSheet("color: #94A3B8; font-size: 10pt; font-style: italic;");
    layout->addWidget(ctxLabel);

    // Main Message
    QString msg = QString("Le stock de <b style='color: #F87171;'>%1</b> (Frigo <b style='color: white;'>%2</b>) nécessite une action immédiate.").arg(type, ref);
    messageLabel = new QLabel(msg);
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setFont(QFont("Segoe UI", 12));
    messageLabel->setStyleSheet("color: #E2E8F0; line-height: 150%;");
    layout->addWidget(messageLabel);

    // Data Card (Premium Look)
    QFrame* dataBox = new QFrame();
    dataBox->setMinimumHeight(120); // Ensure enough height for the big number
    dataBox->setStyleSheet(R"(
        QFrame {
            background-color: rgba(255, 255, 255, 0.03);
            border-radius: 20px;
            border: 1px solid rgba(255, 255, 255, 0.08);
        }
    )");
    QVBoxLayout* dataLayout = new QVBoxLayout(dataBox);
    dataLayout->setContentsMargins(20, 15, 20, 15);
    dataLayout->setSpacing(5);
    
    storageInfoLabel = new QLabel(QString("<span style='color: #94A3B8; font-size: 11pt;'>Temps écoulé</span><br><span style='color: #F87171; font-size: 26pt; font-weight: 900;'>%1 JOURS</span>").arg(days));
    storageInfoLabel->setAlignment(Qt::AlignCenter);
    storageInfoLabel->setWordWrap(true); // Allow natural wrapping
    dataLayout->addWidget(storageInfoLabel);

    layout->addWidget(dataBox);

    layout->addStretch();

    // Action Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(20);

    QPushButton* actionBtn = new QPushButton("GÉRER LE STOCK");
    actionBtn->setFixedHeight(55);
    actionBtn->setCursor(Qt::PointingHandCursor);
    actionBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #FACC15;
            color: #0F172A;
            border: none;
            border-radius: 15px;
            font-weight: 800;
            font-size: 11pt;
            letter-spacing: 1px;
        }
        QPushButton:hover {
            background-color: #EAB308;
            margin: -2px; /* Subtle lift */
        }
    )");
    connect(actionBtn, &QPushButton::clicked, this, &QDialog::accept);

    QPushButton* dismissBtn = new QPushButton("IGNORER");
    dismissBtn->setFixedHeight(55);
    dismissBtn->setCursor(Qt::PointingHandCursor);
    dismissBtn->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(255, 255, 255, 0.05);
            color: #94A3B8;
            border: 1.5px solid rgba(255, 255, 255, 0.1);
            border-radius: 15px;
            font-weight: 700;
            font-size: 10pt;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.1);
            color: white;
        }
    )");
    connect(dismissBtn, &QPushButton::clicked, this, &QDialog::reject);

    btnLayout->addWidget(dismissBtn, 1);
    btnLayout->addWidget(actionBtn, 2);
    layout->addLayout(btnLayout);

    // Center on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}
