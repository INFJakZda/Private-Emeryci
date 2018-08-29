#include "main.h"

void *ThreadBehavior()
{
    data recv;
    data send;
    while (true)
    {
        //1
        MPI_Recv(&recv, 1, mpi_data, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        lamportClock = max(recv.lamportClock, lamportClock) + 1;

        //2
        if (myStatus == ENOUGH_MONEY_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, AGREE_TO_ENTER_CLUB_MSG, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
            }
            else
            {
                if (recv.lamportClock < lamportClock)
                {
                    lamportClock++;
                    send = createPackage(lamportClock, AGREE_TO_ENTER_CLUB_MSG, rank, clubNumber, memberMoney);
                    MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                    printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
                }
            }
        }

        //3
        if ((myStatus != ALONE_STATUS && myStatus != GROUP_BREAK_STATUS) && recv.message == ASK_TO_JOIN_MSG)
        {
            lamportClock++;
            send = createPackage(lamportClock, REJECT_JOIN_MSG, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d\n", rank, lamportClock, recv.rank);
        }

        //4
        if (myStatus == LEADER_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            groupMoney += recv.memberMoney;
            *(askTab + recv.rank) = ACCEPT_ASK_TAB;
            myStatus = ACCEPT_INVITATION_STATUS;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", rank, lamportClock, recv.rank);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock, groupMoney, entryCost);
        }

        //5
        if (myStatus == LEADER_STATUS && recv.message == REJECT_JOIN_MSG)
        {
            *(askTab + recv.rank) = REJECT_ASK_TAB;
            myStatus = REJECT_INVITATION_STATUS;
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d(jestem Kapitanem)\n", rank, lamportClock, recv.rank);
        }

        //6
        if (myStatus == ENOUGH_MONEY_STATUS && recv.message == AGREE_TO_ENTER_CLUB_MSG)
        {
            approveCount++;
            printf("[%d][%ld]Pozwolenie dla mnie na wejscie do klubu o nr: %d od RANK: %d\n", rank, lamportClock, clubNumber, recv.rank);
            if (approveCount == noMembers - 1)
            {
                myStatus = ENTER_CLUB_STATUS;
            }
        }

        //7
        if (myStatus != ENOUGH_MONEY_STATUS && myStatus != ENTER_CLUB_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            lamportClock++;
            send = createPackage(lamportClock, AGREE_TO_ENTER_CLUB_MSG, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
        }

        //8
        if ((myStatus == ALONE_STATUS || myStatus == GROUP_BREAK_STATUS) && recv.message == ASK_TO_JOIN_MSG)
        {
            if (recv.lamportClock < lamportClock)
            {
                myStatus = MEMBER_STATUS;
                lamportClock++;
                send = createPackage(lamportClock, CONFIRM_JOIN_MSG, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Akceptuje zaproszenie do grupy od RANK: %d\n", rank, lamportClock, recv.rank);
            }
            else
            {
                lamportClock++;
                send = createPackage(lamportClock, REJECT_JOIN_MSG, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Odrzucam zaproszenie do grupy od RANK: %d\n", rank, lamportClock, recv.rank);
            }
        }

        //9
        if (myStatus == ALONE_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            myStatus = ACCEPT_INVITATION_STATUS;
            *(askTab + recv.rank) = ACCEPT_ASK_TAB;
            groupMoney += recv.memberMoney;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", rank, lamportClock, recv.rank);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock, groupMoney, entryCost);
        }

        //10
        if (myStatus == ALONE_STATUS && recv.message == REJECT_JOIN_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            *(askTab + recv.rank) = REJECT_ASK_TAB;
            printf("[%d][%ld]Moje zaproszenie zostalo odrzucone od RANK: %d\n", rank, lamportClock, recv.rank);
        }

        //11
        if (myStatus == MEMBER_STATUS && recv.message == CONFIRM_JOIN_MSG)
        {
            lamportClock++;
            send = createPackage(lamportClock, GROUP_BREAK_MSG, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Zrywam grupe z RANK: %d (moje zaproszenie jest juz nie aktualne)\n", rank, lamportClock, recv.rank);
        }

        //12
        if (myStatus == MEMBER_STATUS && recv.message == GROUP_BREAK_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            printf("[%d][%ld]Grupa zostala rozwiazana przez RANK: %d!\n", rank, lamportClock, recv.rank);
        }

        //13
        if (recv.message == EXIT_CLUB_MSG)
        {
            clubNumber = recv.clubNumber;
            printf("[%d][%ld]        Wychodze z klubu jako czlonek grupy! Nr klubu: %d\n", rank, lamportClock, clubNumber);
            myStatus = EXIT_CLUB_STATUS;
        }

        //14
        if (myStatus == ENTER_CLUB_STATUS && recv.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, AGREE_TO_ENTER_CLUB_MSG, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d  dla RANK: %d\n", rank, lamportClock, recv.rank, recv.clubNumber);
            }
        }
    }

    pthread_exit(NULL);
}
