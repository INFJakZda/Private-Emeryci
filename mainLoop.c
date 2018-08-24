#include "main.h"

void mainLoop()
{
  while(true)
    {
        bool restart = false;
        //Zmienne wspoldzielone

        sleep(rand() % 4);  // Konieczny sleep bo się rozpierdala

        money = rand() % (M - 2) + 1;
        groupMoney = money;
        approveCount = 0;
        status = NO_GROUP;
        clubNumber = -1;
        tab = calloc(N, sizeof(int));

        for(int i=0;i<N;i++)
        {
            *(tab+i) = NOT_ASKED;
            if(i == rank)
                *(tab+i) = MY_GROUP;
        }


        data send;
        while(isSomeoneToAsk())
        {
            lamportClock++;
            send = createPackage(lamportClock, GROUP_INVITE, rank, clubNumber, money);
            int random = getRandomFreeElder();
            MPI_Send(&send, 1, mpi_data, random, TAG, MPI_COMM_WORLD);
            printf("[%d][%ld]        Zapytanie o dolaczenie do grupy dla RANK: %d\n",rank, lamportClock,  random);
            while(status == NO_GROUP || status == PARTICIPATOR || status == FOUNDER)
            {
                //waiting for status update
            }

            switch(status)
            {
                case ACCEPT_INVITE:
                  status = FOUNDER;
                  if(groupMoney >= M)
                  {
                      printf("[%d][%ld]        Mamy wystarczajaca ilosc pieniedzy(mamy: %d, wymagane: %d)! Przechodze do wyboru klubu. \n", rank, lamportClock,  groupMoney, M);
                      goto ExitWHILE;
                  }
                  break;

                case REJECT_INVITE:
                  status = FOUNDER;
                  break;

                case GROUP_BREAK:
                  groupMoney = money;
                  status = NO_GROUP;
                  break;

                case EXIT_CLUB:
                  printf("[%d][%ld]        Wychodze jako czlonek grupy z klubu o nr: %d\n", rank, lamportClock,  clubNumber);
                  restart = true;
                  goto ExitWHILE;
                  break;
            }
        }

        ExitWHILE: ;  // Konieczne GOTO

        //Wychodzimy z klubu (dla emerytów nie będących założycielami)
        if(!restart)
        {
          //Jeżeli za mało pieniędzy oznacza że zapytał wszystkich i nie da rady więc rozwiązuje grupę
          if(groupMoney < M && status == FOUNDER)
          {
              for(int i=0;i<N;i++)
              {
                  if(*(tab+i) == MY_GROUP && i != rank)
                  {
                      lamportClock++;
                      send = createPackage(lamportClock, GROUP_BREAK_MSG, rank, clubNumber, money);
                      MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie (oprócz mnie)
                      printf("[%d][%ld]        Rozwiazanie grupy dla RANK: %d\n",rank, lamportClock, i);
                  }
              }
          }

          //Jeżeli mamy siano i możemy ubiegać się o wejście
          if(groupMoney >= M && status == FOUNDER)
          {
             printf("[%d][%ld]        Wybieramy klub!\n", rank, lamportClock);
              status = ENOUGH_MONEY;
              clubNumber = rand() % K;
              printf("[%d][%ld]        Wybralismy klub o nr: %d\n", rank, lamportClock,  clubNumber);
              for(int i=0;i<N;i++)
              {
                  if(i != rank)
                  {
                    lamportClock++;
                    send = createPackage(lamportClock, ENTER_CLUB_QUERY, rank, clubNumber, money);
                    MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich zapytanie o wejście do klubu
                    printf("[%d][%ld]        Zapytanie o wejscie do klubu o nr: %d dla RANK: %d\n",rank, lamportClock, clubNumber, i);
                }
             }
            printf("[%d][%ld]        Czekamy na pozwolenia na wejscie do klubu o nr: %d\n", rank, lamportClock,  clubNumber);
            while(status != ENTER_CLUB)
            {
                  //waiting for perrmisions to go to club
            }

            printf("[%d][%ld]       Mamy pozwolenie na wejscie do klubu o nr: %d\n", rank, lamportClock,  clubNumber);
            if(status == ENTER_CLUB)
            {
                  for(int i=0;i<N;i++)
                  {
                      if(*(tab+i) == MY_GROUP && i != rank)
                      {
                          lamportClock++;
                          send = createPackage(lamportClock, EXIT_CLUB_MSG, rank, clubNumber, money);
                          MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich którzy są w mojej grupie info o wyjściu z klubu
                         printf("[%d][%ld]        Informacja --> Koniec imprezy dla RANK: %d\n",rank, lamportClock,i);
                      }
                  }
                  for(int i=0;i<N;i++)
                  {
                      if(i != rank && *(tab+i) != MY_GROUP)
                      {
                        lamportClock++;
                        send = createPackage(lamportClock, ENTER_PERMISSION, rank, clubNumber, money);
                        MPI_Send(&send, 1, mpi_data, i, TAG, MPI_COMM_WORLD); //Wyślij do wszystkich info o możliwości wejścia do klubu w którym byliśmy
                        printf("[%d][%ld]        Pozwolenie na wejscie do naszego klubu (nr: %d) dla RANK: %d\n",rank, lamportClock, clubNumber, i);
                      }
                 }
          }

          printf("[%d][%ld]        Kapitan wychodzi z klubu o nr: %d\n", rank, lamportClock, clubNumber);
        }
     }
  }
}
