#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H

#include <QThread>
#include <QAudioSink>
#include <QAudioSink>
#include <QFile>
#include <iostream>
#include <cmath>
#include <fstream>
#include <QThread>
#include <QtCore>

class MusicPlayer : public QObject
{
    Q_OBJECT
public:
    explicit MusicPlayer();
    void setUp(int sampleRate, int sampleSize, int playBufferSize, float timeUpdateRate, float deviceBufferUsage);
    void play(char *buffer, int position);
    bool pause = false;
    float volume = 1;
    bool setuped = false;

private:
    int sampleSize;
    int playBufferSize;
    int devBufferUnusedSpace;
    int timeUpdateDistance;
    float positionToSecondsMultiplayer;
    QAudioSink* audio;
    QIODevice* dev;

public slots:
    void onChangeVolume(float);

signals:
    void TimeChanged(float);
};

#endif // MUSICPLAYER_H
