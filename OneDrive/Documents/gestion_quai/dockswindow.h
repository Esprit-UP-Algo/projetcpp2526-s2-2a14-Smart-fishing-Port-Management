#ifndef DOCKSWINDOW_H
#define DOCKSWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>

class docksWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit docksWindow(QWidget *parent = nullptr);

private:
    QTableWidget* table;
};

#endif // DOCKSWINDOW_H
