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
#include <QTimer>
#include <QDateTime>
#include "livraison.h"

class LiveProgressIndicator : public QWidget {
    Q_OBJECT
public:
    explicit LiveProgressIndicator(QWidget* parent = nullptr);
    void setProgress(double progress); // 0.0 to 1.0
protected:
    void paintEvent(QPaintEvent* event) override;
private:
    double m_progress = 0.0;
    double m_animationOffset = 0.0;
    QTimer* m_animationTimer;
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
    void onTrackDelivery(int row);
    void onShowStatistics();
    void onExportAllPDF();
    void onLiveUpdate();

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

    Livraison livraisons;
    QTableWidget* table;
    QLineEdit* searchInput;
    QComboBox* sortCombo;
    QLabel* totalDeliveriesLabel;
    QLabel* efficiencyLabel;
    QTimer* liveUpdateTimer;
};

#endif // LIVRAISONWINDOW_H
