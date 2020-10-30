/*
*	Ilustra a criacao de threads e uso de mutex
*	Compilar com:	gcc -lpthread -o pthreads-tela pthreads-tela.c
*	ou
*	gcc -o pthreads-tela pthreads-tela.c -lpthread
*
*/


#include	<pthread.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include <sys/time.h>
#include <time.h>
#include <omp.h>

#define MAXPOSVET 1500


pthread_barrier_t barreira;
pthread_mutexattr_t protocolo;

///--------------------------------------------------------------------///
typedef struct node {
    int val;
    struct node *next;
} node_t;

int queueSize(node_t *head)
{
    node_t *current = head;
    int size = 0;

    while (current != NULL)
    {
        current = current->next;
        size++;
    }
    return size;
}

void enqueue(node_t **head, int val) {
    node_t *new_node = malloc(sizeof(node_t));
    if (!new_node) return;

    new_node->val = val;
    new_node->next = *head;

    *head = new_node;
}

int dequeue(node_t **head) {
    node_t *current, *prev = NULL;
    int retval = -1;

    if (*head == NULL) return -1;

    current = *head;
    while (current->next != NULL) {
        prev = current;
        current = current->next;
    }

    retval = current->val;
    free(current);

    if (prev)
        prev->next = NULL;
    else
        *head = NULL;

    return retval;
}

void print_list(node_t *head) {
    node_t *current = head;

    while (current != NULL) {
        printf("%d\n", current->val);
        current = current->next;
    }
}



/*************************************************************************/

///Esteira1
/*
{
//recebe o produto
//verificar se o produto ta correto correto
//passar o item ou itens passado (s) para o pc
//passar peso (s) para o pc
}
*/

pthread_mutex_t locaIsso = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pausa = PTHREAD_MUTEX_INITIALIZER;

void aloca_pc( void) {
	pthread_mutex_lock(&locaIsso);
	}

void libera_pc( void) {
	pthread_mutex_unlock(&locaIsso);
	}

void pausa_esteiras( void) {
	pthread_mutex_lock(&pausa);
	}

void libera_esteiras( void) {
	pthread_mutex_unlock(&pausa);
	}

///Global
    int nTotalProdutos = 0;
    int pesoVet[1500];
    int cont = 0;
    int pcLock = 0;

    int pesoTotal=0;
    int pesoAnterior=0;
    int pausarEsteirasFlag=0;


void PausarEsteiras()
{
    pausa_esteiras();
    while (1)
        if(pausarEsteirasFlag==0)
            break;
    libera_esteiras();
}


int ComputadorCentral (int peso)
{

    int i = 0;
    if(cont<MAXPOSVET)
    {
        pesoVet[cont]=peso;
        nTotalProdutos++;
        //printf("PesoProduto[%d]: %d\n",cont, pesoVet[cont]);
        cont++;
    }
    if(cont==MAXPOSVET)
    {
        pausarEsteirasFlag=1;
        struct timespec begin, end;

        printf("\nAcabou %d \ntotal de produtos %d \n\n",cont, nTotalProdutos);
        for(int i=0; i<MAXPOSVET; i++)
            printf("pesoVet[%d]: %d \n",i,pesoVet[i]);

        clock_gettime(CLOCK_REALTIME, &begin);

        //pcLock = 1;
        //printf("\n Tempo necessário para contagem de Pesos: 0,400 ms\n\n");
        //#pragma omp parallel for reduction(+:pesoTotal) num_threads(2)
        #pragma omp parallel for shared(pesoVet) private(i) reduction(+:pesoTotal) num_threads(2)
        for (i = 0; i < MAXPOSVET; i++)
            pesoTotal+=pesoVet[i];

        pesoAnterior= pesoTotal;
        printf("\nPeso Total %d \n\n\n",pesoTotal);
        clock_gettime(CLOCK_REALTIME, &end);
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    //printf("\n Total de tempo demorado pra somar os pesos %lf\n", elapsed);
    //getchar();

        cont = 0;

        for(int i = 0; i<1500; i++)
            pesoVet[i]=0;

        pausarEsteirasFlag=0;
    }
    pcLock = 0;
}

/** Esteiras Tarefas
    recebe o produto
    passar o item ou itens passado (s) para o pc
    passar peso (s) para o pc
*/

void EsteiraUm(void *ID)
{
    int peso = 49, pesoAux = 0;
    node_t *filaUm = NULL;
    struct timespec begin, end;
    while (1)
    {
    sleep(1);
    clock_gettime(CLOCK_REALTIME, &begin);

        if(pausarEsteirasFlag==1)
            PausarEsteiras();
        peso = peso+1;
    //printf("Peso do produto %d\n", peso);
        enqueue(&filaUm, peso);

      //pthread_barrier_wait(&barreira);
        if(pcLock==0)
        {
            aloca_pc();
            pcLock = 1;
            if(cont+queueSize(filaUm)<=MAXPOSVET)
            {
                for(int k = 0; (pesoAux=dequeue(&filaUm)) >0;k++)
                    ComputadorCentral(pesoAux);
            }
            else if (cont<MAXPOSVET)
                ComputadorCentral(dequeue(&filaUm));
            libera_pc();
        }
        clock_gettime(CLOCK_REALTIME, &end);

    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    //printf("\n Total de tempo demorado para atualizar esteira %lf\n", elapsed);
    //getchar();
    }
}

void EsteiraDois(void *ID)
{
    int peso = 0, pesoAux = 0;
    node_t *filaDois = NULL;

    while (1)
    {
    sleep(1);

        if(pausarEsteirasFlag==1)
            PausarEsteiras();
        peso = peso+1;
       //printf("Peso do produto %d\n", peso);
        enqueue(&filaDois, peso);
        //pthread_barrier_wait(&barreira);
        if(pcLock==0)
        {
            aloca_pc();
            pcLock = 1;
            if(cont+queueSize(filaDois)<=MAXPOSVET)
            {

                for(int k = 0; (pesoAux=dequeue(&filaDois)) >0;k++)
                    ComputadorCentral(pesoAux);
            }
            else if (cont<MAXPOSVET)
                ComputadorCentral(dequeue(&filaDois));
           libera_pc();
        }
    }
}

void EsteiraTres(void *ID)
{
    int peso = 199, pesoAux = 0;
    node_t *filaTres = NULL;

    while (1)
    {
sleep(1);
        if(pausarEsteirasFlag==1)
            PausarEsteiras();

        peso = peso+1;
      //printf("Peso do produto %d\n", peso);
        enqueue(&filaTres, peso);

        //pthread_barrier_wait(&barreira);
        if(pcLock==0)
        {
            aloca_pc();
            pcLock = 1;

            if(cont+queueSize(filaTres)<=MAXPOSVET)
            {

                for(int k = 0; (pesoAux=dequeue(&filaTres)) >0;k++)
                    ComputadorCentral(pesoAux);
            }
            else if (cont<MAXPOSVET)
                ComputadorCentral(dequeue(&filaTres));

            libera_pc();
        }
    }
}

void DisplayShow(void * ID)
{

    int ID_thread = (int *)ID;
    struct timespec begin, end;
    while(1){
    clock_gettime(CLOCK_REALTIME, &begin);
    //sleep(1);

    printf("\nTotal de Itens: %d Peso Total: %d\n", nTotalProdutos, pesoAnterior);
    printf("\nTotal de Itens: %d Peso Total: %d\n", nTotalProdutos, pesoAnterior);
    printf("\nTotal de Itens: %d Peso Total: %d\n", nTotalProdutos, pesoAnterior);
    clock_gettime(CLOCK_REALTIME, &end);
    system ("clear");
    long seconds = end.tv_sec - begin.tv_sec;
    long nanoseconds = end.tv_nsec - begin.tv_nsec;
    double elapsed = seconds + nanoseconds*1e-9;

    //printf("\n Total de tempo demorado pra atualizar o display %lf\n", elapsed);
    //getchar();
    }

}

int main(int argc, char *argv[])
{
    //zera vetor de 1500 posições
    for(int i = 0; i<MAXPOSVET; i++)
        pesoVet[i]=0;


    pthread_barrier_init(&barreira,NULL,3);

    pthread_mutexattr_setprotocol(&protocolo,PTHREAD_PRIO_INHERIT);

    pthread_mutex_init(&locaIsso,&protocolo);


    //Esteira 1 2 e 3;
    pthread_t t1, t2, t3, t4;
    int ID = 1;
    //pthread_barrier_init(&barreira,NULL,3);

    pthread_create(&t1, NULL, (void *) EsteiraUm,   (void *)ID);
    ID++;
    pthread_create(&t2, NULL, (void *) EsteiraDois, (void *)ID);
    ID++;
    pthread_create(&t3, NULL, (void *) EsteiraTres, (void *)ID);
    //ID++;
    pthread_create(&t4, NULL, (void *) DisplayShow, (void *)ID);

	pthread_join(t1,NULL);
    pthread_join(t2, NULL);
	pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    return 0;
}
