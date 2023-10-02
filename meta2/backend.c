#include "structs.h"
#include <stdio.h>
#include <fcntl.h>
#include "users_lib.h"

#define BACKEND_FIFO "BACKEND1"
#define FRONTEND_FIFO "FRONTEND%d"
#define BALCAO_FIFO "BALCAO"
pid_t id = -1;
int pd[MAX_PROGPROMOTORES];
int count_clientes=0;//conta frontend
char clientes_list [20][15];  //fifos
char clientes_name [20][15];
int TIME;
int out = 0;
/*
char* list_cat(char cat[]){
    FILE* filedesc;
    filedesc= fopen("filestxt/itens.txt","r");
    char * resposta[MAX_ITEMVENDA][10];
    itens itens;
    int auxi = 0;
    int id_aux = 1;
    int a;
    while(fscanf(filedesc,"%d %s %s %d %d %d %s %s",&a, itens.nome_item,itens.categoria,&itens.valor_base,&itens.compra_imediata,&itens.tempo,itens.vendedor,itens.comprador)==7){
        if(strcmp(itens.categoria, cat) == 0) {
            strcpy(*resposta[auxi], itens.nome_item);
            ++auxi;
        }
    }

    fclose(filedesc);
    return resposta;
}*/

/*void kick(char *cliente){
    B_F sinal;
    sinal.ligado=false;

    int fdEnvia = open(cliente, O_WRONLY);
    if (fdEnvia == -1)
        printf("%s\n",cliente);
    write(fdEnvia,&sinal, sizeof(sinal));
    close(fdEnvia);
}*/
void* balcaoVida(void * dados){

    backend  data=*(backend *)dados;

    int begin= time(NULL),momento;
    int clientes_vida[MAX_USERS];
    for (int i = 0; i < 20; ++i) {
        clientes_vida[i]=-1;
    }
    B_F comand;
    strcpy(comand.comando,"exitthread");
    int heartbeat=6;

    if(data.heartbeat!=0)  heartbeat=data.heartbeat;
    printf("\nheartbeat->%d<-\n",heartbeat);

    char buffer[15];
    for (int i = 0; i < 20; ++i) {
        strcpy(clientes_list[i],"--");
    }

    if (mkfifo(BALCAO_FIFO, 0666) == -1)
    {
        if (errno == EEXIST)
        {
            printf("Servidor em execução ou fifo já existe");
            return ;
        }
        printf("Erro abrir fifo");
        return ;
    }

    do {
        int fdrecebe = open(BALCAO_FIFO, O_RDWR);
        if (fdrecebe == -1)
            printf("Erro");
        read(fdrecebe,buffer, sizeof(buffer));
        close(fdrecebe);
        for (int i = 0; i < 20; ++i) {
            if(strcmp(clientes_list[i],buffer)==0){
                clientes_vida[i]= (time(NULL)-begin) + heartbeat + 2;
                //printf("%s %d %d\n",buffer,(int)(time(NULL)-begin),clientes_vida[i]);
            }
            momento=(time(NULL) - begin);
            if(clientes_vida[i]<=momento){
                //printf("\neu %d %d\n",momento,clientes_vida[i]);
                if(clientes_vida[i]>0){
                    printf("\n[%s saiu...]\n",clientes_name[i]);
                     int fdenvia = open(clientes_list[i], O_RDWR);
                    if (fdenvia == -1)
                        printf("Erro");
                    write(fdenvia,&comand, sizeof(comand));

                    strcpy(clientes_list[i],"--");
                    strcpy(clientes_name[i],"--");
                    clientes_vida[i]=-1;
                }

            }
        }
        strcpy(buffer,"");
    }while(1);


}


void* LaunchPromotor(void * dados){

    prom* data=(prom *)dados;



    int FD[2];
    pipe(FD);
    char promotor1[MAX_COMAND],aux[MAX_COMAND]="";
    char path[MAX_COMAND];


   // printf("\n\n---\n->%s %d\n---\n\n",data.promotor,data.i);
    id=fork();
    //data.pid=id;

    if(id==0){//filho

        close(1);
        dup(FD[1]);
        close(FD[1]);
        close(FD[0]);

        sprintf(path,"promotores/%s",data->name);

        execl(path,data->name,NULL);

        printf("Promotor nao lançado\n");
        exit(1);
    }else{
        data->pid=id;
        close(FD[1]);



        while(1){

            read(FD[0],&promotor1,sizeof (promotor1));

            if(strcmp(promotor1,aux)!=0){
                printf("\nPromotor diz:\n-> %s\n",promotor1);
                strcpy(aux,promotor1);
                strcpy(promotor1,"");
            }
            //if(data.promotor!=NULL)  free(data.promotor);
        }


        //mandar sinal para fechar para pid=0

        close(FD[0]);

    }
    pthread_exit(NULL);
}

void FilePromotor(backend* data){
   //estas a receber mal o tipo de dados



    char prom[20];

    int error;
    pthread_t promotorThread[MAX_PROGPROMOTORES];

    FILE *filedesc;
    filedesc= fopen(data->fpromotores,"r");





    int i=0;

    while(fscanf(filedesc,"%s %d",prom,&error)==1 && i<10){

        strcpy(data->promotor[i].name,prom);

        if(pthread_create(&promotorThread[i],NULL,&LaunchPromotor,&data->promotor[i])!=0)return ;
        ++i;
    }
    i=0;
    while (i<10){
        pd[i]=data->promotor[i].pid;
        ++i;
    }

}


void CancelPromotor(backend* dados,char* nome_prom){
    backend * data=(backend *)dados;
    int i=0;


    do{

        printf("\n\n");
        if(strcmp(data->promotor[i].name,nome_prom)==0){
            union sigval val;
            sigqueue(data->promotor[i].pid,SIGUSR1,val);
            //printf("Promotor: %s saindo\n",data.promotor[i].name);
            strcpy(data->promotor[i].name,"--");
        }
        ++i;
    } while(i<10);

}

void Reprom(backend* dados){
    backend * data=(backend *)dados;

    FILE *filedesc;
    filedesc= fopen(data->fpromotores,"r");
    int i=0;
    char prom[10][20];
    int error;
    pthread_t promotorThread[MAX_PROGPROMOTORES];
    for (int j = 0; j <10 ; ++j) {
        strcpy(prom[j],"");
    }

    while(fscanf(filedesc,"%s %d",prom[i],&error)==1 && i<10){
        ++i;
    }

    for (int j = 0; j < 10; ++j) {
        if(strcmp(prom[0],data->promotor[j].name)==0){ //== significa que ha
            continue;
        }if(strcmp(prom[1],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[2],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[3],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[4],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[5],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[6],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[7],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[8],data->promotor[j].name)==0){
            continue;
        }if(strcmp(prom[9],data->promotor[j].name)==0){
            continue;
        }else{
            CancelPromotor(data,data->promotor[j].name); //Cancela os que ja nao estao no ficheiro

        }

    }

    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < 10; ++k) {
            if(strcmp(data->promotor[k].name,prom[j])==0){
                strcpy(prom[j],"__");               //Ficam so os que ainda nao estao em açao
                break;
            }
        }

    }
    for (int j = 0; j <10 ; ++j) {
    }

    for (int j = 0; j <10 ; ++j) {
      if(strcmp(prom[j],"__")!=0){
          for (int k = 0; k < 10; ++k) {
              if(strlen(data->promotor[k].name)<=2){
                  strcpy(data->promotor[k].name,prom[j]);
                  if(pthread_create(&promotorThread[j],NULL,&LaunchPromotor,&data->promotor[k])!=0)return ;
                  break;
              }
          }
      }
    }
    /*  if(strlen(data->promotor[j].name)<=2){
            for (int k = 0; k < 10; ++k) {
                if(strcmp(prom[k],"__")!=0){
                    strcpy(data->promotor[j].name,prom[k]);
                    if(pthread_create(&promotorThread[j],NULL,&LaunchPromotor,&data->promotor[j])!=0)return ;
                }
            }
        }
*/




















    /*while(fscanf(filedesc,"%s %d",prom[i],&error)==1 && i<10){
        ++i;
    }
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k <10 ; ++k) {
            if(strcmp(data->promotor[j].name,prom[k])==0){
                strcpy(prom[k],"__");
            }else{
                CancelPromotor(data,data->promotor[j].name);
            }
        }
    }
    for (int j = 0; j <10 ; ++j) {
        if (strcmp(data->promotor[j].name, "--") == 0) {        //lançar os que nao estavam
            for (int k = 0; k <10 ; ++k) {
                if(strcmp(prom[k],"__")!=0){
                    if(pthread_create(&promotorThread[j],NULL,&LaunchPromotor,&data->promotor[j])!=0)return ;
                    break;
                }
            }
        }
    }*/
}

void sair(int sign){
    printf("\nSaindo...\n");
    unlink(BACKEND_FIFO);
    unlink(BALCAO_FIFO);
    union sigval valores;
    for (int i = 0; i <MAX_PROGPROMOTORES ; ++i) {
        sigqueue(pd[i],SIGUSR1,valores);
    }

    exit(1);
}

void* atendeAdm(void * dados){
    backend data=*((backend *)dados);

    char comando[MAX_COMAND], extra[50];
    int a;
    char var[20];
    strcpy(var, data.fitens);
    char userAlvo[USRNAME_MAX], name_prom[USRNAME_MAX];
    B_F mensagemEnviar;
    do{
        char com[50];
        printf("\nInserir comando\n>");
        scanf("%[^\n]%*c",com);
        a = sscanf(com,"%s %s %s", comando, name_prom, extra);

        if(strcmp(comando, "users") == 0){
            printf("LISTA DE USERS:\n");
            for(int i = 0; i < 20 && (strcmp(clientes_name[i],"") != 0); i++)
                printf("USER %d: %s\n", i+1, clientes_name[i]);
        }else if(strcmp(comando, "list") == 0){
            FILE* filedesc;
            filedesc= fopen(var,"r");
            char lista[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            while(fscanf(filedesc,"%d %s %s %d %d %d %s %s",&itens.id_item ,itens.nome_item,itens.categoria,&itens.valor_base,&itens.compra_imediata,&itens.tempo,itens.vendedor,itens.comprador)==8){
                strcpy(lista[auxi], itens.nome_item);
                ++auxi;
            }
            strcpy(lista[auxi], "");
            fclose(filedesc);

            printf("LISTA DE ITENS A VENDA:\n");
            for(int i = 0; i < 29 && (strcmp(lista[i],"") != 0); i++) {
                printf("ITEM: %s\n", lista[i]);
            }

        }else if(strcmp(comando, "kick") == 0){
            int aux;
            a = sscanf("%s %s %s", comando, userAlvo, extra);
            if(a == 2) {
                printf("Kick user: %s\n", userAlvo);
                for(int i = 0; i < 20; i++)
                    if(strcmp(userAlvo, clientes_name[i]) == 0)
                        aux = i;
                strcpy(mensagemEnviar.resposta, "Admin deu-lhe kick!\n");
                mensagemEnviar.ligado=false;

            }else
                printf("Parametros Invalidos\n");
        }else if(strcmp(comando, "prom") == 0){
            printf("Teste:\n");
            FilePromotor(&data);
        }else if(strcmp(comando, "reprom") == 0){
            Reprom(&data);    // <= fazer
        }else if(strcmp(comando, "cancel") == 0){
            if(a == 2) {
                CancelPromotor(&data,name_prom);
            }else
                printf("Parametros Invalidos\n");
        }

    }while(strcmp(comando, "close") != 0);
    pthread_exit(NULL);
}

void* atendeCliente(void * dados){
    B_F data=*((B_F*)dados);
    char CLIENT_FIFO_FINAL[20];
    char notclientes_list [30][15];
    sprintf(CLIENT_FIFO_FINAL,FRONTEND_FIFO,data.cli_pid);

    int fdEnvia, fdRecebe;
    int temp_atual;
    int val_aux, aux, saldo, n_item;
    char lista_cat[MAX_ITEMVENDA*10];
    itens lista_venda[MAX_ITEMVENDA];
    char mensagem[200];
    int j = 0;
    char var[20];
    char pathfi[20];
    strcpy(pathfi, data.fileusers);
    printf("f1: %s\n", pathfi);
    strcpy(var, data.fitem);
    FILE* filedesc;
    printf("f: %s\n", data.fitem);
    filedesc= fopen(var,"r");
    if(filedesc == NULL) {
        printf("Erro a abrir ficheiro!\n");
        fclose(filedesc);
    }else {
        while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &lista_venda[j].id_item, lista_venda[j].nome_item,
                      lista_venda[j].categoria, &lista_venda[j].valor_base, &lista_venda[j].compra_imediata,
                      &lista_venda[j].tempo, lista_venda[j].vendedor, lista_venda[j].comprador) == 8) {
            j++;
        }
        fclose(filedesc);
    }

    do{
        fdRecebe = open(CLIENT_FIFO_FINAL, O_RDONLY);
        if (fdRecebe == -1) {
            printf("Erro");
            pthread_exit(NULL);
        }
        int ar = read(fdRecebe, &data, sizeof(data));

        close(fdRecebe);

        sleep(1);

        if (strcmp(data.comando, "list") == 0) {
            FILE *fi;
            fi = fopen(var,"r");
            if(fi == NULL) {
                printf("Erro a abrir ficheiro! \n");
                fclose(filedesc);
            }
            char resposta[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            strcpy(resposta[0], "");
            while(fscanf(fi,"%d %s %s %d %d %d %s %s",&itens.id_item ,itens.nome_item,itens.categoria,&itens.valor_base,&itens.compra_imediata,&itens.tempo,itens.vendedor,itens.comprador)==8){
                strcpy(resposta[auxi], itens.nome_item);
                ++auxi;
            }
            strcpy(resposta[auxi], "");
            fclose(fi);
            printf("chega\n");
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro");
                pthread_exit(NULL);
            }
            write(fdEnvia, resposta, sizeof(resposta));
            close(fdEnvia);
        } else if (strcmp(data.comando, "licat") == 0) {
            char resposta[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            FILE *filedesc;
            filedesc = fopen(var, "rt");
            strcpy(resposta[0], "");
            while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &itens.id_item, itens.nome_item,itens.categoria, &itens.valor_base,&itens.compra_imediata, &itens.tempo, itens.vendedor, itens.comprador) == 8) {
                if (strcmp(itens.categoria, data.item.categoria) == 0) {
                    strcpy(resposta[auxi], itens.nome_item);
                    ++auxi;
                }
            }
            strcpy(resposta[auxi], "");
            fclose(filedesc);
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro\n");
                pthread_exit(NULL);
            }
            int ar = write(fdEnvia, &resposta, sizeof(resposta));
            close(fdEnvia);
        } else if (strcmp(data.comando, "lisel") == 0) {
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro");
                pthread_exit(NULL);
            }
            printf("Lista\n");
            FILE *filedesc;
            filedesc = fopen(var, "r");
            char resposta[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            strcpy(resposta[0], "");
            while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &itens.id_item, itens.nome_item, itens.categoria, &itens.valor_base,
                          &itens.compra_imediata, &itens.tempo, itens.vendedor, itens.comprador) == 8) {
                if (strcmp(itens.vendedor, data.item.vendedor) == 0) {
                    strcpy(resposta[auxi], itens.nome_item);
                    ++auxi;
                }
            }
            strcpy(resposta[auxi], "");
            fclose(filedesc);

            write(fdEnvia, resposta, sizeof(resposta));
            close(fdEnvia);
        } else if (strcmp(data.comando, "lival") == 0) {
            printf("Lista\n");
            FILE *filedesc;
            filedesc = fopen(var, "r");
            char resposta[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            strcpy(resposta[0], "");
            while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &itens.id_item, itens.nome_item, itens.categoria, &itens.valor_base,
                          &itens.compra_imediata, &itens.tempo, itens.vendedor, itens.comprador) == 8) {
                if (itens.valor_base <= data.prec_MAX) {
                    strcpy(resposta[auxi], itens.nome_item);
                    ++auxi;
                }
            }
            strcpy(resposta[auxi], "");
            fclose(filedesc);

            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro");
                pthread_exit(NULL);
            }
            write(fdEnvia, resposta, sizeof(resposta));
            close(fdEnvia);
        } else if (strcmp(data.comando, "litime") == 0) {
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1)
            {
                printf("Erro");
                pthread_exit(NULL);
            }
            char resposta[MAX_ITEMVENDA][10];
            itens itens;
            int auxi = 0;
            strcpy(resposta[0], "");
            for(int i = 0; i < MAX_ITEMVENDA; i++){
                if((lista_venda[i].tempo - TIME) <= data.temp && strcmp(lista_venda[i].nome_item, "")) {
                    strcpy(resposta[auxi], lista_venda[i].nome_item);
                    ++auxi;
                }
            }
            strcpy(resposta[auxi], "");
            write(fdEnvia, resposta, sizeof(resposta));
            close(fdEnvia);
        } else if (strcmp(data.comando, "time") == 0) {
            //temp_atual = 0;
            char tim[20];
            sprintf(tim, "%d", TIME);
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro");
                pthread_exit(NULL);
            }
            write(fdEnvia, tim, sizeof(tim));
            close(fdEnvia);
        } else if (strcmp(data.comando, "sell") == 0) {
            bool encontra;
            int n_item, i = 0, j = 0;

            FILE *filedesc;
            filedesc = fopen(var, "r");
            if (filedesc == NULL) {
                printf("Erro a abrir ficheiro!\n");
                fclose(filedesc);
            } else {
                while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &lista_venda[j].id_item,
                              lista_venda[j].nome_item,
                              lista_venda[j].categoria, &lista_venda[j].valor_base, &lista_venda[j].compra_imediata,
                              &lista_venda[j].tempo, lista_venda[j].vendedor, lista_venda[j].comprador) == 8) {
                    j++;
                }
                fclose(filedesc);

                //pega uma ala vazia do array se é ordenado logo exemplo:lista_venda[0].id é 1
                do {
                    if (lista_venda[i].id_item != i + 1) {
                        n_item = i;
                        encontra = true;
                    } else {
                        i++;
                        if (i > 29)
                            encontra = true;
                    }
                } while (!encontra);
                //printf("I: %d\n", n_item);
                for (i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                    printf("%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                           lista_venda[i].nome_item,
                           lista_venda[i].categoria, lista_venda[i].valor_base, lista_venda[i].compra_imediata,
                           lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                }
                //se  i for maior q 29 é pq já existem 30 itens
                if (i > 29) {
                    printf("Sem espaço para colocar novos itens a venda!\n");
                }
                else {
                    lista_venda[n_item].id_item = n_item + 1;
                    strcpy(lista_venda[n_item].nome_item, data.item.nome_item);
                    strcpy(lista_venda[n_item].categoria, data.item.categoria);
                    strcpy(lista_venda[n_item].comprador, "-");
                    strcpy(lista_venda[n_item].vendedor, data.username);
                    lista_venda[n_item].valor_base = data.item.valor_base;
                    lista_venda[n_item].tempo = data.temp + TIME;

                    if(data.item.compra_imediata <= 0) {
                        lista_venda[n_item].compra_imediata = 0;
                    }else{
                        lista_venda[n_item].compra_imediata = data.item.compra_imediata;
                    }
                    //printf("Id: %d\n", lista_venda[n_item].id_item);
                    sprintf(mensagem, "\nItem posto a venda\nId: %d Nome: %s Categoria: %s Valor que foi lancado: %d Compre ja: %d\n",lista_venda[n_item].id_item, lista_venda[n_item].nome_item,lista_venda[n_item].categoria, lista_venda[n_item].valor_base, lista_venda[n_item].compra_imediata);

                    int aux = 0;
                    while(strcmp(clientes_list[aux], "") != 0) {
                        sprintf(notclientes_list[aux], "NOT%s", clientes_list[aux]);
                        fdEnvia = open(notclientes_list[aux], O_WRONLY);
                        if (fdEnvia == -1) {
                            printf("Erro");
                            pthread_exit(NULL);
                        }
                        write(fdEnvia, &mensagem, sizeof(mensagem));
                        close(fdEnvia);
                        ++aux;
                    }

                    strcpy(mensagem, "");
                    FILE *filesc;
                    filesc = fopen(var, "w");
                    if (filesc == NULL) {
                        printf("Erro a abrir ficheiro!\n");
                    } else {
                        for (i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                            fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                    lista_venda[i].nome_item,
                                    lista_venda[i].categoria, lista_venda[i].valor_base,
                                    lista_venda[i].compra_imediata,
                                    lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                        }
                        printf("Atualizado lista itens!\n");
                    }
                    fclose(filesc);
                    encontra = false;
                }
            }
        } else if (strcmp(data.comando, "buy") == 0) {
            //int val_aux, aERRux, saldo, n_item;
            int j = 0;
            int nitens = 0;
            int sal_vendedor;
            saldo = getUserBalance(data.username);

            FILE *filedesc;
            filedesc = fopen(var, "r");
            if (filedesc == NULL) {
                printf("Erro a abrir ficheiro!\n");
                fclose(filedesc);
            } else {
                while (fscanf(filedesc, "%d %s %s %d %d %d %s %s", &lista_venda[j].id_item,
                              lista_venda[j].nome_item,
                              lista_venda[j].categoria, &lista_venda[j].valor_base, &lista_venda[j].compra_imediata,
                              &lista_venda[j].tempo, lista_venda[j].vendedor, lista_venda[j].comprador) == 8) {
                    j++;
                }
                fclose(filedesc);
                nitens = j;
                //verifica se o tempo acabou
                for(int j = 0; j < MAX_ITEMVENDA; j++) {
                    if (TIME >= lista_venda[j].tempo && strcmp(lista_venda[j].nome_item, "") != 0 && strcmp(lista_venda[j].comprador, "-") == 0){
                        printf("Tempo do item %s expirado e sem comprador!\n", lista_venda[j].nome_item);
                        sprintf(mensagem, "Tempo do item expirado!\nId: %d Nome: %s Categoria: %s Valor que foi lancado: %d Comprador: sem comprador",lista_venda[j].id_item, lista_venda[j].nome_item,lista_venda[j].categoria, lista_venda[j].valor_base);
                        int aux = 0;
                        while(strcmp(clientes_list[aux], "") != 0) {
                            sprintf(notclientes_list[aux], "NOT%s", clientes_list[aux]);
                            fdEnvia = open(notclientes_list[aux], O_WRONLY);
                            if (fdEnvia == -1) {
                                printf("Erro");
                                pthread_exit(NULL);
                            }
                            write(fdEnvia, &mensagem, sizeof(mensagem));
                            close(fdEnvia);
                            ++aux;
                        }

                        strcpy(mensagem, "");
                        if(data.item.id_item > lista_venda[j].id_item)
                            data.item.id_item = data.item.id_item - 1;
                        for (int k = j; k < nitens; k++) {
                            lista_venda[k].id_item = lista_venda[k + 1].id_item - 1;
                            strcpy(lista_venda[k].nome_item, lista_venda[k + 1].nome_item);
                            strcpy(lista_venda[k].categoria, lista_venda[k + 1].categoria);
                            strcpy(lista_venda[k].comprador, lista_venda[k + 1].comprador);
                            strcpy(lista_venda[k].vendedor, lista_venda[k + 1].vendedor);
                            lista_venda[k].valor_base = lista_venda[k + 1].valor_base;
                            lista_venda[k].compra_imediata = lista_venda[k + 1].compra_imediata;
                            lista_venda[k].tempo = lista_venda[k + 1].tempo;
                        }
                        FILE *filesc;
                        filesc = fopen(var, "w");
                        if (filesc == NULL) {
                            printf("Erro a abrir ficheiro!\n");
                        } else {
                            for (int i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                                fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                        lista_venda[i].nome_item,
                                        lista_venda[i].categoria, lista_venda[i].valor_base, lista_venda[i].compra_imediata,
                                        lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                            }
                            printf("Atualizado lista itens!\nItens %d\n", nitens);
                        }
                        fclose(filesc);
                    }
                    if (TIME >= lista_venda[j].tempo && strcmp(lista_venda[j].nome_item, "") != 0 && strcmp(lista_venda[j].comprador, "-") != 0) {
                        printf("tempo do item %s acabou!\n", lista_venda[j].nome_item);
                        sprintf(mensagem, "Tempo do item expirado!\nId: %d Nome: %s Categoria: %s Valor que foi lancado: %d Comprador: %s",lista_venda[j].id_item, lista_venda[j].nome_item,lista_venda[j].categoria, lista_venda[j].valor_base, lista_venda[j].comprador);
                        int aux = 0;
                        while(strcmp(clientes_list[aux], "") != 0) {
                            sprintf(notclientes_list[aux], "NOT%s", clientes_list[aux]);
                            fdEnvia = open(notclientes_list[aux], O_WRONLY);
                            if (fdEnvia == -1) {
                                printf("Erro");
                                pthread_exit(NULL);
                            }
                            write(fdEnvia, &mensagem, sizeof(mensagem));
                            close(fdEnvia);
                            ++aux;
                        }
                        if(data.item.id_item > lista_venda[j].id_item)
                            data.item.id_item = data.item.id_item - 1;
                        aux = loadUsersFile("filestxt/users.txt");
                        if(aux == -1){
                            printf("%s\n", getLastErrorText());
                        }
                        val_aux = getUserBalance(lista_venda[j].comprador) - lista_venda[j].valor_base;
                        //apos ter comprado

                        aux = updateUserBalance(data.username, val_aux);
                        sal_vendedor = getUserBalance(lista_venda[j].vendedor) + lista_venda[j].valor_base;
                        aux = updateUserBalance(lista_venda[j].vendedor, sal_vendedor);
                        if (aux == 0) {
                            printf("%s\n", getLastErrorText());
                        }
                        else if (aux == -1) {
                            printf("%s\n", getLastErrorText());
                        }
                        else if(aux == 1){
                            aux = saveUsersFile(pathfi);
                            if (aux == -1) {
                                printf("%s\n", getLastErrorText());
                            }
                            printf("Saldo de %s alterado!\n", lista_venda[j].comprador);
                            saldo = getUserBalance(lista_venda[j].comprador);
                            if(saldo == -1){
                                printf("%s\n", getLastErrorText());
                            }else{
                                printf("Saldo: %d\n", saldo);
                            }
                        }
                        int i = 0;
                        while (strcmp(lista_venda[i].nome_item, "") != 0) {
                            ++i;
                        }
                        for (int k = j; k < nitens; k++) {
                            lista_venda[k].id_item = lista_venda[k + 1].id_item - 1;
                            strcpy(lista_venda[k].nome_item, lista_venda[k + 1].nome_item);
                            strcpy(lista_venda[k].categoria, lista_venda[k + 1].categoria);
                            strcpy(lista_venda[k].comprador, lista_venda[k + 1].comprador);
                            strcpy(lista_venda[k].vendedor, lista_venda[k + 1].vendedor);
                            lista_venda[k].valor_base = lista_venda[k + 1].valor_base;
                            lista_venda[k].compra_imediata = lista_venda[k + 1].compra_imediata;
                            lista_venda[k].tempo = lista_venda[k + 1].tempo;
                        }
                        FILE *filesc;
                        filesc = fopen(var, "w");
                        if (filesc == NULL) {
                            printf("Erro a abrir ficheiro!\n");
                        } else {
                            for (int i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                                fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                        lista_venda[i].nome_item,
                                        lista_venda[i].categoria, lista_venda[i].valor_base, lista_venda[i].compra_imediata,
                                        lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                            }
                            printf("Atualizado lista itens!\nItens %d\n", nitens);
                        }
                        fclose(filesc);
                    }
                    //fazer notificacao
                }

                for (int i = 0; i < MAX_ITEMVENDA; i++) {
                    if (lista_venda[i].id_item == data.item.id_item)
                        n_item = i;
                }

                //verifica se é possível fazer a licitação
                if(TIME < lista_venda[n_item].tempo && saldo > data.licitacao &&
                   data.licitacao >= lista_venda[n_item].compra_imediata && lista_venda[n_item].compra_imediata > 0){
                    //escreve o nome do comprador e o valor que ofereceu
                    strcpy(lista_venda[n_item].comprador, data.username);
                    lista_venda[n_item].valor_base = data.licitacao;
                    sprintf(mensagem, "Item comprado imediatamente!\nId: %d Nome: %s Categoria: %s Valor que foi lancado: %d Comprador: %s",lista_venda[n_item].id_item, lista_venda[n_item].nome_item,lista_venda[n_item].categoria, lista_venda[n_item].valor_base, lista_venda[n_item].comprador);
                    int aux = 0;
                    while(strcmp(clientes_list[aux], "") != 0) {
                        sprintf(notclientes_list[aux], "NOT%s", clientes_list[aux]);
                        fdEnvia = open(notclientes_list[aux], O_WRONLY);
                        if (fdEnvia == -1) {
                            printf("Erro");
                            pthread_exit(NULL);
                        }
                        write(fdEnvia, &mensagem, sizeof(mensagem));
                        close(fdEnvia);
                        ++aux;
                    }
                    //atualizar apos uma licitacao dados do ficheiro
                    FILE *filesc;
                    filesc = fopen(var, "w");
                    if (filesc == NULL) {
                        printf("Erro a abrir ficheiro!\n");
                    } else {
                        for (int i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                            fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                    lista_venda[i].nome_item,
                                    lista_venda[i].categoria, lista_venda[i].valor_base,
                                    lista_venda[i].compra_imediata,
                                    lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                        }
                        printf("Atualizado lista itens!\n");
                    }
                    fclose(filesc);
                    val_aux = getUserBalance(lista_venda[n_item].comprador) - lista_venda[n_item].valor_base;
                    //apos ter comprado
                    sal_vendedor = getUserBalance(lista_venda[n_item].vendedor) + lista_venda[n_item].valor_base;
                    aux = updateUserBalance(lista_venda[n_item].vendedor, sal_vendedor);
                    aux = updateUserBalance(data.username, val_aux);
                    if (aux == 0) {
                        printf("%s\n", getLastErrorText());
                    }
                    else if (aux == -1) {
                        printf("%s\n", getLastErrorText());
                    }
                    else if(aux == 1){
                        aux = saveUsersFile(pathfi);
                        if (aux == -1) {
                            printf("%s\n", getLastErrorText());
                        }
                        printf("Saldo de %s alterado!\n", lista_venda[n_item].comprador);
                        saldo = getUserBalance(lista_venda[n_item].comprador);
                        if(saldo == -1){
                            printf("%s\n", getLastErrorText());
                        }else{
                            printf("Saldo: %d\n", saldo);
                        }
                    }
                    int i = 0;
                    while (strcmp(lista_venda[i].nome_item, "") != 0) {
                        ++i;
                    }
                    for (int k = n_item; k < nitens; k++) {
                        lista_venda[k].id_item = lista_venda[k + 1].id_item - 1;
                        strcpy(lista_venda[k].nome_item, lista_venda[k + 1].nome_item);
                        strcpy(lista_venda[k].categoria, lista_venda[k + 1].categoria);
                        strcpy(lista_venda[k].comprador, lista_venda[k + 1].comprador);
                        strcpy(lista_venda[k].vendedor, lista_venda[k + 1].vendedor);
                        lista_venda[k].valor_base = lista_venda[k + 1].valor_base;
                        lista_venda[k].compra_imediata = lista_venda[k + 1].compra_imediata;
                        lista_venda[k].tempo = lista_venda[k + 1].tempo;
                    }
                    FILE *fi;
                    fi = fopen(var, "w");
                    if (filesc == NULL) {
                        printf("Erro a abrir ficheiro!\n");
                    } else {
                        for (int i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                            fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                    lista_venda[i].nome_item,
                                    lista_venda[i].categoria, lista_venda[i].valor_base, lista_venda[i].compra_imediata,
                                    lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                        }
                        printf("Atualizado lista itens!\nItens %d\n", nitens);
                    }
                    fclose(fi);

                }else if (TIME < lista_venda[n_item].tempo && saldo > data.licitacao &&
                          data.licitacao > lista_venda[n_item].valor_base) {
                    //escreve o nome do comprador e o valor que ofereceu
                    strcpy(lista_venda[n_item].comprador, data.username);
                    lista_venda[n_item].valor_base = data.licitacao;
                    //atualizar apos uma licitacao dados do ficheiro
                    FILE *filesc;
                    filesc = fopen(var, "w");
                    if (filesc == NULL) {
                        printf("Erro a abrir ficheiro!\n");
                    } else {
                        for (int i = 0; i < MAX_ITEMVENDA && (strcmp(lista_venda[i].nome_item, "") != 0); i++) {
                            fprintf(filesc, "%d %s %s %d %d %d %s %s\n", lista_venda[i].id_item,
                                    lista_venda[i].nome_item,
                                    lista_venda[i].categoria, lista_venda[i].valor_base,
                                    lista_venda[i].compra_imediata,
                                    lista_venda[i].tempo, lista_venda[i].vendedor, lista_venda[i].comprador);
                        }
                        printf("Atualizado lista itens!\n");
                    }
                    fclose(filesc);
                }else{
                    printf("v: %d", lista_venda[n_item].valor_base);
                    printf("Valor baixo rejeitado\n");
                }
            }
        } else if (strcmp(data.comando, "cash") == 0) {
            //int saldo;
            saldo = getUserBalance(data.username);
            if (saldo == -1)
                printf("%s\n", getLastErrorText());
            else {
                fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
                if (fdEnvia == -1) {
                    printf("Erro");
                    pthread_exit(NULL);
                }
                write(fdEnvia, &saldo, sizeof(saldo));
                close(fdEnvia);
            }
        } else if (strcmp(data.comando, "add") == 0) {
            int saldo = getUserBalance(data.username);
            int aux;
            int saldo_atual;
            aux = updateUserBalance(data.username, (data.deposito + saldo));
            if (aux == 0)
                printf("%s\n", getLastErrorText());
            else if (aux == -1)
                printf("%s\n", getLastErrorText());
            else
                printf("Saldo de %s alterado!\n", data.username);
            saldo_atual = getUserBalance(data.username);
            saveUsersFile(pathfi);
            fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
            if (fdEnvia == -1) {
                printf("Erro");
                pthread_exit(NULL);
            }
            write(fdEnvia, &saldo_atual, sizeof(saldo_atual));
            close(fdEnvia);
        }
        printf("%s\n",data.comando);
        strcpy(data.comando,"");
    } while (strcmp(data.comando,"exit")!=0);
    fdEnvia = open(CLIENT_FIFO_FINAL, O_WRONLY);
    if (fdEnvia == -1) {
        printf("Erro");
        pthread_exit(NULL);
    }
    data.ligado = false;
    write(fdEnvia, &data, sizeof(data));
    close(fdEnvia);
    pthread_exit(NULL);
}

static void* Timer(void* data){
    while(out == 0){
        sleep(1);
        TIME++;
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv,char*envp[]) {

    int opt;
    FILE *filedesc;
    backend backend;

    for( int i=0;envp[i] != NULL;i++){
        sscanf(envp[i],"HEARTBEAT=%d",&backend.heartbeat);
    }

    int FD[2];
    pipe(FD);

    //--------------------------------------------------------//
    bool confirma_user; //confirmar credenciais user
    //if(confirma_user == false) printf("Erro na verificação de credenciais!\n");
    char comand[MAX_COMAND];
    char confirma[50];
    clientes admin;
    clientes cli;
    //itens item;
    itens itens[MAX_ITEMVENDA];
    vendas venda;
    int extra;
    //item.vendas1=extra;
    int a;
    char aux[MAX_COMAND]="";
    char nome_exe[30];//nome executavel
    TIME = 0;
    char name[USRNAME_MAX], pswd[PSWDCHAR_MAX];
    int val, totalusers;
    int op, saldo;


    //========================== Var ambiente ===================================
    char user[20];
    char promotor[20];
    char item[20];

    char fileusers[30];
    char fpromotores[30];
    char fitem[30];

    for( int i=0;envp[i] != NULL;i++){
        sscanf(envp[i],"FUSERS=%s",&user);
        sscanf(envp[i],"FPROMOTERS=%s",&promotor);
        sscanf(envp[i],"FITEMS=%s",&item);
    }
    sprintf(fileusers,"filestxt/%s",user);
    sprintf(fpromotores,"filestxt/%s",promotor);
    sprintf(fitem,"filestxt/%s",item);

    strcpy(backend.fpromotores, fpromotores);
    strcpy(backend.fitens, fitem);
    printf("f: %s\n", fitem);

    //===========================================================================
    char pathfi[30];
    strcpy(pathfi , fileusers);
    char *fi = getenv("FITEMS");  //fi file itens | FITEMS var ambiente do nome do ficheiro
    char *fp = getenv("FPROMOTERS");  //fi file itens | FPROMOTERS var ambiente do nome do ficheiro
    char *fu = getenv("FUSERS");  //fi file itens | FUSERS var ambiente do nome do ficheiro
    int var1;

    int fdEnvia,fdRecebe;

    pthread_t t[20];
    pthread_t menu;
    pthread_t balcao;
    pthread_t timer;


    struct sigaction sac;
    memset(&sac,0, sizeof(sac));
    sac.sa_handler=sair;                        //sinal saida
    sigaction(SIGINT,&sac,NULL);

    open(fi, O_RDONLY);

    if (argc != 1) {
        printf("Argumentos a mais!\n");
        exit(1);
    }

    //=================================== FIFO ===========================================================
    B_F mensagemRecebida;
    strcpy(mensagemRecebida.fitem,fitem);
    strcpy(mensagemRecebida.fileusers,fileusers);
    B_F resposta;


    if (mkfifo(BACKEND_FIFO, 0666) == -1)
    {
        if (errno == EEXIST)
        {
            printf("Servidor em execução ou fifo já existe");
            return 1;
        }
        printf("Erro abrir fifo");
        return 1;
    }
        if(pthread_create(&balcao,NULL,&balcaoVida,&backend)!=0)return -1;
        if(pthread_create(&menu,NULL,&atendeAdm,&backend)!=0)return -1;
        if(pthread_create(&timer,NULL,Timer,NULL)!=0)return -1;
        //printf("T: %d", TIME);
    do{

        fdRecebe = open(BACKEND_FIFO, O_RDONLY);
        if (fdRecebe == -1)
        {
            printf("Erro");
            return 1;
        }

        int ar=read(fdRecebe,&mensagemRecebida,sizeof(mensagemRecebida));

        close(fdRecebe);
        if(!mensagemRecebida.ligado) {
            mensagemRecebida.ligado = true;
            sprintf(clientes_list[count_clientes], FRONTEND_FIFO, mensagemRecebida.cli_pid);
            sprintf(clientes_name[count_clientes], "%s", mensagemRecebida.username);
            //printf("\n->%s, %s\n",clientes_list[count_clientes],clientes_name[count_clientes]);
            strcpy(mensagemRecebida.fitem,fitem);
            strcpy(mensagemRecebida.fileusers,fileusers);
            strcpy(backend.fpromotores,fpromotores);
            //==================

            totalusers = loadUsersFile(pathfi);
            int aux, tam;
            if(totalusers == -1) {
                printf("%s\n", getLastErrorText());
            }

            val = isUserValid(mensagemRecebida.username, mensagemRecebida.password);

            //ENVIA CONFIRMAÇAO
            fdEnvia=open(clientes_list[count_clientes],O_WRONLY);
            if(fdEnvia==-1){
                printf("Erro"); return 1;
            }

            if (val == -1) {
                strcpy(confirma,getLastErrorText());
                write(fdEnvia,confirma,sizeof(confirma));
                printf("%s\n", confirma);
                close(fdEnvia);
            } else if (val == 0) {
                strcpy(confirma,"Password ou Utilizador inválido!\n");
                write(fdEnvia,confirma,sizeof (confirma));
                close(fdEnvia);

            } else {
                strcpy(confirma,"Utilizador válido e password correta\n");

                write(fdEnvia,confirma,sizeof (confirma));
                close(fdEnvia);

                //thread
                printf("tempo : %d\n", TIME);
                if(pthread_create(&t[count_clientes],NULL,&atendeCliente,&mensagemRecebida)!=0)return -1;


                count_clientes++;
            }


            //=======================
        }
    } while (1);






    //====================================================================================================


    printf("Deseja testar que funcionalidade?\n"
           "1)Comandos\n"
           "2)Execução do promotor\n"
           "3)Utilizadores\n"
           "4)Itens\n"
           "->");

    fflush(stdin);
    scanf("%d",&opt);
    switch (opt) {
        case 1:

            printf("Deseja testar que comando? :");

            while (getchar()!='\n');
            scanf("%[^\n]s",comand);

            fflush(stdin);
            sscanf(comand,"%s",aux);
            //usei aux em vez de vendas.comando
            //listar users ativos
            if(strcmp(aux,"users") == 0 ) {
                a=sscanf(comand,"%s %d",aux, &extra);
                if(a==1) printf("Comando válido\n");
            }
                //listar itens a venda
            else if(strcmp(aux,"list") == 0){
                a=sscanf(comand,"%s %d",aux, &extra);
                if(a==1 ) printf("Comando válido\n");
            }
                //banir cliente
            else if(strcmp(aux, "kick") == 0){
                a=sscanf(comand,"%s %s %d",aux, cli.nome, &extra);
                if(a==2) printf("Comando válido\n");
            }
                //listar proms
            else if(strcmp(aux, "prom") == 0){
                a= sscanf(comand, "%s %d", aux, &extra);
                if(a==1) printf("Comando válido\n");
            }
                //atualiza promotores
            else if(strcmp(aux, "reprom") == 0){
                a= sscanf(comand, "%s %d", aux, &extra);
                if(a==1) printf("Comando válido\n");
            }
                //por fazer
            else if(strcmp(aux, "reprom") == 0){
                a= sscanf(comand, "%s %d", aux, &extra);
                if(a==1) printf("Comando válido\n");
            }
                //cancelar promotores
            else if(strcmp(aux, "cancel") == 0){
                a= sscanf(comand, "%s %s %d", aux, nome_exe, &extra);
                if(a==2) printf("Comando válido\n");
            }
                //close
            else if(strcmp(aux,"close") == 0){
                a=sscanf(comand,"%s %d", aux,&extra);
                if(a==1){
                    printf("\nAdministrador [%s] saindo...\n",admin.nome);
                    //avisar backend
                    exit(1);
                }
            } else{
                printf("Comando inválido\n");
            }
            break;
        case 2:
            ;
            break;
        case 3:
            totalusers = loadUsersFile(pathfi);
            int aux, tam;
            if(totalusers == -1) {
                printf("%s\n", getLastErrorText());
            }
            printf("Users: %d\n", totalusers);

            char nameusers[MAX_USERS];

            printf("Qual é o username e password do user que deseja validar?\n");
            scanf("%s %s", name, pswd);
            val = isUserValid(name, pswd);
            if (val == -1) {
                printf("%s\n", getLastErrorText());
            } else if (val == 0) {
                printf("Password ou Utilizador inválido!\n");
            } else {
                printf("Utilizador válido e password correta\n");
            }

            printf("Qual o nome do cliente que deseja obter saldo?\n");
            scanf("%s", name);
            val = getUserBalance(name);
            if (val == -1)
                printf("%s\n", getLastErrorText());
            else {
                printf("O saldo de %s é %d\n", name, val);
            }
            printf("Nome e valor do saldo que pretende atualizar\n");
            scanf("%s %d", name, &saldo);
            val = updateUserBalance(name, saldo);
            if(val == 0)
                printf("%s\n", getLastErrorText());
            else if(val == -1)
                printf("%s\n", getLastErrorText());
            else
                printf("Saldo de %s alterado!\n", name);


            char guardaUsers[MAX_SIZE];
            char *token1;
            // nam
            aux = open(pathfi, O_RDONLY);

            if(aux == -1){
                printf("Erro ao abrir ficheiro!\n");
                exit(1);
            }else
                printf("Sucesso ao abrir ficheiro!\n");

            tam = read(aux, guardaUsers, sizeof(guardaUsers));
            guardaUsers[tam] = '\0';
            close(aux);

            int i = 0;

            token1 = strtok(guardaUsers, " ");//users name
            while(token1 != NULL){
                strcpy(&nameusers[i], token1);
                val=updateUserBalance(&nameusers[i], getUserBalance(&nameusers[i])-1);

                //depois de ler tudo apresenta um erro ns pq mas ta tudo operacional
                if(val == 0)
                    printf("%s\n", getLastErrorText());
                else if(val == -1)
                    printf("%s\n", getLastErrorText());
                else
                    printf("Saldo de %s alterado!\n", &nameusers[i]);
                token1 = strtok(NULL, "\n");
                token1 = strtok(NULL, " ");
                i++;
            }

            //guardar info
            val = saveUsersFile(pathfi);
            if(val == -1)
                printf("%s", getLastErrorText());
            break;
        case 4:
            filedesc= fopen("filestxt/itens.txt","r");
            int auxi = 0;
            int id_aux = 1;
            while(fscanf(filedesc,"%s %s %d %d %d %s %s",itens[auxi].nome_item,itens[auxi].categoria,&itens[auxi].valor_base,&itens[auxi].compra_imediata,&itens[auxi].tempo,itens[auxi].vendedor,itens[auxi].comprador)==7){
                //if(auxi>=MAX_ITEMVENDA)break;

                itens[auxi].id_item = id_aux;
                printf("%d %s %s %d %d %d %s %s\n",itens[auxi].id_item,itens[auxi].nome_item,itens[auxi].categoria,itens[auxi].valor_base,itens[auxi].compra_imediata,itens[auxi].tempo,itens[auxi].vendedor,itens[auxi].comprador);
                ++auxi;
                ++id_aux;
            }
            fclose(filedesc);
            break;

    }
}
