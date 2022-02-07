#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QDialog>

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QDialog
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();
    Ui::AuthWindow *ui;
private slots:
    void on_pbConnect_clicked();

    void on_pbClose_clicked();

private:

};

#endif // AUTHWINDOW_H
