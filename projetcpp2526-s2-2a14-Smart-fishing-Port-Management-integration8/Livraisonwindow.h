#ifndef LIVRAISONWINDOW_H
#define LIVRAISONWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QLabel>
#include <QVector>
#include <QComboBox>
#include "livraison.h"

class LivraisonWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LivraisonWindow(QWidget *parent = nullptr);
    ~LivraisonWindow();

private slots:
    void onSearch(const QString& text);
    void onAddLivraison();
    void onEditLivraison(int row);
    void onDeleteLivraison(int row);
    void onSort(int index);
    void onExportPDF(int row);
    void onShowStatistics();
    void onExportAllPDF();

private:
    void setupUi();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createToolbar();
    QFrame* createTableCard();
    QFrame* createStatsArea();
    void updateStats();
    void setupTable();
    void populateTable(const QString& filterText = "", const QString& sortCritere = "", const QString& sortOrdre = "");
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    void loadStyleSheet();
    QString generateLivraisonId();


    Livraison livraisons;
    QTableWidget* table;
    QLineEdit* searchInput;
    QComboBox* sortCombo;
    QLabel* totalDeliveriesLabel;
    QLabel* efficiencyLabel;
};

#endif // LIVRAISONWINDOW_H
