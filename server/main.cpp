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
#include <chrono>
#include "AudioFile.h"

using namespace std;

#define SERVER_PORT 1234
#define MSG_END '\n'
#define MSG_VARIABLE_DELIMITER '\0'
#define SERVER_NAME_MSG 'a'
#define SONGS_LIST_MSG 'b'
#define MY_SONGS_LIST_MSG 'c'
#define USERS_LIST_MSG 'd'
#define VOTES_LIST_MSG 'e'
#define NEW_SONG_MSG 'f'

#define TCP_OUT_BUFFER_MAX_SIZE 255

void handleError(int error, const char *comment)
{
    if(error <0)
    {
        cout << "ERROR" << endl;
        cout << "errno = "<< errno << endl;
        cout << "comment = "<< comment << endl;
        fflush(stdout);
        exit(1);
    }
}

void sendServerName(int fd, char *name, int nameLen)
{
    char msg[nameLen+1];
    bzero(&msg, nameLen+1);
    msg[0] = SERVER_NAME_MSG;
    memcpy(msg+1, name, nameLen);
    msg[nameLen+1] = MSG_END;
    int sentBytes = write(fd, msg, nameLen+2);
    if(sentBytes != nameLen+2)
        cout << "WARNING sent only " << sentBytes << "bytes expected " << nameLen+2 << "bytes" << endl;
    cout << "server name send to " << fd << endl;
}

void sendStringList(int fd, char **strings, int stringCount, char msgType, char *listName)
{
    char msg[TCP_OUT_BUFFER_MAX_SIZE];
    int msgLen = 0;
    bzero(&msg, TCP_OUT_BUFFER_MAX_SIZE);
    msg[msgLen++] = msgType;

    for(int i=0; i<stringCount; i++)
    {
        memcpy(msg + msgLen, strings[i], strlen(strings[i]));
        msgLen += strlen(strings[i]);
        if(i != stringCount-1)
            msg[msgLen++] = MSG_VARIABLE_DELIMITER;
    }

    msg[msgLen++] = MSG_END;
    int sentBytes = write(fd, msg, msgLen);
    if(sentBytes != msgLen)
        cout << "WARNING sent only " << sentBytes << "bytes expected " << msgLen << "bytes" << endl;
    cout << listName <<" list send to " << fd << endl;
}

void sendSongsList(int fd, char **songs, int songsCount)
{
    char name[6] = "songs";
    sendStringList(fd, songs, songsCount, SONGS_LIST_MSG, name);
}

void sendMySongsList(int fd, char **songs, int songsCount)
{
    char name[9] = "my songs";
    sendStringList(fd, songs, songsCount, MY_SONGS_LIST_MSG, name);
}

void sendUsersList(int fd, char **users, int usersCount)
{
    char name[6] = "users";
    sendStringList(fd, users, usersCount, USERS_LIST_MSG, name);
}

void sendVotesList(int fd, uint *votes, char **songs, int songsCount)
{
    char msg[TCP_OUT_BUFFER_MAX_SIZE];
    int msgLen = 0;
    bzero(&msg, TCP_OUT_BUFFER_MAX_SIZE);
    msg[msgLen++] = VOTES_LIST_MSG;

    for(int i=0; i<songsCount; i++)
    {
        memcpy(msg + msgLen, &(votes[i]), 4);
        msgLen += 4;
        memcpy(msg + msgLen, songs[i], strlen(songs[i]));
        msgLen += strlen(songs[i]);
        if(i != songsCount-1)
            msg[msgLen++] = MSG_VARIABLE_DELIMITER;
    }

    msg[msgLen++] = MSG_END;
    int sentBytes = write(fd, msg, msgLen);
    if(sentBytes != msgLen)
        cout << "WARNING sent only " << sentBytes << "bytes expected " << msgLen << "bytes" << endl;
    cout << "votes list send to " << fd << endl;
}

void sendSongInfo(int fd, char **users, int usersCount)
{
    char name[6] = "users";
    sendStringList(fd, users, usersCount, USERS_LIST_MSG, name);
}

int main()
{
    chrono::high_resolution_clock::time_point startupStart = chrono::high_resolution_clock::now();
    cout << endl << "=========================================================" << endl;
    cout << "starting server" << endl;

    int listenfd, udpfd, connfd;
    struct sockaddr_in servaddr;
    char buff[255];
    char recvline[255];
    handleError((listenfd = socket(AF_INET, SOCK_STREAM, 0)), "create tcp socket");
    cout << "(1/5) created tcp socket, listenfd = " << listenfd << endl;

    handleError((udpfd = socket(AF_INET, SOCK_DGRAM, 0)), "create udp socket");
    cout << "(2/5) created udp socket, udpfd = " << listenfd << endl;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERVER_PORT);

    handleError(bind(listenfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind tcp");
    cout << "(3/5) binded tcp socket to port " << SERVER_PORT << endl;

    handleError(bind(udpfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind udp");
    cout << "(4/5) binded udp socket to port " << SERVER_PORT<< endl;

    handleError(listen(listenfd,10), "listen");
    cout << "(5/5) listening for tcp connection on port " << SERVER_PORT << endl;

    chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
    chrono::duration<double> startupTime = now-startupStart;
    cout << "startup ended in " << startupTime.count() << " sec, waiting for connections" << endl << endl;

    while(true)
    {

        cout << "waiting for new connection" << endl;
        fflush(stdout);

        struct sockaddr_in clientAddr;
        bzero(&clientAddr, sizeof(clientAddr));
        socklen_t clientAddrLen = sizeof(clientAddr);
        connfd = accept(listenfd, ( struct sockaddr *) &clientAddr, &clientAddrLen);
        char clientAddrStr[50];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientAddrStr, sizeof(clientAddrStr));
        cout << "new client, fd = " << connfd << ", address = " << clientAddrStr << ":" << ntohs(clientAddr.sin_port) << endl;

        //receiving username
        bzero(recvline, sizeof(recvline));
        int n;
        int inSize = -1;
        while( (n = read(connfd, recvline, 253)) > 0)
        {
            //cout << "n = " << n << endl;

            for(int i=0; i<n; i++)
                if(recvline[i] == '\n')
                {
                    inSize = i;
                    break;
                }
            if(inSize > -1)
                break;
        }
        //cout << "inSize = " << inSize << endl;
        recvline[inSize] = '\0';
        cout << "username = \"" << recvline << "\"" << endl;

        //server name
        char serverName[11] = "server ABC";
        sendServerName(connfd, serverName, strlen(serverName));

        //users list
        char *users[255];
        int usersCount = 0;
        char user1[18] = "short user name A";
        users[usersCount++] = user1;
        char user2[18] = "short user name B";
        users[usersCount++] = user2;
        char user3[29] = "longer user name ABCDEFGHIJK";
        users[usersCount++] = user3;
        sendUsersList(connfd, users, usersCount);

        //songs list
        char *songs[255];
        int songsCount = 0;
        char song1[18] = "short song name A";
        songs[songsCount++] = song1;
        char song2[18] = "short song name B";
        songs[songsCount++] = song2;
        char song3[29] = "longer song name ABCDEFGHIJK";
        songs[songsCount++] = song3;
        sendSongsList(connfd, songs, songsCount);

        //songs votes
        uint votes[255];
        votes[0] = 16*256+16;
        votes[1] = 8*256+8;
        votes[2] = 7;
        sendVotesList(connfd, votes, songs, songsCount);

        //reding song info from file
        cout << endl << "new song info:" << endl;
        AudioFile<double> audioFile;
        audioFile.load("song1.wav");
        audioFile.printSummary();
        cout << endl;

        //sending song info
        float len = (float)(audioFile.getLengthInSeconds());
        bzero(buff, sizeof(buff));
        buff[0] = NEW_SONG_MSG;
        memcpy((char*)(buff+1), &len, sizeof(len));
        snprintf((char*)(buff+5), sizeof(buff)-1, "song 1");
        snprintf((char*)(buff+11), sizeof(buff)-1, "\n");
        write(connfd, (char*)buff, 12);
        cout << "response send" << endl;

        //sending song samples
        cout << "sending song via utp" << endl;
        int numSamples = audioFile.getNumSamplesPerChannel();
        int sampleRate = audioFile.getSampleRate();
        int sampleSize=2; //sample size in bytes
        int packSize = 2048; //pack size in samples
        int packSizeBytes = sampleSize*packSize; //pack size in bytes [+4 bytes for int32 index]
        float packLengthInSec = ((float)packSize)/sampleRate; //pack size in seconds
        int packCount = numSamples/packSize; //number of packets in song
        int channel = 0; //samples channel
        short sample; //current sample
        char packBuffer[packSizeBytes+4]; //current sample pack
        int waitTime; //time to wait for next pack in microseconds
        chrono::high_resolution_clock::time_point timerStart = chrono::high_resolution_clock::now();
        for (int i = 0; i < packCount; i++)
        {
            //put samples into pack
            memcpy((char*)(packBuffer), &i, 4);
            for(int j = 0; j<packSize; j++)
            {
                sample = audioFile.samples[channel][i*packSize + j]*32767;
                memcpy((char*)(packBuffer + 2*j + 4), &sample, 2);
            }

            //send pack
            sendto(udpfd, (const char *)packBuffer, packSizeBytes, MSG_CONFIRM, (const struct sockaddr *) &clientAddr, clientAddrLen);

            //wait for next pack
            chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
            chrono::duration<double> currentSongTime = now-timerStart;
            waitTime = 1000000*((i+1)*packLengthInSec - currentSongTime.count());
            if(waitTime < 0)
                waitTime = 0;
            usleep(waitTime);
        }
        close(connfd);
        cout << "closed" << endl;
    }

    cout << "closing" << endl;
    return 0;
}
