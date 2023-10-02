#include "structs.h"
#include <stdio.h>

#define BACKEND_FIFO "BACKEND1"
#define FRONTEND_FIFO "FRONTEND%d"
#define BALCAO_FIFO "BALCAO"

char CLIENT_FIFO_FINAL[100];
char NCLIENT_FIFO_FINAL[210];

void sair(char user[]){
    printf("\nCliente saindo...\n");
    unlink(user);
    unlink(NCLIENT_FIFO_FINAL);
    exit(1);
}
void* balcaoVida(void * dados){
    int  data=*(int *)dados;
    int heartbeat=6;
    if(data!=0)  heartbeat=data;
    printf("\nheartbeat->%d<-\n",heartbeat);
    do{
        sleep(heartbeat);
        int fdenvia = open(BALCAO_FIFO, O_RDWR);
        if (fdenvia == -1)
            printf("Erro");
        write(fdenvia,CLIENT_FIFO_FINAL, sizeof(CLIENT_FIFO_FINAL));
        close(fdenvia);
    } while (1);
}

void* notifica(void *dados){
    char mensagem[210];
    int fdRecebe;
    sprintf(NCLIENT_FIFO_FINAL,"NOT%s",CLIENT_FIFO_FINAL);
    if (mkfifo(NCLIENT_FIFO_FINAL,0666) == -1)  {
        printf("Erro abrir fifo\n");
        pthread_exit(NULL);
        exit (1);
    }
    do {
        fdRecebe = open(NCLIENT_FIFO_FINAL, O_RDWR);
        if (fdRecebe == -1) {
            printf("Erro\n");
            pthread_exit(NULL);
        }else {
            read(fdRecebe, &mensagem, sizeof(mensagem));
            close(fdRecebe);
            printf("\n[%s]\n", mensagem);
        }
    }while(1);
    pthread_exit(NULL);
}

int main(int argc, char **argv,char*envp[]) {
    bool confirma_user=true; //confirmar credenciais user
    char comand[100];
    char validacao[50];

    clientes cli;
    B_F struc;

    itens item;
    char extra[50];

    pthread_t balcao;
    pthread_t noti;
    int heartbeat;
    for( int i=0;envp[i] != NULL;i++){
        sscanf(envp[i],"HEARTBEAT=%d",&heartbeat);
    }


    if (argc != 3) {
        if (argc < 3) {
            printf("Argumentos insufucientes!\n");
        } else if (argc > 3) {
            printf("Argumentos a mais!\n");
        }
        exit(1); //return da erro ns pq
    }

    if (strlen(argv[1]) > USRNAME_MAX){
        printf("Nome com caracteres a mais!\n");
        exit(1);
    }

    if (strlen(argv[2]) > PSWDCHAR_MAX){
        printf("Password com caracteres a mais!\n");
        exit(1);
    }


    //confirma_user=confirm_pass(argv[1]);

    //enviar credenciais para backend fazer verificacão e confirmar

    //caso confirme

    //=================================== FIFO ===========================================================

    int fd_envio, fd_resposta;

    sprintf(CLIENT_FIFO_FINAL,FRONTEND_FIFO,getpid());
    if (mkfifo(CLIENT_FIFO_FINAL,0666) == -1)  {
        printf("Erro abrir fifo");
        return 1;
    }

    struc.cli_pid=getpid();
    strcpy(struc.username,argv[1]);
    strcpy(struc.password,argv[2]);

    fd_envio=open(BACKEND_FIFO,O_WRONLY);

    int a=write(fd_envio,&struc,sizeof(struc));
    close(fd_envio);

    fd_resposta= open(CLIENT_FIFO_FINAL,O_RDWR);

    a= read(fd_resposta,validacao,sizeof(validacao));

    close(fd_resposta);
    printf("%s\n",validacao);
    if(!strcmp(validacao,"Utilizador válido e password correta\n")==0) return 1;

    sleep(1);



    //====================================================================================================

    if(pthread_create(&balcao,NULL,&balcaoVida,&heartbeat)!=0)return -1;
    if(pthread_create(&noti, NULL, &notifica, NULL) != 0) return -1;

    while (confirma_user){
        int a;
        printf("\nDeseja testar que funcionalidade? :");

        setbuf(stdin,NULL);
        scanf("%[^\n]s",comand);
        fflush(stdin);

        struc.ligado=true;
        sscanf(comand,"%s",struc.comando);
        if(strcmp(struc.comando,"sell") == 0 ) {
            a=sscanf(comand,"%s %s %s %d %d %d %s",struc.comando,struc.item.nome_item,struc.item.categoria,&struc.item.valor_base,&struc.item.compra_imediata,&struc.temp,extra);
            if(a==6) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }

        }
        else if(strcmp(struc.comando,"list") == 0){
            a=sscanf(comand,"%s %s",struc.comando,extra);
            if(a==1) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"licat") == 0) {
            a=sscanf(comand,"%s %s %s", struc.comando, struc.item.categoria, extra);
            if(a==2) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"lisel") == 0) {
            a=sscanf(comand,"%s %s %s",struc.comando,struc.item.vendedor,extra);
            if(a==2) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"lival") == 0) {
            a=sscanf(comand,"%s  %d %s",struc.comando,&struc.prec_MAX,extra);
            if(a==2) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"litime") == 0){
            a=sscanf(comand,"%s %d %s",struc.comando,&struc.temp,extra);
            if(a==2) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"time") == 0) {
            a=sscanf(comand,"%s %s",struc.comando,extra);
            if(a==1) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"buy") == 0) {
            a=sscanf(comand,"%s %d %d %s",struc.comando,&struc.item.id_item,&struc.licitacao,extra);
            if(a==3) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"cash") == 0) {

            a=sscanf(comand,"%s %s",struc.comando,extra);
            if(a==1) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"add") == 0) {
            a=sscanf(comand,"%s %d %s",struc.comando,&struc.deposito,extra);
            if(a==2) printf("Comando válido\n");
            else{
                printf("Parametros errados\n");
                continue;
            }
        }
        else if(strcmp(struc.comando,"exit") == 0){
            a=sscanf(comand,"%s %s",struc.comando,extra);
            if(a==1)
                struc.ligado=false;
            else{
                printf("Parametros errados\n");
                continue;
            }

        } else{
            printf("Comando inválido\n");
            continue;
        }

        //envia comando
        fd_envio= open(CLIENT_FIFO_FINAL,O_WRONLY);
        if(fd_envio == -1) printf("Erro a abrir fifo frontend");
        int e=write(fd_envio, &struc, sizeof(struc));

        close(fd_envio);


        //lista de itens por cat
        if(strcmp(struc.comando, "licat") == 0) {

            char lista1[MAX_ITEMVENDA][10];
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if(fd_resposta == -1) printf("Erro a abrir fifo frontend");
            int ar = read(fd_resposta, lista1, sizeof(lista1));
            close(fd_resposta);

            for(int i = 0; i < 29 && (strcmp(lista1[i],"") != 0); i++) {
                printf("Item: %s\n", lista1[i]);
            }
        }

        if(strcmp(struc.comando, "lival") == 0) {
            char lista1[MAX_ITEMVENDA][10];
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if(fd_resposta == -1) printf("Erro a abrir fifo frontend");
            read(fd_resposta, lista1, sizeof(lista1));
            close(fd_resposta);

            for(int i = 0; i < 29 && (strcmp(lista1[i],"") != 0); i++) {
                printf("Resposta: %s\n", lista1[i]);
            }
        }

        if(strcmp(struc.comando, "litime") == 0) {
            char lista1[MAX_ITEMVENDA][10];
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if(fd_resposta == -1) printf("Erro a abrir fifo frontend");
            read(fd_resposta, lista1, sizeof(lista1));
            close(fd_resposta);

            for(int i = 0; i < 29 && (strcmp(lista1[i],"") != 0); i++) {
                printf("Resposta: %s\n", lista1[i]);
            }
        }

        if(strcmp(struc.comando, "list") == 0) {
            char lista1[MAX_ITEMVENDA][10];

            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if(fd_resposta == -1) printf("Erro a abrir fifo frontend");
            int ar = read(fd_resposta, lista1, sizeof(lista1));
            close(fd_resposta);
            for(int i = 0; i < 29 && (strcmp(lista1[i],"") != 0); i++) {
                printf("Resposta: %s\n", lista1[i]);
            }
        }

        if(strcmp(struc.comando, "lisel") == 0) {
            char lista1[MAX_ITEMVENDA][10];

            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if(fd_resposta == -1) printf("Erro a abrir fifo frontend");
            read(fd_resposta, lista1, sizeof(lista1));
            close(fd_resposta);

            for(int i = 0; i < 29 && (strcmp(lista1[i],"") != 0); i++) {
                printf("Item: %s\n", lista1[i]);
            }
        }

        //consultar saldo / add saldo
        if(strcmp(struc.comando, "cash") == 0) {
            int saldo;
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if (fd_resposta == -1) printf("Erro a abrir fifo frontend");
            int l=read(fd_resposta, &saldo, sizeof(saldo));
            close(fd_resposta);
            printf("Saldo atual: %d\n", saldo);
        }

        if(strcmp(struc.comando, "add") == 0) {
            int saldo;
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if (fd_resposta == -1) printf("Erro a abrir fifo frontend");
            int l=read(fd_resposta, &saldo, sizeof(saldo));
            close(fd_resposta);
            printf("Saldo atualizado: %d\n", saldo);
        }

        if(strcmp(struc.comando, "time") == 0) {
            char temp_atual[20];
            fd_resposta = open(CLIENT_FIFO_FINAL, O_RDONLY);
            if (fd_resposta == -1) printf("Erro a abrir fifo frontend");
            int l=read(fd_resposta, &temp_atual, sizeof(temp_atual));
            close(fd_resposta);
            printf("Tempo atual(segundos): %s\n", temp_atual);
        }

        if (!struc.ligado) {
            int rc= pthread_cancel(balcao);
            if(rc) printf("Backend nao consegui kickar\n");
            sair(CLIENT_FIFO_FINAL);
        }
    }



    return 0;
}
