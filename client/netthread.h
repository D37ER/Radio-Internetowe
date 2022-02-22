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
    void run();

private :
    void sendUserName(QString userName);
    void toStrVec(QVector<QString> *out, QByteArray *ba, int begin, int end, char delimeter);
    void toVotes(QVector<QString> *songs, QVector<uint> *votes, QByteArray *data, int begin, int end, char delimeter);

    MusicPlayer *musicPlayer;
    QTcpSocket *tcpSocket;
    QUdpSocket *udpSocket;
    QTimer *connTimeoutTimer;

    //server connection
    bool connected;
    bool userNameOk;
    bool disconnectingInProgress;
    QString newServerAddress;
    QString newServerPort;
    QString newUserName;
    QString serverAddress;
    QString serverPort;
    QString userName;

    //tcp receive
    QByteArray inputBuffer;
    int inputBufferSize;
    int inputBufferlast0;
    char currentInputType;

    //udp receive
    char udpBuffer[409600];
    int udpBufferIndexes[100];
    int udpBufferStart=0;
    int udpBufferEnd=0;

public slots:
    void onConnectToServer(QString, QString, QString);
    void onChangeVote(QString);
    void onSendSong(QString, QString);

private slots:
    void socketConnected();
    void socketDisconnected();
    void tcpDataReceived();
    void udpDataReceived();
    void socketErrorOccurred(QAbstractSocket::SocketError);
    void onChangeVolume(float);
    void onTimeChanged(float);

signals:
    void ConnectionStateChanged(QString);
    void ConnectionError(int, QString);
    void ConnectionSuccessful();
    void ServerNameChanged(QString);
    void SongsListChanged(QVector<QString>);
    void UsersListChanged(QVector<QString>);
    void SongsVotesChanged(QVector<QString>, QVector<uint>);
    void SongChanged(QString, float);
    void TimeChanged(float);
    void ChangeVolume(float);
};

#endif // NETTHREAD_H
