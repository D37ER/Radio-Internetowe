#include "mainwindow.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    //GUI
    ui->setupUi(this);
    ui->hsTime->setStyleSheet("QSlider {height: 15px;} QSlider::handle:horizontal {background-color: blue;height: 15px;}");
    ui->lwUsers->viewport()->setAutoFillBackground(false);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);
    bBestSongs[0] = ui->bBestSong1;
    bBestSongs[1] = ui->bBestSong2;
    bBestSongs[2] = ui->bBestSong3;
    bBestSongs[3] = ui->bBestSong4;
    bBestSongs[4] = ui->bBestSong5;
    bBestSongs[5] = ui->bBestSong6;
    bBestSongs[6] = ui->bBestSong7;
    bBestSongs[7] = ui->bBestSong8;
    connect(ui->hsVolume, &QSlider::valueChanged, this, &MainWindow::hsVolumeChanged);
    connect(ui->bMute, &QPushButton::clicked, this, &MainWindow::bMuteClicked);
    connect(ui->bBestSong1, &QPushButton::clicked, this, &MainWindow::bBestSong1Clicked);
    connect(ui->bBestSong2, &QPushButton::clicked, this, &MainWindow::bBestSong2Clicked);
    connect(ui->bBestSong3, &QPushButton::clicked, this, &MainWindow::bBestSong3Clicked);
    connect(ui->bBestSong4, &QPushButton::clicked, this, &MainWindow::bBestSong4Clicked);
    connect(ui->bBestSong5, &QPushButton::clicked, this, &MainWindow::bBestSong5Clicked);
    connect(ui->bBestSong6, &QPushButton::clicked, this, &MainWindow::bBestSong6Clicked);
    connect(ui->bBestSong7, &QPushButton::clicked, this, &MainWindow::bBestSong7Clicked);
    connect(ui->bBestSong8, &QPushButton::clicked, this, &MainWindow::bBestSong8Clicked);
    connect(ui->lwSongs, &QListWidget::itemActivated, this, &MainWindow::lwSongsSelectionChanged);
}

void MainWindow::setPlayThread(MusicPlayer *playThread)
{
    this->playThread = playThread;
    connect(playThread, SIGNAL(TimeChanged(float)), this, SLOT(onTimeChanged(float)));
}

void MainWindow::setNetThread(NetThread *netThread)
{
    this->netThread = netThread;
    connect(netThread, SIGNAL(SongChanged(QString, float)), this, SLOT(onSongChanged(QString, float)));
    connect(netThread, SIGNAL(RoomChanged(QString)), this, SLOT(onRoomChanged(QString)));
    connect(netThread, SIGNAL(SongsListChanged(QVector<QString>)), this, SLOT(onSongsListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(UsersListChanged(QVector<QString>)), this, SLOT(onUsersListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(MySongsListChanged(QVector<QString>)), this, SLOT(onMySongsListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(SongsVotesChanged(QVector<QString>, QVector<uint>)), this, SLOT(onSongsVotesChanged(QVector<QString>, QVector<uint>)));
}

void MainWindow::vote(QString songTitle)
{
    this->currentVote = songTitle;
    refreshVote();
    netThread->changeVote(songTitle);
}

void MainWindow::refreshVote()
{
    for(int i=0; i<8 && i<songs.size(); i++)
    {
        if(this->songs[i] == this->currentVote)
            bBestSongs[i]->setStyleSheet("QToolButton { background-color: red; }");
        else
            bBestSongs[i]->setStyleSheet("");
    }


    for(int i=0; i<ui->lwSongs->count();i++)
    {
        if(this->songs[i] == this->currentVote)
            ui->lwSongs->item(i)->setBackground(Qt::red);
        else
            ui->lwSongs->item(i)->setBackground(Qt::white);
    }
}

//GUI

void MainWindow::hsVolumeChanged(int value)
{
    playThread->volume = (muted)? 0 : (float)value/100;
}

void MainWindow::bMuteClicked()
{
    muted = !muted;
    playThread->volume = (muted)? 0 : (float)ui->hsVolume->value()/100;
}

void MainWindow::bBestSong1Clicked()
{
    vote(this->songs[0]);
}

void MainWindow::bBestSong2Clicked()
{
    vote(this->songs[1]);
}

void MainWindow::bBestSong3Clicked()
{
    vote(this->songs[2]);
}

void MainWindow::bBestSong4Clicked()
{
    vote(this->songs[3]);
}

void MainWindow::bBestSong5Clicked()
{
    vote(this->songs[4]);
}

void MainWindow::bBestSong6Clicked()
{
    vote(this->songs[5]);
}


void MainWindow::bBestSong7Clicked()
{
    vote(this->songs[6]);
}

void MainWindow::bBestSong8Clicked()
{
    vote(this->songs[7]);
}

void MainWindow::lwSongsSelectionChanged()
{
    vote(this->songs[ui->lwSongs->row(ui->lwSongs->selectedItems()[0])]);
}

//SLOTS

void MainWindow::onTimeChanged(float time)
{
    ui->hsTime->setValue(time*100);

    int timeInt = (int)time;
    int timeSec = timeInt%60;
    QString timeSecStr = (timeSec < 10)? "0"+QString::number((int)timeSec) : QString::number((int)timeSec);
    int timeMin = timeInt/60;
    QString timeMinStr = (timeMin < 10)? "0"+QString::number((int)timeMin) : QString::number((int)timeMin);

    int lengthInt = (int)this->songLength;
    int lengthSec = lengthInt%60;
    QString lengthSecStr = (lengthSec < 10)? "0"+QString::number((int)lengthSec) : QString::number((int)lengthSec);
    int lengthMin = lengthInt/60;
    QString lengthMinStr = (lengthMin < 10)? "0"+QString::number((int)lengthMin) : QString::number((int)lengthMin);

    ui->lTime->setText(timeMinStr + ":" + timeSecStr + " / " + lengthMinStr + ":" + lengthSecStr);
}

void MainWindow::onSongChanged(QString newSongTitle, float newSongDuration)
{
    ui->lSongTitle->setText(newSongTitle);
    this->songLength = newSongDuration;
    ui->hsTime->setMinimum(0);
    ui->hsTime->setMaximum(this->songLength*100);
    if(this->currentVote == newSongTitle)
    {
        currentVote = "";
        refreshVote();
    }

}

void MainWindow::onRoomChanged(QString newRoomName)
{
    ui->lRoomName->setText(newRoomName);
}

void MainWindow::onSongsListChanged(QVector<QString> newSongsList)
{
    this->songs.clear();
    this->songs.append(newSongsList);

    //gui refresh
    ui->lwSongs->clear();
    for(int i=0;i<this->songs.size();i++)
    {
        ui->lwSongs->addItem(this->songs[i]);
    }
    for(int i=0; i<8&&i<this->songs.size();i++)
    {
        bBestSongs[i]->setEnabled(true);
        bBestSongs[i]->setText(this->songs[i]);
    }
    for(int i=this->songs.size(); i<8;i++)
        bBestSongs[i]->setEnabled(false);

    //TODO nie działa z onSongsVotesChanged - poprawić
}

void MainWindow::onUsersListChanged(QVector<QString> newUsersList)
{
    ui->lwUsers->clear();
    for(int i=0;i<newUsersList.size();i++)
    {
        cout << "user "  << newUsersList[i].toStdString() << endl;
        ui->lwUsers->addItem(newUsersList[i]);
    }
}

void MainWindow::onMySongsListChanged(QVector<QString> newMySongsList)
{

}

void MainWindow::onSongsVotesChanged(QVector<QString> songs, QVector<uint> votes)
{
    for(int i=0; i<songs.size(); i++)
    {
        if(this->songs[i] != songs[i])
            this->songs[i] = songs[i];
        if(i<8)
            bBestSongs[i]->setText(songs[i] + "\n" +  QString::number(votes[i]) + " votes");
        ui->lwSongs->item(i)->setText(songs[i] + " (" +  QString::number(votes[i]) + " votes)");
    }
    refreshVote();
}

MainWindow::~MainWindow()
{
    delete ui;
}

