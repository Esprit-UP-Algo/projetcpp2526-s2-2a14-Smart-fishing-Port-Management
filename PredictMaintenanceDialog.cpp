#include "PredictMaintenanceDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFrame>
#include <QDebug>

PredictMaintenanceDialog::PredictMaintenanceDialog(int age, int moisMaintenance, const QString& etat, QWidget *parent)
    : QDialog(parent), m_age(age), m_moisMaintenance(moisMaintenance), m_etat(etat)
{
    setWindowTitle("Prédiction de Maintenance");
    setFixedSize(450, 420);
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

    QLabel* lblFreqTitle = new QLabel("Fréquence des sorties\n(ex: mensuelles):");
    lblFreqTitle->setFont(labelFont);
    spinFrequence = new QSpinBox();
    spinFrequence->setRange(0, 1000);
    spinFrequence->setValue(1); // Default value
    spinFrequence->setFont(valueFont);
    spinFrequence->setStyleSheet("QSpinBox { border: 1px solid #cbd5e1; border-radius: 5px; padding: 5px; background: white; }");
    formLayout->addRow(lblFreqTitle, spinFrequence);

    mainLayout->addWidget(formFrame);

    QPushButton* calcBtn = new QPushButton("Calculer la prédiction");
    calcBtn->setFont(QFont("Segoe UI", 11, QFont::Bold));
    calcBtn->setCursor(Qt::PointingHandCursor);
    calcBtn->setStyleSheet("QPushButton { background-color: #2563eb; color: white; border-radius: 8px; padding: 10px; }"
                           "QPushButton:hover { background-color: #1d4ed8; }");
    mainLayout->addWidget(calcBtn);

    resultLabel = new QLabel("Cliquez sur calculer pour obtenir la recommandation.");
    resultLabel->setWordWrap(true);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setFont(QFont("Segoe UI", 11));
    resultLabel->setStyleSheet("background-color: #f1f5f9; padding: 15px; border-radius: 8px; color: #475569;");
    mainLayout->addWidget(resultLabel);

    QPushButton* closeBtn = new QPushButton("Fermer");
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet("QPushButton { background-color: #e2e8f0; color: #334155; border-radius: 8px; padding: 8px; }"
                            "QPushButton:hover { background-color: #cbd5e1; }");
    mainLayout->addWidget(closeBtn, 0, Qt::AlignCenter);

    connect(calcBtn, &QPushButton::clicked, this, &PredictMaintenanceDialog::calculateScore);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);

    // Initial first calculation
    calculateScore();
}

PredictMaintenanceDialog::~PredictMaintenanceDialog() {}

void PredictMaintenanceDialog::calculateScore()
{
    double freq = spinFrequence->value();
    
    // Formule: score = (âge * 0.3) + (fréquence * 0.4) + (mois * 0.2) + (état * 0.1)
    double score = (m_age * 0.3) + (freq * 0.4) + (m_moisMaintenance * 0.2) + (m_etatVal * 0.1);
    
    QString decision;
    QString bgColor;
    QString icon;

    if (score < 5.0) {
        decision = "OK - Pas de maintenance nécessaire";
        bgColor = "#16a34a"; // Vert
        icon = "✅";
    } else if (score >= 5.0 && score <= 7.0) {
        decision = "Surveillance recommandée";
        bgColor = "#ea580c"; // Orange
        icon = "⚠️";
    } else {
        decision = "Maintenance urgente !";
        bgColor = "#dc2626"; // Rouge
        icon = "🚨";
    }

    QString text = QString("%1 Résultat\n\nScore calculé : %2\nDécision : %3")
                       .arg(icon)
                       .arg(score, 0, 'f', 2)
                       .arg(decision);

    resultLabel->setText(text);
    resultLabel->setStyleSheet(QString("background-color: %1; color: white; padding: 15px; border-radius: 8px; font-weight: bold; font-family: 'Segoe UI'; font-size: 14px;").arg(bgColor));
}
