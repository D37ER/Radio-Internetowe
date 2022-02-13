#include "connectwindow.h"
#include "ui_connectwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    connect(ui->bConnect, &QPushButton::clicked, this, &ConnectWindow::connectToServer);
}

void ConnectWindow::connectToServer()
{
    ui->lStatus->setStyleSheet("QLabel {color : black; }");
    ui->lStatus->setText("connecting...");
    playThread = new MusicPlayer();
    netThread = new NetThread(this);
    netThread->setMusicPlayer(playThread);
    mw = new MainWindow(nullptr);
    mw->setPlayThread(playThread);
    mw->setNetThread(netThread);
    connect(netThread, &NetThread::ConnectionStateChanged, this, &ConnectWindow::changeStatus);
    connect(netThread, &NetThread::RoomChanged, this, &ConnectWindow::changeToMainWindow);
    connect(netThread, &NetThread::Error, this, &ConnectWindow::showError);
    netThread->connectToServer(ui->leServerAdress->text(), ui->leServerPort->text(), ui->leUserName->text());
}

void ConnectWindow::changeToMainWindow()
{
    netThread->start();
    mw->show();
    this->hide();
}

void ConnectWindow::changeStatus(QString status)
{
    ui->lStatus->setStyleSheet("QLabel {color : black; }");
    ui->lStatus->setText(status);
}

void ConnectWindow::showError(int number, QString msg)
{
    ui->lStatus->setStyleSheet("QLabel {color : red; }");
    ui->lStatus->setText("error " + QString::number(number) + " : " + msg);
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}
