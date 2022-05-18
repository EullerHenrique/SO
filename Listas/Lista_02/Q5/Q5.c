#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


int DIRENT_01(char caminho_pai[]){

        int qtd = 0;

        //O ponteiro dir do tipo DIR será utilizado para armazenar o endereço do diretório desejado
        DIR *dir;
    
        //Esse ponteiro será utilizado para armazenar o endereço de uma struct dirent.
        //Tal struct é utilizada para armazenar os dados  de um arquivo. 
        struct dirent *lsdir;

        /* A struct dirent foi definida da seguinte maneira na biblioteca dirent

         struct dirent {
               ino_t          d_ino;       
               off_t          d_off;       
               unsigned short d_reclen;    // Tamanho do arquvo
               unsigned char  d_type;      // Tipo do arquivo
               char           d_name[256]; // Nome do arquvo 
           }; 

        */

      
        // A função opendir abre o diretório determinado
        // Tal função retorna o endereço do diretório 
        dir = opendir(caminho_pai);
        if(dir == NULL){
            return 0;
        }


        //A função readdir é utilizada para ler arquivos do diretório determinado
        //Tal função possui como parâmetro um ponteiro que receberá o endereço do diretório que será lido.
        //Tal função retorna o endereço da struct dirent do arquivo atual presente no diretório determinado
        //A cada iteração, o próximo arquivo é lido
        
        while((lsdir = readdir(dir)) != NULL){
           
            char nome[256];
            
            //O nome do diretório é acessado por meio do campo d_name da struct dirent
            strcpy(nome, lsdir->d_name);
                    
            if(strcmp(nome, ".") != 0 && strcmp(nome, "..") != 0){

                    char caminho_filho[1000];

                   strcpy(caminho_filho, caminho_pai);
            
                   if(strcmp(caminho_filho, "/") != 0){
                        strcat(caminho_filho, "/");
                    }

                   strcat(caminho_filho, nome); 
              
                   qtd+=1;
                   //printf("Arquivo: %s\n", caminho_filho);
              
                   //If utilizado para um atalho não ser acessado
                   if(lsdir->d_type == 4){

                    qtd+=DIRENT_01(caminho_filho);
                   
                   }

            
            }

        }


        closedir(dir);
     
        return qtd;

}


/*

    Fazer um programa, em linguagem C, para contar e imprimir o número total de arquivos armazenados em
    um disco rígido. Implementar e comparar o tempo de execução de três versões desse programa. 
    A primeira versão deve ser programada como um único processo singlthreaded.

*/

void CONT_01(){

    int qtd_total = DIRENT_01("/");
    printf("\nQTD_TOTAL: %d\n", qtd_total);
}


/*
    A segunda versão deve ser programada como múltiplos processos singlthreaded, onde o número de processos (n) deve corresponder
    ao número de processadores do computador. Caso o computador tenha apenas um processador, então utilize n = 2.

*/

#include <sys/wait.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <math.h>


char diretorio_raiz[100][100];
int n_p;
int qtd_raiz;

void DIRENT_02(){
   
    DIR *dir;
    struct dirent *lsdir;
    dir = opendir("/");
    int i = 0;
    while((lsdir = readdir(dir)) != NULL){
           
            char nome[256];
            
            //O nome do diretório é acessado por meio do campo d_name da struct dirent
            strcpy(nome, lsdir->d_name);
                    
            if(strcmp(nome, ".") != 0 && strcmp(nome, "..") != 0){
                    
                
                strcpy(diretorio_raiz[i], "/");
                strcat(diretorio_raiz[i], nome);
                
                i++;
                qtd_raiz++;
            }
        
        }

        closedir(dir);
   
}


void F_01(int n, double i, double j){

    pid_t pid;
    int status=0;
    int qtd_diretorio = 0; 
    FILE *file;

    if(n > 0){

        pid = fork(); //O programa se divide e começa a ser executado ao mesmo tempo em n_p processos
        printf("fork: %d        ->       ", pid);

       if(pid > 0){
        
            //Pai

            wait(&status);

            printf("PID PAI: %d     ->->   ", getpid());
            printf("f(n-1)\n");

            F_01(n-1, j, ceil(j+(double)qtd_raiz/(double)n_p));

        }else if (pid == 0){

            //Filho

            printf("PID FILHO: %d   ->->   ", getpid());
            printf("PROCESSO %d-> i: %lf, j: %lf\n\n", n, i, j);

            for(; i < j ; i++){
                
                printf("%s\n",diretorio_raiz[(int)i]);
                qtd_diretorio+=DIRENT_01(diretorio_raiz[(int)i]);
            }

            file = fopen("qtd.txt", "a");
            fprintf(file,"%d\n",qtd_diretorio);
            fclose(file);


            exit(status);

        }
                
    }else{

            int qtd_final = 0;
            int i = 0;
            file = fopen("qtd.txt","r");
            while(feof(file) == 0){
                
                fscanf(file,"%d\n",&qtd_diretorio);
                printf("\nPROCESSO %d: Qtd anterior: %d + Qtd atual %d", i+1, qtd_final, qtd_diretorio);
                qtd_final+=qtd_diretorio;
                printf(" = %d\n\n", qtd_final);
                i++;

            }   
        
            printf("QTD_TOTAL: %d\n", qtd_final);

            file = fopen("qtd.txt","w");
            fprintf(file,"%s", "");


            fclose(file);
        }
}


void CONT_02(){

    qtd_raiz = 0;

    DIRENT_02();

    //Número de processadores == Número de processos

    n_p = get_nprocs();

    F_01(n_p, 0, ceil((double)qtd_raiz/(double)n_p));
    
}

/*
    A terceira versão deve ser programada como múltiplos processos, tal como a segunda versão,
    contudo cada processo deve utilizar múltiplas threads (mt). O valor de mt deve ser 2. 

    Na segunda e terceira versões, o algoritmo de busca e contagem de arquivos deve ser paralelizado; por exemplo,
    enquanto um processo conta os arquivos em uma parte do disco (ex. C:\ no Windows ou /dev/sda1 no
    Linux) o outro processo conta os arquivos em outra parte (ex. D:\ ou /dev/sda2). 
    O mesmo aplica-se para múltiplas threads. 
    
    A estratégia de paralelização do algoritmo de contagem de arquivos é de livre escolha,
    assim como a plataforma de SO escolhida para realizar esse exercício.

*/

#include <pthread.h>

typedef struct {
    int i1;
    int j1;

    int i2;
    int j2;
} INTERVALO;

int cont = 1;

void *THREAD_FUNCTION(void *args){

   
    INTERVALO *intervalo = args;
    int i = 0;
    int j = 0;

    printf("THREAD -> \n");

    if(cont == 1){

        i = intervalo->i1;
        j = intervalo->j1;
        cont++;

    }else if(cont == 2){

        i = intervalo->i2;
        j = intervalo->j2;  

        cont = 0;

    }

    int qtd_diretorio = 0;

    printf("\n");

    for(; i < j; i++){

        printf(" i: %d j: %d\n",i,j);
        qtd_diretorio+=DIRENT_01(diretorio_raiz[i]);
        printf("%s\n",diretorio_raiz[i]);
    }


    printf("\n");

    FILE *file = fopen("qtd.txt", "a");
    fprintf(file,"%d\n",qtd_diretorio);
    fclose(file);
    
    
    pthread_exit(NULL);
}

void F_02(int n, double i, double j){

    pid_t pid;
    int status=0;
    int qtd_diretorio = 0; 

    if(n > 0){

        pid = fork(); //O programa se divide e começa a ser executado ao mesmo tempo em n processos
        printf("fork: %d        ->       ", pid);

       if(pid > 0){
        
            //Pai

            wait(&status);
            printf("PID PAI: %d     ->->   ", getpid());
            printf("f(n-1)\n");
            
            F_02(n-1, j, ceil(j+(double)qtd_raiz/(double)n_p));

        }else if (pid == 0){

            //Filho

            printf("PID FILHO: %d   ->->   ", getpid());
            printf("PROCESSO %d-> i: %f, j: %f\n\n", n, i, j);

           
            pthread_t thread_id[2];
            INTERVALO* intervalo =  malloc(sizeof *intervalo);

            intervalo->i1 = i;
            intervalo->j1 = j-2;

            intervalo->i2 = j-2;
            intervalo->j2 = j;

            for(int x = 0; x < 2; x++){

                if(pthread_create(&thread_id[x], NULL, THREAD_FUNCTION, intervalo)){
                    free(intervalo);
                }
                
            }

            printf("\n");
            
            for(int y = 0; y < 2; y++){
                
                pthread_join(thread_id[y], NULL);

            }

            exit(status);

            
        }
                
    }else{

            
            int qtd_final = 0;
            int i = 0;
            FILE *file = fopen("qtd.txt","r");
            while(feof(file) == 0){
                
                fscanf(file,"%d\n",&qtd_diretorio);
                
                
                printf("\nP||T: %d -> Qtd atual: %d + Qtd anterior %d", i+1, qtd_diretorio, qtd_final);         
                qtd_final+=qtd_diretorio;
                printf(" = %d\n\n", qtd_final);
                i++;

            }   
        
            printf("QTD_TOTAL: %d\n", qtd_final);

            file = fopen("qtd.txt","w");
            fprintf(file,"%s", "");


            fclose(file);
        }
}


int CONT_03(){
    
    qtd_raiz = 0;

    DIRENT_02();

    //Número de processadores == Número de processos

    n_p = get_nprocs();

    F_02(n_p, 0, ceil((double) qtd_raiz/(double)n_p));
    
}

int main(void){

    
    int op;

    printf("LEITOR E CONTADOR DE ARQUIVOS\n");
    printf("Autor: Euller Henrique Bandeira Oliveira \n");
    printf("Matrícula: 11821BSI210\n");
    printf("Período: 5º\n");
    printf("Curso: Sistemas De Informação\n");
    printf("Universidade: Universidade Federal De Uberlândia\n\n");

    printf("Você deseja ler e contar os arquivos do seu computador de que maneira? \n\n");
    printf("0 - Nenhum\n");
    printf("1 - Processo singlthreaded.\n");
    printf("2 - Múltiplos processos singlthreaded\n");
    printf("3 - Múltiplos processos multithreaded\n");
    while(1){
        printf("\n>>>  ");
        scanf("%d", &op);
        if(op == 0){
            return 0;
        }else if(op ==1){
            CONT_01();
        }else if(op==2){
            CONT_02();
        }else if(op==3){
            CONT_03();
        }
    }


    return 0;
}



































