#include <stdio.h>
#include <stdlib.h>

#define MAX_SIZE 10          // Tamanho máximo de processos por fila
#define NUM_PROC 5            // Numero de Processos a serem criados
#define QUANTUM 1         // Tempo de fatia para cada processo
#define MAX_COUNT 50       // Tempo maximo de execução do Simulador
#define TIOFITA 4        // Tempo de serviço da Fita
#define TIODISCO 5       // Tempo de serviço do disco
#define TIOIMPRESSORA 6         // Tempo de serviço da Impressora

/*status dos processos para identificação de prioridade*/
#define ALTAPRI 5
#define STATUSALTAPRI 1
#define PREENPTADO 3

typedef struct Queue{
    int size;
    int first;
    int last;
    int items[MAX_SIZE];
} Queue;

Queue* createQueue() {
    Queue *queue = (Queue*)malloc(sizeof(Queue));
    queue->size = 0;
    queue->first = 0;
    queue->last = MAX_SIZE - 1;

    return queue;
}

void enq(Queue *queue, int item) {
    if(queue->size >= MAX_SIZE) {
        printf("Fila cheia!");
    }

    else {
        queue->last = (queue->last + 1) % MAX_SIZE;
        queue->items[queue->last] = item;
        queue->size++;
    }
}

void deq(Queue *queue) {
    if(queue->size <= 0) {
        printf("Fila vazia!");
    }

    else {
        queue->first = (queue->first + 1) % MAX_SIZE;
        queue->size--;
    }
}

/*estrutura PCB*/
typedef struct {
      int PID;
      int PPID;
      int status;
      int tempoAtual;
      int tempoChegada;
      int tserv;
      int tIOFita;
      int tIODisco;
      int tIOImpressora;
      int prioridade;
      int esperaIO;
} ProcessoPCB;

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
    {
        r = rand();
    } while (r >= limit);

    return min + (r / buckets);
}


int main(void) {

    int i, x;
    int probIO = 0;

    // Inicializar as filas
    Queue *Proc = createQueue();
    Queue *BaixaP = createQueue();
    Queue *IO_Fita = createQueue();
    Queue *IO_Disco = createQueue();
    Queue *IO_Imp = createQueue();

    // Criando os processos e enfilerando na fila de ALTA PRIORIDADE
    ProcessoPCB proce[NUM_PROC];

    for ( i = 0; i < NUM_PROC; i++) {

      proce[i].PID = i;
      proce[i].PPID = rand_interval(0,NUM_PROC);
      proce[i].tempoChegada = rand_interval(1,MAX_COUNT);
      proce[i].tempoAtual = 0;
      proce[i].status = STATUSALTAPRI;
      proce[i].prioridade = ALTAPRI;
      proce[i].tserv = rand_interval(2,10);

      probIO = (rand() % 12);
      if (probIO <= 2) {
          proce[i].tIOImpressora = 1 + (rand() % proce[i].tserv);
          proce[i].tIOFita = 0;
          proce[i].tIODisco = 0;

      }
      if (probIO > 2 && probIO <= 4) {
          proce[i].tIOImpressora = 0;
          proce[i].tIOFita = 1 + (rand() % proce[i].tserv);
          proce[i].tIODisco = 0;
      }
      if (probIO > 4 && probIO <= 6) {
          proce[i].tIOImpressora = 0;
          proce[i].tIOFita = 0;
          proce[i].tIODisco = 1 + (rand() % proce[i].tserv);
      }
      if (probIO > 6) {
          proce[i].tIOImpressora = 0;
          proce[i].tIOFita = 0;
          proce[i].tIODisco = 0;
      }
      //rand_interval(0,proce[i].tserv);

      printf("PID : %d\n", proce[i].PID );
      printf("PPID : %d\n", proce[i].PPID );
      printf("tempoChegada : %d\n", proce[i].tempoChegada );
      printf("tserv : %d\n", proce[i].tserv );
      printf("tIOImpressora : %d\n", proce[i].tIOImpressora );
      printf("tIOFita : %d\n", proce[i].tIOFita );
      printf("tIODisco : %d\n\n", proce[i].tIODisco );

      enq(Proc, proce[i].PID);
    }


    for ( int i = 0; i < Proc->size; i++ ) {
      printf( "Item na posicao %d da Proc, PID : %d\n", i, Proc->items[ i ] );
    }

    printf("\n ------ Comeco DO WHILE -------- \n\n");

    // Começando o contador do Simulador
    int countime = 0;

      while ((Proc->size) != 0 || (IO_Imp->size) != 0|| (IO_Disco->size)!= 0 || (IO_Fita->size)!= 0 || (BaixaP->size)!= 0) {
        printf("\n ---------- Tempo AGORA: %d ---------------- \n\n", countime);
        /*
          printf("PROC SIZE : %d\n", Proc->size );
          printf("BAIXA SIZE : %d\n", BaixaP->size );
          printf("IO_Imp SIZE : %d\n", IO_Imp->size );
          printf("IO_Disco SIZE : %d\n", IO_Disco->size );
          printf("IO_Fita SIZE : %d\n", IO_Fita->size );
        */

          if (IO_Imp->size > 0) {
              int y;
              y = IO_Imp->items[0];
              proce[y].esperaIO = proce[y].esperaIO - 1;
  			      proce[y].tempoAtual += 1;
              if (proce[y].esperaIO == 0) {
                  //proce[y].tserv = proce[y].tserv - (QUANTUM);
  				    proce[y].tempoAtual -= TIOIMPRESSORA + 1;
                  if(proce[y].tserv>0){
                      enq(Proc, proce[y].PID);
                      deq(IO_Imp);
                  }
                  else {
                      deq(IO_Imp);
                  }

              }
          }
          if (IO_Fita->size > 0) {
              int y;
              y = IO_Fita->items[0];
              proce[y].esperaIO = proce[y].esperaIO - 1;
  			      proce[y].tempoAtual += 1;
              if (proce[y].esperaIO == 0) {
                  //proce[y].tserv = proce[y].tserv - (QUANTUM);
  				        proce[y].tempoAtual -= TIOFITA + 1;
                  if(proce[y].tserv>0){
                      enq(Proc, proce[y].PID);
                      deq(IO_Fita);
                  }
                  else {
                      deq(IO_Fita);
                  }

              }
          }
          if (IO_Disco->size > 0) {
              int y;
              y = IO_Disco->items[0];
              proce[y].esperaIO = proce[y].esperaIO - 1;
  			      proce[y].tempoAtual += 1;
              if (proce[y].esperaIO == 0) {
  				    proce[y].tempoAtual -= TIODISCO + 1;
                  if(proce[y].tserv>0){
                      enq(BaixaP, proce[y].PID);
                      deq(IO_Disco);
                  }
                  else {
                      deq(IO_Disco);
                  }

              }
          }

          if (Proc->size > 0) {

              x = Proc->items[0];

              if ((proce[x].tempoAtual !=0) && (proce[x].tIOFita == proce[x].tempoAtual || proce[x].tIOImpressora == proce[x].tempoAtual ||
                  proce[x].tIODisco == proce[x].tempoAtual)) {

                  if (proce[x].tIOFita == proce[x].tempoAtual) {
                      //proce[x].tserv = proce[x].tserv - (QUANTUM);
                      proce[x].esperaIO = TIOFITA;
                      enq(IO_Fita, proce[x].PID);
                      deq(Proc);
                  }
                  if (proce[x].tIOImpressora == proce[x].tempoAtual) {
                      //proce[x].tserv = proce[x].tserv - (QUANTUM);
                      proce[x].esperaIO = TIOIMPRESSORA;
                      enq(IO_Imp, proce[x].PID);
                      deq(Proc);
                  }
                  if (proce[x].tIODisco == proce[x].tempoAtual) {
                      //proce[x].tserv = proce[x].tserv - (QUANTUM);
                      proce[x].esperaIO = TIODISCO;
                      enq(IO_Disco, proce[x].PID);
                      deq(Proc);
                  }
              }
              else {

                      proce[x].tserv = proce[x].tserv - (QUANTUM);
  					          proce[x].tempoAtual += QUANTUM;
                      if(proce[x].tserv > 0) {
                          enq(BaixaP,proce[x].PID);
                          deq(Proc);
                      }
                      else {
                          deq(Proc);
                      }
                  }
              }
              if(Proc->size == 0 && BaixaP->size>0) {
                  int y = BaixaP->items[0];
                  enq(Proc, proce[y].PID);
                  deq(BaixaP);
              }

          for ( int i = 0; i < Proc->size; i++ ) {
            printf( "Item na posicao %d da Proc, PID : %d\n", i, Proc->items[ i ] );
          }

          for ( int i = 0; i < BaixaP->size; i++ ) {
            printf( "Item na posicao %d da BaixaP, PID : %d\n", i, BaixaP->items[ i ] );
          }


          for ( int i = 0; i < IO_Imp->size; i++ ) {
            printf( "Item na posicao %d da IO_Imp, PID : %d\n", i, IO_Imp->items[ i ] );
          }


          for ( int i = 0; i < IO_Fita->size; i++ ) {
            printf( "Item na posicao %d da IO_Fita, PID : %d\n", i, IO_Fita->items[ i ] );
          }

          for ( int i = 0; i < IO_Disco->size; i++ ) {
            printf( "Item na posicao %d da IO_Disco, PID : %d\n", i, IO_Disco->items[ i ] );
          }


  		countime += 1;
      }
  		printf("\nTempo total: %d ", countime);


    return 0;
}
