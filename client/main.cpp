#include "connectwindow.h"
#include "mysongswindow.h"
#include "mainwindow.h"
#include "musicplayer.h"
#include "netthread.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ConnectWindow *connectWindow = new ConnectWindow(nullptr);
    MainWindow *mainWindow = new MainWindow(nullptr);

    NetThread *netThread = new NetThread(nullptr);
    QObject::connect(connectWindow, SIGNAL(ConnectToServer(QString, QString, QString)), netThread, SLOT(onConnectToServer(QString, QString, QString)));
    QObject::connect(netThread, SIGNAL(ConnectionStateChanged(QString)), connectWindow, SLOT(onChangeStatus(QString)));
    QObject::connect(netThread, SIGNAL(ConnectionError(int, QString)), connectWindow, SLOT(onShowError(int, QString)));
    QObject::connect(netThread, SIGNAL(ConnectionSuccessful()), connectWindow, SLOT(onHide()));
    QObject::connect(netThread, SIGNAL(ConnectionSuccessful()), mainWindow, SLOT(onShow()));
    QObject::connect(netThread, SIGNAL(ServerNameChanged(QString)), mainWindow, SLOT(onServerNameChanged(QString)));
    QObject::connect(netThread, SIGNAL(SongsListChanged(QVector<QString>)), mainWindow, SLOT(onSongsListChanged(QVector<QString>)));
    QObject::connect(netThread, SIGNAL(UsersListChanged(QVector<QString>)), mainWindow, SLOT(onUsersListChanged(QVector<QString>)));
    QObject::connect(netThread, SIGNAL(SongsVotesChanged(QVector<QString>, QVector<uint>)), mainWindow, SLOT(onSongsVotesChanged(QVector<QString>, QVector<uint>)));
    QObject::connect(netThread, SIGNAL(SongChanged(QString, float)), mainWindow, SLOT(onSongChanged(QString, float)));
    QObject::connect(mainWindow, SIGNAL(ChangeVote(QString)), netThread, SLOT(onChangeVote(QString)));
    QObject::connect(mainWindow, SIGNAL(SendSong(QString, QString)), netThread, SLOT(onSendSong(QString, QString)));
    QObject::connect(mainWindow, SIGNAL(showConnectWindow()), connectWindow, SLOT(onShow()));
    QObject::connect(netThread, SIGNAL(TimeChanged(float)), mainWindow, SLOT(onTimeChanged(float)));
    QObject::connect(mainWindow, SIGNAL(ChangeVolume(float)), netThread, SLOT(onChangeVolume(float)));

    netThread->start();

    connectWindow->show();
    return a.exec();
}
