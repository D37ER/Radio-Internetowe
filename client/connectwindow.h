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

public slots:
    void onChangeStatus(QString);
    void onShowError(int, QString);
    void onHide();
    void onShow();

private slots:
    void connectPressed();

signals:
    void ConnectToServer(QString, QString, QString);
};

#endif // CONNECTWINDOW_H
