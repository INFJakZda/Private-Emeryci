#include "main.h"

void mainLoop()
{
    while (true)
    {
        bool restart = false;
        //Zmienne wspoldzielone

        sleep(rand() % 4); // Konieczny sleep bo się rozpierdala

        memberMoney = rand() % (entryCost - 2) + 1;
        groupMoney = memberMoney;
        approveCount = 0;
        myStatus = ALONE_STATUS;
        preferedClubId = -1;
        askTab = calloc(noMembers, sizeof(int));

        for (int i = 0; i < noMembers; i++)
        {
            *(askTab + i) = READY_ASK_TAB;
            if (i == memberId)
                *(askTab + i) = ACCEPT_ASK_TAB;
        }

        data send;
        while (isSomeoneToAsk())
        {
            localClock++;
            send = createPackage(localClock, ASK_TO_JOIN_MSG, memberId, preferedClubId, memberMoney);
            int random = getRandomFreeElder();
            MPI_Send(&send, 1, mpi_data, random, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]        Zapytanie o dolaczenie do grupy dla RANK: %d\n", memberId, localClock, random);
            while (myStatus == ALONE_STATUS || myStatus == MEMBER_STATUS || myStatus == LEADER_STATUS)
            {
                //waiting for myStatus update
            }

            switch (myStatus)
            {
            case ACCEPT_INVITATION_STATUS:
                myStatus = LEADER_STATUS;
                if (groupMoney >= entryCost)
                {
                    printf("[%d][%ld]        Mamy wystarczajaca ilosc pieniedzy(mamy: %d, wymagane: %d)! Przechodze do wyboru klubu. \n", memberId, localClock, groupMoney, entryCost);
                    goto ExitWHILE;
                }
                break;

            case REJECT_INVITATION_STATUS:
                myStatus = LEADER_STATUS;
                break;

            case GROUP_BREAK_STATUS:
                groupMoney = memberMoney;
                myStatus = ALONE_STATUS;
                break;

            case EXIT_CLUB_STATUS:
                printf("[%d][%ld]        Wychodze jako czlonek grupy z klubu o nr: %d\n", memberId, localClock, preferedClubId);
                restart = true;
                goto ExitWHILE;
                break;
            }
        }

    ExitWHILE:; // Konieczne GOTO

        //Wychodzimy z klubu (dla emerytów nie będących założycielami)
        if (!restart)
        {
            //Jeżeli za mało pieniędzy oznacza że zapytał wszystkich i nie da rady więc rozwiązuje grupę
            if (groupMoney < entryCost && myStatus == LEADER_STATUS)
            {
                for (int i = 0; i < noMembers; i++)
                {
                    if (*(askTab + i) == ACCEPT_ASK_TAB && i != memberId)
                    {
                        localClock++;
                        send = createPackage(localClock, GROUP_BREAK_MSG, memberId, preferedClubId, memberMoney);
                        MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie (oprócz mnie)
                        printf("[%d][%ld]        Rozwiazanie grupy dla RANK: %d\n", memberId, localClock, i);
                    }
                }
            }

            //Jeżeli mamy siano i możemy ubiegać się o wejście
            if (groupMoney >= entryCost && myStatus == LEADER_STATUS)
            {
                printf("[%d][%ld]        Wybieramy klub!\n", memberId, localClock);
                myStatus = ENOUGH_MONEY_STATUS;
                preferedClubId = rand() % noClubs;
                printf("[%d][%ld]        Wybralismy klub o nr: %d\n", memberId, localClock, preferedClubId);
                for (int i = 0; i < noMembers; i++)
                {
                    if (i != memberId)
                    {
                        localClock++;
                        send = createPackage(localClock, ASK_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                        MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich zapytanie o wejście do klubu
                        printf("[%d][%ld]        Zapytanie o wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, preferedClubId, i);
                    }
                }
                printf("[%d][%ld]        Czekamy na pozwolenia na wejscie do klubu o nr: %d\n", memberId, localClock, preferedClubId);
                while (myStatus != ENTER_CLUB_STATUS)
                {
                    //waiting for perrmisions to go to club
                }

                printf("[%d][%ld]       Mamy pozwolenie na wejscie do klubu o nr: %d\n", memberId, localClock, preferedClubId);
                if (myStatus == ENTER_CLUB_STATUS)
                {
                    for (int i = 0; i < noMembers; i++)
                    {
                        if (*(askTab + i) == ACCEPT_ASK_TAB && i != memberId)
                        {
                            localClock++;
                            send = createPackage(localClock, EXIT_CLUB_MSG, memberId, preferedClubId, memberMoney);
                            MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie info o wyjściu z klubu
                            printf("[%d][%ld]        Informacja --> Koniec imprezy dla RANK: %d\n", memberId, localClock, i);
                        }
                    }
                    for (int i = 0; i < noMembers; i++)
                    {
                        if (i != memberId && *(askTab + i) != ACCEPT_ASK_TAB)
                        {
                            localClock++;
                            send = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                            MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich info o możliwości wejścia do klubu w którym byliśmy
                            printf("[%d][%ld]        Pozwolenie na wejscie do naszego klubu (nr: %d) dla RANK: %d\n", memberId, localClock, preferedClubId, i);
                        }
                    }
                }

                printf("[%d][%ld]        Kapitan wychodzi z klubu o nr: %d\n", memberId, localClock, preferedClubId);
            }
        }
    }
}
