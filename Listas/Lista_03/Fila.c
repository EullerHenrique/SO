
#include <stdio.h>
#include <stdlib.h>

typedef struct queue QUEUE;
typedef struct queue_node QNODE;

struct queue{
  QNODE *begin;
  QNODE *end;
  int size;
};

struct queue_node{
   int data;
   QNODE *next;
};

int queue_size(QUEUE *Q){
  if(Q == NULL){
    return -1;
  }
  else{
    return Q -> size;
  }
}

int queue_empty(QUEUE *Q){
  if(Q == NULL){
    return -1;
  }
  
  else
  if(Q -> size == 0){
    return 1;//true
  }
  else{
    return 0;//false
  }
}

int queue_full(QUEUE *Q){
   if(Q == NULL){
     return -1;
   }
   else if(queue_size(Q) == 10){
     return 1;
   }else{
    return 0;
   }
}

int queue_free(QUEUE *Q){
  if(Q == NULL){
    return -1;
  }
  else{
    QNODE *NODE_PRESENT = NULL;
    
    //Enquanto existir um nó inicial
    while(Q -> begin != NULL){
      NODE_PRESENT = Q -> begin;// O nó inicial se torna o nó atual
      Q -> begin = Q -> begin -> next; // O nó posterior ao primeiro nó se torna o primeiro nó
      free(NODE_PRESENT); // A memória do nó atual é liberada
    }
    free(Q);// A memória da fila é liberada
    return 0;
  }
}

QUEUE* queue_create(){
  QUEUE *Q = NULL;
  Q = malloc(1 * sizeof(QUEUE));
  
  if(Q == NULL){
    return NULL;
  }
  else{
    Q -> begin = NULL; 
    Q -> end = NULL;
    Q -> size = 0;
  }
  return Q;
}

int queue_push(QUEUE *Q, int num){
   if(Q == NULL || queue_size(Q) == 10){
     return -1;
   }
   else{

     QNODE *NODE_PRESENT  = NULL;
     NODE_PRESENT = malloc(1 * sizeof(QNODE));
     if(NODE_PRESENT == NULL){
       return -1;
     }
     else{
       NODE_PRESENT -> data = num; 
       NODE_PRESENT -> next = NULL;
       
      // Se a fila estiver vazia
       if(Q -> size == 0){
         Q -> begin  = NODE_PRESENT; //O nó atual se torna o nó inicial 
         Q -> end = NODE_PRESENT; //O nó atual se torna o nó final
         Q -> size = Q -> size + 1; 
       }
       
       // Se a fila não estiver vazia
       else{
         Q -> end -> next = NODE_PRESENT; // O nó atual se torna o nó posterior ao último nó 
         Q -> end = NODE_PRESENT; // O nó atual se torna o último nó
         Q -> size =  Q -> size + 1;
         return 0;
     }
   }
 }
}

int queue_pop(QUEUE *Q){
  if(Q == NULL){
    return -1;
  }
  else{
    if(Q -> size == 0){
      return -1;
    }
    else{
      
      QNODE *NODE_PRESENT  = NULL;
      NODE_PRESENT = Q -> begin; //O primeiro nó se torna o nó atual
      
      Q -> begin = Q -> begin -> next; // O nó posterior ao primeiro nó se torna o primeiro nó
      
      //Se o primeiro nó não existir, ou seja, se a fila possuir somente um elemento
      if(Q -> begin == NULL){ 
        Q -> end = NULL;//O último nó deixa de existir
      }
      
      free(NODE_PRESENT); //A memória do nó atual é liberada
      Q -> size = Q -> size - 1;
    }
  }
}


