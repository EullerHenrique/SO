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

#include  <sys/stat.h>
#include  <fcntl.h>
#define O_RDONLY 00

void P5_P7(){

   pid_t pid;
   
   pid=fork();

   //Processo 5
   if (pid > 0){

      printf("\nPID: %d -> PROCESSO 5\n", getpid());
      shared_area_ptr->pidd[4] = getpid();
      
      char *pipe1="pipe1";
      int num;

      while(1){

        //Abre o pipe nomeado no modo de escrita 
        int fd = open(pipe1, O_RDONLY);

        if(fd != -1){
                
          //Escreve no pipe nomeado
          read(fd, &num, sizeof(int));

          printf("P5 -> NUM: %d\n", num);

        }

        //Fecha o pipe nomeado
        close(fd);

      }
     
    }
    else if(pid==0){
      
      pid = fork();

      //Processo 6
      if (pid > 0){

        printf("\nPID: %d -> PROCESSO 6\n", getpid());
        shared_area_ptr->pidd[5] = getpid();
        
        char *pipe2="pipe2";
        int num;

        while(1){

          //Abre o pipe nomeado no modo de escrita 
          int fd = open(pipe2, O_RDONLY);

          if(fd != -1){
                  
            //Escreve no pipe nomeado
            read(fd, &num, sizeof(int));

            printf("P6 -> NUM: %d\n", num);

          }

          //Fecha o pipe nomeado
          close(fd);
          
        }
      
      }

      //Processo 7
      else if(pid==0){

          printf("\nPID: %d -> PROCESSO 7\n", getpid());
          shared_area_ptr->pidd[6] = getpid();
         
                                                  
      }
      
  
  }

}  
    
int main(void){

  shared_area_ptr = p_c();

  //Cria os processos 5, 6 e 7
  P5_P7();
  
}

/*          char *pipe2="pipe2";

            //Cria um pipe nomeado
            mkfifo(pipe2, 0666);

            //Abre o pipe nomeado no modo de escrita 
            int fd = open(pipe2, O_WRONLY);
               
            //Escreve no pipe nomeado
            write(fd, &num, sizeof(int));
               
            //Fecha o pipe nomeado
            close(fd);
            
            printf("NUM ESCRITO NO PIPE 2: %d\n", num);
*/
