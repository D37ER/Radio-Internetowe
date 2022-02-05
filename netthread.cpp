#include "netthread.h"
using namespace std;

NetThread::NetThread(QObject *parent): QThread{parent}
{

}

void NetThread::connectToServer(QString serverName, QString serverPort, QString userName)
{
    //delay
    QTime dieTime= QTime::currentTime().addSecs(2);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);


    this->roomName = serverName;
    this->connected = true;

    QString znaki[18] = {"0","1","2","3","4","5","6","7","8","9","a","b","c","d","e","f","g","h"};

    for(int i=0;i<15;i++)
        this->songs.append("Long song name "+ znaki[i]);
    emit SongsListChanged(this->songs);

    QVector<QString> users;
    for(int i=0;i<15;i++)
        users.append("long user name "+ znaki[i]);
    emit UsersListChanged(users);


    this->votes.clear();
    for(int i=0;i<this->songs.size();i++)
        this->votes.append(QRandomGenerator::global()->bounded(11));

    int tempVotes, maxVotesId;
    QString tempSongName;
    for(int i=0; i<songs.size();i++)
    {
        maxVotesId=i;
        for(int j=i+1; j<songs.size();j++)
        {
            if(votes[j] > votes[maxVotesId])
                maxVotesId = j;
            else if(votes[j] == votes[maxVotesId])
            {
                int k=0;
                while(k <songs[j].size() && k <songs[maxVotesId].size() && songs[j][k] == songs[maxVotesId][k] )k++;
                if(k == songs[maxVotesId].size())
                    ;
                else if(k == songs[j].size())
                    maxVotesId = j;
                else if(songs[j][k] < songs[maxVotesId][k])
                    maxVotesId = j;
            }
        }
        if(i!=maxVotesId)
        {
            tempVotes = votes[i];
            tempSongName = songs[i];
            votes[i] = votes[maxVotesId];
            songs[i] = songs[maxVotesId];
            votes[maxVotesId] = tempVotes;
            songs[maxVotesId] = tempSongName;
        }
    }

    emit SongsVotesChanged(this->songs, this->votes);
}

void NetThread::changeVote(QString newSongTitle)
{
    cout << "changeVote " << newSongTitle.toStdString() << endl;
    int t = songs.indexOf(currentVote);
    if(t != -1)
        votes[t]--;
    currentVote = newSongTitle;
    t = songs.indexOf(newSongTitle);
    if(t != -1)
        votes[t]++;

    int tempVotes, maxVotesId;
    QString tempSongName;
    for(int i=0; i<songs.size();i++)
    {
        maxVotesId=i;
        for(int j=i+1; j<songs.size();j++)
        {
            if(votes[j] > votes[maxVotesId])
                maxVotesId = j;
            else if(votes[j] == votes[maxVotesId])
            {
                int k=0;
                while(k <songs[j].size() && k <songs[maxVotesId].size() && songs[j][k] == songs[maxVotesId][k] )k++;
                if(k == songs[maxVotesId].size())
                    ;
                else if(k == songs[j].size())
                    maxVotesId = j;
                else if(songs[j][k] < songs[maxVotesId][k])
                    maxVotesId = j;
            }
        }
        if(i!=maxVotesId)
        {
            tempVotes = votes[i];
            tempSongName = songs[i];
            votes[i] = votes[maxVotesId];
            songs[i] = songs[maxVotesId];
            votes[maxVotesId] = tempVotes;
            songs[maxVotesId] = tempSongName;
        }
    }

    emit SongsVotesChanged(songs, votes);
}

void NetThread::setMusicPlayer(MusicPlayer *musicPlayer)
{
    this->musicPlayer = musicPlayer;
}

void NetThread::run()
{
    while(!this->connected);
    emit RoomChanged(this->roomName);
    int bufferSize = 1000;
    musicPlayer->setUp(100000, 2, bufferSize, 0.1, 0.1);
    QFile sourceFile;
    sourceFile.setFileName("D:/usunMnie/song1.wav");
    sourceFile.open(QIODevice::ReadOnly);
    cout << "File raport: exists "<< sourceFile.exists() << " fileSize "<< sourceFile.size() << endl;
    emit SongChanged("D:/usunMnie/song1.wav", ((float)sourceFile.size())/100000/2);
    char buffer[bufferSize];
    while(!sourceFile.atEnd())
    {
        sourceFile.read(buffer, bufferSize);
        musicPlayer->play(buffer);
    }
}
