#include "main.h"

void *ThreadBehavior()
{
    data recv;
    data send;
    while (true)
    {
        //1
        MPI_Recv(&recv, 1, mpi_data, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        localClock = max(recv.localClock, localClock) + 1;

        //2
        if (myStatus == ENOUGH_MONEY_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recv.preferedClubId != preferedClubId)
            {
                localClock++;
                send = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recv.preferedClubId, recv.memberId);
            }
            else
            {
                if (recv.localClock < localClock)
                {
                    localClock++;
                    send = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                    MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
                    printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recv.preferedClubId, recv.memberId);
                }
            }
        }

        //3
        if ((myStatus != ALONE_STATUS && myStatus != GROUP_BREAK_STATUS) && recv.message == ASK_TO_JOIN_MSG)
        {
            localClock++;
            send = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d\n", memberId, localClock, recv.memberId);
        }

        //4
        if (myStatus == LEADER_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            groupMoney += recv.memberMoney;
            *(askTab + recv.memberId) = ACCEPT_ASK_TAB;
            myStatus = ACCEPT_INVITATION_STATUS;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", memberId, localClock, recv.memberId);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", memberId, localClock, groupMoney, entryCost);
        }

        //5
        if (myStatus == LEADER_STATUS && recv.message == REJECT_JOIN_MSG)
        {
            *(askTab + recv.memberId) = REJECT_ASK_TAB;
            myStatus = REJECT_INVITATION_STATUS;
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d(jestem Kapitanem)\n", memberId, localClock, recv.memberId);
        }

        //6
        if (myStatus == ENOUGH_MONEY_STATUS && recv.message == AGREE_TO_ENTER_CLUB_MSG)
        {
            approveCount++;
            printf("[%d][%ld]Pozwolenie dla mnie na wejscie do klubu o nr: %d od RANK: %d\n", memberId, localClock, preferedClubId, recv.memberId);
            if (approveCount == noMembers - 1)
            {
                myStatus = ENTER_CLUB_STATUS;
            }
        }

        //7
        if (myStatus != ENOUGH_MONEY_STATUS && myStatus != ENTER_CLUB_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            localClock++;
            send = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recv.preferedClubId, recv.memberId);
        }

        //8
        if ((myStatus == ALONE_STATUS || myStatus == GROUP_BREAK_STATUS) && recv.message == ASK_TO_JOIN_MSG)
        {
            if (recv.localClock < localClock)
            {
                myStatus = MEMBER_STATUS;
                localClock++;
                send = createPackage(localClock, CONFIRM_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Akceptuje zaproszenie do grupy od RANK: %d\n", memberId, localClock, recv.memberId);
            }
            else
            {
                localClock++;
                send = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Odrzucam zaproszenie do grupy od RANK: %d\n", memberId, localClock, recv.memberId);
            }
        }

        //9
        if (myStatus == ALONE_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            myStatus = ACCEPT_INVITATION_STATUS;
            *(askTab + recv.memberId) = ACCEPT_ASK_TAB;
            groupMoney += recv.memberMoney;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", memberId, localClock, recv.memberId);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", memberId, localClock, groupMoney, entryCost);
        }

        //10
        if (myStatus == ALONE_STATUS && recv.message == REJECT_JOIN_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            *(askTab + recv.memberId) = REJECT_ASK_TAB;
            printf("[%d][%ld]Moje zaproszenie zostalo odrzucone od RANK: %d\n", memberId, localClock, recv.memberId);
        }

        //11
        if (myStatus == MEMBER_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            localClock++;
            send = createPackage(localClock, GROUP_BREAK_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Zrywam grupe z RANK: %d (moje zaproszenie jest juz nie aktualne)\n", memberId, localClock, recv.memberId);
        }

        //12
        if (myStatus == MEMBER_STATUS && recv.message == GROUP_BREAK_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            printf("[%d][%ld]Grupa zostala rozwiazana przez RANK: %d!\n", memberId, localClock, recv.memberId);
        }

        //13
        if (recv.message == EXIT_CLUB_MSG)
        {
            preferedClubId = recv.preferedClubId;
            printf("[%d][%ld]        Wychodze z klubu jako czlonek grupy! Nr klubu: %d\n", memberId, localClock, preferedClubId);
            myStatus = EXIT_CLUB_STATUS;
        }

        //14
        if (myStatus == ENTER_CLUB_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recv.preferedClubId != preferedClubId)
            {
                localClock++;
                send = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d  dla RANK: %d\n", memberId, localClock, recv.memberId, recv.preferedClubId);
            }
        }
    }

    pthread_exit(NULL);
}
