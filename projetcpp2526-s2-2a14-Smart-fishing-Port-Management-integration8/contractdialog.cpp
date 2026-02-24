#include "contractdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

ContractDialog::ContractDialog(const Quai& quai, QWidget* parent)
    : QDialog(parent), m_quai(quai) {

    setFixedSize(560, 620);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(40);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 80));

    QWidget* container = new QWidget(this);
    container->setGeometry(10, 10, 540, 600);  // inset for shadow
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 20px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // --- Header ---
    QFrame* header = new QFrame();
    header->setFixedHeight(75);
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #059669, stop:1 #34D399);
            border-radius: 20px 20px 0 0;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(25, 0, 18, 0);

    QLabel* titleLbl = new QLabel("📄  Nouveau contrat de location");
    titleLbl->setFont(QFont("Segoe UI", 14, QFont::Bold));
    titleLbl->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleLbl, 1);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none;
                      border-radius: 16px; font-size: 13px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(header);

    // --- Quai preview card ---
    QFrame* previewCard = new QFrame();
    previewCard->setFixedHeight(72);
    previewCard->setStyleSheet(R"(
        QFrame {
            background: #F0FDF4;
            border: 2px solid #6EE7B7;
            border-radius: 12px;
            margin: 16px 20px 0 20px;
        }
    )");
    QHBoxLayout* previewLay = new QHBoxLayout(previewCard);
    previewLay->setContentsMargins(14, 8, 14, 8);

    QLabel* iconLbl = new QLabel("⚓");
    iconLbl->setFont(QFont("Segoe UI", 22));
    iconLbl->setStyleSheet("color: #059669; background: transparent; border: none;");
    previewLay->addWidget(iconLbl);

    QVBoxLayout* infoLay = new QVBoxLayout();
    infoLay->setSpacing(2);
    QLabel* titlePreview = new QLabel(quai.getNom() + "  —  " + quai.getId());
    titlePreview->setFont(QFont("Segoe UI", 11, QFont::Bold));
    titlePreview->setStyleSheet("color: #065F46; background: transparent; border: none;");
    QLabel* detailsPreview = new QLabel(
        QString("Capacité: %1  |  Taille max: %2 m  |  Tarif: %3 DT/j")
            .arg(quai.getCapacite()).arg(quai.getTailleMax()).arg(quai.getTarif()));
    detailsPreview->setFont(QFont("Segoe UI", 9));
    detailsPreview->setStyleSheet("color: #6b7280; background: transparent; border: none;");
    infoLay->addWidget(titlePreview);
    infoLay->addWidget(detailsPreview);
    previewLay->addLayout(infoLay, 1);
    mainLay->addWidget(previewCard);

    // --- Form area ---
    QWidget* formArea = new QWidget();
    formArea->setStyleSheet("background: transparent;");
    QVBoxLayout* formLay = new QVBoxLayout(formArea);
    formLay->setContentsMargins(20, 14, 20, 8);
    formLay->setSpacing(6);

    auto addField = [&](const QString& labelText, QLineEdit*& fieldPtr,
                        const QString& placeholder) {
        QLabel* lbl = new QLabel(labelText);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
        lbl->setStyleSheet("color: #374151; background: transparent;");
        formLay->addWidget(lbl);

        fieldPtr = new QLineEdit();
        fieldPtr->setPlaceholderText(placeholder);
        fieldPtr->setFixedHeight(40);
        fieldPtr->setStyleSheet(R"(
            QLineEdit {
                background: #F9FAFB; border: 2px solid #E5E7EB;
                border-radius: 10px; padding: 4px 12px;
                color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
            }
            QLineEdit:focus { border: 2px solid #059669; background: white; }
        )");
        formLay->addWidget(fieldPtr);
    };

    addField("Nom complet du client *", clientNameField, "ex: Jean Dupont");
    addField("Société / Organisation *", companyField, "ex: Sea Harvest Ltd");
    addField("Email", emailField, "ex: contact@entreprise.com");
    addField("Téléphone", phoneField, "ex: +216 XX XXX XXX");

    // Date + Duration on same row
    QHBoxLayout* dateRow = new QHBoxLayout();
    dateRow->setSpacing(12);

    QVBoxLayout* dateCol = new QVBoxLayout();
    dateCol->setSpacing(4);
    QLabel* dateLbl = new QLabel("Date de début *");
    dateLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    dateLbl->setStyleSheet("color: #374151; background: transparent;");
    startDateField = new QDateEdit(QDate::currentDate());
    startDateField->setCalendarPopup(true);
    startDateField->setFixedHeight(40);
    startDateField->setStyleSheet(R"(
        QDateEdit {
            background: #F9FAFB; border: 2px solid #E5E7EB;
            border-radius: 10px; padding: 4px 12px;
            color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
        }
        QDateEdit:focus { border: 2px solid #059669; background: white; }
    )");
    dateCol->addWidget(dateLbl);
    dateCol->addWidget(startDateField);
    dateRow->addLayout(dateCol);

    QVBoxLayout* durCol = new QVBoxLayout();
    durCol->setSpacing(4);
    QLabel* durLbl = new QLabel("Durée *");
    durLbl->setFont(QFont("Segoe UI", 9, QFont::Medium));
    durLbl->setStyleSheet("color: #374151; background: transparent;");
    durationField = new QComboBox();
    durationField->addItems({"1 mois", "3 mois", "6 mois", "1 an", "2 ans"});
    durationField->setFixedHeight(40);
    durationField->setStyleSheet(R"(
        QComboBox {
            background: #F9FAFB; border: 2px solid #E5E7EB;
            border-radius: 10px; padding: 4px 12px;
            color: #1f2937; font-family: 'Segoe UI'; font-size: 10pt;
        }
        QComboBox:focus { border: 2px solid #059669; background: white; }
        QComboBox::drop-down { border: none; width: 28px; }
        QComboBox QAbstractItemView {
            background: white; border: 1px solid #e2e8f0;
            border-radius: 10px; selection-background-color: #D1FAE5;
            selection-color: #065F46; padding: 6px;
        }
    )");
    durCol->addWidget(durLbl);
    durCol->addWidget(durationField);
    dateRow->addLayout(durCol);

    formLay->addLayout(dateRow);
    mainLay->addWidget(formArea, 1);

    // --- Buttons ---
    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setContentsMargins(20, 8, 20, 20);
    btnLay->setSpacing(12);

    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(42);
    cancelBtn->setMinimumWidth(110);
    cancelBtn->setFont(QFont("Segoe UI", 10, QFont::Medium));
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet(R"(
        QPushButton { background: #F3F4F6; color: #374151; border: none; border-radius: 10px; padding: 0 18px; }
        QPushButton:hover { background: #E5E7EB; }
    )");
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton* generateBtn = new QPushButton("📄  Générer le contrat");
    generateBtn->setFixedHeight(42);
    generateBtn->setFont(QFont("Segoe UI", 10, QFont::Bold));
    generateBtn->setCursor(Qt::PointingHandCursor);
    generateBtn->setStyleSheet(R"(
        QPushButton { background: qlineargradient(x1:0,y1:0,x2:1,y2:0,
                          stop:0 #059669, stop:1 #34D399);
                      color: white; border: none; border-radius: 10px; padding: 0 20px; }
        QPushButton:hover { background: #047857; }
    )");
    connect(generateBtn, &QPushButton::clicked, this, &QDialog::accept);

    btnLay->addStretch();
    btnLay->addWidget(cancelBtn);
    btnLay->addWidget(generateBtn);
    mainLay->addLayout(btnLay);
}

QString ContractDialog::getClientName() const { return clientNameField->text(); }
QString ContractDialog::getCompany()    const { return companyField->text(); }
QString ContractDialog::getDuration()   const { return durationField->currentText(); }
QDate   ContractDialog::getStartDate()  const { return startDateField->date(); }
