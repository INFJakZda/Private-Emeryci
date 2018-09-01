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
        printf("[myId: %d][clock: %d][from: %d] Odebrałem wiadomość {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);

       //2 TODO what's that if else ?!?!?!?
        if (myStatus == ENOUGH_MONEY_STATUS && recvMsg.message == ASK_TO_ENTER_CLUB_MSG)
        {
            if (recvMsg.preferedClubId != preferedClubId)
            {
                localClock++;
                sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[myId: %d][clock: %d][to:   %d] Pozwolenie na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, recvMsg.memberId, recvMsg.preferedClubId, myStatus);
            }
            else
            {
                if (recvMsg.localClock < localClock)
                {
                    localClock++;
                    sendMsg = createPackage(localClock, AGREE_TO_ENTER_CLUB_MSG, memberId, preferedClubId, memberMoney);
                    MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                    printf("[myId: %d][clock: %d][to:   %d] Pozwolenie na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, recvMsg.memberId, recvMsg.preferedClubId, myStatus);
                }
            }
        }

        //3
        if ((myStatus != ALONE_STATUS && myStatus != GROUP_BREAK_STATUS) && recvMsg.message == ASK_TO_JOIN_MSG)
        {
            localClock++;
            sendMsg = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
            printf("[myId: %d][clock: %d][to:   %d] Odrzucenie zaproszenia do grupy {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);
        }

        //4 i //9
        if ((myStatus == LEADER_STATUS || myStatus == ALONE_STATUS)  && recvMsg.message == CONFIRM_JOIN_MSG)
        {
            groupMoney += recvMsg.memberMoney;
            *(askTab + recvMsg.memberId) = ACCEPT_ASK_TAB;
            myStatus = ACCEPT_INVITATION_STATUS;
            printf("[myId: %d][clock: %d][from: %d] dołączył do grupy! Mamy teraz %d na %d pieniedzy {%d}\n", memberId, localClock, recvMsg.memberId, groupMoney, entryCost, myStatus);
        }

        //5
        if (myStatus == LEADER_STATUS && recvMsg.message == REJECT_JOIN_MSG)
        {
            *(askTab + recvMsg.memberId) = REJECT_ASK_TAB;
            myStatus = REJECT_INVITATION_STATUS;
            printf("[myId: %d][clock: %d][from: %d] Odrzucenie propozycji dołączenia do grupy {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);
        }

        //6
        if (myStatus == ENOUGH_MONEY_STATUS && recvMsg.message == AGREE_TO_ENTER_CLUB_MSG)
        {
            approveCount++;
            printf("[myId: %d][clock: %d][from: %d] Pozwolenie dla mnie na wejscie do klubu %d {%d}\n", memberId, localClock, recvMsg.memberId, preferedClubId, myStatus);
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
            printf("[myId: %d][clock: %d][to:   %d] Pozwolenie na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, recvMsg.memberId, recvMsg.preferedClubId, myStatus);
        }

        //8 TODO what that?!?! if elses every time localClock will be higher than recvMsg.localClock becouse we take max() + 1 to localClock
        if ((myStatus == ALONE_STATUS || myStatus == GROUP_BREAK_STATUS) && recvMsg.message == ASK_TO_JOIN_MSG)
        {
            if (recvMsg.localClock < localClock)
            {
                myStatus = MEMBER_STATUS;
                localClock++;
                sendMsg = createPackage(localClock, CONFIRM_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[myId: %d][clock: %d][to:   %d] Akceptuje zaproszenie do grupy {%d} \n", memberId, localClock, recvMsg.memberId, myStatus);
            }
            else
            {
                localClock++;
                sendMsg = createPackage(localClock, REJECT_JOIN_MSG, memberId, preferedClubId, memberMoney);
                MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
                printf("[myId: %d][clock: %d][to:   %d] Odrzucam zaproszenie do grupy {%d} \n", memberId, localClock, recvMsg.memberId, myStatus);
            }
        }


        //10 TODO why group break? could continue find another members?
        if (myStatus == ALONE_STATUS && recvMsg.message == REJECT_JOIN_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            *(askTab + recvMsg.memberId) = REJECT_ASK_TAB;
            printf("[myId: %d][clock: %d][from: %d] Moje zaproszenie zostalo odrzucone {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);
        }

        //11
        if (myStatus == MEMBER_STATUS && recvMsg.message == CONFIRM_JOIN_MSG)
        {
            localClock++;
            sendMsg = createPackage(localClock, GROUP_BREAK_MSG, memberId, preferedClubId, memberMoney);
            MPI_Send(&sendMsg, 1, mpiMsgType, recvMsg.memberId, TAG, MPI_COMM_WORLD);
            printf("[myId: %d][clock: %d][from: %d] Zrywam grupe (moje zaproszenie jest juz nie aktualne) {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);
        }

        //12
        if (myStatus == MEMBER_STATUS && recvMsg.message == GROUP_BREAK_MSG)
        {
            myStatus = GROUP_BREAK_STATUS;
            printf("[myId: %d][clock: %d][from: %d] Grupa zostala rozwiazana {%d}\n", memberId, localClock, recvMsg.memberId, myStatus);
        }

        //13
        if (recvMsg.message == EXIT_CLUB_MSG)
        {
            preferedClubId = recvMsg.preferedClubId;
            printf("[myId: %d][clock: %d][from: %d] Wychodze z klubu jako czlonek grupy! Nr klubu: %d {%d}\n", memberId, localClock, recvMsg.memberId, preferedClubId, myStatus);
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
                printf("[myId: %d][clock: %d][to:   %d] Pozwolenie na wejscie do klubu o nr: %d {%d}\n", memberId, localClock, recvMsg.memberId, recvMsg.preferedClubId, myStatus);
            }
        }
    }
    pthread_exit(NULL);
}