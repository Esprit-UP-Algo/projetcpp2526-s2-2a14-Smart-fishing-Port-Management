#ifndef FRIGOWINDOW_H
#define FRIGOWINDOW_H

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

struct Frigo {
    QString id;
    QString capacite;
    QString humidite;
    QString temperature;
    QString statut;  // Disponible / Occupé / Maintenance
    QString poisson;
};

class FrigoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FrigoWindow(QWidget *parent = nullptr);
    ~FrigoWindow();

private slots:
    void onSearch(const QString& text);
    void onAddFrigo();
    void onEditFrigo(int row);
    void onDeleteFrigo(int row);
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
    QWidget* createStatusBadge(const QString& status);
    QWidget* createActionButtons(int row);
    QString generateFrigoId();

    QVector<Frigo> frigos;
    QTableWidget* table;
    QLineEdit* searchInput;
};

#endif // FRIGOWINDOW_H
