#ifndef BATEAUWINDOW_H
#define BATEAUWINDOW_H

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
#include <QMap>

struct Bateau {
    QString idBateau;
    QString nomBateau;
    QString immatriculation;
    QString capacitePeche;
    QString longueur;
    QString proprietaire;
    QString etatBateau;  // En mer / Au port / En maintenance
    QString dateDerniereMaintenance;
    QString disponible;  // Oui / Non
};

class BateauWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit BateauWindow(QWidget *parent = nullptr);
    ~BateauWindow();

private slots:
    void onSearch(const QString& text);
    void onAddBateau();
    void onEditBateau(int row);
    void onDeleteBateau(int row);
    void onLogout();

private:
    void setupUi();
    QFrame* createSidebar();
    QWidget* createContentArea();
    QFrame* createHeader();
    QFrame* createTableCard();
    QPushButton* createNavButton(const QString& icon, const QString& text, bool isActive = false, bool isLogout = false);
    void setupTable();
    void populateTable(const QString& filterText = "");
    QWidget* createStatusBadge(const QString& etat);
    QWidget* createDisponibleBadge(const QString& disponible);
    QWidget* createActionButtons(int row);
    QWidget* createDeleteButton(int row);
    QString generateBateauId();

    QVector<Bateau> bateaux;
    QTableWidget* table;
    QLineEdit* searchInput;
};

#endif // BATEAUWINDOW_H
