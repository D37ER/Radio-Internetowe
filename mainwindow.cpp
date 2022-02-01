#include "mainwindow.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bBestSongs[0] = ui->bBestSong1;
    bBestSongs[1] = ui->bBestSong2;
    bBestSongs[2] = ui->bBestSong3;
    bBestSongs[3] = ui->bBestSong4;
    bBestSongs[4] = ui->bBestSong5;
    bBestSongs[5] = ui->bBestSong6;
    bBestSongs[6] = ui->bBestSong7;
    bBestSongs[7] = ui->bBestSong8;

    connect(ui->bPlay, &QPushButton::clicked, this, &MainWindow::play);
    connect(ui->hsVolume, &QSlider::valueChanged, this, &MainWindow::hsVolumeChanged);
    connect(ui->bMute, &QPushButton::clicked, this, &MainWindow::bMuteClicked);
    playThread = new PlayThread();
    netThread = new NetThread(this);
    netThread->setPlayThread(playThread);
    netThread->start();
    connect(playThread, SIGNAL(TimeChanged(float)), this, SLOT(onTimeChanged(float)));
    connect(netThread, SIGNAL(SongChanged(QString, float)), this, SLOT(onSongChanged(QString, float)));
    connect(netThread, SIGNAL(RoomChanged(QString)), this, SLOT(onRoomChanged(QString)));
    connect(netThread, SIGNAL(SongsListChanged(QVector<QString>)), this, SLOT(onSongsListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(UsersListChanged(QVector<QString>)), this, SLOT(onUsersListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(MySongsListChanged(QVector<QString>)), this, SLOT(onMySongsListChanged(QVector<QString>)));
    connect(netThread, SIGNAL(SongsVotesChanged(QHash<QString, int>)), this, SLOT(onSongsVotesChanged(QHash<QString, int>)));

    //QListWidgetItem *item = ui->lwSongs->selectedItems()[0];
    //cout << item->text().toStdString() << endl;
}

//GUI

void MainWindow::play()
{
    netThread->connectToRoom("Best room");
}

void MainWindow::hsVolumeChanged(int value)
{
    cout << (float)value/100 << endl;
    playThread->volume = (muted)? 0 : (float)value/100;
}

void MainWindow::bMuteClicked()
{
    muted = !muted;
    cout << muted << endl;
    playThread->volume = (muted)? 0 : (float)ui->hsVolume->value()/100;
}

//SLOTS

void MainWindow::onTimeChanged(float time)
{
    ui->pbTime->setRange(0,this->songLength*100);
    ui->pbTime->setValue(time*100);

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
}

void MainWindow::onRoomChanged(QString newRoomName)
{
    ui->lRoomName->setText(newRoomName);
}

void MainWindow::onSongsListChanged(QVector<QString> newSongsList)
{
    ui->lwSongs->clear();
    for(int i=0;i<newSongsList.size();i++)
    {
        cout << "song " << newSongsList[i].toStdString() << endl;
        ui->lwSongs->addItem(newSongsList[i]);
    }
    for(int i=0; i<8&&i<newSongsList.size();i++)
    {
        bBestSongs[i]->setEnabled(true);
        bBestSongs[i]->setText(newSongsList[i]);
    }
    for(int i=newSongsList.size(); i<8;i++)
        bBestSongs[i]->setEnabled(false);
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

void MainWindow::onSongsVotesChanged(QHash<QString, int> newSongVotes)
{

}

MainWindow::~MainWindow()
{
    delete ui;
}

