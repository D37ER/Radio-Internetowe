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
    NetThread *netThread;
    float songLength = 1;
    QPushButton *bBestSongs[8];

protected:
    void play();
    void hsVolumeChanged(int value);
    void bMuteClicked();

private:
    Ui::MainWindow *ui;
    bool muted = false;

public slots:
    void onTimeChanged(float);
    void onSongChanged(QString, float);
    void onRoomChanged(QString);
    void onSongsListChanged(QVector<QString>);
    void onUsersListChanged(QVector<QString>);
    void onMySongsListChanged(QVector<QString>);
    void onSongsVotesChanged(QHash<QString, int>);

};
#endif // MAINWINDOW_H
