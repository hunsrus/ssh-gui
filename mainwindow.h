#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDialog>
#include "sessionhandler.h"
#include <list>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QDialog
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    void refresh_list_widget();
    void show_path_wo_home();
    ~MainWindow();

private slots:
    void on_pbConnect_clicked();

    void on_pbClose_clicked();

    void on_lwFiles_itemDoubleClicked(QListWidgetItem *item);

    void on_pbBack_clicked();

    void on_pbUpload_clicked();

    void on_pbDownload_clicked();

private:
    Ui::MainWindow *ui;
    SessionHandler *sh;
};
#endif // MAINWINDOW_H
