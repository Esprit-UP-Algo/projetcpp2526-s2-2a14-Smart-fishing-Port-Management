#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAddFrigo_clicked();
    void onDeleteRow();
    void onUpdateRow();

private:
    Ui::MainWindow *ui;

    // Add this declaration:
    void setupLogo();
    void setupTable();
    void setupShadows();
    void loadSampleData();
    void addFrigoData(const QString &id, const QString &cap,
                      const QString &hum, const QString &temp,
                      const QString &stat, const QString &fish);
};

#endif // MAINWINDOW_H
