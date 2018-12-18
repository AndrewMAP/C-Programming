#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

//CONSTANTES
#define MAX_PROCESSES 20
#define MAX_FRAMES 64
#define MAX_PROCESS_TIME 8
#define MIN_PROCESS_TIME 5
#define TEMPO_PAGINA 0.1

//HEADER DAS FUNÇÕES
void exibe_memoria();
void exibe_swap();
void swap_out();
void atualizaMemLastPag();
int swap_in(int processo);

//STRUCTS
typedef struct Pagina{
    int num;
    int proc_id;
	struct Pagina *prox;
} Pagina;

typedef struct Memoria{
    Pagina *paginas;
	Pagina *lastPag;
    int num_paginas;
} Memoria;

//VARIÀVEIS GLOBAIS

time_t sys_tempo_i, sys_tempo_f;

Memoria mem;
Memoria swap;

pthread_mutex_t lock;

int curr_pid = 1;


//FUNÇÔES

int conta_pags(int id, int n_pag, int* existe){
	int num_pags = 0;
    *existe = 0;
	Pagina *pag;
	pag = mem.paginas;
	if (mem.num_paginas > 0) {
		while (pag->prox != NULL)
		{
			if (pag->proc_id == id) {
				num_pags++;
                if(pag->num == n_pag)
                    *existe = 1;
			}
			pag = pag->prox;
		}
		if(pag->proc_id == id) num_pags++;
	}
	return num_pags;
}

void mover_pagina_reutilizada(int id, int n_pag){
	Pagina *curr;
	curr = mem.paginas;
	if (mem.num_paginas > 0) {
        if(curr->proc_id == id && curr->num == n_pag){
            mem.lastPag->prox = mem.paginas;
            mem.paginas = mem.paginas->prox;
            mem.lastPag = mem.lastPag->prox;
            mem.lastPag->prox = NULL;
        } else {
            while (curr->prox != NULL)
            {
                if (curr->prox->proc_id == id && curr->prox->num == n_pag){
                    mem.lastPag->prox = curr->prox;
                    curr->prox = curr->prox->prox;
                    mem.lastPag = mem.lastPag->prox;
                    mem.lastPag->prox = NULL;
                }
                curr = curr->prox;
            }
        }
        printf("[%d] Página %d já estava na memória, movida para mais recente.\n", id, n_pag);
	}
}

void carrega_paginas(int id, int n_pag){
    pthread_mutex_lock(&lock);
    printf("[%d] solicitou a página %d\n", id, n_pag);
    //pede página
    //caso carregada, mover pagina para o inicio
    //caso contrário, conta as páginas e se possível carrega a nova no início.
	Pagina pag, *curr_page;
    int jaExiste = 0;
    int paginas_carregadas = conta_pags(id, n_pag, &jaExiste);

    if(paginas_carregadas ==  0){
        // Se o processo ainda não estava na memória, tenta fazer swap in
        paginas_carregadas = swap_in(id);
    }

    if(jaExiste == 1){
        mover_pagina_reutilizada(id, n_pag);
    } else {
        if (paginas_carregadas >= 4) { //Já tem 4 páginas, substituir a mais antiga.
            if(mem.paginas->proc_id == id){
                mem.paginas = mem.paginas->prox;
            } else {
                curr_page = mem.paginas;
                while (curr_page->prox != NULL)
                {
                    if (curr_page->prox->proc_id == id) {
                        curr_page->prox = curr_page->prox->prox;
                        break;
                    }
                    curr_page = curr_page->prox;
                }
            }
            mem.lastPag->prox = malloc(sizeof(Pagina));
            mem.lastPag = mem.lastPag->prox;
            mem.lastPag->proc_id = id;
            mem.lastPag->num = n_pag;
        } else {  // Ainda tem menos de 4 páginas
            if (mem.num_paginas < MAX_FRAMES) { // a memória ainda tem espaço
                if (mem.num_paginas == 0) {
                    mem.paginas = malloc(sizeof(Pagina));
                    mem.lastPag = mem.paginas;
                } else {
                    mem.lastPag->prox = malloc(sizeof(Pagina));
                    mem.lastPag = mem.lastPag->prox;
                }
                mem.lastPag->proc_id = id;
                mem.lastPag->num = n_pag;
                mem.num_paginas++;
                sleep(TEMPO_PAGINA);
            } else { // A memória está lotada
                printf("[%d] Memória cheia, liberar espaço fazendo swap out.\n", id);
                swap_out(id);
                mem.lastPag->prox = malloc(sizeof(Pagina));
                mem.lastPag = mem.lastPag->prox;
                mem.lastPag->proc_id = id;
                mem.lastPag->num = n_pag;
                mem.num_paginas++;
                sleep(TEMPO_PAGINA);
            }
        }
    }
    exibe_memoria();
    pthread_mutex_unlock(&lock);
}

// Tira o processo mais antigo da fila que não seja o processo_solicitante
void swap_out(int processo_solicitante){

    Pagina *curr_p = mem.paginas;
    int posicao_processo = 1;
    int processos[MAX_PROCESSES] = {};

    while(curr_p != NULL){
        processos[curr_p->proc_id-1] = posicao_processo++;
        curr_p = curr_p->prox;
    }

    int processo_mais_velho = 0;
    posicao_processo = MAX_FRAMES;

    for(int i = 0; i < MAX_PROCESSES; i++){
        if(processos[i] != 0 && processos[i] <= posicao_processo && i+1 != processo_solicitante){
            posicao_processo = processos[i];
            processo_mais_velho = i+1;
        }
    }
    printf("\nSWAP OUT do processo %d...\n", processo_mais_velho);

    while(mem.paginas != NULL && mem.paginas->proc_id == processo_mais_velho){
        if(swap.lastPag == NULL) {
            swap.lastPag = mem.paginas;
            swap.paginas = swap.lastPag;
        } else {
            swap.lastPag->prox = mem.paginas;
            swap.lastPag = swap.lastPag->prox;
        }
        mem.paginas = mem.paginas->prox;
        swap.lastPag->prox = NULL;
        swap.num_paginas++;
        mem.num_paginas--;
    }

    curr_p = mem.paginas;
    while(curr_p -> prox != NULL){
        if(curr_p->prox->proc_id == processo_mais_velho){
            if(swap.lastPag == NULL) {
                swap.lastPag = curr_p->prox;
                swap.paginas = swap.lastPag;
            } else {
                swap.lastPag->prox = curr_p->prox;
                swap.lastPag = swap.lastPag->prox;
            }
            curr_p->prox = curr_p->prox->prox;
            swap.lastPag->prox = NULL;
            swap.num_paginas++;
            mem.num_paginas--;
        } else {
            curr_p = curr_p->prox;
        }
    }

    atualizaMemLastPag();
    printf("Swap Out finalizado!\n");
    exibe_memoria();
    exibe_swap();
    printf("\n");
}

// Bota novamente o processo na memória, caso ele não esteja na memória
// Retorna o número de páginas do processo que foram incluídas na memória.
// Retorna 0 se o processo não estava a área de swap.
int swap_in(int processo){
    int pags_no_swap = 0;

    Pagina *curr_p = swap.paginas;
    while(curr_p != NULL){
        if(curr_p->proc_id == processo)
            pags_no_swap++;
        curr_p = curr_p->prox;
    }

    if(pags_no_swap == 0) // Se não tem páginas no swap, faz nada.
        return 0;

    if(mem.num_paginas + pags_no_swap > MAX_FRAMES){
        printf("[%d] Não há espaço para swap in. Liberando espaço...\n", processo);
        swap_out(processo);
    }

    printf("SWAP IN do processo %d...\n", processo);


    if(swap.paginas != NULL){
        if(swap.paginas->proc_id == processo){
            while(swap.paginas != NULL &&
                    swap.paginas->proc_id == processo){
                mem.lastPag->prox = swap.paginas;
                swap.paginas = swap.paginas->prox;
                mem.lastPag = mem.lastPag->prox;
                mem.lastPag->prox = NULL;
            }
        } else {
            curr_p = swap.paginas;
            while(curr_p != NULL){
                if(curr_p->proc_id == processo){
                    mem.lastPag->prox = curr_p;
                    curr_p->prox = curr_p->prox->prox;
                    mem.lastPag = mem.lastPag->prox;
                    mem.lastPag->prox = NULL;
                }
                curr_p = curr_p->prox;
            }
        }
        mem.num_paginas += pags_no_swap;
        swap.num_paginas -= pags_no_swap;
    }

    printf("Swap In finalizado!\n");
    exibe_memoria();
    exibe_swap();
    printf("\n");

    return pags_no_swap;
}

void exibe_memoria(){
    printf("[MEM] Paginas: %d/%d  [ ", mem.num_paginas, MAX_FRAMES);
    Pagina *p = mem.paginas;
    while(p != NULL){
        printf("%d(%d), ", p->proc_id, p->num);
        p = p->prox;
    }
    printf("]\n");
}

void exibe_swap(){
    printf("[SWAP] Paginas: %d/%d  [", swap.num_paginas, MAX_FRAMES);
    Pagina *p = swap.paginas;
    while(p != NULL){
        printf("%d(%d), ", p->proc_id, p->num);
        p = p->prox;
    }
    printf("]\n");
}

int gettid(void){return (syscall(SYS_gettid));}

void iniciaMem() {
	mem.num_paginas = 0;
	mem.paginas = NULL;
	mem.lastPag = mem.paginas;
}

void iniciaSwap() {
    swap.num_paginas = 0;
    swap.paginas = NULL;
    swap.lastPag = swap.paginas;
}

void atualizaMemLastPag() {
    mem.lastPag = mem.paginas;
    while(mem.lastPag -> prox != NULL)
        mem.lastPag = mem.lastPag->prox;
}
//THREADS

void *processo(){
    srand(time(NULL));
    int pid = curr_pid++;
    int ppid;
    int temp_i=time(NULL);
    int temp_f;
    int temp_em_exec;
    int paginas = rand()%MAX_FRAMES + 1;
    int temp_tot_necessario=rand() % (MAX_PROCESS_TIME-MIN_PROCESS_TIME) + MIN_PROCESS_TIME + 1;
    int turnaround;
    int fora_cpu;
    int tamanho_prog;

    printf("PROCESSO DE PID:%d, CRIADO NO TEMPO : %ld\n", pid, temp_i - sys_tempo_i);
    carrega_paginas(pid, 0);
    sleep(3);

    while(1){
        int prox_pagina = rand() % paginas;
        carrega_paginas(pid, prox_pagina);
        sleep(3);
    }
}





void *proc_admin_func()
{

  printf("Administrador de processos iniciado!\n\n");
  pthread_t lista_processos[MAX_PROCESSES];
  int num_proc=0;

  while(num_proc<MAX_PROCESSES){
    pthread_create(&lista_processos[num_proc],NULL,processo,NULL);
    sleep(3);
    num_proc++;
  }

  for(int i = 0; i < MAX_PROCESSES; i++){
    pthread_join(lista_processos[i], NULL);
  }
}


//MAIN


int main (int argc, char* argv[]){
    int flag;
    iniciaMem();
    iniciaSwap();
    srand(time(NULL));
    //pthread_join(lista_processos[0], NULL);
    printf("\n****************************************************************************");
    printf("\n| LEGENDA DO LOG:\n|\n");
    printf("| [PID] Ação do processo\n");
    printf("| [MEM] [SWAP] -> Mostram o estado atual da memória ou swap\n");
    printf("| [MEM] Paginas: OCUPADAS/TAMANHO [ PID(num_pagina), PID(num_pag), ... ]\n");
    printf("|\n****************************************************************************\n\n");
    sys_tempo_i=time(NULL);
    printf("Iniciando sistema...\n");

    pthread_t proc_admin;
    flag = pthread_create(&proc_admin,NULL,proc_admin_func,NULL);
    pthread_join(proc_admin, NULL);
    printf("%d\n",flag);
    return 0;
}
