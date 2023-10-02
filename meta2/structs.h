//
// Created by Daniel Rodrigues on 02/11/2022.
//


#ifndef TP_STRUCTS_H
#define TP_STRUCTS_H


/////includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>

///constantes
#define MAX_USERS 20            //numero maximo de users ativos
#define MAX_PROGPROMOTORES 10   //numero maximo de promotores ativos
#define MAX_ITEMVENDA 30        //numero max de itens ativos em venda
#define USRNAME_MAX 50      //numero max de caracteres maximos de username
#define PSWDCHAR_MAX 100        //numero max de caracteres
#define MAX_CHAR 30             //caracters max de itens e categorias
#define MAX_COMAND  50            //caracteres max de comandos
#define MAX_SIZE 200
/////structs
typedef struct item itens;
typedef struct cliente clientes;
typedef struct venda vendas;
typedef struct B_F B_F;


struct venda{
    char username_vendedor[USRNAME_MAX];
    char username_maiorlance[USRNAME_MAX];
};

struct item
{
    int id_item;
    char nome_item[MAX_CHAR];
    char categoria[MAX_CHAR];
    int valor_base;
    int compra_imediata;
    int tempo;
    char vendedor[USRNAME_MAX];
    char comprador[USRNAME_MAX];
};

struct cliente
{
    char nome[USRNAME_MAX]; //username
    char passwd[PSWDCHAR_MAX];
    bool estado_on;
    int saldo;
};

struct B_F{
    int temp;
    int licitacao,deposito,prec_MAX;
    char comando[MAX_COMAND];
    itens item;
    pid_t cli_pid;
    bool ligado;
    char username[USRNAME_MAX];
    char password[PSWDCHAR_MAX];
    char resposta[100];

    char fitem[30];
    char fileusers[30];
};

typedef struct{
    char name[20];
    int pid;

}prom;

typedef struct{
    prom promotor[MAX_PROGPROMOTORES];
    int heartbeat;
    char fpromotores[30];
    char fitens[30];

}backend;

#endif //TP_STRUCTS_H
