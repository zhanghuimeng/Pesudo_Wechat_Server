#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // print to TestBrowser
    connect(&myServer, SIGNAL(signal_error_box(QString)), this, SLOT(slot_error_box(QString)));
    connect(&myServer, SIGNAL(signal_info_box(QString)), this, SLOT(slot_info_box(QString)));
    // clear
    connect(ui->clearButton, SIGNAL(clicked(bool)), this, SLOT(slot_clear_textBrowser()));
    myServer.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::slot_error_box(QString msg)
{
    ui->textBrowser->textCursor().insertText("Error: ");
    ui->textBrowser->textCursor().insertText(msg);
    ui->textBrowser->textCursor().insertText("\n");
    // QMessageBox::about(this, "Error", msg);
}

void MainWindow::slot_info_box(QString msg)
{
    ui->textBrowser->textCursor().insertText("Info: ");
    ui->textBrowser->textCursor().insertText(msg);
    ui->textBrowser->textCursor().insertText("\n");
    // QMessageBox::about(this, "Info", msg);
}

void MainWindow::slot_clear_textBrowser()
{
    ui->textBrowser->clear();
}
