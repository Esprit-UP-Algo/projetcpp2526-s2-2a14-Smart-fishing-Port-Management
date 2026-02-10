#ifndef DOCKSWINDOW_H
#define DOCKSWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QList>
#include "dock.h"

class DocksWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit DocksWindow(QWidget *parent = nullptr);

private:
    // UI
    QTableWidget* dockTable;
    QLineEdit* searchInput;

    // Data
    QList<Dock> docks;

    // Setup functions
    void setupUI();
    QFrame* createSidebar();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createTableCard();
    void setupDockTable();
    void populateTable(const QString& filterText = "");

    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QPushButton* createNavButton(const QString& icon, const QString& text,
                                 bool isActive = false, bool isLogout = false);

private slots:
    void onSearch(const QString& text);
    void onAddDock();
    void onEditDock(int row);
    void onDeleteDock(int row);
    void onUpdateDock(int row);
    void onLogout();
     void onSort(int index);
};

#endif // DOCKSWINDOW_H
