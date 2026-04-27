#include "temperaturealert.h"
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPropertyAnimation>
#include <QGuiApplication>
#include <QScreen>
#include <QCloseEvent>

TemperatureAlert::TemperatureAlert(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
    
    setWindowOpacity(0);
    QPropertyAnimation* anim = new QPropertyAnimation(this, "windowOpacity");
    anim->setDuration(300);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

TemperatureAlert::~TemperatureAlert() {}

void TemperatureAlert::setupUi()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowModality(Qt::NonModal);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);
    setFixedWidth(550);

    QFrame* card = new QFrame(this);
    card->setObjectName("MainCard");
    card->setStyleSheet(R"(
        #MainCard {
            background-color: #1E293B;
            border-radius: 28px;
            border: 2px solid #334155;
        }
    )");

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect();
    shadow->setBlurRadius(30);
    shadow->setXOffset(0);
    shadow->setYOffset(10);
    shadow->setColor(QColor(0, 0, 0, 160));
    card->setGraphicsEffect(shadow);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(card);

    QVBoxLayout* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(35, 35, 35, 35);
    cardLayout->setSpacing(20);

    QLabel* iconLabel = new QLabel("⚠️");
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("font-size: 48px;");
    cardLayout->addWidget(iconLabel);

    QLabel* titleLabel = new QLabel("ALERTE TEMPÉRATURE");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setFont(QFont("Segoe UI", 18, QFont::Bold));
    titleLabel->setStyleSheet("color: #F87171; letter-spacing: 2px;");
    cardLayout->addWidget(titleLabel);

    messageLabel = new QLabel("🚨 <b>DANGER IMMÉDIAT</b> 🚨");
    messageLabel->setAlignment(Qt::AlignCenter);
    messageLabel->setStyleSheet("color: #FDA4AF; font-size: 13pt;");
    cardLayout->addWidget(messageLabel);

    contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(15);
    cardLayout->addLayout(contentLayout);

    cardLayout->addStretch();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(15);

    QPushButton* checkBtn = new QPushButton("Vérifier Frigos");
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
    cardLayout->addLayout(btnLayout);

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move((screenGeometry.width() - width()) / 2, (screenGeometry.height() - height()) / 2);
}

void TemperatureAlert::addOrUpdateFridge(const QString& ref, double threshold, double current)
{
    if (fridgeRows.contains(ref)) {
        fridgeRows[ref].currentLabel->setText(QString::number(current, 'f', 1) + " °C");
        fridgeRows[ref].thresholdLabel->setText(QString::number(threshold, 'f', 1) + " °C");
        return;
    }

    QWidget* row = createFridgeRow(ref, threshold, current);
    contentLayout->addWidget(row);
    adjustSize();
    
    QScreen *screen = QGuiApplication::primaryScreen();
    move((screen->geometry().width() - width()) / 2, (screen->geometry().height() - height()) / 2);
}

void TemperatureAlert::removeFridge(const QString& ref)
{
    if (fridgeRows.contains(ref)) {
        QWidget* container = fridgeRows[ref].container;
        contentLayout->removeWidget(container);
        container->deleteLater();
        fridgeRows.remove(ref);
        
        if (fridgeRows.isEmpty()) {
            close();
        } else {
            adjustSize();
            QScreen *screen = QGuiApplication::primaryScreen();
            move((screen->geometry().width() - width()) / 2, (screen->geometry().height() - height()) / 2);
        }
    }
}

void TemperatureAlert::closeEvent(QCloseEvent *event)
{
    if (!fridgeRows.isEmpty()) {
        emit fridgesDismissed(fridgeRows.keys());
    }
    event->accept();
}

QWidget* TemperatureAlert::createFridgeRow(const QString& ref, double threshold, double current)
{
    QFrame* row = new QFrame();
    row->setStyleSheet("background-color: #0F172A; border-radius: 15px; border: 1px solid #334155;");
    
    QVBoxLayout* rowLayout = new QVBoxLayout(row);
    rowLayout->setContentsMargins(15, 10, 15, 10);
    rowLayout->setSpacing(5);

    QLabel* refLabel = new QLabel("FRIGO: " + ref);
    refLabel->setStyleSheet("color: #38BDF8; font-weight: bold; font-size: 10pt; border: none;");
    rowLayout->addWidget(refLabel);

    QHBoxLayout* dataLayout = new QHBoxLayout();
    
    auto createValBox = [&](const QString& label, double val, const QString& color, QLabel** outLabel) {
        QVBoxLayout* v = new QVBoxLayout();
        QLabel* l = new QLabel(label);
        l->setStyleSheet("color: #64748B; font-size: 8pt; font-weight: bold; border: none;");
        l->setAlignment(Qt::AlignCenter);
        *outLabel = new QLabel(QString::number(val, 'f', 1) + " °C");
        (*outLabel)->setStyleSheet(QString("color: %1; font-size: 16pt; font-weight: bold; border: none;").arg(color));
        (*outLabel)->setAlignment(Qt::AlignCenter);
        v->addWidget(l);
        v->addWidget(*outLabel);
        return v;
    };

    QLabel *curLab, *threshLab;
    dataLayout->addLayout(createValBox("ACTUELLE", current, "#F87171", &curLab));
    
    QFrame* sep = new QFrame();
    sep->setFixedWidth(1);
    sep->setStyleSheet("background-color: #334155;");
    dataLayout->addWidget(sep);
    
    dataLayout->addLayout(createValBox("SEUIL", threshold, "#38BDF8", &threshLab));
    
    rowLayout->addLayout(dataLayout);

    FridgeWidgets fw;
    fw.container = row;
    fw.currentLabel = curLab;
    fw.thresholdLabel = threshLab;
    fridgeRows[ref] = fw;

    return row;
}
