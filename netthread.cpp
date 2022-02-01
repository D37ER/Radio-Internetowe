#include "netthread.h"
using namespace std;

NetThread::NetThread(QObject *parent): QThread{parent}
{

}

void NetThread::connectToRoom(QString roomName)
{
    cout << "connectToRoom " << roomName.toStdString() << endl;
    this->roomName = roomName;
    this->connected = true;

    QVector<QString> songs;
    for(int i=0;i<6;i++)
        songs.append("song"+QString::number(i));
    emit SongsListChanged(songs);

    QVector<QString> users;
    for(int i=0;i<15;i++)
        users.append("user"+QString::number(i));
    emit UsersListChanged(users);
}

void NetThread::changeVote(QString newSongTitle)
{
    cout << "changeVote " << newSongTitle.toStdString() << endl;
}

void NetThread::setPlayThread(PlayThread *playThread)
{
    this->playThread = playThread;
}

void NetThread::run()
{
    while(!this->connected);
    emit RoomChanged(this->roomName);
    int bufferSize = 1000;
    QFile sourceFile;
    sourceFile.setFileName("D:/usunMnie/song1.wav");
    sourceFile.open(QIODevice::ReadOnly);
    cout << "File raport: exists "<< sourceFile.exists() << " fileSize "<< sourceFile.size() << endl;
    emit SongChanged("D:/usunMnie/song1.wav", ((float)sourceFile.size())/100000/2);
    char buffer[bufferSize];
    while(!sourceFile.atEnd())
    {
        sourceFile.read(buffer, bufferSize);
        playThread->play(buffer);
    }
}
