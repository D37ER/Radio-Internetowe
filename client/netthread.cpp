#include "netthread.h"
using namespace std;

NetThread::NetThread(QObject *parent): QThread{parent}
{
    inputBufferSize = 0;
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
    musicPlayer->setUp(48000, 2, 4096, 0.1f, 0.8f);
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

void NetThread::toVotes(QVector<QString> *songs, QVector<uint> *votes, QByteArray *data, int begin, int end, char delimeter)
{
    int recordStart = 0;
    uint t;
    for(int i=begin; i<=end; i++)
        if(data->at(i) == delimeter || i == end)
        {
            if(i-recordStart < 5)
            {
                recordStart = i+1;
                continue;
            }
            t=0;
            t+= ((int)data->at(recordStart++));
            t+= ((int)data->at(recordStart++))*256;
            t+= ((int)data->at(recordStart++))*65536;
            t+= ((int)data->at(recordStart++))*16777216;
            votes->append(t);
            songs->append(QString(data->mid(recordStart, i-recordStart)));
            recordStart = i+1;
        }
}

void NetThread::tcpDataReceived()
{
    cout << "dataReceived " << tcpSocket->bytesAvailable() << endl;
    if(tcpSocket->bytesAvailable() <1)
        return;

    if(currentInputType == '\0')
        currentInputType = tcpSocket->read(1).at(0);

    inputBuffer.append(tcpSocket->readAll());
    cout << (QString(inputBuffer)).toStdString() << endl;
    for(; inputBufferSize<inputBuffer.size(); inputBufferSize++)
    {
        cout << "adding buffer " << endl;
        if(inputBuffer.at(inputBufferSize) == '\n')
        {
            cout << "enter " << endl;
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
                float length=0;
                memcpy(&length, inputBuffer, sizeof(length));
                emit SongChanged(QString(inputBuffer.mid(sizeof(length), inputBufferSize-sizeof(length))), length);
                break;
            }
            default:
                emit Error(3, "unrecognized data type");
            }

            if(inputBufferSize+1 < inputBuffer.size())
            {
                currentInputType = inputBuffer.at(inputBufferSize+1);
                inputBuffer.remove(0,inputBufferSize+2);
            }
            else
            {
                currentInputType = '\0';
                inputBuffer.remove(0,inputBufferSize+1);
            }
            inputBufferSize = 0;
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
        musicPlayer->play(buf);
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
    ;
}

void NetThread::run()
{
    while(true);
}
