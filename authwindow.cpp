#include "authwindow.h"
#include "ui_authwindow.h"

AuthWindow::AuthWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
    ui->lePass->setEchoMode(QLineEdit::Password);
}

AuthWindow::~AuthWindow()
{
    delete ui;
}

void AuthWindow::on_pbConnect_clicked()
{
    accept();
}


void AuthWindow::on_pbClose_clicked()
{
    close();
}
