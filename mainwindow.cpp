#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QProcess>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    QProcess qprocess;
    QString command;
    QStringList args;

    command = "echo";
    //args<<"-X"<<"hunsrus@192.168.0.186"<<"/home/hunsrus/build-ssh-gui-Qt5-Debug/ssh-gui";
    args<<"aeea"<<"|"<<"ssh"<<"-tt"<<"hunsrus@192.168.0.186";

    qprocess.start(command,args,QIODevice::ReadWrite);
    qprocess.waitForStarted();
    qprocess.write("aeea");
    qprocess.waitForFinished();

    QString stdout = qprocess.readAllStandardOutput();
    std::cout << stdout.toStdString() << std::endl;

    QString stderror = qprocess.readAllStandardError();
    std::cout << stderror.toStdString() << std::endl;

    system("echo aeea | ssh -tt hunsrus@192.168.0.186");
}

