#ifndef NETTHREAD_H
#define NETTHREAD_H

#include "musicplayer.h"

#include <QThread>
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
#include <QtCore>
#include <QRandomGenerator>
#include <QTime>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QNetworkDatagram>

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
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;
    MusicPlayer *musicPlayer;
    QTimer *connTimeoutTimer;
    QString userName;
    QByteArray inputBuffer;
    int inputBufferSize;
    char currentInputType;

    char udpBuffer[409600];
    int udpBufferIndexes[100];
    int udpBufferStart=0;
    int udpBufferEnd=0;

    void toStrVec(QVector<QString> *out, QByteArray *ba, int begin, int end, char delimeter);
    void toVotes(QVector<QString> *songs, QVector<uint> *votes, QByteArray *data, int begin, int end, char delimeter);

private slots:
    void socketConnected();
    void socketDisconnected();
    void tcpDataReceived();
    void udpDataReceived();
    void errorOccurred(QAbstractSocket::SocketError);

signals:
    void ConnectionStateChanged(QString);
    void Error(int, QString);
    void SongChanged(QString, float);
    void RoomChanged(QString);
    void SongsListChanged(QVector<QString>);
    void UsersListChanged(QVector<QString>);
    void MySongsListChanged(QVector<QString>);
    void SongsVotesChanged(QVector<QString>, QVector<uint>);
};

#endif // NETTHREAD_H
