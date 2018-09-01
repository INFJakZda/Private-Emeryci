#include "main.h"

void *ThreadBehavior()
{
    msg recvMsg;
    msg sendMsg;
    while (true)
    {
        //1
        MPI_Recv(&recvMsg, 1, mpiMsgType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        localClock = max(recvMsg.localClock, localClock) + 1;

        //2
        if (myStatus == ENOUGH_MONEY_STATUS && recvMsg.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recvMsg.preferedClubId != preferedClubId)
            {
                localClock++;
                sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recvMsg.preferedClubId, recvMsg.memberId);
            }
            else
            {
                if (recvMsg.localClock < localClock)
                {
                    localClock++;
                    sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                    MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                    printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recvMsg.preferedClubId, recvMsg.memberId);
                }
            }
        }

        //3
        if ((myStatus != ALONE_STATUS && myStatus != GROUP_BREAK_STATUS) && recvMsg.message == ASK_TO_JOIN_MSG)
        {
            localClock++;
            sendMsg = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d\n", memberId, localClock, recvMsg.memberId);
        }

        //4
        if (myStatus == LEADER_STATUS && recvMsg.message == CONFIRM_JOIN_MSG)
        {
            groupMoney += recvMsg.memberMoney;
            *(askTab + recvMsg.memberId) = ACCEPT_ASK_TAB;
            myStatus = ACCEPT_INVITATION_STATUS;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", memberId, localClock, recvMsg.memberId);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", memberId, localClock, groupMoney, entryCost);
        }

        //5
        if (myStatus == LEADER_STATUS && recvMsg.message == REJECT_JOIN_MSG)
        {
            *(askTab + recvMsg.memberId) = REJECT_ASK_TAB;
            myStatus = REJECT_INVITATION_STATUS;
            printf("[%d][%ld]Odrzucenie proponowanej grupy od RANK: %d(jestem Kapitanem)\n", memberId, localClock, recvMsg.memberId);
        }

        //6
        if (myStatus == ENOUGH_MONEY_STATUS && recvMsg.message == AGREE_TO_ENTER_CLUB_MSG)
        {
            approveCount++;
            printf("[%d][%ld]Pozwolenie dla mnie na wejscie do klubu o nr: %d od RANK: %d\n", memberId, localClock, preferedClubId, recvMsg.memberId);
            if (approveCount == noMembers - 1)
            {
                myStatus = ENTER_CLUB_STATUS;
            }
        }

        //7
        if (myStatus != ENOUGH_MONEY_STATUS && myStatus != ENTER_CLUB_STATUS && recvMsg.message == ASK_TO_ENTER_CLUB_MSG)
        {
            localClock++;
            sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d dla RANK: %d\n", memberId, localClock, recvMsg.preferedClubId, recvMsg.memberId);
        }

        //8
        if ((myStatus == ALONE_STATUS || myStatus == GROUP_BREAK_STATUS) && recvMsg.message == ASK_TO_JOIN_MSG)
        {
            if (recvMsg.localClock < localClock)
            {
                myStatus = MEMBER_STATUS;
                localClock++;
                sendMsg = createPackage(localClock, CONFIRM_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Akceptuje zaproszenie do grupy od RANK: %d\n", memberId, localClock, recvMsg.memberId);
            }
            else
            {
                localClock++;
                sendMsg = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Odrzucam zaproszenie do grupy od RANK: %d\n", memberId, localClock, recvMsg.memberId);
            }
        }

        //9
        if (myStatus == ALONE_STATUS && recvMsg.message == CONFIRM_JOIN_MSG)
        {
            myStatus = ACCEPT_INVITATION_STATUS;
            *(askTab + recvMsg.memberId) = ACCEPT_ASK_TAB;
            groupMoney += recvMsg.memberMoney;
            printf("[%d][%ld]RANK: %d dalacza do grupy!\n", memberId, localClock, recvMsg.memberId);
            printf("[%d][%ld]Jestem kapitanem, mamy na razie: %d a potrzeba %d pieniedzy.\n", memberId, localClock, groupMoney, entryCost);
        }

        //10
        if (myStatus == ALONE_STATUS && recvMsg.message == REJECT_JOIN_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            *(askTab + recvMsg.memberId) = REJECT_ASK_TAB;
            printf("[%d][%ld]Moje zaproszenie zostalo odrzucone od RANK: %d\n", memberId, localClock, recvMsg.memberId);
        }

        //11
        if (myStatus == MEMBER_STATUS && recvMsg.message == CONFIRM_JOIN_MSG)
        {
            localClock++;
            sendMsg = createPackage(localClock, GROUP_BREAK_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]Zrywam grupe z RANK: %d (moje zaproszenie jest juz nie aktualne)\n", memberId, localClock, recvMsg.memberId);
        }

        //12
        if (myStatus == MEMBER_STATUS && recvMsg.message == GROUP_BREAK_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            printf("[%d][%ld]Grupa zostala rozwiazana przez RANK: %d!\n", memberId, localClock, recvMsg.memberId);
        }

        //13
        if (recvMsg.message == EXIT_CLUB_MSG)
        {
            preferedClubId = recvMsg.preferedClubId;
            printf("[%d][%ld]        Wychodze z klubu jako czlonek grupy! Nr klubu: %d\n", memberId, localClock, preferedClubId);
            myStatus = EXIT_CLUB_STATUS;
        }

        //14
        if (myStatus == ENTER_CLUB_STATUS && recvMsg.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recvMsg.preferedClubId != preferedClubId)
            {
                localClock++;
                sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[%d][%ld]Pozwolenie na wejscie do klubu o nr: %d  dla RANK: %d\n", memberId, localClock, recvMsg.memberId, recvMsg.preferedClubId);
            }
        }
    }

    pthread_exit(NULL);
}
