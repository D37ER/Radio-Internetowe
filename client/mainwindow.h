#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include "ui_mainwindow.h"
#include "musicplayer.h"
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
    void setPlayThread(MusicPlayer *playThread);
    void setNetThread(NetThread *netThread);
    MusicPlayer *playThread;
    NetThread *netThread;
    float songLength = 1;
    QToolButton *bBestSongs[8];

protected:
    void play();
    void hsVolumeChanged(int value);
    void bMuteClicked();
    void bBestSong1Clicked();
    void bBestSong2Clicked();
    void bBestSong3Clicked();
    void bBestSong4Clicked();
    void bBestSong5Clicked();
    void bBestSong6Clicked();
    void bBestSong7Clicked();
    void bBestSong8Clicked();
    void lwSongsSelectionChanged();

private:
    void vote(QString songTitle);
    void refreshVote();
    Ui::MainWindow *ui;
    QString currentVote = "";
    bool muted = false;
    QVector<QString> songs;

public slots:
    void onTimeChanged(float);
    void onSongChanged(QString, float);
    void onRoomChanged(QString);
    void onSongsListChanged(QVector<QString>);
    void onUsersListChanged(QVector<QString>);
    void onMySongsListChanged(QVector<QString>);
    void onSongsVotesChanged(QVector<QString>, QVector<int>);

};
#endif // MAINWINDOW_H
