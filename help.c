#include "main.h"

msg createPackage(int localClock, int message, int memberId, int preferedClubId, int memberMoney)
{
    msg package;

    package.localClock = localClock;
    package.message = message;
    package.memberId = memberId;
    package.preferedClubId = preferedClubId;
    package.memberMoney = memberMoney;
    return package;
}

void createThread()
{
    int create_result = 0;

    pthread_t thread1;

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, NULL);
    if (create_result)
    {
        printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
        exit(-1);
    }
}

bool isSomeoneToAsk()
{
    for (int i = 0; i < noMembers; i++)
    {
        if (*(askTab + i) == 0)
        {
            return true;
        }
    }
    return false;
}

int getRandomFreeElder()
{
    int rnd = rand() % noMembers;
    if (*(askTab + rnd) == 0)
        return rnd;
    for (int i = rnd; i < noMembers; i++)
    {
        if (*(askTab + i) == 0)
        {
            return i;
        }
    }
    for (int i = 0; i < noMembers; i++)
    {
        if (*(askTab + i) == 0)
        {
            return i;
        }
    }
    return -1;
}

int max(int a, int b)
{
    if (a > b)
        return a;
    else
        return b;
}
