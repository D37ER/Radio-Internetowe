#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include "ui_mainwindow.h"
#include "playthread.h"
#include "netthread.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    PlayThread *playThread;

protected:
    void play();
    void hsVolumeChanged(int value);
    void bMuteClicked();

private:
    Ui::MainWindow *ui;
    bool muted = false;

public slots:
    void onTimeChanged(float,float);
    void onSongChanged(QString);

};
#endif // MAINWINDOW_H
