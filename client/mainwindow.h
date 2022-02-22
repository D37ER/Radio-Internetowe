#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "musicplayer.h"
#include "netthread.h"

#include <QMainWindow>
#include <iostream>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    float songLength = 1;
    QToolButton *bBestSongs[8];

protected:
    void play();
    void bChangeServerClicked();
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
    void showMySongsClicked();
    void bMySongsClicked();

private:
    void vote(QString songTitle);
    QString addNewLines(QString string, int maxWidth);
    void refreshVoteGUI();
    Ui::MainWindow *ui;
    QIcon *unmuttedIcon;
    QIcon *muttedIcon;
    QString currentVote = "";
    bool muted = false;
    QVector<QString> songs;

public slots:
    void onHide();
    void onShow();
    void onTimeChanged(float);
    void onServerNameChanged(QString);
    void onSongsListChanged(QVector<QString>);
    void onUsersListChanged(QVector<QString>);
    void onSongsVotesChanged(QVector<QString>, QVector<uint>);
    void onSongChanged(QString, float);

signals:
    void ChangeVote(QString);
    void SendSong(QString, QString);
    void ChangeVolume(float);
    void showConnectWindow();
};
#endif // MAINWINDOW_H
