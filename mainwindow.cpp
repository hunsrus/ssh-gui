#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <iostream>
#include "sessionhandler.h"



MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lePass->setEchoMode(QLineEdit::Password);
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_pushButton_clicked()
{
    SessionHandler* sh = new SessionHandler();
    std::string userName = ui->leUser->text().toStdString();
    std::string host = ui->leServer->text().toStdString();//"152.169.146.69"
    std::string pass = ui->lePass->text().toStdString();

    sh->start_session(userName,host,pass);

    sh->show_remote_processes();

    delete(sh);
}

