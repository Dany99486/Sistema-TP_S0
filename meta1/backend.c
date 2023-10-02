//
// Created by Daniel Rodrigues on 05/11/2022.
//
#include "structs.h"
#include <stdio.h>
#include <fcntl.h>
#include "users_lib.h"

int pd;
void sair(int sign){
    printf("\nSaindo...\n");
    union sigval valores;
    sigqueue(pd,SIGUSR1,valores);
    exit(1);
}

int main(int argc, char **argv) {
    int opt;
    FILE *filedesc;
    char promotor1[MAX_COMAND];

    int FD[2];
    pipe(FD);

    //--------------------------------------------------------//
    bool confirma_user; //confirmar credenciais user
    //if(confirma_user == false) printf("Erro na verificação de credenciais!\n");
    char comand[MAX_COMAND];

    clientes admin;
    clientes cli;
    itens item;
    vendas venda;
    int extra,a;
    //item.vendas1=extra;

    char aux[MAX_COMAND]="";
    char nome_exe[30];//nome executavel

    char name[USRNAME_MAX], pswd[PSWDCHAR_MAX];
    int val, totalusers;
    int op, saldo;

    char pathfi[] = "filestxt/users.txt";
    char *fi = getenv("FITEMS");  //fi file itens | FITEMS var ambiente do nome do ficheiro
    char *fp = getenv("FPROMOTERS");  //fi file itens | FPROMOTERS var ambiente do nome do ficheiro
    char *fu = getenv("FUSERS");  //fi file itens | FUSERS var ambiente do nome do ficheiro
    int var1;

    open(fi, O_RDONLY);

    if (argc != 1) {
        printf("Argumentos a mais!\n");
        exit(1);
    }


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
            pd=fork();
            if(pd==0){//filho
                close(1);
                dup(FD[1]);
                close(FD[1]);
                close(FD[0]);

                execl("promotores/black_friday","black_friday",NULL);

                printf("Promotor nao lançado\n");
                exit(1);
            }else{
                close(FD[1]);

                struct sigaction sac;
                memset(&sac,0, sizeof(sac));
                sac.sa_handler=sair;
                sigaction(SIGINT,&sac,NULL);

                while(1){

                    read(FD[0],promotor1,sizeof (promotor1));
                    if(strcmp(promotor1,aux)!=0){
                        printf("\n\nPromotor diz:\n-> %s",promotor1);
                        strcpy(aux,promotor1);
                        strcpy(promotor1,"");
                    }
                }
                //mandar sinal para fechar para pid=0

                close(FD[0]);

            }
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

            while(fscanf(filedesc,"%d %s %s %d %d %d %s %s",&item.id_item,item.nome_item,item.categoria,&item.valor_base,&item.compra_imediata,&item.tempo,item.vendedor,item.comprador)==8){
                printf("%d %s %s %d %d %d %s %s\n",item.id_item,item.nome_item,item.categoria,item.valor_base,item.compra_imediata,item.tempo,item.vendedor,item.comprador);
            }

            fclose(filedesc);
            break;
    }
}