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
#define NO_GROUP 0
#define FOUNDER 1
#define PARTICIPATOR -1
#define GROUP_BREAK 2
#define ENTER_CLUB 3
#define EXIT_CLUB 4
#define ACCEPT_INVITE 5
#define REJECT_INVITE 6
#define ENOUGH_MONEY 7

//MESSAGE
#define ENTER_CLUB_QUERY 0
#define GROUP_INVITE 1
#define GROUP_CONFIRMATION 2
#define GROUP_BREAK_MSG 3
#define REJECT_INVITE_MSG 4
#define ENTER_PERMISSION 5
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
extern int clubNumber;
extern int rank;
extern long lamportClock; //dopisać inkrementowanie zegara
extern MPI_Datatype mpi_data;

typedef struct data_s
{
        int lamportClock;
        int message;
        int rank;
        int clubNumber;
        int memberMoney;
} data;

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
data createPackage(int, int, int, int, int);

#endif
