#include "main.h"

//ZMIENNE WPOLDZILEONE
int memberMoney;
int groupMoney;
int approveCount;
int myStatus;
int *askTab;
int noMembers;
int entryCost;
int noClubs;
int preferedClubId;
int memberId;
int localClock;
MPI_Datatype mpiMsgType;

int main(int argc, char *argv[])
{
    entryCost = 100;
    noClubs = 4;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &noMembers);
    MPI_Comm_rank(MPI_COMM_WORLD, &memberId);

    const int nitems = 5;
    int blocklengths[5] = {1, 1, 1, 1, 1};
    MPI_Datatype types[5] = {MPI_INT, MPI_INT, MPI_INT, MPI_INT, MPI_INT};
    MPI_Aint offsets[5];

    offsets[0] = offsetof(msg, localClock);
    offsets[1] = offsetof(msg, message);
    offsets[2] = offsetof(msg, memberId);
    offsets[3] = offsetof(msg, preferedClubId);
    offsets[4] = offsetof(msg, memberMoney);

    MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpiMsgType);
    MPI_Type_commit(&mpiMsgType);

    srand(time(0) + memberId); //srand(time(NULL)) sprawdź

    createThread();
    localClock = rand() % 4;

    mainLoop();

    MPI_Type_free(&mpiMsgType);
    MPI_Finalize();

    return 0;
}
