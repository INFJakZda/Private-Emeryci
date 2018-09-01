#include "main.h"

void mainLoop()
{
    while (true)
    {
        bool restart = false;
        //Zmienne wspoldzielone

        sleep(rand() % noMembers);

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

        msg sendMsg;
        while (isSomeoneToAsk())
        {
            if (myStatus != MEMBER_STATUS)
            {
                localClock++;
                sendMsg = createPackage(localClock, ASK_TO_JOIN_MSG, memberId, preferedClubId, memberMoney);
                int random = getRandomFreeElder();
                MPI_Send(&sendMsg, 1, mpiMsgType, random, TAG, MPI_COMM_WORLD);
                printf("[myId: %d][clock: %d][to:   %d] Zapytanie o dolaczenie do grupy {%d}\n", memberId, localClock, selectedMember, myStatus);
            }

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
                    printf("[myId: %d][clock: %d]           Zebraliśmy pieniądze: %d na %d. Wybieramy klub. {%d}\n", memberId, localClock, groupMoney, entryCost, myStatus);
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
                printf("[myId: %d][clock: %d] Wychodzę z klubu [%d] {%d}\n", memberId, localClock, preferedClubId, myStatus);
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
                        sendMsg = createPackage(localClock, GROUP_BREAK_MSG, memberId, preferedClubId, memberMoney);
                        MPI_Send(&sendMsg, 1, mpiMsgType, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie (oprócz mnie)
                        printf("[myId: %d][clock: %d]        Rozwiazanie grupy dla RANK: %d {%d}\n", memberId, localClock, i, myStatus);
                    }
                }
            }

            //Jeżeli mamy siano i możemy ubiegać się o wejście
            if (groupMoney >= entryCost && myStatus == LEADER_STATUS)
            {
                printf("[myId: %d][clock: %d]           Wybieramy klub! {%d}\n", memberId, localClock, myStatus);
                myStatus = ENOUGH_MONEY_STATUS;
                preferedClubId = rand() % noClubs;
                printf("[myId: %d][clock: %d]           Wybralismy klub o nr: %d {%d}\n", memberId, localClock, preferedClubId, myStatus);
                for (int i = 0; i < noMembers; i++)
                {
                    if (i != memberId)
                    {
                        localClock++;
                        sendMsg = createPackage(localClock, ASK_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                        MPI_Send(&sendMsg, 1, mpiMsgType, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich zapytanie o wejście do klubu
                        printf("[myId: %d][clock: %d]           Zapytanie o wejscie do klubu o nr: %d dla RANK: %d {%d}\n", memberId, localClock, preferedClubId, i, myStatus);
                    }
                }
                printf("[myId: %d][clock: %d]           Czekamy na pozwolenia na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, preferedClubId, myStatus);
                while (myStatus != ENTER_CLUB_STATUS)
                {
                    //waiting for perrmisions to go to club
                }

                printf("[myId: %d][clock: %d]       Mamy pozwolenie na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, preferedClubId, myStatus);
                if (myStatus == ENTER_CLUB_STATUS)
                {
                    for (int i = 0; i < noMembers; i++)
                    {
                        if (*(askTab + i) == ACCEPT_ASK_TAB && i != memberId)
                        {
                            localClock++;
                            sendMsg = createPackage(localClock, EXIT_CLUB_MSG, memberId, preferedClubId, memberMoney);
                            MPI_Send(&sendMsg, 1, mpiMsgType, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie info o wyjściu z klubu
                            printf("[myId: %d][clock: %d]        Informacja --> Koniec imprezy dla RANK: %d {%d}\n", memberId, localClock, i, myStatus);
                        }
                    }
                    for (int i = 0; i < noMembers; i++)
                    {
                        if (i != memberId && *(askTab + i) != ACCEPT_ASK_TAB)
                        {
                            localClock++;
                            sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                            MPI_Send(&sendMsg, 1, mpiMsgType, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich info o możliwości wejścia do klubu w którym byliśmy
                            printf("[myId: %d][clock: %d]        Pozwolenie na wejscie do naszego klubu (nr: %d) dla RANK: %d {%d}\n", memberId, localClock, preferedClubId, i, myStatus);
                        }
                    }
                }

                printf("[myId: %d][clock: %d]        Kapitan wychodzi z klubu o nr: %d {%d}\n", memberId, localClock, preferedClubId, myStatus);

            }
            //EXIT CLUBS IN DIFFERERNT TIME
            // int sleepTime = rand() % noMembers;
            // localClock += sleepTime;
            // sleep(sleepTime);
            // printf("[myId: %d][clock: %d]           Spałem %d sekund\n", memberId, localClock, sleepTime);
        }
    }
}
