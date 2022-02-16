#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
#include <pthread.h>
#include <climits>
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
#define MAX_USERS_COUNT 255
#define MAX_SONGS_COUNT 255

#define PACK_SIZE_IN_SAMPLES 2048
#define SAMPLE_SIZE 2

#define ACTIONS_QUEUE_SIZE 32

char serverName[11] = "server ABC";

//songs list
char *songsNames[MAX_SONGS_COUNT];
char *songsFileNames[MAX_SONGS_COUNT];
uint songsVotes[MAX_SONGS_COUNT];
int songsCount = 0;

//users list
int usersTcpFd[MAX_USERS_COUNT];
struct sockaddr_in usersAddresses[MAX_USERS_COUNT];
socklen_t usersAddressesSizes[MAX_USERS_COUNT];
char *usersNames[MAX_USERS_COUNT];
int usersVotes[MAX_USERS_COUNT];
int usersCount = 0;

//threads
pthread_t acceptThread;
pthread_t samplesThread;
pthread_t usersThreads[MAX_USERS_COUNT];
char actionsQueue[MAX_USERS_COUNT][ACTIONS_QUEUE_SIZE];
int actionsQueueStart[MAX_USERS_COUNT];
int actionsQueueEnd[MAX_USERS_COUNT];

//current song
AudioFile<double> loadedSong;
char *loadedSongName;
int loadedSongId = -1;


void sendNewSongInfo(int fd, int sampleCount, int sampleRate, int packSize, char *name, int nameLen);
void *userThreadDo(void *userId);
void addToActionsQueue(int userId, char action);

int addUser(int tcpFd, struct sockaddr_in address, socklen_t addressSize, char *name)
{
    if(songsCount >= MAX_USERS_COUNT)
    {
        cout << "WARNING Reached max users count" << endl;
        return -1;
    }
    usersTcpFd[usersCount] = tcpFd;
    usersAddresses[usersCount] = address;
    usersAddressesSizes[usersCount] = addressSize;
    usersNames[usersCount] = new char[strlen(name) +1];
    strcpy(usersNames[usersCount], name);
    usersVotes[usersCount] = -1;
    usersCount++;
    return 0;
}

int addSong(char *name, char *fileName, uint votes)
{
    if(songsCount >= MAX_SONGS_COUNT)
    {
        cout << "WARNING Reached max songs count" << endl;
        return -1;
    }
    songsNames[songsCount] = new char[strlen(name)+1];
    strcpy(songsNames[songsCount], name);
    songsFileNames[songsCount] = fileName;
    songsVotes[songsCount] = votes;
    songsCount++;
    return 0;
}

void changeVote(int userId, int newSongId)
{
    if(usersVotes[userId] != -1)
        songsVotes[usersVotes[userId]]--;

    usersVotes[userId] = newSongId;
    songsVotes[newSongId]++;
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

    bzero(actionsQueue, sizeof(actionsQueue));
    for(int i=0; i<MAX_USERS_COUNT; i++)
    {
        actionsQueueStart[i] = 0;
        actionsQueueEnd[i] = 0;
    }

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

void loadNewSong(AudioFile<double> *currentSong, char *fileName, char* songName)
{
    currentSong->load(fileName);
    loadedSongName = songName;

    //displaying song info in console
    cout << endl << "new song info:" << endl;
    currentSong->printSummary();
    cout << endl;

    //adding to actions queues
    for(int i=0; i<usersCount; i++)
            addToActionsQueue(i, NEW_SONG_INFO_MSG);
}

int nextSongId(uint *songsVotes, int songsCount)
{
    cout << "next song counting" << endl;
    if(songsCount <1)
        return -1;
    uint maxVotes = songsVotes[0];
    int maxIds[MAX_SONGS_COUNT];
    maxIds[0] = 0;
    int maxIdsCount = 1;

    for(int i=1; i<songsCount; i++)
    {
        if(songsVotes[i] > maxVotes)
        {
            maxVotes = songsVotes[i];
            maxIds[0] = i;
            maxIdsCount = 1;
        }
        else if(songsVotes[i] == maxVotes)
            maxIds[maxIdsCount++] = i;
    }
    int out = maxIds[rand()%maxIdsCount];
    cout << "next song counting" << endl;
    return out;
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
    uint votesCopy[songsCount];
    memcpy(votesCopy, votes, sizeof(uint)*songsCount);
    char msg[TCP_OUT_BUFFER_MAX_SIZE];
    int msgLen = 0;
    bzero(&msg, TCP_OUT_BUFFER_MAX_SIZE);
    msg[msgLen++] = VOTES_LIST_MSG;

    int maxId;
    for(int i=0; i<songsCount; i++)
    {
        maxId = 0;
        for(int j=1; j<songsCount; j++)
            if(votesCopy[maxId] == UINT_MAX || (votesCopy[j] != UINT_MAX && votesCopy[j] > votesCopy[maxId]))
                maxId = j;
        votesCopy[maxId] = UINT_MAX;

        memcpy(msg + msgLen, &(votes[maxId]), 4);
        msgLen += 4;
        memcpy(msg + msgLen, songs[maxId], strlen(songs[maxId]));
        msgLen += strlen(songs[maxId]);
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
    bzero(&msg, nameLen+14);
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
void sendSongSamples(AudioFile<double> *currentSong, int packSizeInSamples, int sampleSize, int udpfd, struct sockaddr_in *clientsAddresses,  socklen_t *clientsAddressesLengths, int *usersCount)
{
    //sending song samples
    cout << "sending song via utp" << endl;
    int numSamples = currentSong->getNumSamplesPerChannel();
    int sampleRate = currentSong->getSampleRate();
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
            sample = currentSong->samples[channel][i*packSizeInSamples + j]*32767;
            memcpy((char*)(packBuffer + 2*j + 4), &sample, 2);
        }

        //send pack
        for(int i=0; i < *usersCount; i++)
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

//THREADS
void addToActionsQueue(int userId, char action)
{
    //TODO zabezpieczenie przed powtórzeniem wartości
    actionsQueue[userId][actionsQueueEnd[userId]++] = action;
    if(actionsQueueEnd[userId] >= ACTIONS_QUEUE_SIZE)
        actionsQueueEnd[userId] = 0;
}

char removeFromActionsQueue(int userId)
{
    char out = actionsQueue[userId][actionsQueueStart[userId]++];
    if(actionsQueueStart[userId] >= ACTIONS_QUEUE_SIZE)
        actionsQueueStart[userId] = 0;
    return out;
}

void *acceptThreadDo(void *tcpFdVoid) //users -adding, usersCount -read, queues -insert
{
    while(true)
    {
        int tcpfd = *((int*) tcpFdVoid);
        cout << "waiting for new connection" << endl;
        fflush(stdout);

        //accepting connection
        struct sockaddr_in clientAddr;
        bzero(&clientAddr, sizeof(clientAddr));
        socklen_t clientAddrLen = sizeof(clientAddr);
        int userTcpFd;
        userTcpFd = accept(tcpfd, (struct sockaddr *) &clientAddr, &clientAddrLen);

        //displaying new user info
        char clientAddrStr[50];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientAddrStr, sizeof(clientAddrStr));
        cout << "new client, userTcpFd = " << userTcpFd << ", address = " << clientAddrStr << ":" << ntohs(clientAddr.sin_port) << endl;

        //receiving user name
        char recvline[255];
        bzero(recvline, sizeof(recvline));
        int n;
        int inSize = -1;
        while( (n = read(userTcpFd, recvline, 253)) > 0)
        {
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

        int userId = usersCount-1;
        addToActionsQueue(userId, SERVER_NAME_MSG);
        for(int i=0; i<usersCount; i++)
            addToActionsQueue(i, USERS_LIST_MSG);
        addToActionsQueue(userId, SONGS_LIST_MSG);
        addToActionsQueue(userId, VOTES_LIST_MSG);
        addToActionsQueue(userId, NEW_SONG_INFO_MSG);
        pthread_create(&(usersThreads[0]), NULL, &userThreadDo, (void *) &userId);
    }
    return NULL;
}

void *samplesThreadDo(void *udpFdVoid)
{
    int udpfd = *((int*) udpFdVoid);
    //usleep(15000000);
    cout << "starting music" << endl;
    while(true)
    {
        loadedSongId = nextSongId(songsVotes, songsCount);
        cout << "next song id " << loadedSongId << endl;
        songsVotes[loadedSongId] = 0;
        for(int i=0; i<usersCount; i++)
            if(usersVotes[i] == loadedSongId)
                usersVotes[i] = -1;
        for(int i=0; i<usersCount; i++)
            addToActionsQueue(i, VOTES_LIST_MSG);
        cout << "loading song.." << endl;
        loadNewSong(&loadedSong, songsFileNames[loadedSongId], songsNames[loadedSongId]);
        cout << "loaded, playing.." << loadedSongId << endl;
        sendSongSamples(&loadedSong, PACK_SIZE_IN_SAMPLES, SAMPLE_SIZE, udpfd, usersAddresses, usersAddressesSizes, &usersCount);
    }
    return NULL;
}

void *userThreadDo(void *userIdVoid) //users[userId] -read,  serverName -read, usersNames -read, songsNames -read, songsVotes -read, queues -pull
{
    int userId = *((int*)userIdVoid);

    char songName[255];
    int songNameSize = 0;
    bzero(songName, sizeof(songName));

    while(true)
    {
        //checking vote change
        int n;
        while(true)
        {
            ioctl(usersTcpFd[userId], FIONREAD, &n);
            if(n <= 0)
                break;
            read(usersTcpFd[userId], songName+songNameSize, n);
            for(int i=songNameSize; i<songNameSize+n; i++)
                if(songName[i] == '\n')
                {
                    songName[i] = '\0';
                    for(int songId=0; songId<songsCount; songId++)
                    {
                        if(strcmp(songName, songsNames[songId]) == 0)
                        {
                            changeVote(userId, songId);
                            for(int i=0; i<usersCount; i++)
                                addToActionsQueue(i, VOTES_LIST_MSG);
                            break;
                        }
                    }
                }
        }

        //sending all actions from queue
        while(actionsQueueStart[userId] != actionsQueueEnd[userId])
        {
            switch(removeFromActionsQueue(userId))
            {
            case SERVER_NAME_MSG:
                sendServerName(usersTcpFd[userId], serverName, strlen(serverName));
                break;
            case SONGS_LIST_MSG:
                sendSongsList(usersTcpFd[userId], songsNames, songsCount);
                break;
            case MY_SONGS_LIST_MSG:

                break;
            case USERS_LIST_MSG:
                sendUsersList(usersTcpFd[userId], usersNames, usersCount);
                break;
            case VOTES_LIST_MSG:
                sendVotesList(usersTcpFd[userId], songsVotes, songsNames, songsCount);
                break;
            case NEW_SONG_INFO_MSG:
                if(loadedSongId >= 0)
                    sendNewSongInfo(usersTcpFd[userId], loadedSong.getNumSamplesPerChannel(), loadedSong.getSampleRate(), PACK_SIZE_IN_SAMPLES*SAMPLE_SIZE, loadedSongName, strlen(loadedSongName));
                break;
            default:
                cout << "WARNING unrecognized action in action queue" << endl;
            }
        }
        usleep(10000);
    }
    return NULL;
}

int main()
{
    srand(time(NULL));
    //add song0
    char songName0[4] = "abc";
    char songFileName0[10] = "song0.wav";
    uint songVotes0 = 0;
    addSong(songName0, songFileName0, songVotes0);

    //add song1
    char songName1[15] = "AAAAAAAAAAAAAA";
    char songFileName1[10] = "song1.wav";
    uint songVotes1 = 0;
    addSong(songName1, songFileName1, songVotes1);

    //add song2
    char songName2[9] = "tego nie";
    char songFileName2[10] = "song2.wav";
    uint songVotes2 = 0;
    addSong(songName2, songFileName2, songVotes2);

    int tcpfd, udpfd;
    setup(&tcpfd, &udpfd, SERVER_PORT);

    pthread_create(&acceptThread, NULL, &acceptThreadDo, (void *) &tcpfd);
    pthread_create(&samplesThread, NULL, &samplesThreadDo, (void *) &udpfd);

    pthread_join(acceptThread, NULL);
    pthread_join(samplesThread, NULL);

    cout << "closing server" << endl;
    return 0;
}
