#ifndef PECHEEXPORTDIALOG_H
#define PECHEEXPORTDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QDateEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>

class PecheExportDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PecheExportDialog(QWidget *parent = nullptr);

    enum ExportType { Total, ByBoat, ByDate };
    ExportType exportType() const;
    QString selectedBoat() const;
    QDate startDate() const;
    QDate endDate() const;

private slots:
    void onTypeChanged(int index);

private:
    QComboBox* typeCombo;
    QComboBox* boatCombo;
    QDateEdit* startDateEdit;
    QDateEdit* endDateEdit;
    QWidget* boatWidget;
    QWidget* dateWidget;

    void loadBoats();
};

#endif // PECHEEXPORTDIALOG_H
