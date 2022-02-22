
#include "main.h"

//SERVER DATA STRUCTURE PROCEDURES
int addUser(int tcpFd, struct sockaddr_in address, socklen_t addressSize)
{
    int newUserId = -1;
    if(usersCount >= MAX_USERS_COUNT)
    {
        cout << "WARNING Reached max users count" << endl;
        return -1;
    }
    newUserId = usersCount;
    usersCount++;
    usersTcpFd[newUserId] = tcpFd;
    usersAddresses[newUserId] = address;
    usersAddressesSizes[newUserId] = addressSize;
    usersNames[newUserId] = new char[0];
    usersHasNames[newUserId] = false;
    usersActive[newUserId] = false;
    usersVotes[newUserId] = -1;
    return newUserId;
}

int removeUser(int userId)
{
    cout << "removing user id = " << userId << endl;
    usersActive[userId] = false;
    for(int i=0; i<usersCount; i++)
        if(usersActive[i])
            addToActionsQueue(i, USERS_LIST_MSG);
    if(usersVotes[userId] >= 0)
        songsVotes[usersVotes[userId]]--;
    usersVotes[userId] = -1;
    return 0;
}

void setUserName(int userId, char *name)
{
    usersHasNames[userId] = true;
    usersNames[userId] = new char[strlen(name) +1];
    strcpy(usersNames[userId], name);
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
    songsFileNames[songsCount] = new char[strlen(fileName)+1];
    strcpy(songsFileNames[songsCount], fileName);
    songsVotes[songsCount] = votes;
    songsActive[songsCount] = true;
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

int getSongIdFromVotes(uint *songsVotes, int songsCount)
{
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
    return maxIds[rand()%maxIdsCount];
}

int setCurrentSong(AudioFile<double> *currentSong, char *fileName, char* songName)
{
    chrono::high_resolution_clock::time_point songLoadingStart = chrono::high_resolution_clock::now();
    cout << "loading song file... (" << fileName << ")" << endl;
    if (currentSong->load(fileName) == 0)
        return -1;
    currentSongName = songName;

    //displaying song info in console
    /*cout << endl << "new song info:" << endl;
    currentSong->printSummary();
    cout << endl;*/

    //adding to actions queues
    for(int i=0; i<usersCount; i++)
        if(usersActive[i])
            addToActionsQueue(i, NEW_SONG_INFO_MSG);

    chrono::high_resolution_clock::time_point now = chrono::high_resolution_clock::now();
    chrono::duration<double> songLoadingTime = now-songLoadingStart;
    cout << "song file loaded in " << songLoadingTime.count() << "sec" << endl;
    return 0;
}



//SEND TCP
int sendServerName(int fd, char *name, int nameLen)
{
    cout << "sending server name to " << fd << endl;
    char msg[nameLen+2];
    bzero(&msg, nameLen+1);
    msg[0] = SERVER_NAME_MSG;
    memcpy(msg+1, name, nameLen);
    msg[nameLen+1] = MSG_END;
    int sentBytes = write(fd, msg, nameLen+2);
    if(sentBytes != nameLen+2)
    {
        cout << "WARNING sent only " << sentBytes << "bytes expected " << nameLen+2 << "bytes" << endl;
        return -1;
    }
    cout << "server name send to " << fd << endl;
    return 0;
}

int sendStringList(int fd, char **strings, int stringCount, bool *active, char msgType, char *listName)
{
    cout << "sending " << listName << " list to " << fd << endl;
    char msg[TCP_OUT_BUFFER_MAX_SIZE];
    int msgLen = 0;
    bzero(&msg, TCP_OUT_BUFFER_MAX_SIZE);
    msg[msgLen++] = msgType;

    for(int i=0; i<stringCount; i++)
        if(active[i])
        {

            memcpy(msg + msgLen, strings[i], strlen(strings[i]));
            msgLen += strlen(strings[i]);
            if(i != stringCount-1)
                msg[msgLen++] = MSG_VARIABLE_DELIMITER;
        }

    msg[msgLen++] = MSG_END;
    int sentBytes = write(fd, msg, msgLen);
    if(sentBytes != msgLen)
    {
        cout << "WARNING sent only " << sentBytes << "bytes expected " << msgLen << "bytes" << endl;
        return -1;
    }
    cout << listName <<" list send to " << fd << endl;
    return 0;
}

int sendSongsList(int fd, char **songs, bool *active, int songsCount)
{
    char name[6] = "songs";
    return sendStringList(fd, songs, songsCount, active, SONGS_LIST_MSG, name);
}

int sendUsersList(int fd, char **users, bool *active, int usersCount)
{
    char name[6] = "users";
    return sendStringList(fd, users, usersCount, active, USERS_LIST_MSG, name);;
}

int sendVotesList(int fd, uint *votes, char **songs, bool *active, int songsCount)
{
    cout << "sending votes list to " << fd << endl;
    uint votesCopy[songsCount];
    memcpy(votesCopy, votes, sizeof(uint)*songsCount);
    char msg[TCP_OUT_BUFFER_MAX_SIZE];
    int msgLen = 0;
    bzero(&msg, TCP_OUT_BUFFER_MAX_SIZE);
    msg[msgLen++] = VOTES_LIST_MSG;

    int maxId;
    for(int i=0; i<songsCount; i++)
        if(active[i])
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
    {
        cout << "WARNING sent only " << sentBytes << "bytes expected " << msgLen << "bytes" << endl;
        return -1;
    }
    cout << "votes list send to " << fd << endl;
    return 0;
}

int sendNewSongInfo(int fd, int sampleCount, int sampleRate, int packSize, char *name, int nameLen)
{
    cout << "sending new song info to " << fd << endl;
    char msg[nameLen+14]; // 1bytes*[NEW_SONG_INFO_MSG]  4bytes[sampleCount]  4bytes*[sampleRate] 4bytes[packSize]  nameLen bytes*[name]  1bytes*[MSG_END]
    bzero(&msg, nameLen+14);
    msg[0] = NEW_SONG_INFO_MSG;

    memcpy(msg+1, &sampleCount, 4);
    memcpy(msg+5, &sampleRate, 4);
    memcpy(msg+9, &packSize, 4);
    memcpy(msg+13, name, nameLen);

    msg[nameLen+13] = MSG_END;
    int sentBytes = write(fd, msg, nameLen+14);
    if(sentBytes != nameLen+14)
    {
        cout << "WARNING sent only " << sentBytes << "bytes expected " << nameLen+14 << "bytes" << endl;
        return -1;
    }
    cout << "new song info name send to " << fd << endl;
    return 0;
}

int sendUserNameReply(int fd, bool ok, const char *errorMsg, int msgLen)
{
    cout << "sending user name msg to " << fd << endl;
    char msg[msgLen+1];
    bzero(&msg, msgLen+1);
    if(ok)
        msg[0] = USER_NAME_OK_MSG;
    else
        msg[0] = USER_NAME_WRONG_MSG;

    memcpy(msg+1, errorMsg, msgLen);

    msg[msgLen+1] = MSG_END;
    int sentBytes = write(fd, msg, msgLen+2);
    if(sentBytes != msgLen+2)
    {
        cout << "WARNING sent only " << sentBytes << "bytes expected " << msgLen+2 << "bytes" << endl;
        return -1;
    }
    cout << "user name msg send to " << fd << endl;
    return 0;
}



//RECIEVE FROM USER
int recieveUserName(int fd, char *userName, int *userNameSize)
{
    //receiving user name
    bzero(userName, sizeof(userName));
    int msgSize = 0;
    int curUserNameSize = 0;
    bool endFound = false;
    while( (msgSize = read(fd, userName+curUserNameSize, sizeof(userName))) > 0)
    {
        for(int i=0; i<msgSize; i++)
            if(userName[curUserNameSize+i] == '\n')
            {
                curUserNameSize += i;
                endFound = true;
                break;
            }
        if(endFound)
            break;
        curUserNameSize += msgSize;
    }
    if(msgSize < 1)
        return -1;
    userName[curUserNameSize] = '\0';
    *userNameSize = curUserNameSize+1;
    return 0;
}

int validateUserName(char *userName, int userNameSize)
{
    if(userNameSize-1 < MIN_USERS_NAME_SIZE)
       return -1;
    if(userNameSize-1 > MAX_USERS_NAME_SIZE)
       return -1;
    if(userName[userNameSize-1] != '\0')
       return -1;
    for(int i=0; i<userNameSize-1; i++)
    {
        if(userName[i] >= 'a' && userName[i] <= 'z')
            continue;
        if(userName[i] >= 'A' && userName[i] <= 'Z')
            continue;
        if(userName[i] >= '0' && userName[i] <= '9')
            continue;
        if(userName[i] == ' ')
            continue;
        return -2;
    }
    for(int i=0; i<usersCount; i++)
        if(usersHasNames[i] && usersActive[i] && strcmp(userName, usersNames[i]) == 0)
            return -3;
    return 0;
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
            if(usersActive[i])
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

void *acceptThreadDo(void *tcpFdVoid)
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

        usersMutex.lock();
        int userId = addUser(userTcpFd, clientAddr, clientAddrLen);
        if(userId == -1)
            continue;
        usersMutex.unlock();

        actionQueuesMutex.lock();
        addToActionsQueue(userId, SERVER_NAME_MSG);
        for(int i=0; i<usersCount; i++)
            if(usersActive[i])
                addToActionsQueue(i, USERS_LIST_MSG);
        addToActionsQueue(userId, USERS_LIST_MSG);
        addToActionsQueue(userId, SONGS_LIST_MSG);
        addToActionsQueue(userId, VOTES_LIST_MSG);
        addToActionsQueue(userId, NEW_SONG_INFO_MSG);
        actionQueuesMutex.unlock();
        pthread_create(&(usersThreads[0]), NULL, &userThreadDo, (void *) &userId);
    }
    return NULL;
}

void *samplesThreadDo(void *udpFdVoid)
{
    int udpfd = *((int*) udpFdVoid);
    cout << "starting music" << endl;
    while(true)
    {
        int loadFailure;
        currentSongNameMutex.lock();
        do
        {
            songsMutex.lock();
            currentSongId = getSongIdFromVotes(songsVotes, songsCount);
            cout << "next song : \"" << songsNames[currentSongId] << "\" song id = " << currentSongId << " song file name : \"" << songsFileNames[currentSongId] << "\"" << endl;
            loadFailure = setCurrentSong(&currentSongData, songsFileNames[currentSongId], songsNames[currentSongId]);
            if(loadFailure != 0)
                cout << "WARNING song id = " << currentSongId << " has wrong file name : \"" << songsFileNames[currentSongId] << "\"" << endl;
            songsMutex.unlock();
        }
        while(loadFailure != 0);
        currentSongNameMutex.unlock();

        actionQueuesMutex.lock();
        songsVotes[currentSongId] = 0;
        for(int i=0; i<usersCount; i++)
            if(usersVotes[i] == currentSongId)
                usersVotes[i] = -1;
        for(int i=0; i<usersCount; i++)
            if(usersActive[i])
                addToActionsQueue(i, VOTES_LIST_MSG);
        actionQueuesMutex.unlock();

        sendSongSamples(&currentSongData, PACK_SIZE_IN_SAMPLES, SAMPLE_SIZE, udpfd, usersAddresses, usersAddressesSizes, &usersCount);
    }
    return NULL;
}

void *userThreadDo(void *userIdVoid)
{
    int userId = *((int*)userIdVoid);

    //receiving user name
    char userName[MAX_USERS_NAME_SIZE];
    int userNameSize;
    int wrongUserName = -1;
    do
    {
        if(recieveUserName(usersTcpFd[userId], userName, &userNameSize) != 0)
        {
            removeUser(userId);
            return NULL;
        }
        wrongUserName = validateUserName(userName, userNameSize);
        if(wrongUserName == -1)
            sendUserNameReply(usersTcpFd[userId], false, UN_WRONG_LEN_MSG, strlen(UN_WRONG_LEN_MSG));
        else if(wrongUserName == -2)
            sendUserNameReply(usersTcpFd[userId], false, UN_WRONG_CHAR_MSG, strlen(UN_WRONG_CHAR_MSG));
        else if(wrongUserName == -3)
            sendUserNameReply(usersTcpFd[userId], false, UN_TAKEN_MSG, strlen(UN_TAKEN_MSG));
    }
    while(wrongUserName != 0);
    sendUserNameReply(usersTcpFd[userId], true, UN_OK_MSG, strlen(UN_OK_MSG));
    usersMutex.lock();
    setUserName(userId, userName);
    usersActive[userId] = true;
    usersMutex.unlock();
    cout << "user id = " << userId << " user name : \"" << userName << "\"" << endl;

    char songName[MAX_SONG_NAME_SIZE];
    int songNameSize = 0;
    bzero(songName, sizeof(songName));
    int connectionStatus = 0;
    char pingMsg[3] = "x\n";
    //recieving user
    while(true)
    {
        //checking vote change
        int n;
        while(true)
        {
            int error = 0; //checking connection status
            socklen_t len = sizeof (error);
            getsockopt (usersTcpFd[userId], SOL_SOCKET, SO_ERROR, &error, &len);
            if(error || write(usersTcpFd[userId], pingMsg, 2) == -1)
            {
                fflush(stdout);
                removeUser(userId);
                return NULL;
            }

            ioctl(usersTcpFd[userId], FIONREAD, &n); //sprawdzanie czy są dane przychodzące
            if(n <= 0)
                break;
            read(usersTcpFd[userId], songName+songNameSize, n);
            for(int i=songNameSize; i<songNameSize+n; i++)
                if(songName[i] == '\n')
                {
                    songName[i] = '\0';
                    songsMutex.lock();
                    for(int songId=0; songId<songsCount; songId++)
                    {
                        if(strcmp(songName, songsNames[songId]) == 0)
                        {
                            votesMutex.lock();
                            changeVote(userId, songId);
                            for(int i=0; i<usersCount; i++)
                                if(usersActive[i])
                                    addToActionsQueue(i, VOTES_LIST_MSG);
                            votesMutex.unlock();
                            break;
                        }
                    }
                    songsMutex.unlock();
                }
        }
        //sending all actions from queue
        while(actionsQueueStart[userId] != actionsQueueEnd[userId])
        {
            switch(removeFromActionsQueue(userId))
            {
            case SERVER_NAME_MSG:
                connectionStatus = sendServerName(usersTcpFd[userId], serverName, strlen(serverName));
                break;
            case SONGS_LIST_MSG:
                songsMutex.lock();
                connectionStatus = sendSongsList(usersTcpFd[userId], songsNames, songsActive, songsCount);
                songsMutex.unlock();
                break;
            case USERS_LIST_MSG:
                usersMutex.lock();
                connectionStatus = sendUsersList(usersTcpFd[userId], usersNames, usersActive, usersCount);
                usersMutex.unlock();
                break;
            case VOTES_LIST_MSG:
                songsMutex.lock();
                votesMutex.lock();
                connectionStatus = sendVotesList(usersTcpFd[userId], songsVotes, songsNames, songsActive, songsCount);
                votesMutex.unlock();
                songsMutex.unlock();
                break;
            case NEW_SONG_INFO_MSG:
                currentSongNameMutex.lock();
                if(currentSongId >= 0)
                    connectionStatus = sendNewSongInfo(usersTcpFd[userId], currentSongData.getNumSamplesPerChannel(), currentSongData.getSampleRate(), PACK_SIZE_IN_SAMPLES*SAMPLE_SIZE, currentSongName, strlen(currentSongName));
                currentSongNameMutex.unlock();
                break;
            default:
                cout << "WARNING unrecognized action in action queue" << endl;
            }
            if(connectionStatus != 0)
            {
                removeUser(userId);
                return NULL;
            }
        }
        usleep(USER_THREAD_SLEEP_TIME);
    }
    return NULL;
}


//MAIN THREAD
void closeAndDelete(int sinalNum)
{
    cout << endl << "=========================================================" << endl;
    cout << "closing server" << endl;

    //songs lists
    for(int i=0; i<songsCount; i++)
    {
        delete[] songsNames[i];
        delete[] songsFileNames[i];
    }

    //users lists
    for(int i=0; i<usersCount; i++)
    {
        //users lists
        delete[] usersNames[i];
    }

    //current song
    delete[] currentSongName;

    exit(sinalNum);
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

void loadServerParametersAndSongs()
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(SONGS_LOCATION)) != NULL)
    {
        /* print all the files and directories within directory */
        while((ent = readdir (dir)) != NULL)
        {
            char *fileName = ent->d_name;
            char *longFileName;
            char *name;
            int len = strlen(fileName);
            if(len >4 && fileName[len-4] == '.' &&  fileName[len-3] == 'w' &&  fileName[len-2] == 'a' &&  fileName[len-1] == 'v')
            {
                name = new char[len-3];
                memcpy(name, fileName, len-4);
                name[len-4] = '\0';
                longFileName = new char[strlen(SONGS_LOCATION)+len+2];
                memcpy(longFileName, SONGS_LOCATION, strlen(SONGS_LOCATION));
                longFileName[strlen(SONGS_LOCATION)] = '/';
                memcpy(longFileName + strlen(SONGS_LOCATION)+1, fileName, len);
                longFileName[strlen(SONGS_LOCATION)+len+1] = '\0';

                addSong(name, longFileName, 0);
            }
        }
        closedir(dir);
    }
    else
    {
        cout << "cannot find songs directory : \"" << SONGS_LOCATION << "\"" << endl;
        exit(0);
    }
    if(songsCount < 1)
    {
        cout << "cannot find any song's files (.wav) in directory : \"" << SONGS_LOCATION << "\"" << endl;
        exit(0);
    }
}

void setupSockets(int *tcpfd, int *udpfd, int port)
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

int main(int argc, char** argv)
{
    srand(time(NULL));
    signal(SIGINT, closeAndDelete);

    if(argc > 1)
        serverPort = atoi(argv[1]);
    else
        serverPort = DEFAULT_SERVER_PORT;

    if(argc > 2)
        serverName = argv[2];
    else
    {
        serverName = new char[sizeof(DEFAULT_SERVER_NAME)];
        strcpy(serverName, DEFAULT_SERVER_NAME);
    }

    loadServerParametersAndSongs();

    int tcpfd, udpfd;
    setupSockets(&tcpfd, &udpfd, serverPort);

    pthread_create(&acceptThread, NULL, &acceptThreadDo, (void *) &tcpfd);
    pthread_create(&samplesThread, NULL, &samplesThreadDo, (void *) &udpfd);

    pthread_join(acceptThread, NULL);
    pthread_join(samplesThread, NULL);

    closeAndDelete(0);
    return 0;
}
