#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "serverthread.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slot_error_box(QString);
    void slot_info_box(QString);
    void slot_clear_textBrowser();

private:
    Ui::MainWindow *ui;
    ServerThread myServer;
};

#endif // MAINWINDOW_H
