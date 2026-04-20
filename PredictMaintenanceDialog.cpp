#include "PredictMaintenanceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QDebug>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QDate>

PredictMaintenanceDialog::PredictMaintenanceDialog(int age, int moisMaintenance, int frequence, const QString& etat, QWidget *parent)
    : QDialog(parent), m_age(age), m_moisMaintenance(moisMaintenance), m_frequence(frequence), m_etat(etat)
{
    setWindowTitle("Prédiction de Maintenance");
    setFixedSize(450, 480);
    setStyleSheet("QDialog { background-color: white; }");

    QString etatLower = etat.trimmed().toLower();
    if (etatLower == "en mer") m_etatVal = 3;
    else if (etatLower == "au port") m_etatVal = 1;
    else if (etatLower == "en maintenance") m_etatVal = 0;
    else m_etatVal = 1; // Default fallback

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    QLabel* titleLabel = new QLabel("🔮 Système de Prédiction");
    titleLabel->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleLabel->setStyleSheet("color: #1e3a8a;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    QFrame* formFrame = new QFrame();
    formFrame->setStyleSheet("QFrame { background-color: #f8fafc; border-radius: 10px; border: 1px solid #e2e8f0; }");
    QFormLayout* formLayout = new QFormLayout(formFrame);
    formLayout->setContentsMargins(15, 15, 15, 15);
    formLayout->setSpacing(15);

    QFont labelFont("Segoe UI", 10, QFont::Bold);
    labelFont.setBold(true);
    QFont valueFont("Segoe UI", 10);

    QLabel* lblAgeTitle = new QLabel("Âge du bateau:");
    lblAgeTitle->setFont(labelFont);
    QLabel* lblAge = new QLabel(QString::number(m_age) + " ans");
    lblAge->setFont(valueFont);
    formLayout->addRow(lblAgeTitle, lblAge);

    QLabel* lblMoisTitle = new QLabel("Mois depuis maintenance:");
    lblMoisTitle->setFont(labelFont);
    QLabel* lblMois = new QLabel(QString::number(m_moisMaintenance) + " mois");
    lblMois->setFont(valueFont);
    formLayout->addRow(lblMoisTitle, lblMois);

    QLabel* lblEtatTitle = new QLabel("État actuel:");
    lblEtatTitle->setFont(labelFont);
    QLabel* lblEtat = new QLabel(m_etat + QString(" (Valeur: %1)").arg(m_etatVal));
    lblEtat->setFont(valueFont);
    formLayout->addRow(lblEtatTitle, lblEtat);

    QLabel* lblFreqTitle = new QLabel("Fréquence des sorties\n(Automatique):");
    lblFreqTitle->setFont(labelFont);
    QLabel* lblFreq = new QLabel(QString::number(m_frequence) + " sorties (Total)");
    lblFreq->setFont(valueFont);
    formLayout->addRow(lblFreqTitle, lblFreq);

    mainLayout->addWidget(formFrame);

    resultLabel = new QLabel("");
    resultLabel->setWordWrap(true);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setFont(QFont("Segoe UI", 11));
    mainLayout->addWidget(resultLabel);

    // Initial calculation to populate resultLabel
    calculateScore();

    QHBoxLayout* btnLayout = new QHBoxLayout();
    
    btnAccept = new QPushButton("Ok (Valider)");
    btnAccept->setFont(QFont("Segoe UI", 10, QFont::Bold));
    btnAccept->setCursor(Qt::PointingHandCursor);
    btnAccept->setStyleSheet("QPushButton { background-color: #2563eb; color: white; border-radius: 8px; padding: 10px; }"
                             "QPushButton:hover { background-color: #1d4ed8; }");
                             
    btnCustom = new QPushButton("Annuler (Choisir date)");
    btnCustom->setFont(QFont("Segoe UI", 10, QFont::Bold));
    btnCustom->setCursor(Qt::PointingHandCursor);
    btnCustom->setStyleSheet("QPushButton { background-color: #f1f5f9; color: #334155; border: 1px solid #cbd5e1; border-radius: 8px; padding: 10px; }"
                             "QPushButton:hover { background-color: #e2e8f0; }");

    btnLayout->addWidget(btnAccept);
    btnLayout->addWidget(btnCustom);
    mainLayout->addLayout(btnLayout);

    customDateWidget = new QWidget();
    QHBoxLayout* customLayout = new QHBoxLayout(customDateWidget);
    customLayout->setContentsMargins(0, 0, 0, 0);
    
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setFont(QFont("Segoe UI", 10));
    dateEdit->setStyleSheet("QDateEdit { border: 1px solid #cbd5e1; border-radius: 6px; padding: 5px; }");
    
    btnConfirmCustom = new QPushButton("Valider");
    btnConfirmCustom->setCursor(Qt::PointingHandCursor);
    btnConfirmCustom->setStyleSheet("QPushButton { background-color: #059669; color: white; border-radius: 6px; padding: 5px 15px; }");
    
    customLayout->addWidget(new QLabel("Nouvelle date :"));
    customLayout->addWidget(dateEdit);
    customLayout->addWidget(btnConfirmCustom);
    
    customDateWidget->setVisible(false);
    mainLayout->addWidget(customDateWidget);

    connect(btnAccept, &QPushButton::clicked, this, &PredictMaintenanceDialog::onAcceptBtn);
    connect(btnCustom, &QPushButton::clicked, this, &PredictMaintenanceDialog::onCustomBtn);
    connect(btnConfirmCustom, &QPushButton::clicked, this, &PredictMaintenanceDialog::onConfirmCustomBtn);
}

PredictMaintenanceDialog::~PredictMaintenanceDialog() {}

void PredictMaintenanceDialog::calculateScore()
{
    double freq = m_frequence;
    
    // Formule: score = (âge * 0.3) + (fréquence * 0.4) + (mois * 0.2) + (état * 0.1)
    double score = (m_age * 0.3) + (freq * 0.4) + (m_moisMaintenance * 0.2) + (m_etatVal * 0.1);
    
    QString decision;
    QString bgColor;
    QString icon;

    int daysToAdd = 0;
    if (score < 5.0) {
        decision = "OK - Pas de maintenance nécessaire";
        bgColor = "#16a34a"; // Vert
        icon = "✅";
        daysToAdd = 180; // 6 mois
    } else if (score >= 5.0 && score <= 7.0) {
        decision = "Surveillance recommandée";
        bgColor = "#ea580c"; // Orange
        icon = "⚠️";
        daysToAdd = 30; // 1 mois
    } else {
        decision = "Maintenance urgente !";
        bgColor = "#dc2626"; // Rouge
        icon = "🚨";
        daysToAdd = 0; // Immédiat
    }

    m_systemPredictedDate = QDate::currentDate().addDays(daysToAdd);
    m_finalDate = m_systemPredictedDate;

    QString text = QString("%1 Résultat de l'analyse\n\nDécision : %2\nProchaine maintenance estimée : %3")
                       .arg(icon)
                       .arg(decision)
                       .arg(m_systemPredictedDate.toString("dd/MM/yyyy"));

    resultLabel->setText(text);
    resultLabel->setStyleSheet(QString("background-color: %1; color: white; padding: 15px; border-radius: 8px; font-weight: bold; font-family: 'Segoe UI'; font-size: 14px;").arg(bgColor));
}

void PredictMaintenanceDialog::onAcceptBtn()
{
    m_finalDate = m_systemPredictedDate;
    accept();
}

void PredictMaintenanceDialog::onCustomBtn()
{
    customDateWidget->setVisible(true);
}

void PredictMaintenanceDialog::onConfirmCustomBtn()
{
    m_finalDate = dateEdit->date();
    accept();
}

