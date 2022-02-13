#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include "AudioFile.h"

using namespace std;

void handleError(int error, const char *comment)
{
    if(error <0)
    {
        cout << "some error" << endl;
        cout << errno << endl;
        cout << comment << endl;
        exit(1);
    }
}

int main()
{
    int PORT = 1234;
    cout << "starting" << endl;

    int listenfd, udpfd, connfd;
    struct sockaddr_in servaddr;
    char buff[255];
    char recvline[255];
    handleError((listenfd = socket(AF_INET, SOCK_STREAM, 0)), "create tcp socket");
    cout << "created tcp socket, listenfd = " << listenfd << endl;

    handleError((udpfd = socket(AF_INET, SOCK_DGRAM, 0)), "create udp socket");
    cout << "created udp socket, udpfd = " << listenfd << endl;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    handleError(bind(listenfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind tcp");
    cout << "binded tcp" << endl;

    handleError(bind(udpfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind udp");
    cout << "binded udp" << endl;

    handleError(listen(listenfd,10), "listen");
    cout << "listening" << endl;

    while(true)
    {
        struct sockaddr_in clientAddr;
        bzero(&clientAddr, sizeof(clientAddr));
        socklen_t clientAddrLen = sizeof(clientAddr);

        cout << "waiting for new connection" << endl;
        fflush(stdout);

        connfd = accept(listenfd, ( struct sockaddr *) &clientAddr, &clientAddrLen);
        cout << "accepted, connfd = " << connfd << endl;
        char clientAddrStr[255];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientAddrStr, sizeof(clientAddrStr));
        cout << "new client address = " << clientAddrStr << " : " << ntohs(clientAddr.sin_port) << endl;

        memset(recvline, 0, 254);
        int n;
        int inSize = -1;
        while( (n = read(connfd, recvline, 253)) > 0)
        {
            cout << "n = " << n << endl;

            for(int i=0; i<n; i++)
                if(recvline[i] == '\n')
                {
                    inSize = i;
                    break;
                }
            if(inSize > -1)
                break;
        }
        cout << "inSize = " << inSize << endl;
        cout << "recvline = " << recvline << endl;
        snprintf((char*)buff, sizeof(buff), "aserver A\n");
        write(connfd, (char*)buff, strlen((char*)buff));
        cout << "response send" << endl;

        bzero(buff, sizeof(buff));
        buff[0] = 'd';
        snprintf((char*)(buff+1), sizeof(buff)-1, "user A");
        snprintf((char*)(buff+8), sizeof(buff)-1, "user B");
        snprintf((char*)(buff+14), sizeof(buff)-1, "\n");
 /*     for(int i=0;i<255;i++)
        {
            cout << buff[i];
        }
        cout << endl;
        for(int i=0;i<255;i++)
        {
            cout << (int) buff[i] << endl;
        }*/
        write(connfd, (char*)buff, 15);
        cout << "response send" << endl;

        bzero(buff, sizeof(buff));
        buff[0] = 'b';
        snprintf((char*)(buff+1), sizeof(buff)-1, "song 1");
        snprintf((char*)(buff+8), sizeof(buff)-1, "song 2");
        snprintf((char*)(buff+14), sizeof(buff)-1, "\n");
        write(connfd, (char*)buff, 15);
        cout << "response send" << endl;

        bzero(buff, sizeof(buff));
        buff[0] = 'e';
        buff[1] = 1;
        buff[2] = 2;
        buff[3] = 3;
        buff[4] = 4;
        snprintf((char*)(buff+5), sizeof(buff)-1, "song 1");
        buff[12] = 2;
        buff[13] = 3;
        buff[14] = 4;
        buff[15] = 5;
        snprintf((char*)(buff+16), sizeof(buff)-1, "song 2");
        snprintf((char*)(buff+22), sizeof(buff)-1, "\n");
        write(connfd, (char*)buff, 23);
        cout << "response send" << endl;


        float len = 228.066;
        bzero(buff, sizeof(buff));
        buff[0] = 'f';
        memcpy((char*)(buff+1), &len, sizeof(len));
        snprintf((char*)(buff+5), sizeof(buff)-1, "song 1");
        snprintf((char*)(buff+11), sizeof(buff)-1, "\n");
        write(connfd, (char*)buff, 12);
        cout << "response send" << endl;

        cout << "sending song via utp" << endl;

        AudioFile<double> audioFile;
        audioFile.load("song1.wav");

        int sampleRate = audioFile.getSampleRate();
        int bitDepth = audioFile.getBitDepth();

        int numSamples = audioFile.getNumSamplesPerChannel();
        double lengthInSeconds = audioFile.getLengthInSeconds();

        int numChannels = audioFile.getNumChannels();
        bool isMono = audioFile.isMono();
        bool isStereo = audioFile.isStereo();

        audioFile.printSummary();

        cout << "sampleRate = " << sampleRate << endl;
        cout << "bitDepth = " << bitDepth << endl;
        cout << "numSamples = " << numSamples << endl;
        cout << "lengthInSeconds = " << lengthInSeconds << endl;
        cout << "numChannels = " << numChannels << endl;
        cout << "isMono = " << isMono << endl;
        cout << "isStereo = " << isStereo << endl;

        int channel = 0;
        double sample;
        short int16Sample;
        char outBuffer[4100];
        for (int i = 0; i < numSamples/2048; i++)
        {
            memcpy((char*)(outBuffer), &i, 4);
            for(int j = 0; j<2048; j++)
            {
                sample = audioFile.samples[channel][i*2048 + j];
                int16Sample = sample*32767;
                memcpy((char*)(outBuffer + 2*j + 4), &int16Sample, 2);
            }
            sendto(udpfd, (const char *)outBuffer, 4096, MSG_CONFIRM, (const struct sockaddr *) &clientAddr, clientAddrLen);
            usleep(40000);
        }

        close(connfd);
        cout << "closed" << endl;
    }

    cout << "closing" << endl;
    return 0;
}
