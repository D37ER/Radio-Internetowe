#ifndef PLAYTHREAD_H
#define PLAYTHREAD_H

#include <QThread>
#include <QAudioSink>
#include <QAudioSink>
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
#include <QtCore>

class PlayThread : public QThread
{
    Q_OBJECT
public:
    explicit PlayThread(QObject *parent = 0);
    void run();
    bool pause;

private:


signals:
    void TimeChanged(float,float);
    void SongChanged(QString);

};

#endif // PLAYTHREAD_H
