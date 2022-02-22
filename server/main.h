#ifndef MAIN_H
#define MAIN_H

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
#include <netinet/tcp.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <chrono>
#include <pthread.h>
#include <climits>
#include <mutex>
#include <dirent.h>
#include "AudioFile.h"

using namespace std;

#define CONFIG_FILE_LOCATION "./config.txt"
#define SONGS_LOCATION "./songs"

#define DEFAULT_SERVER_PORT 1234
#define DEFAULT_SERVER_NAME "Internet Radio"
#define MSG_END '\n'
#define MSG_VARIABLE_DELIMITER '\0'
#define SERVER_NAME_MSG 'a'
#define SONGS_LIST_MSG 'b'
#define USERS_LIST_MSG 'd'
#define VOTES_LIST_MSG 'e'
#define NEW_SONG_INFO_MSG 'f'
#define USER_NAME_OK_MSG 'g'
#define USER_NAME_WRONG_MSG 'h'

#define TCP_OUT_BUFFER_MAX_SIZE 1024
#define MAX_USERS_COUNT 1024
#define MAX_SONGS_COUNT 1024
#define MIN_USERS_NAME_SIZE 3
#define MAX_USERS_NAME_SIZE 50
#define MAX_SONG_NAME_SIZE 255
#define MAX_SONG_FILE_SIZE 255

#define PACK_SIZE_IN_SAMPLES 2048
#define SAMPLE_SIZE 2

#define ACTIONS_QUEUE_SIZE 32
#define USER_THREAD_SLEEP_TIME 10000

#define UN_WRONG_LEN_MSG "Wrong user name length."
#define UN_WRONG_CHAR_MSG "Wrong user name."
#define UN_TAKEN_MSG "User name has taken."
#define UN_OK_MSG "ok"

//server parameters
int serverPort;
char *serverName;

//songs lists
mutex songsMutex;
mutex votesMutex;
char *songsNames[MAX_SONGS_COUNT];
char *songsFileNames[MAX_SONGS_COUNT];
uint songsVotes[MAX_SONGS_COUNT];
bool songsActive[MAX_SONGS_COUNT];
int songsCount = 0;

//users lists
mutex usersMutex;
int usersTcpFd[MAX_USERS_COUNT];
char *usersNames[MAX_USERS_COUNT];
bool usersHasNames[MAX_USERS_COUNT];
bool usersActive[MAX_USERS_COUNT];
struct sockaddr_in usersAddresses[MAX_USERS_COUNT];
socklen_t usersAddressesSizes[MAX_USERS_COUNT];
int usersVotes[MAX_USERS_COUNT];
int usersCount = 0;

//threads and action queues
pthread_t acceptThread;
pthread_t samplesThread;
pthread_t usersThreads[MAX_USERS_COUNT];
mutex actionQueuesMutex;
char actionsQueue[MAX_USERS_COUNT][ACTIONS_QUEUE_SIZE];
int actionsQueueStart[MAX_USERS_COUNT];
int actionsQueueEnd[MAX_USERS_COUNT];

//current song
mutex currentSongNameMutex;
AudioFile<double> currentSongData;
char *currentSongName;
int currentSongId = -1;

//sockets
int tcpfd;
int udpfd;

//SERVER DATA STRUCTURE PROCEDURES
int addUser(int tcpFd, struct sockaddr_in address, socklen_t addressSize);
int removeUser(int userId);
void setUserName(int userId, char *name);
int addSong(char *name, char *fileName, uint votes);
void changeVote(int userId, int newSongId);
int getSongIdFromVotes(uint *songsVotes, int songsCount);
int setCurrentSong(AudioFile<double> *currentSongData, char *fileName, char* songName);

//SEND TCP
int sendServerName(int fd, char *name, int nameLen);
int sendStringList(int fd, char **strings, int stringCount, bool *active, char msgType, char *listName);
int sendSongsList(int fd, char **songs, bool *active, int songsCount);
int sendMySongsList(int fd, char **songs, bool *active, int songsCount);
int sendUsersList(int fd, char **users, bool *active, int usersCount);
int sendVotesList(int fd, uint *votes, char **songs, bool *active, int songsCount);
int sendNewSongInfo(int fd, int sampleCount, int sampleRate, int packSize, char *name, int nameLen);
int sendUserNameReply(int fd, bool ok, char *errorMsg, int msgLen);

//RECIEVE FROM USER
int recieveUserName(int fd, char *userName, int *userNameSize);
int validateUserName(char *userName, int userNameSize);

//SEND UTP
void sendSongSamples(AudioFile<double> *currentSong, int packSizeInSamples, int sampleSize, int udpfd, struct sockaddr_in *clientsAddresses,  socklen_t *clientsAddressesLengths, int *usersCount);

//THREADS
void addToActionsQueue(int userId, char action);
char removeFromActionsQueue(int userId);
void *acceptThreadDo(void *tcpFdVoid);
void *samplesThreadDo(void *udpFdVoid);
void *userThreadDo(void *userIdVoid);

//MAIN THREAD
void closeAndDelete(int sigNum);
void handleError(int error, const char *comment);
void loadServerParametersAndSongs();
void setupSockets(int *tcpfd, int *udpfd, int port);
int main(int argc, char** argv);

#endif
