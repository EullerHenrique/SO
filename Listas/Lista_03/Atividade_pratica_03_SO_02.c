
/*

Os processos P1, P2 e P3 são singlethreads. Cada um desses processos se comunica com o processo P4 por meio 
de uma área de memória compartilhada (shared memory). Essa área compartilhada deve ser usada como uma fila 
(F1) do tipo FIFO (first-in-first-out), a qual armazena valores do tipo inteiro. A capacidade da fila é de 10 valo

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>
#include <pthread.h>



#define MAXSIZE 10
/*
int queue[MAXSIZE];
int size = 0;
int end = 0;


int queue_empty(){

  if(size == 0){

    return 1;

  }else{

    return 0;

  }

}

int queue_full(){

  if(size == MAXSIZE){

    return 1;

  }else{

    return 0;

  }

}

void queue_push(int v){

    if(size < MAXSIZE){
        
        //Se a fila estiver vazia

        if(size == 0){

            queue[0] = v; //O elemento atual se torna o elemento inicial

            size++;
            
        }

        //Se a fila não estiver vazia
        else{

            queue[end+1] = v; // O elemento atual se torna o nó elemento posterior ao último índice
            end++;            //O elemento posterior ao elemento final se torna o elemento final
            size++;           //O tamanho da fila aumenta
      
        }
    }

}

int queue_pop(){

    if(size > 0){ 

        for(int i = 0; i < 10; i++){

            queue[i] = queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial

        }

        size--; // O tamanho da fila diminui  
    }

}
*/
#define MEM_SZ 4096

struct shared_area{	
	sem_t mutex;
	int queue[MAXSIZE];
  int end;
  int size;
  pid_t pidd[3];
};

struct shared_area *shared_area_ptr;


struct shared_area *p_c(){
  
  int i;
	key_t key=5678;
	struct shared_area *shared_area_ptr;
	void *shared_memory = (void *)0;
	int shmid;

	shmid = shmget(key,MEM_SZ,0666|IPC_CREAT);
	if ( shmid == -1 ){
		printf("shmget falhou\n");
		exit(-1);
	}
	
	printf("shmid=%d\n",shmid);
	
	shared_memory = shmat(shmid,(void*)0,0);
	
	if (shared_memory == (void *) -1 ){

		printf("shmat falhou\n");
		exit(-1);
	
  }
		
	printf("Memoria compartilhada no endereco=%p\n", shared_memory);

	shared_area_ptr = (struct shared_area *) shared_memory;

	if (sem_init((sem_t *)&shared_area_ptr->mutex,1,1) != 0){

    printf("sem_init falhou\n");
    exit(-1);

  }
  
  return shared_area_ptr;

}

void queue_push(){
  
  for(;;){

    sem_wait((sem_t*)&shared_area_ptr->mutex);
  
    srand(shared_area_ptr->size);

    printf("%d\n", shared_area_ptr->size);

    //Se a fila estiver vazia
    if(shared_area_ptr->size == 0){
    
      shared_area_ptr->queue[0] = rand()%1000+1; //O elemento atual se torna o elemento inicial
      shared_area_ptr->end = 0;
      shared_area_ptr->size++;

    }

    //Se a fila não estiver vazia
    else if(shared_area_ptr->size < 10){

      shared_area_ptr->queue[shared_area_ptr->end+1] = rand()%1000+1; // O elemento atual se torna o nó elemento posterior ao último índice
      shared_area_ptr->end++;                                         //O elemento posterior ao elemento final se torna o elemento final
      shared_area_ptr->size++;
    }    

    sem_post((sem_t*)&shared_area_ptr->mutex);


    if(shared_area_ptr->size == 10){
      printf("aa\n");
      break;
    }

  }
  
}


void queue_pop(){

    sem_wait((sem_t*)&shared_area_ptr->mutex);
        
    if(shared_area_ptr->size > 0){ 

      shared_area_ptr->size--;
      printf("\n\nsize: %d\n\n", shared_area_ptr->size);

      for(int i = 0; i < shared_area_ptr->size; i++){

        shared_area_ptr->queue[i] = shared_area_ptr->queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial
        printf("i: %d %d\n", i, shared_area_ptr->queue[i]);
        
      }
    }

    sem_post((sem_t*)&shared_area_ptr->mutex);   

}

int flag = 0;

void *THREAD_FUNCTION(void *args){

    printf("QUEUE_POP\n");

    int i = 0;
    int j = 0;

    if(flag == 0){
      i = 0;
      j = 5;
      flag = 1;
    }
    else{
      i = 5;
      j = 10;
      flag = 0; 
    }
    
    for(; i < j; i++){
      
      queue_pop();

    }

    pthread_exit(NULL);
    
}

void pop(int signum){    
    
    if (signum == SIGUSR1){

        printf("*O processo 4 recebeu o sinal do processo 3*\n\n");

        pthread_t thread_id[2];
        
        for(int i = 0; i < 2; i++){
          printf("THREAD %d -> \n", i+1);
          pthread_create(&thread_id[i], NULL, THREAD_FUNCTION, NULL);
        }
            
        for(int i = 0; i < 2;i++){
                
          pthread_join(thread_id[i], NULL);

        }

        if(shared_area_ptr->size == 0){

          
          printf("*O processo 4 irá enviar um sinal para o processo 3*\n\n"); 
          sleep(3);
          kill(shared_area_ptr->pidd[2], SIGUSR1);
          printf("*O processo 4 irá enviar um sinal para o processo 2*\n\n"); 
          sleep(3);
          kill(shared_area_ptr->pidd[1], SIGUSR1);
          printf("*O processo 4 irá enviar um sinal para o processo 1*\n\n"); 
          sleep(3);
          kill(shared_area_ptr->pidd[0], SIGUSR1);
        
        }

        
    }
}


void push(int signum){

  if (signum == SIGUSR1){
    
    printf("*O processo 1 || 2 || 3 recebeu o sinal do processo 4*\n\n");

    printf("\n\nQUEUE_PUSH\n\n");

    queue_push();
  
  }

}

void F_02(int n){

    pid_t pid;
    int status=0;

    if(n > 0){

        pid = fork(); //O programa se divide e começa a ser executado ao mesmo tempo em n_p processos
        printf("fork: %d        ->       ", pid);
     
       if(pid > 0){
        
            //Pai

            printf("PID PAI: %d     ->->   ", getpid());
            printf("PROCESSO %d \n\n", n+4);

           

        }else if (pid == 0){

            //Filho

            printf("PID FILHO: %d   ->->   ", getpid());
            printf("f(n-1)\n\n");
            F_02(n-1);

        }
                
    }
}

void F_01(int n){

    pid_t pid;
    int status=0;

    if(n > 0){

        pid = fork(); //O programa se divide e começa a ser executado ao mesmo tempo em n_p processos
        printf("fork: %d        ->       ", pid);
     

       if(pid > 0){
        
            //Pai

            printf("PID PAI: %d     ->->   ", getpid());
            printf("PROCESSO %d \n\n", n);
            
            sem_wait((sem_t*)&shared_area_ptr->mutex);

            shared_area_ptr->pidd[n-1] = getpid();

            sem_post((sem_t*)&shared_area_ptr->mutex);

            if(n > 0){

              printf("\n\nQUEUE_PUSH\n\n");
              queue_push();  
            
            }
            
            if(n - 2 < 0){  
              
              printf("*O processo 3 irá enviar um sinal para o processo 4*\n\n"); 
              sleep(6);
              kill(pid, SIGUSR1);//Envia ao processo filho o sinal
            
            }
            

            while(1){
              
              printf("*O processo %d está esperando um sinal do processo 4*\n\n", n);
              signal(SIGUSR1, push);
              pause();

              if(n - 2 < 0){  
              
                printf("*O processo 3 irá enviar um sinal para o processo 4*\n\n"); 
                sleep(6);
                kill(pid, SIGUSR1);//Envia ao processo filho o sinal
            
              }

            }

        }else if (pid == 0){

            //Filho
            printf("PID FILHO: %d   ->->   ", getpid());
            
            //Processo 1, 2 e 3
            if(n - 1 > 0){

              printf("f(n-1)\n\n");
              F_01(n-1);
              
            }

            //Processo 4
            else
            if(n-2 < 0){

              printf("PROCESSO 4 \n\n");
              //Cria o processo 5 e 6
              F_02(2);
              
              while(1){
                  
                  printf("*O processo 4 está esperando um sinal do processo 3*\n\n");
                  signal(SIGUSR1, pop);
                  pause();

              }

              
            }
        }
                
    }
}


int main(void){

  shared_area_ptr = p_c();

  sem_wait((sem_t*)&shared_area_ptr->mutex);

  shared_area_ptr->size = 0;

  sem_post((sem_t*)&shared_area_ptr->mutex);

  printf("\n");

  //Cria os processos 1, 2, 3 e 4
  F_01(3);
  
}         