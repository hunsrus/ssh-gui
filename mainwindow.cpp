#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "authwindow.h"
#include "ui_authwindow.h"

#include <QProcess>
#include <iostream>
#include <QFileDialog>


MainWindow::MainWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pBarUpload->hide();
    ui->pBarDownload->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete sh;
}



void MainWindow::on_pbConnect_clicked()
{
    AuthWindow auth(this);
    this->sh = new SessionHandler();
    int ssh_ok;
    std::string path;

    if(auth.exec())
    {
        std::string userName = auth.ui->leUser->text().toStdString();
        std::string host = "152.169.146.69";
        std::string pass = auth.ui->lePass->text().toStdString();

        ssh_ok = sh->start_session(userName,host,pass);
        if(!ssh_ok)
        {
            sh->list_files();
            sh->move_path(userName.c_str());
            sh->list_files();
            refresh_list_widget();
            path = ui->labPath->text().toStdString();
            path.append(sh->getUserName());
            ui->labPath->setText(QString::fromStdString(path));
            ui->pbConnect->setText("Conectado");
        }
    }
}

void MainWindow::refresh_list_widget()
{
    std::list<Item*> file_list;
    QListWidgetItem* item_aux;

    ui->lwFiles->clear();

    file_list = sh->get_file_list();
    for(std::list<Item*>::iterator it = file_list.begin(); it != file_list.end(); it++)
    {
        std::cout << (*it)->dir << " " << (*it)->name << std::endl;
        if((*it)->dir) item_aux = new QListWidgetItem(QIcon::fromTheme("folder"), QString::fromStdString((*it)->name));
        else item_aux = new QListWidgetItem(QIcon::fromTheme("text"), QString::fromStdString((*it)->name));

        ui->lwFiles->addItem(item_aux);
    }
}


void MainWindow::on_pbClose_clicked()
{
    close();
}

void MainWindow::on_lwFiles_itemDoubleClicked(QListWidgetItem *item)
{
    if(!item->icon().name().toStdString().compare("folder"))
    {
        sh->move_path(item->text().toStdString().c_str());
        sh->list_files();
        refresh_list_widget();
        show_path_wo_home();
    }
}

void MainWindow::on_pbBack_clicked()
{
    sh->back_path();
    sh->list_files();
    refresh_list_widget();
    show_path_wo_home();
}

void MainWindow::show_path_wo_home()
{
    std::string path;
    path = sh->getCurrentPath();
    path.erase(0,5);                    //5 letras tiene "/home"
    ui->labPath->setText(path.c_str());
}

void MainWindow::on_pbUpload_clicked()
{
    QFileDialog search_window(this);

    QStringList selected_files;
    std::string pathToFile;

    if(search_window.exec())
    {
        selected_files = search_window.selectedFiles();
        pathToFile = selected_files.at(0).toStdString();
    }

    ui->pbUpload->hide();
    ui->pBarUpload->show();

    sh->scp_upload(pathToFile, ui->pBarUpload);

    ui->pBarUpload->hide();
    ui->pbUpload->show();

    sh->list_files();
    refresh_list_widget();
}

void MainWindow::on_pbDownload_clicked()
{
    ui->pbDownload->hide();
    ui->pBarDownload->show();
    QList<QListWidgetItem*> selected_items = ui->lwFiles->selectedItems();
    for(QList<QListWidgetItem*>::iterator it = selected_items.begin(); it != selected_items.end(); it++)
    {
        sh->scp_download((*it)->text().toStdString(), ui->pBarDownload);
    }
    ui->pBarDownload->hide();
    ui->pbDownload->show();
}
