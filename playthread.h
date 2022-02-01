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

class PlayThread : public QObject
{
    Q_OBJECT
public:
    explicit PlayThread();
    void run();
    void play(char buffer[]);
    bool pause = false;
    float volume = 1;

private:
    int sampleRate;
    int sampleSize;
    QAudioSink* audio;
    QAudioFormat format;
    int bufferSize;
    int devBufferUnusedSpace;
    int position;
    int positionUpdate;
    float positionToSecondsMultiplayer;
    bool pauseStatus;
    QIODevice* dev;

signals:
    void TimeChanged(float);

};

#endif // PLAYTHREAD_H
