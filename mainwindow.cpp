#include "mainwindow.h"
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->bPlay, &QPushButton::clicked, this, &MainWindow::play);
    connect(ui->hsVolume, &QSlider::valueChanged, this, &MainWindow::hsVolumeChanged);
    connect(ui->bMute, &QPushButton::clicked, this, &MainWindow::bMuteClicked);
    playThread = new PlayThread(this);
    connect(playThread, SIGNAL(TimeChanged(float,float)), this, SLOT(onTimeChanged(float,float)));
    connect(playThread, SIGNAL(SongChanged(QString)), this, SLOT(onSongChanged(QString)));
}

void MainWindow::play()
{
    if(!playThread->isRunning())
    {
        playThread->pause = false;
        playThread->start();
    }
    else
    {
        playThread->pause = !playThread->pause;
    }

}

void MainWindow::onTimeChanged(float time, float length)
{
    ui->pbTime->setRange(0,length*100);
    ui->pbTime->setValue(time*100);

    int timeInt = (int)time;
    int timeSec = timeInt%60;
    QString timeSecStr = (timeSec < 10)? "0"+QString::number((int)timeSec) : QString::number((int)timeSec);
    int timeMin = timeInt/60;
    QString timeMinStr = (timeMin < 10)? "0"+QString::number((int)timeMin) : QString::number((int)timeMin);

    int lengthInt = (int)length;
    int lengthSec = lengthInt%60;
    QString lengthSecStr = (lengthSec < 10)? "0"+QString::number((int)lengthSec) : QString::number((int)lengthSec);
    int lengthMin = lengthInt/60;
    QString lengthMinStr = (lengthMin < 10)? "0"+QString::number((int)lengthMin) : QString::number((int)lengthMin);

    ui->lTime->setText(timeMinStr + ":" + timeSecStr + " / " + lengthMinStr + ":" + lengthSecStr);
}

void MainWindow::onSongChanged(QString newTitle)
{
    ui->lSongTitle->setText(newTitle);
}

MainWindow::~MainWindow()
{
    delete ui;
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

