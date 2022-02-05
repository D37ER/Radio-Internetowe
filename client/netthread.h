#ifndef NETTHREAD_H
#define NETTHREAD_H

#include <QThread>
#include "musicplayer.h"
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
#include <QtCore>
#include <QRandomGenerator>
#include <QTime>

class NetThread : public QThread
{
    Q_OBJECT
public:
    explicit NetThread(QObject *parent = nullptr);
    void connectToServer(QString serverName, QString serverPort, QString userName);
    void changeVote(QString newSongTitle);
    void setMusicPlayer(MusicPlayer *playThread);
    void run();
private :
    MusicPlayer *musicPlayer;
    bool connected = false;
    QString roomName = "";
    QVector<QString> songs;
    QVector<int> votes;
    QString currentVote = "";

signals:
    void SongChanged(QString, float);
    void RoomChanged(QString);
    void SongsListChanged(QVector<QString>);
    void UsersListChanged(QVector<QString>);
    void MySongsListChanged(QVector<QString>);
    void SongsVotesChanged(QVector<QString>, QVector<int>);
};

#endif // NETTHREAD_H
