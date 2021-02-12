/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mainServer.h
 * Author: matej
 *
 * Created on Utorok, 2019, januára 15, 14:21
 */

#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 1234
#define BUFF_SIZE 1024
#define LIST_SIZE 5
#define STRING_SIZE 20
#define SPZ_SIZE 8 // spz ma 7 znakov
#define ENUM_SIZE 6
#define RODC_SIZE 11 // R.C 10 cisel

typedef struct majitel{
    char meno[STRING_SIZE];
    char priezvisko[STRING_SIZE];
    char rodnecislo[RODC_SIZE];
}MAJITEL;

typedef enum {ZELENA, MODRA, CERVENA, ZLTA, CIERNA, BIELA} FARBA;
typedef struct car{
    char znacka [STRING_SIZE];
    char model[STRING_SIZE];
    char spz [SPZ_SIZE];
    FARBA farba;
    MAJITEL* majitel;
}CAR;

typedef struct list{
    MAJITEL* majitel;
    CAR* auta;
    int kapacitaAut;
    int kapacitaM;
    int pocetaut;
    int pocetmajitelov;
}LIST;
char* databazaMenu(char* buff);
char* concat(const char *s1, const char *s2);
void toUpperString(char* str);
FARBA strToenum (char *str);
char* enumToStr(FARBA farba);
void initList(LIST* zoznam);
void freeList(LIST* zoznam);
_Bool porovnajMajitelov(MAJITEL* majitel1,MAJITEL* majitel2);
_Bool porovnajAuta(CAR* car1, CAR* car2);
_Bool porovnajSpz(LIST* zoznam, char* buffer);
_Bool realokujZoznam(LIST* zoznam);
MAJITEL* skusPridatMajitela(LIST* zoznam,MAJITEL* majitel);
_Bool skusPridatAuto(LIST* zoznam,CAR* car);
char* printCar(const CAR* car, char* pole);
char* printMajitel(const MAJITEL* majitel, char* pole);
char* hladajAutoVypis(CAR* car, char* buffer);
void printZoznamCars(LIST* zoznam, int socket);
void printZoznamMajitelia(LIST* zoznam,int socket);
_Bool importTxt(LIST* zoznam, const char* fileName);
_Bool exportTxt(LIST* zoznam,const char* fileName);
void pridajDoDatabazy(LIST* zoznam, int socket);
void vymazMajitela(LIST* zoznam,int socket);
void hladajAuto(LIST* zoznam,int socket);
void hladajMajitela(LIST* zoznam,int socket);
void triedDatab(LIST* zoznam,int socket);
void databazaRun(LIST* zoznam,int socket, int volba);
void vypis(char* buffer);
void signinVypis(char* buffer);
int login(const char* filename, char* input, int socket);
void signin(const char* filename, char* input);
void spracujSignal(int signal);



#endif /* MAINSERVER_H */

