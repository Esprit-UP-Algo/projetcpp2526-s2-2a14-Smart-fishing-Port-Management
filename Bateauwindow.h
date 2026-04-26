#ifndef BATEAUWINDOW_H
#define BATEAUWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFrame>
#include <QList>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMap>
#include "bateau.h"

class BateauWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BateauWindow(QWidget *parent = nullptr);
    ~BateauWindow();
    static void refreshAllTables();
private slots:
    void onSearch(const QString& text);
    void onSort(int index);
    void onAddBateau();
    void onEditBateau(int row);
    void onDeleteBateau(int row);
    void onPredictMaintenanceGlobal();
    void onGeneratePDF();
    void onShowStatistics();
    void onLogout();

private:
    void     setupUi();
    QFrame*  createSidebar();
    QWidget* createContentArea();
    QFrame*  createHeader();
    QFrame*  createToolbar();
    QFrame*  createTableCard();
    QPushButton* createNavButton(const QString& icon, const QString& text,
                                 bool isActive = false, bool isLogout = false);
    void     setupTable();
    void     populateTable(const QString& filterText = "");
    QWidget* createDisponibleBadge(const QString& s);
    QWidget* createActionButtons(int row);
    QString  generateBateauId();

    QTableWidget*   table      = nullptr;
    QLineEdit*      searchInput= nullptr;
    QComboBox*      sortCombo  = nullptr;
     static QList<BateauWindow*> s_instances;
};

#endif // BATEAUWINDOW_H
