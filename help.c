#include "main.h"

data createPackage(int lC, int mcd, int rnk, int clubNum, int mny)
{
  data package;

  package.lamportClock = lC;
  package.message = mcd;
  package.rank = rnk;
  package.clubNumber = clubNum;
  package.money = mny;
  return package;
}

void createThread()
{
    int create_result = 0;

    pthread_t thread1;

    create_result = pthread_create(&thread1, NULL, ThreadBehavior, NULL);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }

}

bool isSomeoneToAsk()
{
	for(int i=0;i<N;i++)
  {
		if(*(tab+i)==0)
    {
			return true;
		}
	}
	return false;
}

int getRandomFreeElder()
{
	int rnd = rand() % N;
	if(*(tab+rnd)==0)
		return rnd;
	for(int i=rnd;i<N;i++)
  {
		if(*(tab+i)==0)
    {
			return i;
		}
	}
	for(int i=0;i<N;i++)
  {
		if(*(tab+i)==0)
    {
			return i;
		}
	}
	return -1;
}

int max(int a, int b)
{
	if(a>b)
		return a;
	else
		return b;
}
