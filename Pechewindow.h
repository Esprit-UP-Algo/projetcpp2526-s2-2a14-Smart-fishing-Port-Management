#ifndef PECHEWINDOW_H
#define PECHEWINDOW_H

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

struct Peche {
    QString idLot;
    QString reference;
    QString espece;
    QString quantiteKg;
    QString dateCapture;
};

class PecheWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit PecheWindow(QWidget *parent = nullptr);
    ~PecheWindow();

private slots:
    void onSearch(const QString& text);
    void onAddPeche();
    void onEditPeche(int row);
    void onDeletePeche(int row);
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
    QWidget* createEspeceBadge(const QString& espece);
    QWidget* createActionButtons(int row);
    QString generatePecheId();

    QVector<Peche> peches;
    QTableWidget* table;
    QLineEdit* searchInput;
};

#endif // PECHEWINDOW_H
