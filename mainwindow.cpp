#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAudioSink>
#include <QAudioSink>
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
using namespace std;

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->bPlay, &QPushButton::clicked, this, &MainWindow::play);
    playThread = new PlayThread(this);
    connect(playThread, SIGNAL(TimeChanged(float)), this, SLOT(onTimeChanged(float)));
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

void MainWindow::onTimeChanged(float time)
{
    ui->lName->setText(QString::number(time));
}

MainWindow::~MainWindow()
{
    delete ui;
}

