#include "temperaturealert.h"
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>

TemperatureAlert::TemperatureAlert(const QString& fridgeRef, double threshold, double current, QWidget *parent)
    : QWidget(parent)
{
    setupUi(fridgeRef, threshold, current);
    
    // Smooth entry animation
    setWindowOpacity(0);
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

TemperatureAlert::~TemperatureAlert() {}

void TemperatureAlert::setupUi(const QString& ref, double threshold, double current)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedSize(500, 560);

    // Main Card
    QFrame* card = new QFrame(this);
    card->setObjectName("MainCard");
    card->setFixedSize(500, 560);
    card->setStyleSheet(R"(
        #MainCard {
            background-color: #1E293B;
            border-radius: 28px;
            border: 2px solid #334155;
        }
    )");

    // Shadow Effect
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(10);
    shadow->setColor(QColor(0, 0, 0, 160));
    card->setGraphicsEffect(shadow);

    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(35, 35, 35, 35);
    layout->setSpacing(20);

    // Icon Header (Pulsing Warning)
    iconLabel = new QLabel("⚠️");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 64px; margin-bottom: 10px;");
    layout->addWidget(iconLabel);

    // Title
    titleLabel = new QLabel("ALERTE TEMPÉRATURE");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Segoe UI", 20, QFont::Bold));
    titleLabel->setStyleSheet("color: #F87171; letter-spacing: 3px;");
    layout->addWidget(titleLabel);

    // Message
    messageLabel = new QLabel(QString("🚨 <b>DANGER IMMÉDIAT</b> 🚨<br>Le frigo <b>%1</b> a dépassé sa température de sécurité !").arg(ref));
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setWordWrap(true);
    messageLabel->setFont(QFont("Segoe UI", 13));
    messageLabel->setStyleSheet("color: #FDA4AF; line-height: 1.5; margin-bottom: 5px;");
    layout->addWidget(messageLabel);

    // Temp comparison
    QFrame* dataBox = new QFrame();
    dataBox->setStyleSheet("background-color: #0F172A; border-radius: 20px; border: 1px solid #334155;");
    dataBox->setMinimumHeight(120); // Ensure enough height
    QHBoxLayout* dataLayout = new QHBoxLayout(dataBox);
    dataLayout->setContentsMargins(25, 20, 25, 20);
    dataLayout->setSpacing(15);

    auto createTempBox = [this](const QString& label, double val, const QString& color) {
        QVBoxLayout* v = new QVBoxLayout();
        v->setSpacing(8);
        QLabel* l = new QLabel(label);
        l->setStyleSheet("color: #94A3B8; font-weight: bold; font-size: 11pt; text-transform: uppercase;");
        l->setAlignment(Qt::AlignCenter);
        
        QLabel* vL = new QLabel(QString::number(val, 'f', 1) + " °C");
        vL->setStyleSheet(QString("color: %1; font-size: 26pt; font-weight: 900;").arg(color));
        vL->setAlignment(Qt::AlignCenter);
        
        v->addWidget(l);
        v->addWidget(vL);
        return v;
    };

    dataLayout->addLayout(createTempBox("ACTUELLE", current, "#F87171"));
    
    // Vertical separator
    QFrame* sep = new QFrame();
    sep->setFixedWidth(2);
    sep->setStyleSheet("background-color: #334155;");
    dataLayout->addWidget(sep);
    
    dataLayout->addLayout(createTempBox("SEUIL", threshold, "#38BDF8"));

    layout->addWidget(dataBox);

    // Buttons
    layout->addStretch();
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    QPushButton* checkBtn = new QPushButton("Vérifier Frigo");
    checkBtn->setFixedHeight(50);
    checkBtn->setCursor(Qt::PointingHandCursor);
    checkBtn->setStyleSheet(R"(
        QPushButton {
            background-color: #EF4444; color: white; border: none; border-radius: 12px; font-weight: bold; font-size: 11pt;
        }
        QPushButton:hover { background-color: #DC2626; }
    )");
    connect(checkBtn, &QPushButton::clicked, this, [this]() {
        emit requestNavigation();
        close();
    });

    QPushButton* dismissBtn = new QPushButton("Ignorer");
    dismissBtn->setFixedHeight(50);
    dismissBtn->setCursor(Qt::PointingHandCursor);
    dismissBtn->setStyleSheet(R"(
        QPushButton {
            background-color: transparent; color: #64748B; border: 1.5px solid #334155; border-radius: 12px; font-weight: bold; font-size: 11pt;
        }
        QPushButton:hover { background-color: #334155; color: white; }
    )");
    connect(dismissBtn, &QPushButton::clicked, this, &QWidget::close);

    btnLayout->addWidget(dismissBtn, 1);
    btnLayout->addWidget(checkBtn, 2);
    layout->addLayout(btnLayout);

    // Center on screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}
