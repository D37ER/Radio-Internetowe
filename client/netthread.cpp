#include "netthread.h"
using namespace std;

NetThread::NetThread(QObject *parent): QThread{parent}
{
    inputBufferSize = 0;
    inputBufferlast0 = 0;
    currentInputType = '\0';
    tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, &QTcpSocket::connected, this, &NetThread::socketConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &NetThread::socketDisconnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &NetThread::tcpDataReceived);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &NetThread::errorOccurred);

    udpSocket = new QUdpSocket(this);
    connect(udpSocket, &QUdpSocket::readyRead, this, &NetThread::udpDataReceived);
}

void NetThread::setMusicPlayer(MusicPlayer *musicPlayer)
{
    this->musicPlayer = musicPlayer;
}

//SOCKET SLOTS
void NetThread::socketConnected()
{
    udpSocket->bind(QHostAddress::Any, tcpSocket->localPort());
    emit ConnectionStateChanged("connected");
    connTimeoutTimer->stop();
    connTimeoutTimer->deleteLater();
    QByteArray baUserName = this->userName.toLocal8Bit();
    tcpSocket->write(baUserName.data());
    tcpSocket->write("\n");
}

void NetThread::socketDisconnected()
{
    //TODO show info
    tcpSocket->deleteLater();
}

void NetThread::toStrVec(QVector<QString> *out, QByteArray *data, int begin, int end, char delimeter)
{
    int strStart = 0;
    for(int i=begin; i<=end; i++)
        if(data->at(i) == delimeter || i == end)
        {
            out->append(QString(data->mid(strStart, i-strStart)));
            strStart = i+1;
        }
}

void NetThread::toVotes(QVector<QString> *songs, QVector<uint> *votes, QByteArray *data, int begin, int end, char delimiter)
{
    int recordStart = 0;
    uint t = 0;
    for(int i=begin; i<=end; i++)
    {
        if(i-recordStart == 0)
            t+= (unsigned char)data->at(i);
        else if(i-recordStart == 1)
            t+= (unsigned char)data->at(i)*256;
        else if(i-recordStart == 2)
            t+= (unsigned char)data->at(i)*256*256;
        else if(i-recordStart == 3)
            t+= (unsigned char)data->at(i)*256*256*256;
        else if(data->at(i) == delimiter || i == end)
        {
            votes->append(t);
            t = 0;
            songs->append(QString(data->mid(recordStart+4, i-recordStart-4)));
            recordStart = i+1;
        }
    }
}

void NetThread::tcpDataReceived()
{
    if(tcpSocket->bytesAvailable() <1)
        return;

    if(currentInputType == '\0')
        currentInputType = tcpSocket->read(1).at(0);

    inputBuffer.append(tcpSocket->readAll());


    for(; inputBufferSize<inputBuffer.size(); inputBufferSize++)
    {
        if(currentInputType == 'e' && inputBufferSize-inputBufferlast0 > 4 && inputBuffer.at(inputBufferSize) == '\0')
            inputBufferlast0 = inputBufferSize;
        else if(inputBuffer.at(inputBufferSize) == '\n' && (currentInputType != 'f' || inputBufferSize > 12) && (currentInputType != 'e' || inputBufferSize-inputBufferlast0 > 4))
        {
            switch(currentInputType)
            {
            case 'a':
                emit ConnectionStateChanged("server name received");
                emit RoomChanged(QString(inputBuffer.mid(0, inputBufferSize)));
                break;
            case 'b':
            {
                QVector<QString> songs;
                toStrVec(&songs, &inputBuffer, 0, inputBufferSize, '\0');
                emit SongsListChanged(songs);
                break;
            }
            case 'c':
            {
                QVector<QString> mySongs;
                toStrVec(&mySongs, &inputBuffer, 0, inputBufferSize, '\0');
                emit MySongsListChanged(mySongs);
                break;
            }
            case 'd':
            {
                QVector<QString> users;
                toStrVec(&users, &inputBuffer, 0, inputBufferSize, '\0');
                emit UsersListChanged(users);
                break;
            }
            case 'e':
            {
                QVector<QString> songs;
                QVector<uint> votes;
                toVotes(&songs, &votes, &inputBuffer, 0, inputBufferSize, '\0');
                emit SongsVotesChanged(songs, votes);
                break;
            }
            case 'f':
            {
                char data[12];
                memcpy(data, inputBuffer, 12);
                int sampleCount = 0;
                memcpy(&sampleCount, data, 4);
                int sampleRate = 0;
                memcpy(&sampleRate, data + 4, 4);
                int packSize = 0;
                memcpy(&packSize, data + 8, 4);
                QString name = QString(inputBuffer.mid(8, inputBufferSize-8));
                emit SongChanged(name, (float)(sampleCount)/sampleRate);
                musicPlayer->setUp(sampleRate, 2, packSize, 0.1f, 0.8f);
                break;
            }
            default:
                emit Error(3, "unrecognized data type");
            }

            if(inputBufferSize+1 < inputBuffer.size())
            {
                currentInputType = inputBuffer.at(inputBufferSize+1);
                inputBufferlast0 = 0;
                inputBuffer.remove(0,inputBufferSize+2);
            }
            else
            {
                currentInputType = '\0';
                inputBuffer.remove(0,inputBufferSize+1);
            }
            inputBufferSize = -1;
        }
    }
}

void NetThread::udpDataReceived()
{
    QByteArray ba;

    while(udpSocket->hasPendingDatagrams())
    {
        ba = udpSocket->receiveDatagram().data();
        memcpy(udpBufferIndexes+udpBufferEnd, ba, 4);
        memcpy(udpBuffer+4096*udpBufferEnd, ba+4, 4096);
        udpBufferEnd++;
        if(udpBufferEnd>=100)
            udpBufferEnd=0;
    }
    while(udpBufferStart != udpBufferEnd)
    {

        char buf[4096];
        memcpy(buf, udpBuffer+4096*udpBufferStart, 4096);
        if(musicPlayer->setuped)
            musicPlayer->play(buf, udpBufferIndexes[udpBufferStart]);
        udpBufferStart++;
        if(udpBufferStart>=100)
            udpBufferStart=0;
    }
}

void NetThread::errorOccurred(QAbstractSocket::SocketError socketError)
{
    //TODO poprawić komunikaty
    emit Error(100 + socketError,"socket error occurred");
}

void NetThread::connectToServer(QString serverName, QString serverPort, QString userName)
{
    this->userName = userName;
    tcpSocket->connectToHost(serverName, serverPort.toInt());
    connTimeoutTimer = new QTimer(this);
    connTimeoutTimer->setSingleShot(true);
    connect(connTimeoutTimer, &QTimer::timeout, [&]{
        tcpSocket->abort();
        connTimeoutTimer->deleteLater();
        emit Error(1, "Connection time out.");
    });
    connTimeoutTimer->start(5000);
}

void NetThread::changeVote(QString newSongTitle)
{
    QByteArray ba = newSongTitle.toLocal8Bit();
    tcpSocket->write(ba.data());
    tcpSocket->write("\n");
}

void NetThread::run()
{
    while(true);
}
