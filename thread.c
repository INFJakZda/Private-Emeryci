#include "main.h"

void *ThreadBehavior()
{
    data recv;
    data send;
    while(true)
    {
        //1
        MPI_Recv(&recv, 1, mpi_data, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        lamportClock = max(recv.lamportClock, lamportClock) + 1;

        //2
        if(status == ENOUGH_MONEY && recv.message == ENTER_CLUB_QUERY)
        {
            if(recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, money);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
		            printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n",rank, lamportClock, recv.clubNumber, recv.rank);
            }
            else
            {
                if(recv.lamportClock < lamportClock)
                {
                    lamportClock++;
                    send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, money);
                    MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
		                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n",rank, lamportClock, recv.clubNumber, recv.rank);
                }
            }
        }

        //3
        if((status != NO_GROUP && status != GROUP_BREAK) && recv.message == GROUP_INVITE)
        {
            lamportClock++;
            send = createPackage(lamportClock, REJECT_INVITE_MSG, rank, clubNumber, money);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
	          printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d\n",rank, lamportClock, recv.rank);
        }

        //4
        if(status == FOUNDER && recv.message == GROUP_CONFIRMATION)
        {
            groupMoney += recv.money;
            *(tab+recv.rank) = MY_GROUP;
            status = ACCEPT_INVITE;
      	    printf("[%d][%ld]RANK: %d dalacza do grupy!\n",rank, lamportClock, recv.rank);
      	    printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock, groupMoney, M);
        }

        //5
        if(status == FOUNDER && recv.message == REJECT_INVITE_MSG)
        {
            *(tab+recv.rank) = NOT_MY_GROUP;
            status = REJECT_INVITE;
	          printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d(jestem Kapitanem)\n",rank, lamportClock, recv.rank);
        }

        //6
        if(status == ENOUGH_MONEY && recv.message == ENTER_PERMISSION)
        {
            approveCount++;
	           printf("[%d][%ld]Pozwolenie dla mnie na wejscie do klubu o nr: %d od RANK: %d\n",rank, lamportClock, clubNumber, recv.rank);
            if(approveCount == N-1)
            {
                status = ENTER_CLUB;
            }
        }

        //7
        if(status != ENOUGH_MONEY && status != ENTER_CLUB && recv.message == ENTER_CLUB_QUERY)
        {
            lamportClock++;
            send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, money);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
	          printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n",rank, lamportClock, recv.clubNumber, recv.rank);
        }

        //8
        if((status == NO_GROUP || status == GROUP_BREAK) && recv.message == GROUP_INVITE)
        {
            if(recv.lamportClock < lamportClock)
            {
                status = PARTICIPATOR;
                lamportClock++;
                send = createPackage(lamportClock, GROUP_CONFIRMATION, rank, clubNumber, money);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
		            printf("[%d][%ld]Akceptuje zaproszenie do grupy od RANK: %d\n",rank, lamportClock, recv.rank);
            }
            else
            {
                lamportClock++;
                send = createPackage(lamportClock, REJECT_INVITE_MSG, rank, clubNumber, money);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
		            printf("[%d][%ld]Odrzucam zaproszenie do grupy od RANK: %d\n",rank, lamportClock, recv.rank);
            }
        }

        //9
        if(status == NO_GROUP && recv.message == GROUP_CONFIRMATION)
        {
            status = ACCEPT_INVITE;
            *(tab+recv.rank) = MY_GROUP;
            groupMoney += recv.money;
      	    printf("[%d][%ld]RANK: %d dalacza do grupy!\n",rank, lamportClock,  recv.rank);
      	    printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", rank, lamportClock,  groupMoney, M);
        }

        //10
        if(status == NO_GROUP && recv.message == REJECT_INVITE_MSG)
        {
            status = GROUP_BREAK;
            *(tab+recv.rank) = NOT_MY_GROUP;
	           printf("[%d][%ld]Moje zaproszenie zostalo odrzucone od RANK: %d\n",rank, lamportClock,  recv.rank);
        }

        //11
        if(status == PARTICIPATOR && recv.message == GROUP_CONFIRMATION)
        {
            lamportClock++;
            send = createPackage(lamportClock, GROUP_BREAK_MSG, rank, clubNumber, money);
            MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
	          printf("[%d][%ld]Zrywam grupe z RANK: %d (moje zaproszenie jest juz nie aktualne)\n",rank, lamportClock, recv.rank);
        }

        //12
        if(status == PARTICIPATOR && recv.message == GROUP_BREAK_MSG)
        {
            status = GROUP_BREAK;
	          printf("[%d][%ld]Grupa zostala rozwiazana przez RANK: %d!\n",rank, lamportClock, recv.rank);
        }

        //13
        if(recv.message == EXIT_CLUB_MSG)
        {
	           clubNumber = recv.clubNumber;
              printf("[%d][%ld]        Wychodze z klubu jako czlonek grupy! Nr klubu: %d\n", rank, lamportClock, clubNumber);
	            status = EXIT_CLUB;
        }

        //14
        if(status == ENTER_CLUB && recv.message == ENTER_CLUB_QUERY)
        {
            if(recv.clubNumber != clubNumber)
            {
                lamportClock++;
                send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, money);
                MPI_Send(&send, 1, mpi_data, recv.rank, TAG, MPI_COMM_WORLD);
		           printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d  dla RANK: %d\n",rank, lamportClock, recv.rank,  recv.clubNumber);
            }
        }
    }

    pthread_exit(NULL);
}
