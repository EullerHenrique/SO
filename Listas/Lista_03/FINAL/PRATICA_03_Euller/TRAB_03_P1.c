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
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>    
#include <string.h>
#define MEM_SZ 4096
#define O_WRONLY 01
#define __NR_gettid 186

sem_t mutex;
int go = 0;

struct shared_area{	
    
    sem_t mutex;
    pid_t pidd[4];

    int queue[10];
    int end;
    int size;
    int stop;

};
struct shared_area *shared_area_ptr;
struct shared_area* produtor(){
  
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
    
    //sem_init (& sem , pshared , valor );
    
    //Escopo Interprocess -> Quando pshared é diferente de zero, o semáforo pode 
    //ser compartilhado por outros processos.
	if (sem_init((sem_t *)&shared_area_ptr->mutex,1,1) != 0){

        printf("sem_init falhou\n");
        exit(-1);

    }

    return shared_area_ptr; 
  
}
void CREATE_P1_P4(){

   pid_t pid;
   pid = fork();

    //Processo 1 
    if (pid > 0){
     
        printf("\nPID: %d -> PROCESSO 1\n", getpid());
        shared_area_ptr->pidd[0] = getpid(); 
        
    }
    else if (pid == 0){

        pid=fork();

        //Processo 2
        if (pid > 0){

            printf("\nPID: %d -> PROCESSO 2\n", getpid());
            shared_area_ptr->pidd[1] = getpid();

        }   		
        else if (pid == 0){	

            pid=fork();

            //Processo 3
            if (pid > 0){

                printf("\nPID: %d -> PROCESSO 3\n", getpid());
                shared_area_ptr->pidd[2] = getpid();
        
            }

            //Processo 4
            else if(pid==0){
                   
                printf("\nPID: %d -> PROCESSO 4\n", getpid());
                shared_area_ptr->pidd[3] = getpid();

            }   

        }	
    }
}
void queue_push(){


  while(1){
    
        if(shared_area_ptr->stop == 0){
            
            //Down: (SEMÁFARO VERDE) -> (SEMÁFARO VERMELHO) 

            //      Se a região crítica já estiver sendo utilizada por um processo, o acesso à região crítica é bloqueada
            //      para os próximos processos e esses processos são colocados em uma fila de espera. 

            sem_wait((sem_t*)&shared_area_ptr->mutex);
            
            srand(getpid() *time(NULL) * rand());
            
            //Se a fila estiver vazia
            if(shared_area_ptr->size == 0){

                shared_area_ptr->queue[0] = rand()%1000+1; //O elemento atual se torna o elemento inicial
                shared_area_ptr->end = 0;
                shared_area_ptr->size++;
                
            }

            //Se a fila não estiver vazia
            else if(shared_area_ptr->size < 10){
                
                shared_area_ptr->queue[shared_area_ptr->end+1] = rand()%1000+1; // O elemento atual se torna o elemento posterior ao último elemento
                shared_area_ptr->end++;                                  //O elemento posterior ao elemento final se torna o elemento final
                shared_area_ptr->size++;

            }

            if(shared_area_ptr->size < 10){

                printf("\n\nPUSH -> PID: %d -> ", getpid());
                printf("SIZE: %d -> ", shared_area_ptr->size);
                printf("NUM: %d\n", shared_area_ptr->queue[shared_area_ptr->size-1]);
            }

            //Se a fila estiver cheia
            if(shared_area_ptr->size == 10 && shared_area_ptr->pidd[0]==getpid()){

                printf("\n\nPUSH -> PID: %d -> ", getpid());
                printf("SIZE: %d -> ", shared_area_ptr->size);
                printf("NUM: %d\n", shared_area_ptr->queue[shared_area_ptr->size-1]);

                printf("\n----------------------\n\n");
                printf("FILA CHEIA\n");
                printf("\n----------------------\n");

                printf("\n*PROCESSO 1: ESTOU MANDANDO O SINAL, PROCESSO 4 :)*\n"); 
                
                shared_area_ptr->stop=1;
                kill(shared_area_ptr->pidd[3], SIGUSR1);
            }

            //Up: (SEMÁFARO VERMELHO) -> (SEMÁFARO VERDE)
            //    O processo que estiver no começo da fila de espera poderá acessar a região crítica
            sem_post((sem_t*)&shared_area_ptr->mutex);
        }  

    }
}

void queue_pop(char pipe[]){
        
        //Down: (SEMÁFARO VERMELHO) -> (SEMÁFARO VERDE) 
        //O processo que estiver no começo da fila de espera poderá acessar a região crítica

        sem_wait(&mutex);

            if(shared_area_ptr->size > 0){  
                
                int num = shared_area_ptr->queue[0];
                shared_area_ptr->size--;

                for(int i = 0; i < shared_area_ptr->size; i++){

                    shared_area_ptr->queue[i] = shared_area_ptr->queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial
              
                }
                
                printf("\n\nPOP -> TID: %d -> SIZE: %d -> NUM: %d -> PIPE -> %s\n", syscall(__NR_gettid), shared_area_ptr->size, num, pipe);

                int fd = open(pipe, O_WRONLY);
                write(fd, &num, sizeof(int));
                close(fd);
                
            }else if(shared_area_ptr->size == 0){

                shared_area_ptr->stop = 0;
                go=2;
        
            }

        // Up: (SEMÁFARO VERDE) -> (SEMÁFARO VERMELHO)	
        //Se a região crítica já estiver sendo utilizada por um processo, o acesso à região crítica é bloqueado para os próximos processos e esses processos são colocados em uma fila de espera. 

        sem_post(&mutex);  

}

void *thread_1(){
    
    mkfifo("pipe_01", 0666);
    
    while(1){

        if(go==1){
            
            queue_pop("pipe_01");
                 
        }
        
    }

}
void thread_2(){   
    
    mkfifo("pipe_02", 0666); 

    while(1){
    
        if(go==1){
            
            queue_pop("pipe_02");
                             
        }else if (go==2){

            break;

        }
        
    }

}
void call_thread_2(int signum){    

    if (signum == SIGUSR1){

        printf("\n*PROCESSO 4: PROCESSO 1, RECEBI O SINAL :)*\n\n");
                
        go=1;
        thread_2();
        
    }

}
void EXEC_P1_P3(){

    if(shared_area_ptr->pidd[0]  == getpid() || shared_area_ptr->pidd[1] ==  getpid() || shared_area_ptr->pidd[2] == getpid()){

        queue_push();

    }

}

void EXEC_P4(){

    if(shared_area_ptr->pidd[3] == getpid()){
    
        //sem_init (& sem , pshared , valor );
    
        //Escopo Intraprocesso -> Quando pshared é igual a zero, o semáforo pode 
        //ser compartilhado pelas threads do processo atual
        sem_init(&mutex, 0,1);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, thread_1, NULL);
        while(1){
            
            signal(SIGUSR1, call_thread_2);  
            pause();
            
        }

    }

}

int main(void){
 
  shared_area_ptr = produtor();

  shared_area_ptr->pidd[0] = 0;
  shared_area_ptr->pidd[1] = 0;
  shared_area_ptr->pidd[2] = 0;
  shared_area_ptr->pidd[3] = 0;
    
  shared_area_ptr->size = 0;
  shared_area_ptr->stop = 0;
  
  //Cria os processos 1, 2 e 3
  CREATE_P1_P4();

  while(shared_area_ptr->pidd[0] == 0 || shared_area_ptr->pidd[1] == 0 || shared_area_ptr->pidd[2] == 0 || shared_area_ptr->pidd[3] == 0 ){}

  //Executa os processos 1, 2 e 3
  EXEC_P1_P3();
  
  while(shared_area_ptr->pidd[3] == 0){}

  //Executa o processo 4
  EXEC_P4();


  
}     