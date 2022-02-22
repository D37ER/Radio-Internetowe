#include "mainwindow.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->hsTime->setStyleSheet("QSlider {height:10px;} .QSlider::handle:horizontal {background-color:blue;border-radius: 5px;}");
    ui->lwUsers->viewport()->setAutoFillBackground(false);
    ui->splitter->setStretchFactor(0, 0);
    ui->splitter->setStretchFactor(1, 1);

    QPixmap pixmap1(":/unmuted.png");
    unmuttedIcon = new QIcon(pixmap1);
    QPixmap pixmap2(":/muted.png");
    muttedIcon = new QIcon(pixmap2);
    ui->bMute->setIcon(*unmuttedIcon);
    ui->bMute->setIconSize(pixmap1.rect().size());

    bBestSongs[0] = ui->bBestSong1;
    bBestSongs[1] = ui->bBestSong2;
    bBestSongs[2] = ui->bBestSong3;
    bBestSongs[3] = ui->bBestSong4;
    bBestSongs[4] = ui->bBestSong5;
    bBestSongs[5] = ui->bBestSong6;
    bBestSongs[6] = ui->bBestSong7;
    bBestSongs[7] = ui->bBestSong8;
    connect(ui->bChangeServer, &QPushButton::clicked, this, &MainWindow::bChangeServerClicked);
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

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::vote(QString songTitle)
{
    this->currentVote = songTitle;
    refreshVoteGUI();
    emit ChangeVote(songTitle);
}

void MainWindow::refreshVoteGUI()
{
    for(int i=0; i<8 && i<songs.size(); i++)
    {
        if(this->songs[i] == this->currentVote)
            bBestSongs[i]->setStyleSheet("QToolButton { background-color: #A0A0FF; }");
        else
            bBestSongs[i]->setStyleSheet("");
    }


    for(int i=0; i<ui->lwSongs->count();i++)
    {
        if(this->songs[i] == this->currentVote)
            ui->lwSongs->item(i)->setBackground(QColor(160, 160, 255));
        else
            ui->lwSongs->item(i)->setBackground(Qt::white);
    }
}

//GUI slots
void MainWindow::bChangeServerClicked()
{
    emit showConnectWindow();
}

void MainWindow::hsVolumeChanged(int value)
{
    emit ChangeVolume((muted)? 0 : (float)value/100);
}

void MainWindow::bMuteClicked()
{
    muted = !muted;
    if(muted)
        ui->bMute->setIcon(*muttedIcon);
    else
        ui->bMute->setIcon(*unmuttedIcon);
    emit ChangeVolume((muted)? 0 : (float)ui->hsVolume->value()/100);
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
void MainWindow::onHide()
{
    hide();
}

void MainWindow::onShow()
{
    show();
}

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
        this->currentVote = "";
        refreshVoteGUI();
    }

}

void MainWindow::onServerNameChanged(QString newServerName)
{
    ui->lRoomName->setText(newServerName);
}

QString MainWindow::addNewLines(QString text, int maxWidth)
{
    for(int i=maxWidth; i<text.length(); i+=maxWidth)
    {
        text.insert(i, '\n');
    }
    return text;
}

void MainWindow::onSongsListChanged(QVector<QString> newSongsList)
{
    this->songs.clear();
    this->songs.append(newSongsList);

    //gui refresh
    ui->lwSongs->clear();
    for(int i=0;i<this->songs.size();i++)
    {
        ui->lwSongs->addItem(songs[i] + " (- votes)");
    }
    for(int i=0; i<8&&i<this->songs.size();i++)
    {
        bBestSongs[i]->setEnabled(true);
        bBestSongs[i]->setText(addNewLines(songs[i], 18) + "\n- votes");
    }
    for(int i=this->songs.size(); i<8;i++)
        bBestSongs[i]->setEnabled(false);
}

void MainWindow::onUsersListChanged(QVector<QString> newUsersList)
{
    ui->lwUsers->clear();
    for(int i=0;i<newUsersList.size();i++)
        ui->lwUsers->addItem(newUsersList[i]);
}


void MainWindow::onSongsVotesChanged(QVector<QString> songs, QVector<uint> votes)
{
    for(int i=0; i<songs.size(); i++)
    {
        if(this->songs[i] != songs[i])
            this->songs[i] = songs[i];
        if(i<8)
            bBestSongs[i]->setText(addNewLines(songs[i], 18) + "\n" +  QString::number(votes[i]) + " votes");
        ui->lwSongs->item(i)->setText(songs[i] + " (" +  QString::number(votes[i]) + " votes)");
    }
    refreshVoteGUI();
}


