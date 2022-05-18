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
#define MEM_SZ 4096

struct shared_area{	
    
    sem_t mutex;
    int queue[MAXSIZE];
    pid_t pidd[7];
  
    int end;
    int size;

};

struct shared_area *shared_area_ptr;

struct shared_area *p_c(){
  
    int i;
	key_t key=5678;
	struct shared_area *shared_area_ptr;
	void *shared_memory = (void*)0;
	int shmid;

	shmid = shmget(key,MEM_SZ,0666|IPC_CREAT);
	if (shmid == -1){

		printf("shmget falhou\n");
		exit(-1);
	
    }
	
	printf("shmid=%d\n",shmid);
	
	shared_memory = shmat(shmid,(void*)0,0);
	
	if (shared_memory == (void*) -1){

		printf("shmat falhou\n");
		exit(-1);
	
    }
		
	printf("Memoria compartilhada no endereco=%p\n", shared_memory);

	shared_area_ptr = (struct shared_area*) shared_memory;

	if (sem_init((sem_t *)&shared_area_ptr->mutex,1,1) != 0){

        printf("sem_init falhou\n");
        exit(-1);

     } 
  
    return shared_area_ptr;
}

#include <time.h>

void queue_push_signal(int signum){

  if (signum == SIGUSR1){

    printf("\nPID: %d -> PROCESSO 1\n", getpid());
    printf("\n*PROCESSO 1|2|3: RECEBI O SINAL, PROCESSO 4\n*");
    
    printf("SIZEEEEEEEEEEE: %d", shared_area_ptr->size);

  }

}

void queue_push(){
  
  while(1){
        
        sleep(1);

        //Down: (SEMÁFARO VERDE) -> (SEMÁFARO VERMELHO) 

        //      Se a região crítica já estiver sendo utilizada por um processo, o acesso à região crítica é bloqueada
        //      para os próximos processos e esses processos são colocados em uma fila de espera. 

        sem_wait((sem_t*)&shared_area_ptr->mutex);
        
        srand(getpid()*time(NULL));

        printf("\n----------------------\n");

        //Se a fila estiver vazia
        if(shared_area_ptr->size == 0){

            printf("\nPID: %d\n\n", getpid());

            shared_area_ptr->queue[0] = rand()%1000+1; //O elemento atual se torna o elemento inicial
            shared_area_ptr->end = 0;
            shared_area_ptr->size++;

        }

        //Se a fila não estiver vazia
        else if(shared_area_ptr->size < 10){
            
            printf("\nPID: %d\n\n", getpid());

            shared_area_ptr->queue[shared_area_ptr->end+1] = rand()%1000+1; // O elemento atual se torna o nó elemento posterior ao último índice
            shared_area_ptr->end++;                                  //O elemento posterior ao elemento final se torna o elemento final
            shared_area_ptr->size++;

        }

        printf("\nSIZE: %d\n\n", shared_area_ptr->size);
        printf("\nnum: %d\n", shared_area_ptr->queue[shared_area_ptr->size-1]);
        printf("\n----------------------\n");

        //Se a fila estiver cheia
        if(shared_area_ptr->size == 10){
            
            printf("\nFILA CHEIA\n");
            
            printf("\n*PROCESSO 3: ESTOU MANDANDO O SINAL, PROCESSO 4 :)*\n"); 
            
            //Retira os elementos da fila por meio do processo 4
            
            kill(shared_area_ptr->pidd[3], SIGUSR1);
            
            //Suspende o processo 1, 2 e 3
            printf("\nPROCESSO 1 DESATIVADO\n");
            kill(shared_area_ptr->pidd[0], SIGSTOP);
            printf("\nPROCESSO 2 DESATIVADO\n");
            kill(shared_area_ptr->pidd[1], SIGSTOP);
            printf("\nPROCESSO 3 DESATIVADO\n");
            kill(shared_area_ptr->pidd[2], SIGSTOP);
            
        }

        //Up: (SEMÁFARO VERMELHO) -> (SEMÁFARO VERDE)
        //    O processo que estiver no começo da fila de espera poderá acessar a região crítica
        sem_post((sem_t*)&shared_area_ptr->mutex);
        

    }

}

int num = 0;

void queue_pop(){
    
    if(shared_area_ptr->size > 0){

        printf("\nSIZE: %d\n\n", shared_area_ptr->size-1);  
        num = shared_area_ptr->queue[0];

        for(int i = 0; i < shared_area_ptr->size-1; i++){

            shared_area_ptr->queue[i] = shared_area_ptr->queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial
        
        }
        shared_area_ptr->size--;

    }
}

int t = 0;

#define __NR_gettid 186

void *queue_pop_thread(void *args){

    while(shared_area_ptr->size > 0){ 
        
       sleep(1); 
       printf("\n-------------------------------\n");
       printf("\nTID: %d\n", syscall(__NR_gettid));
       queue_pop();
       if(num != 0){
          
           printf("NUM RETIRADO DA FILA: %d\n", num);
            

       }else{
           
           printf("\nFILA VAZIA\n");
           printf("\n-------------------------------\n");
           break;

       }
       
    }

    pthread_exit(NULL);
    
}

void queue_pop_signal(int signum){    
    
    if (signum == SIGUSR1){

        printf("\n*PROCESSO 4: PROCESSO 3, RECEBI O SINAL :)*\n");

        pthread_t thread_id[2];
        
        //Cria as duas threads
        for(int i = 0; i < 2; i++){
          
          pthread_create(&thread_id[i], NULL, queue_pop_thread, NULL);
         
        }
        
        //Encerra as duas threads criadas
        for(int i = 0; i < 2;i++){
                
          pthread_join(thread_id[i], NULL);

        }

        //Ativa o processo 1, 2 e 3
        if(shared_area_ptr->size == 0){
          
          sleep(1);
          printf("\nPROCESSO 1 ATIVADO\n");
          kill(shared_area_ptr->pidd[0], SIGCONT);
          
          sleep(1);
          printf("\nPROCESSO 2 ATIVADO\n");
          kill(shared_area_ptr->pidd[1], SIGCONT);
          
          sleep(1);
          printf("\nPROCESSO 3 ATIVADO\n");
          kill(shared_area_ptr->pidd[2], SIGCONT);

        }

    }
}


void P1_P4(){

   pid_t pid;
   pid = fork();


    //Processo 1 
    if (pid > 0){
     
        printf("\nPID: %d -> PROCESSO 1\n", getpid());
        shared_area_ptr->pidd[0] = getpid();
        queue_push();       
        
    }
    else if (pid == 0){

        pid=fork();

        //Processo 2
        if (pid > 0){

            printf("\nPID: %d -> PROCESSO 2\n", getpid());
            shared_area_ptr->pidd[1] = getpid();
            queue_push();
        }   		
        else if (pid == 0){	

            pid=fork();

            //Processo 3
            if (pid > 0){

                printf("\nPID: %d -> PROCESSO 3\n", getpid());
                shared_area_ptr->pidd[2] = getpid();
                queue_push();
        
            }

            else if(pid==0){

                   //Processo 4
                   printf("\nPID: %d -> PROCESSO 4\n", getpid());
                   shared_area_ptr->pidd[3] = getpid();
                   while(1){

                    signal(SIGUSR1, queue_pop_signal);  
                    pause();    
                       
                   }
            }  
        }	
    }
}



int main(void){

  shared_area_ptr = p_c();
  shared_area_ptr->size = 0;
  
  //Cria os processos 1, 2, 3 e 4
  P1_P4();
  

  //Encerra os processos 1, 2 e 3
  //for(int i=3; i >= 0; i--){
  //  kill(shared_area_ptr->pidd[i], SIGKILL);
  //}
  
}     