#ifndef CONNECTWINDOW_H
#define CONNECTWINDOW_H

#include "mainwindow.h"

#include <QMainWindow>

namespace Ui {
class ConnectWindow;
}

class ConnectWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConnectWindow(QWidget *parent = nullptr);
    ~ConnectWindow();

private:
    Ui::ConnectWindow *ui;
    MusicPlayer *playThread;
    NetThread *netThread;
    MainWindow *mw;

private slots:
    void connectToServer();
    void changeToMainWindow();
    void changeStatus(QString);
    void showError(int, QString);
};

#endif // CONNECTWINDOW_H
