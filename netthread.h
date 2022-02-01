#ifndef NETTHREAD_H
#define NETTHREAD_H

#include <QThread>
#include "playthread.h"
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
#include <QtCore>

class NetThread : public QThread
{
    Q_OBJECT
public:
    explicit NetThread(QObject *parent = nullptr);
    void connectToRoom(QString roomName);
    void changeVote(QString newSongTitle);
    void setPlayThread(PlayThread *playThread);
    void run();
private :
    PlayThread *playThread;
    bool connected = false;
    QString roomName = "";

signals:
    void SongChanged(QString, float);
    void RoomChanged(QString);
    void SongsListChanged(QVector<QString>);
    void UsersListChanged(QVector<QString>);
    void MySongsListChanged(QVector<QString>);
    void SongsVotesChanged(QHash<QString, int>);

};

#endif // NETTHREAD_H
