#ifndef QUAISWINDOW_H
#define QUAISWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QList>
#include <QComboBox>
#include <QFrame>
#include "quai.h"

class QuaisWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QuaisWindow(QWidget *parent = nullptr);


private:
    // UI
    QTableWidget* quaiTable;
    QLineEdit* searchInput;
    QComboBox* sortCombo;

    // Data
    QList<Quai> quais;

    // Setup functions
    void setupUI();
    QFrame* createSidebar();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createToolbar();
    QFrame* createTableCard();
    void setupQuaiTable();
    void populateTable(const QString& filterText = "");

    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QPushButton* createNavButton(const QString& icon, const QString& text,
                                 bool isActive = false, bool isLogout = false);

private slots:
    void onSearch(const QString& text);
    void onAddQuai();
    void onEditQuai(int row);
    void onDeleteQuai(int row);
    void onUpdateQuai(int row);
    void onLogout();
    void onSort(int index);
    void afficherStatistiques();
    void onGenerateContract(int row);

};

#endif // QUAISWINDOW_H
