#include "connectwindow.h"
#include "ui_connectwindow.h"

ConnectWindow::ConnectWindow(QWidget *parent) : QMainWindow(parent),ui(new Ui::ConnectWindow)
{
    ui->setupUi(this);
    connect(ui->bConnect, &QPushButton::clicked, this, &ConnectWindow::connectToServer);
}

void ConnectWindow::connectToServer()
{
    ui->lStatus->setText("connecting...");
    MusicPlayer *playThread = new MusicPlayer();
    NetThread *netThread = new NetThread(this);
    netThread->setMusicPlayer(playThread);
    netThread->start();
    MainWindow *mw = new MainWindow(this);
    mw->setPlayThread(playThread);
    mw->setNetThread(netThread);
    netThread->connectToServer(ui->leServerAdress->text(), ui->leServerPort->text(), ui->leUserName->text());
    ui->lStatus->setText("connected");

    //change window
    mw->show();
    this->hide();
}

ConnectWindow::~ConnectWindow()
{
    delete ui;
}
