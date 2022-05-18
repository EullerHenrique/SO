#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <semaphore.h>

#define MAXSIZE 10
#define MEM_SZ 4096

struct shared_area{	
	sem_t mutex;
	int queue[MAXSIZE];
	int end;
	int size;
};


void consumidor(){
  
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
	
	if (sem_init((sem_t*)&shared_area_ptr->mutex,1,1) != 0){

		printf("sem_init falhou\n");
		exit(-1);
	
    }
	
	sem_wait((sem_t*)&shared_area_ptr->mutex);
	
	printf("\n\n");
	printf("size: %d\n\n", shared_area_ptr->size);

	for(int i = 0; i < shared_area_ptr->size; i++){
		
		printf("i: %d %d\n", i, shared_area_ptr->queue[i]);
        
    }
	
	printf("\n\n\n\n");
	
	if(shared_area_ptr->size > 0){ 

		shared_area_ptr->size--;
		printf("size: %d\n\n", shared_area_ptr->size);

		
		for(int i = 0; i < shared_area_ptr->size; i++){

			shared_area_ptr->queue[i] = shared_area_ptr->queue[i+1];  // O elemento posterior ao elemento inicial se torna o elemento inicial
			printf("i: %d %d\n", i, shared_area_ptr->queue[i]);
			
		}
	}



	sem_post((sem_t*)&shared_area_ptr->mutex);   


    exit(0);

}

int main(){
    consumidor();
}