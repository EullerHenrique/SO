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
#include <string.h>

#define MAXSIZE 10
#define MEM_SZ 4096

struct shared_area{	

    sem_t mutex;
    int queue[MAXSIZE];
    pid_t pidd[3];
    
    int end;
    int size;

    int stop;
    
    int flag_push;
    
};

struct shared_area *shared_area_ptr;

struct shared_area *produtor(){
  
  int i;
	key_t key=1234;
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
  
  return shared_area_ptr;

}

#include  <sys/stat.h>
#include  <fcntl.h>
#define O_RDONLY 00

void CREATE_P5_P6(){

   pid_t pid;
   
   pid = fork();

   //Processo 5
   if (pid > 0){

      printf("\nPID: %d -> PROCESSO 5\n", getpid());
      shared_area_ptr->pidd[0] = getpid();
     
   }

   else if(pid==0){

     pid = fork();
      
    if(pid > 0){ 
      
      //Processo 6

      printf("\nPID: %d -> PROCESSO 6\n", getpid());
      shared_area_ptr->pidd[1] = getpid();
    
    }else if(pid ==0){
      
      //Processo 7

      printf("\nPID: %d -> PROCESSO 7\n", getpid());
      shared_area_ptr->pidd[2] = getpid();
    
    }
 
  }

}  

;

void queue_push(){

  int num;

  int fd_1 = open("pipe_01", O_RDONLY);
  int fd_2 = open("pipe_02", O_RDONLY);

  while(1){

         //BUSY WAIT

            //A_RC

              if(shared_area_ptr->pidd[0] == getpid()){

                  while(shared_area_ptr->flag_push != 1){}

                  if(shared_area_ptr->size < 10){
                
                    shared_area_ptr->stop = 0;

                  }

                  if(shared_area_ptr->stop == 0){

                    //Lê o pipe nomeado
                    read(fd_1, &num, sizeof(int));

                    printf("\nPID: %d   ->   PIPE: %s   SIZE -> %d ->   NUM: %d\n", getpid(), "pipe_01", shared_area_ptr->size, num);

                  }
                    

                }else if(shared_area_ptr->pidd[1] == getpid()){

                    while(shared_area_ptr->flag_push != 2){}

                    if(shared_area_ptr->size < 10){
                
                      shared_area_ptr->stop = 0;

                    }

                    if(shared_area_ptr->stop == 0){

                      //Lê o pipe nomeado
                      read(fd_2, &num, sizeof(int));
                    
                      printf("\nPID: %d   ->   PIPE: %s   SIZE -> %d ->   NUM: %d\n", getpid(), "pipe_02", shared_area_ptr->size, num);
                    
                    }
                    
                }

              
              if(shared_area_ptr->stop == 0){

                //Se a fila estiver vazia
                if(shared_area_ptr->size == 0){
                
                    shared_area_ptr->queue[0] = num; //O elemento atual se torna o elemento inicial
                    shared_area_ptr->end = 0;
                    shared_area_ptr->size++;
                    
                }

                //Se a fila não estiver vazia
                else if(shared_area_ptr->size < 10){
                    
                    shared_area_ptr->queue[shared_area_ptr->end+1] = num; // O elemento atual se torna o nó elemento posterior ao último índice
                    shared_area_ptr->end++;                                  //O elemento posterior ao elemento final se torna o elemento final
                    shared_area_ptr->size++;

                }
                

                if(shared_area_ptr->size < 10){

                  printf("\nPUSH  ->  PID: %d   -> SIZE: %d ->  NUM: %d\n", getpid(),  shared_area_ptr->size,  shared_area_ptr->queue[shared_area_ptr->size-1]);
                
                }

                if(shared_area_ptr->size == 10 && shared_area_ptr->pidd[0]){

                  printf("\nPUSH  ->  PID: %d   -> SIZE: %d ->  NUM: %d\n", getpid(),  shared_area_ptr->size,  shared_area_ptr->queue[shared_area_ptr->size-1]);
                  shared_area_ptr->stop = 1;

                }
              
              }

              //D_RC

              if(shared_area_ptr->pidd[0] == getpid()){
                
                shared_area_ptr->flag_push = 2;
              
              }else if(shared_area_ptr->pidd[1] == getpid()){

                shared_area_ptr->flag_push = 1;

              }

            }

  }


int tidd[3] = {0,0,0};
int flag_pop = 0;
int go = 0;
#define __NR_gettid 186

void queue_pop(){
 
  while(1){

    //A_RC
     
     if(tidd[0] == syscall(__NR_gettid)){

      while(flag_pop != 1){}
      
     }else if(tidd[1] == syscall(__NR_gettid)){

      while(flag_pop != 2){}

     }else if(tidd[2] ==syscall(__NR_gettid)){

      while(flag_pop != 3){}

     }

    
     //RC
      
      int num;


      if(shared_area_ptr->size > 0){  
     
        num = shared_area_ptr->queue[0];
        shared_area_ptr->size--;

        for(int i = 0; i < shared_area_ptr->size; i++){

          shared_area_ptr->queue[i] = shared_area_ptr->queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial
                      
        }
                  
        printf("POP -> TID: %d -> SIZE: %d -> NUM: %d\n", syscall(__NR_gettid), shared_area_ptr->size, num);
          
                
      }

    //D_RC
     if(tidd[0] == syscall(__NR_gettid)){

      flag_pop = 2;
     
     }else if(tidd[1] == syscall(__NR_gettid)){

      flag_pop = 3;

     }else if(tidd[2] == syscall(__NR_gettid)){

      flag_pop = 1;
     
     }
  }

}



void EXEC_P5_P6(){
    
    if(shared_area_ptr->pidd[0] == getpid() || shared_area_ptr->pidd[1] == getpid()){
  
      queue_push();
        
    }

}


void *thread_1(){

  tidd[0] = syscall(__NR_gettid);

  if(go == 1){
  
    queue_pop();
  
  }

}


void *thread_2(){

  tidd[1] = syscall(__NR_gettid);

  if(go == 1){
   
    queue_pop();
  
  }
}

void thread_3(){

    tidd[2] = syscall(__NR_gettid);
    
    go=1;

    if(go == 1){

      queue_pop();
    
    }  
}

void EXEC_P7(){

  if(shared_area_ptr->pidd[2] == getpid()){

    pthread_t thread_id[2];
    
    //Thread 1
    pthread_create(&thread_id[1], NULL, thread_1, NULL);
    
    //Thread 2
    pthread_create(&thread_id[2], NULL, thread_2, NULL);

    //Thread 3
    tidd[2] = pthread_self();
  
    thread_3();    

  }

}

    
int main(void){

  shared_area_ptr = produtor();

  shared_area_ptr->pidd[0] = 0;
  shared_area_ptr->pidd[1] = 0;
  shared_area_ptr->pidd[2] = 0;

  shared_area_ptr->size = 0;
  shared_area_ptr->flag_push = 1;
  shared_area_ptr->stop = 0;
  flag_pop=1;

  //Cria o processo 5 e 6
  CREATE_P5_P6();

  while(shared_area_ptr->pidd[0] == 0 || shared_area_ptr->pidd[1] == 0){}

  //Executa o processo 5 e 6
  EXEC_P5_P6();

  while(shared_area_ptr->pidd[2] == 0){}

  //Executa o processo 7

  EXEC_P7();


}
