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
#include <QComboBox>
#include <QScrollArea>
#include <QSizePolicy>

struct Bateau {
    QString idBateau;
    QString nomBateau;
    QString immatriculation;
    QString capacitePeche;
    QString longueur;
    QString proprietaire;
    QString etatBateau;
    QString dateDerniereMaintenance;
    QString disponible;
    QString ageBateau;
};

class BateauWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit BateauWindow(QWidget *parent = nullptr);
    ~BateauWindow();

private slots:
    void onSearch(const QString& text);
    void onSort(int index);
    void onAddBateau();
    void onEditBateau(int row);
    void onDeleteBateau(int row);
    void onGeneratePDF();
    void onShowStatistics();
    void onLogout();

private:
    void     setupUi();
    QFrame*  createSidebar();
    QWidget* createContentArea();
    QFrame*  createHeader();
    QFrame*  createToolbar();       // ← new: search + sort bar
    QFrame*  createTableCard();
    QPushButton* createNavButton(const QString& icon, const QString& text,
                                 bool isActive = false, bool isLogout = false);
    void     setupTable();
    void     populateTable(const QString& filterText = "");
    QWidget* createStatusBadge(const QString& etat);
    QWidget* createDisponibleBadge(const QString& disponible);
    QWidget* createActionButtons(int row);
    QString  generateBateauId();

    QVector<Bateau> bateaux;
    QTableWidget*   table      = nullptr;
    QLineEdit*      searchInput= nullptr;
    QComboBox*      sortCombo  = nullptr;
};

#endif // BATEAUWINDOW_H
