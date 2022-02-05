#include "musicplayer.h"
using namespace std;

MusicPlayer::MusicPlayer() : QObject()
{

}

void MusicPlayer::setUp(int sampleRate, int sampleSize, int playBufferSize, float timeUpdateRate, float deviceBufferUsage)
{
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    if(sampleSize == 1)
        format.setSampleFormat(QAudioFormat::UInt8);
    else if(sampleSize == 2)
        format.setSampleFormat(QAudioFormat::Int16);
    else if(sampleSize == 4)
        format.setSampleFormat(QAudioFormat::Int32);
    this->audio = new QAudioSink(format, this);

    this->sampleSize = sampleSize;
    this->playBufferSize = playBufferSize;
    this->timeUpdateDistance = sampleSize*sampleRate/playBufferSize*timeUpdateRate;
    this->positionToSecondsMultiplayer = ((float)playBufferSize)/sampleSize/sampleRate;
    this->devBufferUnusedSpace = audio->bufferSize() *(1-deviceBufferUsage);
    this->position = 0;

    this->dev = audio->start();
}

void MusicPlayer::play(char *buffer)
{
    //pause
    while(this->pause);

    //emit time
    position++;
    if(position%timeUpdateDistance==0)
        emit TimeChanged(position*positionToSecondsMultiplayer);

    //volume change
    short t;
    for(int i=0; i<playBufferSize;i+=sampleSize)
    {
        memcpy(&t, &buffer[i], sampleSize);
        t *= this->volume;
        memcpy(&buffer[i], &t, sampleSize);
    }

    //send data to speaker
    while(audio->bytesFree()-devBufferUnusedSpace < playBufferSize);
    dev->write(buffer, playBufferSize);
}
