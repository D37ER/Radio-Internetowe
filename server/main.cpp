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
#define NEW_SONG_INFO_MSG 'f'

#define TCP_OUT_BUFFER_MAX_SIZE 255

#define PACK_SIZE_IN_SAMPLES 2048
#define SMAPLE_SIZE 2

//server name
char serverName[11] = "server ABC";

//songs list
char *songsNames[255];
char *songsFileNames[255];
uint songsVotes[255];
int songsCount = 0;

//users list
int usersTcpFd[255];
struct sockaddr_in usersAddresses[255];
socklen_t usersAddressesSizes[255];
char *usersNames[255];
int usersVotes[255];
int usersCount = 0;

void sendNewSongInfo(int fd, int sampleCount, int sampleRate, int packSize, char *name, int nameLen);

void addUser(int tcpFd, struct sockaddr_in address, socklen_t addressSize, char *name)
{
    usersTcpFd[usersCount] = tcpFd;
    usersAddresses[usersCount] = address;
    usersAddressesSizes[usersCount] = addressSize;
    usersNames[usersCount] = name;
    usersVotes[usersCount] = -1;
    usersCount++;
}

void addSong(char *name, char *fileName, uint votes)
{
    songsNames[songsCount] = name;
    songsFileNames[songsCount] = fileName;
    songsVotes[songsCount] = votes;
    songsCount++;
}

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

void setup(int *tcpfd, int *udpfd, int port)
{
    chrono::high_resolution_clock::time_point startupStart = chrono::high_resolution_clock::now();
    cout << endl << "=========================================================" << endl;
    cout << "starting server" << endl;

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    handleError((*tcpfd = socket(AF_INET, SOCK_STREAM, 0)), "create tcp socket");
    cout << "(1/5) created tcp socket, tcpfd = " << *tcpfd << endl;

    handleError((*udpfd = socket(AF_INET, SOCK_DGRAM, 0)), "create udp socket");
    cout << "(2/5) created udp socket, udpfd = " << *udpfd << endl;

    handleError(bind(*tcpfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind tcp");
    cout << "(3/5) binded tcp socket to port " << port << endl;

    handleError(bind(*udpfd,(sockaddr *) &servaddr, sizeof(servaddr)), "bind udp");
    cout << "(4/5) binded udp socket to port " << port<< endl;

    handleError(listen(*tcpfd,10), "listen");
    cout << "(5/5) listening for tcp connection on port " << port << endl;

    chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
    chrono::duration<double> startupTime = now-startupStart;
    cout << "startup ended in " << startupTime.count() << " sec, waiting for connections" << endl << endl;
}

void loadNewSong(AudioFile<double> *currentSong, char *fileName, char* songName, int packSizeInSamples, int sampleSize, int *usersfd, int usersCount)
{
    currentSong->load(fileName);

    //displaying in console
    cout << endl << "new song info:" << endl;
    currentSong->printSummary();
    cout << endl;

    for(int i=0; i<usersCount; i++)
        sendNewSongInfo(usersfd[i], currentSong->getNumSamplesPerChannel(), currentSong->getSampleRate(), packSizeInSamples*sampleSize, songName, strlen(songName));
}

//SEND TCP
void sendServerName(int fd, char *name, int nameLen)
{
    char msg[nameLen+2];
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

void sendNewSongInfo(int fd, int sampleCount, int sampleRate, int packSize, char *name, int nameLen)
{
    char msg[nameLen+14]; // 1bytes*[NEW_SONG_INFO_MSG]  4bytes[sampleCount]  4bytes*[sampleRate] 4bytes[packSize]  nameLen bytes*[name]  1bytes*[MSG_END]
    bzero(&msg, nameLen+13);
    msg[0] = NEW_SONG_INFO_MSG;

    memcpy(msg+1, &sampleCount, nameLen);
    memcpy(msg+5, &sampleRate, nameLen);
    memcpy(msg+9, &packSize, nameLen);
    memcpy(msg+13, name, nameLen);

    msg[nameLen+13] = MSG_END;
    int sentBytes = write(fd, msg, nameLen+14);
    if(sentBytes != nameLen+14)
        cout << "WARNING sent only " << sentBytes << "bytes expected " << nameLen+2 << "bytes" << endl;
    cout << "new song info name send to " << fd << endl;
}

//SEND UTP

void sendSongSamples(AudioFile<double> currentSong, int packSizeInSamples, int sampleSize, int udpfd, struct sockaddr_in *clientsAddresses,  socklen_t *clientsAddressesLengths, int usersCount)
{
    //sending song samples
    cout << "sending song via utp" << endl;
    int numSamples = currentSong.getNumSamplesPerChannel();
    int sampleRate = currentSong.getSampleRate();
    int packSizeBytes = sampleSize*packSizeInSamples; //pack size in bytes [+4 bytes for int32 index]
    float packLengthInSec = ((float)packSizeInSamples)/sampleRate; //pack size in seconds
    int packCount = numSamples/packSizeInSamples; //number of packets in song
    int channel = 0; //samples channel
    short sample; //current sample
    char packBuffer[packSizeBytes+4]; //current sample pack
    int waitTime; //time to wait for next pack in microseconds
    chrono::high_resolution_clock::time_point timerStart = chrono::high_resolution_clock::now();
    for (int i = 0; i < packCount; i++)
    {
        //put samples into pack
        memcpy((char*)(packBuffer), &i, 4);
        for(int j = 0; j<packSizeInSamples; j++)
        {
            sample = currentSong.samples[channel][i*packSizeInSamples + j]*32767;
            memcpy((char*)(packBuffer + 2*j + 4), &sample, 2);
        }

        //send pack
        for(int i=0; i<usersCount; i++)
            sendto(udpfd, (const char *)packBuffer, packSizeBytes, MSG_CONFIRM, (const struct sockaddr *) &(clientsAddresses[i]), clientsAddressesLengths[i]);

        //wait for next pack
        chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
        chrono::duration<double> currentSongTime = now-timerStart;
        waitTime = 1000000*((i+1)*packLengthInSec - currentSongTime.count());
        if(waitTime < 0)
            waitTime = 0;
        usleep(waitTime);
    }
}

int main()
{

    //add song0
    char songName0[7] = "song 0";
    char songFileName0[10] = "song0.wav";
    uint songVotes0 = 100;
    addSong(songName0, songFileName0, songVotes0);

    //add song1
    char songName1[7] = "song 1";
    char songFileName1[10] = "song1.wav";
    uint songVotes1 = 200;
    addSong(songName1, songFileName1, songVotes1);

    //add song2
    char songName2[7] = "song 2";
    char songFileName2[10] = "song2.wav";
    uint songVotes2 = 300;
    addSong(songName2, songFileName2, songVotes2);

    int tcpfd, udpfd;
    setup(&tcpfd, &udpfd, SERVER_PORT);

    while(true)
    {
        cout << "waiting for new connection" << endl;
        fflush(stdout);

        //accepting connection
        struct sockaddr_in clientAddr;
        bzero(&clientAddr, sizeof(clientAddr));
        socklen_t clientAddrLen = sizeof(clientAddr);
        int userTcpFd;
        char recvline[255];
        userTcpFd = accept(tcpfd, (struct sockaddr *) &clientAddr, &clientAddrLen);

        //displaying new user info
        char clientAddrStr[50];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientAddrStr, sizeof(clientAddrStr));
        cout << "new client, userTcpFd = " << userTcpFd << ", address = " << clientAddrStr << ":" << ntohs(clientAddr.sin_port) << endl;

        //receiving user name
        bzero(recvline, sizeof(recvline));
        int n;
        int inSize = -1;
        while( (n = read(userTcpFd, recvline, 253)) > 0)
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
        recvline[inSize] = '\0';
        cout << "username = \"" << recvline << "\"" << endl;

        addUser(userTcpFd, clientAddr, clientAddrLen, recvline);
        sendServerName(userTcpFd, serverName, strlen(serverName));
        sendUsersList(userTcpFd, usersNames, usersCount); //TODO to all users
        sendSongsList(userTcpFd, songsNames, songsCount);
        sendVotesList(userTcpFd, songsVotes, songsNames, songsCount);

        AudioFile<double> currentSong;
        int currentSongId = 1;
        loadNewSong(&currentSong, songsFileNames[currentSongId], songsNames[currentSongId], PACK_SIZE_IN_SAMPLES, SMAPLE_SIZE, usersTcpFd, usersCount);
        sendSongSamples(currentSong, PACK_SIZE_IN_SAMPLES, SMAPLE_SIZE, udpfd, usersAddresses, usersAddressesSizes, usersCount);
    }

    cout << "closing" << endl;
    return 0;
}
