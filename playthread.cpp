#include "playthread.h"
using namespace std;

PlayThread::PlayThread(QObject *parent) : QThread(parent)
{

}

void PlayThread::run()
{
    cout << "running" << endl;
    int sampleRate = 88200;
    int sampleSize = 2;
    QAudioSink* audio;
    QAudioFormat format;
    format.setSampleRate(sampleRate);
    format.setChannelCount(1);
    format.setSampleFormat(QAudioFormat::Int16);
    audio = new QAudioSink(format, this);

    QFile sourceFile;
    {
        QIODevice* dev = audio->start();



        sourceFile.setFileName("D:/usunMnie/test2.wav");
        sourceFile.open(QIODevice::ReadOnly);
        cout << "File raport: exists "<< sourceFile.exists() << " fileSize "<< sourceFile.size() << endl;
        emit SongChanged("D:/usunMnie/test2.wav");

        int bufferSize = sampleRate/100;
        int devBufferUnusedSpace = audio->bufferSize()*9/10;
        int position=0;
        int positionUpdate = sampleSize*sampleRate/bufferSize/10;
        float positionToSecondsMultiplayer = ((float)bufferSize)/sampleSize/sampleRate;
        float lengthInSeconds = ((float)sourceFile.size())/sampleSize/sampleRate;
        char buffer[bufferSize];
        bool pauseStatus = false;
        while(!sourceFile.atEnd())
        {
            //pause
            do
            {
                QMutex mutex;
                mutex.lock();
                pauseStatus = this->pause;
                mutex.unlock();
            }while(pauseStatus);

            //read
            sourceFile.read(buffer, bufferSize);

            //send time
            position++;
            if(position%positionUpdate==0)
                emit TimeChanged(position*positionToSecondsMultiplayer, lengthInSeconds);
                //cout << setprecision(3) << position*positionToSecondsMultiplayer << "s /" << lengthInSeconds << "s " << endl;

            //volume change
            QMutex mutex;
            mutex.lock();
            short t;
            for(int i=0; i<bufferSize;i+=sampleSize)
            {
                memcpy(&t, &buffer[i], sampleSize);
                t *= this->volume;
                memcpy(&buffer[i], &t, sampleSize);
            }
            mutex.unlock();


            //send to speaker
            while(audio->bytesFree()-devBufferUnusedSpace < bufferSize);
            dev->write(buffer, bufferSize);
        }
    }
}
