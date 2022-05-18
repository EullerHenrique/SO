#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

struct tm *obterDataHora(){

        //Este ponteiro será utilizado para armazenar o endereço de uma struct tm.
        //Tal struct é utilizada para armazenar os dados relacionados a data e hora.
    
        struct tm *data_hora_atual;
       
        /* A struct tm foi definida da seguinte maneira na biblioteca time.h

        struct tm {
            int tm_sec; // Segundos (0-60)
            int tm_min; // Minutos (0-59)
            int tm_hour; // Horas (0-23)
            int tm_mday: // Dia do mês (1-31)
            int tm_mon; // Mês do ano (0-11)
            int tm_year; // Ano ( a partir de 1900 )
            int tm_wday; // Dia númerico da semana (0,1,2,3,4,5,6)
            int tm_yday; // Dia do ano ( 0-365)
            int tm_isdst; // Horário de verão (!0)
        };

        */

        // Variável do tipo time_t que irá armazenar o tempo em segundos entre a data local e primeiro de janeiro de 1970
        time_t segundos;
    
        // A função time é utilizada para obter o tempo em segundos
        // Tal função possui como argumento o endereço da variável segundos.
        // O tempo em segundos será armazenado no conteúdo do endereço da variável segundos.

        time(&segundos);

        // A função localtime é utilizada para converter o tempo em segundos para valores inteiros 
        // que representam cada atributo do tempo (segundos, minutos, hora, dia, mes, ano.. etc)
        // Esses valores serão armazenados nos respectivos campos da struct tm
        // Tal função retorna o endereço da struct com os dados de data e hora
        // O ponteiro data_hora_atual do tipo struct tm recebe o endereço da struct que contém os dados de data e hora
        // Dessa maneira, é possível acessar todos os dados dessa struct

        //O ponteiro data_hora_atual recebe o endereço da struct tm (ponteiro para a struct tm) que contém os dados de data e hora

        data_hora_atual = localtime(&segundos);

        return data_hora_atual;
}


void TIME( struct tm *data_hora_atual ){

    char hora[20];
        
    // A função strftime formata a data para hh:mm:ss
    // Tal função recebe a hora por meio da variavel data_hora_atual
    // do tipo struct tm, acessa a hora, os minutos e os segundos armazenados na struct por meio
    // dos formatadores %H (hora), %M (minutos), %S (segundos), formata a data para hh:mm:ss e 
    // finalmente armazena a hora formatada na variável hora
        
    strftime(hora, sizeof(hora), "%H:%M:%S\n",data_hora_atual);
        
    //Imprime a hora formatada
    printf("\nHora atual: %s\n", hora);

}

void DATE(struct tm *data_hora_atual){

    char data[20];

    // A função strftime formata a data para dd/mm/aaaa
    // Tal função recebe a data por meio da variavel data_hora_atual
    // do tipo struct tm, acessa o dia, o mes e o ano armazenados na struct por meio
    // dos formatadores %d (dia), %m (mes), %y (ano), formata a data para dd/mm/aaaa e 
    // finalmente armazena a data formatada na variável data

    strftime(data, sizeof(data), "%d/%m/%y\n",data_hora_atual);
    
    //Imprime a data formatada
    printf("\nData atual: %s\n", data);
    
    
}

void DIRENT(){

        //O ponteiro dir do tipo DIR será utilizado para armazenar o endereço do diretório desejado
        DIR *dir;
    
        //Este ponteiro será utilizado para armazenar o endereço de uma struct dirent.
        //Tal struct é utilizada para armazenar os dados  de um arquivo. 
        struct dirent *lsdir = NULL;

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
        dir = opendir(".");
   
        //A função readdir é utilizada para ler arquivos do diretório determinado
        //Tal função possui como parâmetro um ponteiro que receberá o endereço do diretório que será lido.
        //Tal função retorna o endereço da struct dirent do arquivo atual presente no diretório determinado
        //A cada iteração, o próximo arquivo é lido
        
        printf("\n");
        
        while((lsdir = readdir(dir)) != NULL){

            char nome[256];
            
            //O nome do diretório é acessado por meio do campo d_name da struct dirent
            strcpy(nome, lsdir->d_name);
            
            //. = diretório atual
            //.. = diretório pai
            //.arquivo = arquivo oculto


            if(strcmp(nome, ".") != 0 && strcmp(nome, "..") != 0){

                printf("<DIR>          %s\n", nome);
            
            }
    
        }

        printf("\n");

        //A função closedir fecha o diretório determinado
        closedir(dir);

}

void CLS(){

    printf("\e[1;1H\e[2J");
    
    // Código de escape ansi
    // Ao utilizar um código de escape o terminal utilizado o executa,
    // alterando a cor do texto, movendo o cursor do mouse
    // ou pulando linhas

    // \e é o caractere de escape
    // Um caracter de escape é utilizado para indicar ao terminal que ele deve interpretar
    // os caracteres subsequentes a sequencia de escape de uma forma diferente da forma padrão imposta pela linguagem utilizada
    // O caractere de escape \e é utilizado especificadamente para fazer com que a posição do cursor do mouse
    // possa ser alterada e a tela possa ser limpa. Sem o caractere de escape \e, o terminal não 
    // conseguiria perceber que aquela sequencia de caracteres poderia gerar ações, logo, ao executar o código,
    // os caracteres seriam somente impressos. 

    // [1;1h move o cursor para a posição (1,1)
    // [2J limpa a tela (pula linhas)
    // \e[1;1H\e[2J move o cursor para a posição(1,1) e limpa a tela de acordo com a posição do cursor do mouse,
    // ou seja, como o mouse está na posição (1,1), /e[2J pulará linha até chegar na posição (1,1).


}

void executar(char comando[]){
    
    if(strcmp(comando, "CLS") == 0){

        CLS();
 
    }else if(strcmp(comando, "DIR") == 0){
        
        DIRENT();

    }else if(strcmp(comando, "DATE") == 0 || strcmp(comando, "TIME") == 0){

        //Este ponteiro será utilizado para armazenar o endereço de uma struct tm.
        //Tal struct é utilizada para armazenar os dados relacionados a data e hora.
    
        struct tm *data_hora_atual =  obterDataHora();

        if(strcmp(comando, "DATE") == 0){
            
            DATE(data_hora_atual);

        }else if(strcmp(comando, "TIME") == 0){
        
            TIME(data_hora_atual);
        }
   
    }else{
        printf("\n%s : não encontrado\n\n",comando);
    }
    
}

int main(void){

    char comando[10];

    printf("\nShell v0.1 (Última atualização: 07/03/21) \n\n");
    printf("Autor: Euller Henrique Bandeira Oliveira \n");
    printf("Matrícula: 11821BSI210\n");
    printf("Período: 5º\n");
    printf("Curso: Sistemas De Informação\n");
    printf("Universidade: Universidade Federal De Uberlândia\n\n");

    while(1){
        printf(">>> ");
        fgets(comando, 10, stdin);
        comando[strcspn(comando,"\n")] = '\0';
        executar(comando);
    }

    return 0;
}
