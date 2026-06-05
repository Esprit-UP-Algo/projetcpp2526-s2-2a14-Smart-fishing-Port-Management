#include "PecheExportDialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QMouseEvent>
#include <QAbstractItemView>
#include <QLineEdit>
#include <QApplication>

// ==================== HELPERS POUR DIALOGUE STYLÉ ====================
class ComboClickFilter : public QObject {
public:
    ComboClickFilter(QObject* parent = nullptr) : QObject(parent) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease) {
            QLineEdit* le = qobject_cast<QLineEdit*>(watched);
            if (le && le->parentWidget()) {
                QComboBox* cb = qobject_cast<QComboBox*>(le->parentWidget());
                if (cb && event->type() == QEvent::MouseButtonPress) {
                    cb->showPopup();
                    return true;
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

class DialogMoveFilter : public QObject {
public:
    DialogMoveFilter(QDialog* dialog, QObject* parent = nullptr) : QObject(parent), m_dialog(dialog), m_dragging(false) {}
protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (!m_dialog) return QObject::eventFilter(watched, event);
        switch (event->type()) {
            case QEvent::MouseButtonPress: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) {
                    m_dragging = true;
                    m_dragOffset = me->globalPosition().toPoint() - m_dialog->frameGeometry().topLeft();
                    return true;
                }
                break;
            }
            case QEvent::MouseMove: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                    m_dialog->move(me->globalPosition().toPoint() - m_dragOffset);
                    return true;
                }
                break;
            }
            case QEvent::MouseButtonRelease: {
                auto* me = static_cast<QMouseEvent*>(event);
                if (me->button() == Qt::LeftButton) { m_dragging = false; return true; }
                break;
            }
            default: break;
        }
        return QObject::eventFilter(watched, event);
    }
private:
    QDialog* m_dialog;
    bool m_dragging;
    QPoint m_dragOffset;
};

static void makeDialogMovable(QDialog* dialog, QWidget* dragHandle) {
    if (!dialog || !dragHandle) return;
    dragHandle->setCursor(Qt::OpenHandCursor);
    dragHandle->installEventFilter(new DialogMoveFilter(dialog, dragHandle));
}

PecheExportDialog::PecheExportDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Options d'Export PDF");
    setFixedSize(500, 550);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 8);
    shadow->setColor(QColor(0, 0, 0, 100));

    QWidget* container = new QWidget(this);
    container->setGeometry(15, 15, 470, 520);
    container->setGraphicsEffect(shadow);
    container->setStyleSheet("QWidget { background: white; border-radius: 24px; }");

    QVBoxLayout* mainLay = new QVBoxLayout(container);
    mainLay->setContentsMargins(0, 0, 0, 0);
    mainLay->setSpacing(0);

    // Header Orange (Thème Pêche)
    QFrame* header = new QFrame();
    header->setFixedHeight(100);
    header->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #F97316, stop:1 #FB923C);
            border-top-left-radius: 24px;
            border-top-right-radius: 24px;
        }
    )");
    QHBoxLayout* headerLay = new QHBoxLayout(header);
    headerLay->setContentsMargins(25, 0, 20, 0);

    QLabel* titleL = new QLabel("📄 Export Rapport PDF");
    titleL->setFont(QFont("Segoe UI", 16, QFont::Bold));
    titleL->setStyleSheet("color: white; background: transparent;");
    headerLay->addWidget(titleL);

    QPushButton* closeBtn = new QPushButton("✕");
    closeBtn->setFixedSize(32, 32);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setStyleSheet(R"(
        QPushButton { background: rgba(255,255,255,0.2); color: white; border: none; border-radius: 16px; font-weight: bold; }
        QPushButton:hover { background: rgba(255,255,255,0.4); }
    )");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    headerLay->addWidget(closeBtn);
    mainLay->addWidget(header);
    makeDialogMovable(this, header);

    QWidget* content = new QWidget();
    QVBoxLayout* contentLay = new QVBoxLayout(content);
    contentLay->setContentsMargins(35, 25, 35, 25);
    contentLay->setSpacing(15);

    auto addLabel = [&](const QString& text) {
        QLabel* lbl = new QLabel(text);
        lbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
        lbl->setStyleSheet("color: #475569; margin-bottom: -5px;");
        contentLay->addWidget(lbl);
    };

    QString comboStyle = R"(
        QComboBox, QDateEdit {
            background: #F8FAFC; border: 2px solid #E2E8F0; border-radius: 10px; padding: 8px 12px; font-size: 11pt; color: #1E293B; outline: none;
        }
        QComboBox:focus, QDateEdit:focus { border: 2px solid #F97316; background: white; outline: none; }
        QComboBox::drop-down { border: none; width: 30px; outline: none; }
        QComboBox QAbstractItemView {
            background: white;
            color: #1e293b;
            border: 1px solid #e2e8f0;
            border-radius: 8px;
            selection-background-color: #EBF5FF;
            selection-color: #1e293b;
            outline: none;
            padding: 4px;
        }
        QComboBox QAbstractItemView::item {
            min-height: 32px;
            padding: 4px 10px;
            border-radius: 4px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: #F1F5F9;
        }
    )";

    addLabel("TYPE DE RAPPORT");
    typeCombo = new QComboBox();
    typeCombo->addItems({"Rapport Complet (Total)", "Rapport par Bateau", "Rapport par Date", "Rapport par Bateau & Date"});
    typeCombo->setStyleSheet(comboStyle);
    contentLay->addWidget(typeCombo);

    boatWidget = new QWidget();
    QVBoxLayout* boatLay = new QVBoxLayout(boatWidget);
    boatLay->setContentsMargins(0, 5, 0, 0);
    QLabel* bLbl = new QLabel("CHOISIR LE BATEAU");
    bLbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
    bLbl->setStyleSheet("color: #475569;");
    boatLay->addWidget(bLbl);
    boatCombo = new QComboBox();
    boatCombo->setStyleSheet(comboStyle);
    boatLay->addWidget(boatCombo);
    boatWidget->setVisible(false);
    contentLay->addWidget(boatWidget);

    dateWidget = new QWidget();
    QVBoxLayout* dateLayV = new QVBoxLayout(dateWidget);
    dateLayV->setContentsMargins(0, 5, 0, 0);
    QLabel* dLbl = new QLabel("PÉRIODE");
    dLbl->setFont(QFont("Segoe UI", 9, QFont::Bold));
    dLbl->setStyleSheet("color: #475569;");
    dateLayV->addWidget(dLbl);
    
    QHBoxLayout* dateLayH = new QHBoxLayout();
    startDateEdit = new QDateEdit(QDate::currentDate().addMonths(-1));
    startDateEdit->setCalendarPopup(true);
    startDateEdit->setStyleSheet(comboStyle);
    endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);
    endDateEdit->setStyleSheet(comboStyle);
    
    dateLayH->addWidget(new QLabel("De:"));
    dateLayH->addWidget(startDateEdit, 1);
    dateLayH->addWidget(new QLabel("À:"));
    dateLayH->addWidget(endDateEdit, 1);
    dateLayV->addLayout(dateLayH);
    dateWidget->setVisible(false);
    contentLay->addWidget(dateWidget);

    contentLay->addStretch();

    QHBoxLayout* btnLay = new QHBoxLayout();
    btnLay->setSpacing(15);
    QPushButton* cancelBtn = new QPushButton("Annuler");
    cancelBtn->setFixedHeight(45);
    cancelBtn->setCursor(Qt::PointingHandCursor);
    cancelBtn->setStyleSheet("QPushButton { background: #F1F5F9; color: #475569; border-radius: 12px; font-weight: 600; } QPushButton:hover { background: #E2E8F0; }");
    
    QPushButton* okBtn = new QPushButton("💾 Générer PDF");
    okBtn->setFixedHeight(45);
    okBtn->setCursor(Qt::PointingHandCursor);
    okBtn->setStyleSheet("QPushButton { background: #F97316; color: white; border-radius: 12px; font-weight: bold; } QPushButton:hover { background: #EA580C; }");
    
    btnLay->addWidget(cancelBtn, 1);
    btnLay->addWidget(okBtn, 2);
    contentLay->addLayout(btnLay);

    mainLay->addWidget(content);

    connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &PecheExportDialog::onTypeChanged);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    loadBoats();

    // Fix black popup background on combo dropdowns
    ComboClickFilter* comboFilter = new ComboClickFilter(this);
    for (QComboBox* combo : findChildren<QComboBox*>()) {
        if (!combo->isEditable()) {
            combo->setEditable(true);
            combo->lineEdit()->setReadOnly(true);
            combo->lineEdit()->setCursor(Qt::PointingHandCursor);
            combo->lineEdit()->installEventFilter(comboFilter);
        }
        if (combo->view() && combo->view()->window()) {
            combo->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
            combo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
        }
    }

    // Apply dark mode
    if (qApp->property("isDarkMode").toBool()) {
        // Dark container background
        QWidget* cont = findChild<QWidget*>();
        if (cont) cont->setStyleSheet("QWidget { background: #1E293B; border-radius: 28px; }");

        // All labels dark-mode text
        for (QLabel* lbl : findChildren<QLabel*>()) {
            QString s = lbl->styleSheet();
            if (s.contains("#4B5563"))
                lbl->setStyleSheet(s.replace("#4B5563", "#94A3B8"));
            if (s.contains("#475569"))
                lbl->setStyleSheet(s.replace("#475569", "#CBD5E1"));
        }

        // Inputs: dark background + light text
        QString darkInput = R"(
            QLineEdit, QComboBox, QDateEdit, QDoubleSpinBox, QSpinBox, QTextEdit {
                background-color: #2D3748;
                border: 2px solid #4A5568;
                border-radius: 12px;
                padding: 10px 15px;
                color: #F1F5F9;
                font-size: 11pt;
                outline: none;
            }
            QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QDoubleSpinBox:focus, QSpinBox:focus, QTextEdit:focus {
                border: 2px solid #3B82F6;
                background-color: #374151;
                outline: none;
            }
            QComboBox QLineEdit, QDateEdit QLineEdit, QSpinBox QLineEdit, QDoubleSpinBox QLineEdit {
                padding: 0px;
                background: transparent;
                border: none;
                color: #F1F5F9;
            }
            QComboBox::drop-down { border: none; width: 30px; outline: none; }
            QComboBox QAbstractItemView {
                background: #2D3748;
                color: #F1F5F9;
                border: 1px solid #4A5568;
                border-radius: 8px;
                selection-background-color: #3B82F6;
                selection-color: white;
            }
        )";
        for (QWidget* w : findChildren<QWidget*>()) {
            if (w->inherits("QLineEdit") || w->inherits("QComboBox") || w->inherits("QDateEdit") ||
                w->inherits("QDoubleSpinBox") || w->inherits("QSpinBox") || w->inherits("QTextEdit")) {
                w->setStyleSheet(darkInput);
            }
        }
    }
}

void PecheExportDialog::loadBoats()
{
    QSqlQuery q("SELECT IDBATEAU, NOMBATEAU FROM BATEAUX");
    while(q.next()){
        QString id = q.value(0).toString();
        QString name = q.value(1).toString();
        boatCombo->addItem(name + " (" + id + ")", id);
    }
}

void PecheExportDialog::onTypeChanged(int index)
{
    boatWidget->setVisible(index == 1 || index == 3);
    dateWidget->setVisible(index == 2 || index == 3);
}

PecheExportDialog::ExportType PecheExportDialog::exportType() const 
{
    return static_cast<ExportType>(typeCombo->currentIndex());
}

QString PecheExportDialog::selectedBoat() const { return boatCombo->currentData().toString(); }
QDate PecheExportDialog::startDate() const { return startDateEdit->date(); }
QDate PecheExportDialog::endDate() const { return endDateEdit->date(); }
