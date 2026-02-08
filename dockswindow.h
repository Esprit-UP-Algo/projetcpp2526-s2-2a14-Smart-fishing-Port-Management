#ifndef DOCKSWINDOW_H
#define DOCKSWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

class DocksWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit DocksWindow(QWidget *parent = nullptr);

private:
    QTableWidget* dockTable;

    void setupUI();
    void setupDockTable();
    void populateTable();
};

#endif // DOCKSWINDOW_H
