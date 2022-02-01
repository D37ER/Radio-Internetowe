#include "playthread.h"
using namespace std;

PlayThread::PlayThread() : QObject()
{
    this->sampleRate = 100000;
    this->sampleSize = 2;
    this->format.setSampleRate(sampleRate);
    this->format.setChannelCount(1);
    this->format.setSampleFormat(QAudioFormat::Int16);
    this->audio = new QAudioSink(format, this);
    this->bufferSize = sampleRate/100;
    this->devBufferUnusedSpace = audio->bufferSize()*9/10;
    this->position=0;
    this->positionUpdate = sampleSize*sampleRate/bufferSize/10;
    this->positionToSecondsMultiplayer = ((float)bufferSize)/sampleSize/sampleRate;
    //float lengthInSeconds = ((float)sourceFile.size())/sampleSize/sampleRate;
    this->pauseStatus = false;
    this->dev = audio->start();
}

void PlayThread::play(char buffer[])
{
    //pause
    while(this->pause);

    //send time
    position++;
    if(position%positionUpdate==0)
        emit TimeChanged(position*positionToSecondsMultiplayer);
        //cout << setprecision(3) << position*positionToSecondsMultiplayer << "s /" << lengthInSeconds << "s " << endl;

    //volume change
    short t;
    for(int i=0; i<bufferSize;i+=sampleSize)
    {
        memcpy(&t, &buffer[i], sampleSize);
        t *= this->volume;
        memcpy(&buffer[i], &t, sampleSize);
    }


    //send to speaker
    while(audio->bytesFree()-devBufferUnusedSpace < bufferSize);
    dev->write(buffer, bufferSize);
}
