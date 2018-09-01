#ifndef MAIN_H
#define MAIN_H

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <limits.h>
#include <mpi.h>
#include <unistd.h>

//STATUS
#define ALONE_STATUS 0
#define LEADER_STATUS 1
#define MEMBER_STATUS -1
#define GROUP_BREAK_STATUS 2
#define ENTER_CLUB_STATUS 3
#define EXIT_CLUB_STATUS 4
#define ACCEPT_INVITATION_STATUS 5
#define REJECT_INVITATION_STATUS 6
#define ENOUGH_MONEY_STATUS 7

//MESSAGE
#define ASK_TO_ENTER_CLUB_MSG 0
#define ASK_TO_JOIN_MSG 1
#define CONFIRM_JOIN_MSG 2
#define GROUP_BREAK_MSG 3
#define REJECT_JOIN_MSG 4
#define AGREE_TO_ENTER_CLUB_MSG 5
#define EXIT_CLUB_MSG 6

//TAB
#define READY_ASK_TAB 0
#define ACCEPT_ASK_TAB 1
#define REJECT_ASK_TAB -1

//TAG
#define TAG 22

//ZMIENNE WPOLDZILEONE
extern int memberMoney;
extern int groupMoney;
extern int approveCount;
extern int myStatus;
extern int *askTab;
extern int noMembers;
extern int entryCost;
extern int noClubs;
extern int preferedClubId;
extern int memberId;
extern long localClock; //dopisać inkrementowanie zegara
extern MPI_Datatype mpiMsgType;

typedef struct msg_s
{
    int localClock;
    int message;
    int memberId;
    int preferedClubId;
    int memberMoney;
} msg;

//BOOL
typedef int bool;
enum
{
    false,
    true
};

//FUNCTIONS
void createThread();
bool isSomeoneToAsk();
int getRandomFreeElder();
int max(int, int);
void *ThreadBehavior();
void mainLoop();
msg createPackage(int, int, int, int, int);

#endif
