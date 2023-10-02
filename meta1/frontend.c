#include "structs.h"
#include <stdio.h>




int main(int argc, char **argv) {
    bool confirma_user=true; //confirmar credenciais user
    char comand[100];

    clientes cli;
    B_F struc;
    itens item;
    int extra;



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
    strcpy(cli.nome,argv[1]);
    strcpy(cli.passwd,argv[2]);


    //confirma_user=confirm_pass(argv[1]);

    //enviar credenciais para backend fazer verificacão e confirmar

    //caso confirme


    while (confirma_user){
        int a;
        printf("\nDeseja testar que funcionalidade? :");

        setbuf(stdin,NULL);
        scanf("%[^\n]s",comand);
        fflush(stdin);

        
        sscanf(comand,"%s",struc.comando);
        if(strcmp(struc.comando,"sell") == 0 ) {
            a=sscanf(comand,"%s %s %s %d %d %d %d",struc.comando,struc.item.nome_item,struc.item.categoria,&struc.item.valor_base,&struc.item.compra_imediata,&struc.temp,&extra);
            if(a==6) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"list") == 0){
            a=sscanf(comand,"%s %d ",struc.comando,&extra);
            if(a==1) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"lisel") == 0) {
            a=sscanf(comand,"%s %s %d",struc.comando,struc.item.vendedor,&extra);
            if(a==2) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"lival") == 0) {
            a=sscanf(comand,"%s  %d %d",struc.comando,&struc.prec_MAX,&extra);
            if(a==2) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"litime") == 0){
            a=sscanf(comand,"%s %d %d",struc.comando,&struc.temp,&extra);
            if(a==2) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"time") == 0) {
            a=sscanf(comand,"%s %d",struc.comando,&extra);
            if(a==1) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"buy") == 0) {
            a=sscanf(comand,"%s %d %d %d",struc.comando,&struc.item.id_item,&struc.licitacao,&extra);
            if(a==3) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"cash") == 0) {
            a=sscanf(comand,"%s %d",struc.comando,&extra);
            if(a==1) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"add") == 0) {
            a=sscanf(comand,"%s %d %d",struc.comando,&struc.deposito,&extra);
            if(a==2) printf("Comando válido\n");
            else printf("Parametros errados\n");
        }
        else if(strcmp(struc.comando,"exit") == 0){
            a=sscanf(comand,"%s %d",struc.comando,&extra);
            if(a==1){
                printf("\nCliente [%s] saindo...\n",cli.nome);
                //avisar backend
                exit(1);
            }
        } else{
            printf("Comando inválido\n");
        }
    }
    if(confirma_user == false) printf("Erro na verificação de credenciais!\n");


    return 0;
}
