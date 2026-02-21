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

struct Livraison {
    QString id;
    QString date;
    QString adresse;
    QString statut; // En attente / En cours / Livré
    QString transport;
    QString prix;
};

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

private:
    void setupUi();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createToolbar();
    QFrame* createTableCard();
    QFrame* createStatsArea();
    void updateStats();
    void setupTable();
    void populateTable(const QString& filterText = "");
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QString generateLivraisonId();


    QVector<Livraison> livraisons;
    QTableWidget* table;
    QLineEdit* searchInput;
    QComboBox* sortCombo;
    QLabel* totalDeliveriesLabel;
    QLabel* efficiencyLabel;
};

#endif // LIVRAISONWINDOW_H
