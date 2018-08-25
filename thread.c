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
        if (myStatus == ENOUGH_MONEY && recv.message == ENTER_CLUB_QUERY)
        {
            if (recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
            }
            else
            {
                if (recv.lamportClock < lamportClock)
                {
                    lamportClock++;
                    send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, memberMoney);
                    MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                    printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
                }
            }
        }

        //3
        if ((myStatus != NO_GROUP && myStatus != GROUP_BREAK) && recv.message == GROUP_INVITE)
        {
            lamportClock++;
            send = createPackage(lamportClock, REJECT_INVITE_MSG, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d\n", rank, lamportClock, recv.rank);
        }

        //4
        if (myStatus == FOUNDER && recv.message == GROUP_CONFIRMATION)
        {
            groupMoney += recv.memberMoney;
            *(tab + recv.rank) = MY_GROUP;
            myStatus = ACCEPT_INVITE;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", rank, lamportClock, recv.rank);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock, groupMoney, entryCost);
        }

        //5
        if (myStatus == FOUNDER && recv.message == REJECT_INVITE_MSG)
        {
            *(tab + recv.rank) = NOT_MY_GROUP;
            myStatus = REJECT_INVITE;
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d(jestem Kapitanem)\n", rank, lamportClock, recv.rank);
        }

        //6
        if (myStatus == ENOUGH_MONEY && recv.message == ENTER_PERMISSION)
        {
            approveCount++;
            printf("[%d][%ld]Pozwolenie dla mnie na wejscie do klubu o nr: %d od RANK: %d\n", rank, lamportClock, clubNumber, recv.rank);
            if (approveCount == noMembers - 1)
            {
                myStatus = ENTER_CLUB;
            }
        }

        //7
        if (myStatus != ENOUGH_MONEY && myStatus != ENTER_CLUB && recv.message == ENTER_CLUB_QUERY)
        {
            lamportClock++;
            send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", rank, lamportClock, recv.clubNumber, recv.rank);
        }

        //8
        if ((myStatus == NO_GROUP || myStatus == GROUP_BREAK) && recv.message == GROUP_INVITE)
        {
            if (recv.lamportClock < lamportClock)
            {
                myStatus = PARTICIPATOR;
                lamportClock++;
                send = createPackage(lamportClock, GROUP_CONFIRMATION, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Akceptuje zaproszenie do grupy od RANK: %d\n", rank, lamportClock, recv.rank);
            }
            else
            {
                lamportClock++;
                send = createPackage(lamportClock, REJECT_INVITE_MSG, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Odrzucam zaproszenie do grupy od RANK: %d\n", rank, lamportClock, recv.rank);
            }
        }

        //9
        if (myStatus == NO_GROUP && recv.message == GROUP_CONFIRMATION)
        {
            myStatus = ACCEPT_INVITE;
            *(tab + recv.rank) = MY_GROUP;
            groupMoney += recv.memberMoney;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", rank, lamportClock, recv.rank);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock, groupMoney, entryCost);
        }

        //10
        if (myStatus == NO_GROUP && recv.message == REJECT_INVITE_MSG)
        {
            myStatus = GROUP_BREAK;
            *(tab + recv.rank) = NOT_MY_GROUP;
            printf("[%d][%ld]Moje zaproszenie zostalo odrzucone od RANK: %d\n", rank, lamportClock, recv.rank);
        }

        //11
        if (myStatus == PARTICIPATOR && recv.message == GROUP_CONFIRMATION)
        {
            lamportClock++;
            send = createPackage(lamportClock, GROUP_BREAK_MSG, rank, clubNumber, memberMoney);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Zrywam grupe z RANK: %d (moje zaproszenie jest juz nie aktualne)\n", rank, lamportClock, recv.rank);
        }

        //12
        if (myStatus == PARTICIPATOR && recv.message == GROUP_BREAK_MSG)
        {
            myStatus = GROUP_BREAK;
            printf("[%d][%ld]Grupa zostala rozwiazana przez RANK: %d!\n", rank, lamportClock, recv.rank);
        }

        //13
        if (recv.message == EXIT_CLUB_MSG)
        {
            clubNumber = recv.clubNumber;
            printf("[%d][%ld]        Wychodze z klubu jako czlonek grupy! Nr klubu: %d\n", rank, lamportClock, clubNumber);
            myStatus = EXIT_CLUB;
        }

        //14
        if (myStatus == ENTER_CLUB && recv.message == ENTER_CLUB_QUERY)
        {
            if (recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, memberMoney);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d  dla RANK: %d\n", rank, lamportClock, recv.rank, recv.clubNumber);
            }
        }
    }

    pthread_exit(NULL);
}
