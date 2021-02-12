/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   mainServer.c
 * Author: matej
 *
 * Created on Pondelok, 2018, decembra 10, 20:59
 */

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
#include "server.h"

char* databazaMenu(char* buff){
    strcpy(buff,"0 - Vypis aut\n"
            "1 - Vypis majitelov\n"
            "2 - Pridaj auto\n"
            "3 - Najdi auto podla SPZ\n"
            "4 - Vyhladaj majitela\n"
            "5 - Tried databazu\n"
            "6 - Vymaz majitela a jeho auto/(á)\n"
            "7 - Nacitaj zo suboru\n"
            "Napis: \"exit\" pre ukoncenie\n"
            "Zadaj cislo: ");

    return buff;
}
char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void toUpperString(char* str){
    for(char *i = str;i<str+strlen(str);i++){
        *i=(toupper(*i));
    }
}
FARBA strToenum (char *str){
    toUpperString(str);
    char* pomocneP [ENUM_SIZE] ={"ZELENA","MODRA","CERVENA","ZLTA","CIERNA","BIELA"};
     for (int i = 0;i < sizeof(pomocneP) / sizeof(pomocneP[0]);++i){
         if (!strcmp (str, pomocneP[i]))
           return i;   
     }
}
char* enumToStr(FARBA farba){
    char* pomocneP [ENUM_SIZE] ={"ZELENA","MODRA","CERVENA","ZLTA","CIERNA","BIELA"};
    return pomocneP[farba];
}
void initList(LIST* zoznam){
    //zoznam = calloc(LIST_SIZE,sizeof(LIST));
    zoznam->pocetaut = 0;
    zoznam->kapacitaAut = LIST_SIZE;
    zoznam->auta = calloc(LIST_SIZE,sizeof(CAR));
    
    zoznam->pocetmajitelov = 0;
    zoznam->kapacitaM = LIST_SIZE;
    zoznam->majitel = calloc(LIST_SIZE,sizeof(MAJITEL));
    
}
void freeList(LIST* zoznam){
    free(zoznam->auta);
    zoznam->auta=NULL;
    zoznam->kapacitaAut = 0;
    zoznam->pocetaut = 0;
    
    free(zoznam->majitel);
    zoznam->majitel=NULL;
    zoznam->kapacitaM = 0;
    zoznam->pocetmajitelov = 0;
    free(zoznam);
    zoznam = NULL;
}
_Bool porovnajMajitelov(MAJITEL* majitel1,MAJITEL* majitel2){
    return strcmp(majitel1->meno,majitel2->meno)==0 &&
       strcmp(majitel1->priezvisko,majitel2->priezvisko)==0 && 
       strcmp(majitel1->rodnecislo,majitel2->rodnecislo)==0;
}

_Bool porovnajAuta(CAR* car1, CAR* car2){
    return strcmp(car1->znacka,car2->znacka)==0 &&
       strcmp(car1->model,car2->model)==0 && 
       strcmp(car1->spz,car2->spz)==0 && 
       car1->farba ==car2->farba && 
       porovnajMajitelov(car1->majitel,car2->majitel);
}
_Bool porovnajSpz(LIST* zoznam, char* buffer) {
    if (strlen(zoznam->auta[0].spz) != strlen(buffer))
        return false;
    else {
        for (int i = 0; i < zoznam->pocetaut; i++) {
            if (strcmp(zoznam->auta[i].spz, buffer) == 0)
                return false;
        }
    }
    return true;
}
_Bool realokujZoznam(LIST* zoznam){
    CAR* realocCar = realloc(zoznam->auta,2*sizeof(CAR)*zoznam->kapacitaAut);
    MAJITEL* realocM = realloc(zoznam->majitel,2*sizeof(MAJITEL)*zoznam->kapacitaM);
    if(realocCar == NULL || realocM == NULL){
        perror("Chyba pri realokovani\n");
        return false;
    }
    zoznam->auta = realocCar;
    zoznam->kapacitaAut = 2*zoznam->kapacitaAut;
    
    zoznam->majitel = realocM;
    zoznam->kapacitaM = 2*zoznam->kapacitaM;
    
}
MAJITEL* skusPridatMajitela(LIST* zoznam,MAJITEL* majitel){
    if(zoznam->pocetmajitelov >= zoznam->kapacitaM){
        realokujZoznam(zoznam);
    }

    for(int i=0;i<zoznam->pocetmajitelov;i++){
        if(porovnajMajitelov(&(zoznam->majitel[i]),majitel)){
        return &(zoznam->majitel[i]);                           //ak su rovnaky majitelia vrati adresu na majitela v Zozname
        }
    }
    zoznam->majitel[zoznam->pocetmajitelov] = *majitel;
    zoznam->pocetmajitelov++;
    return &(zoznam->majitel[zoznam->pocetmajitelov-1]);          //ak niesu rovnaky ,tak prida majitela do Zoznamu a vrati &
}

_Bool skusPridatAuto(LIST* zoznam,CAR* car){
    //printf("Pocet AUT: %d\n",zoznam->pocetaut);
    for(int i=0;i<zoznam->pocetaut;i++){
        if(porovnajAuta(&(zoznam->auta[i]),car)){
           // printf("Pocet AUT: %d\n",zoznam->pocetaut);
            return true;      
        }
    }
        if(zoznam->pocetaut >= zoznam->kapacitaAut ){
            realokujZoznam(zoznam);
        }
    MAJITEL* majitel = skusPridatMajitela(zoznam,car->majitel); //ked dereferencujem car struct tak majitel je zadefinovany ako pointer
    if(majitel != NULL){
        zoznam->auta[zoznam->pocetaut] = *car;
        zoznam->pocetaut++;
        zoznam->auta[zoznam->pocetaut-1].majitel = majitel;
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
char* printCar(const CAR* car, char* pole){
    snprintf(pole,BUFF_SIZE,"Znacka: %s Model: %s Farba: %s SPZ: %s \n",car->znacka,car->model,enumToStr(car->farba),car->spz);
    return pole;
}
char* printMajitel(const MAJITEL* majitel, char* pole){
    snprintf(pole,BUFF_SIZE,"Meno: %s Priezvisko: %s RodneCislo: %s \n",majitel->meno,majitel->priezvisko,majitel->rodnecislo);
    return pole;
}
char* hladajAutoVypis(CAR* car, char* buffer){
    snprintf(buffer,BUFF_SIZE,"Znacka: %s Model: %s Farba: %s SPZ: %s Meno: %s Priezvisko: %s\n",
            car->znacka,car->model,enumToStr(car->farba),car->spz,car->majitel->meno,car->majitel->priezvisko);
    return buffer;
}

void printZoznamCars(LIST* zoznam, int socket) {

    char* buffer = calloc(BUFF_SIZE,sizeof(char));
    char* pom = calloc(BUFF_SIZE,sizeof(char));
    for (int i = 0; i < zoznam->pocetaut; i++) {
        if (zoznam->auta[i].znacka[0] != '\0')
            printCar(&(zoznam->auta[i]),pom);
            strncat(buffer,pom,strlen(pom)+1);
    }
    write(socket,buffer, BUFF_SIZE);
    free(buffer);
    buffer = NULL;
    free(pom);
    pom = NULL;
}

void printZoznamMajitelia(LIST* zoznam,int socket){
    char* buffer = calloc(BUFF_SIZE,sizeof(char));
    char* pom = calloc(BUFF_SIZE,sizeof(char));
    for(int i=0 ; i<zoznam->pocetmajitelov ; i++){
       if(zoznam->auta[i].znacka[0] != '\0')
           printMajitel(&(zoznam->majitel[i]),pom);
           strncat(buffer,pom,strlen(pom)+1);         
 }
    write(socket,buffer,BUFF_SIZE);
    free(buffer);
    buffer = NULL;
    free(pom);
    pom = NULL;
}

//------------------------------------------------------------------------------
_Bool importTxt(LIST* zoznam, const char* fileName){
    FILE *src = fopen(fileName, "r");
    char farba[STRING_SIZE];
    char buffer[BUFF_SIZE];
    if (src == NULL)
        return false;
    MAJITEL* majitel = (MAJITEL*) malloc(sizeof(MAJITEL));
    CAR* car = (CAR*) malloc(sizeof(CAR));
 while(fgets(buffer,BUFF_SIZE,src) != NULL && buffer[0] != '\n'){
    char* tmp1 = strchr(buffer,'_');
    strncpy(car->znacka,buffer,tmp1-buffer);
    car->znacka[tmp1-buffer] = '\0';
    //printf("Znacka: %s\n",car.znacka);
    
    char* tmp2 = strchr(tmp1+1,'_');
    strncpy(car->model,tmp1+1,tmp2-tmp1-1);
    car->model[tmp2-tmp1-1] = '\0';
   // printf("model: %s\n",car.model);
    

    char* tmp3 = strchr(tmp2+1,'_');
    strncpy(farba,tmp2+1,tmp3-tmp2-1);
    farba[tmp3-tmp2-1] = '\0';
    farba[10] = ' ';
    car->farba = strToenum(farba);
    //printf("%s\n",enumToStr(car.farba));
    
    char* tmp4 = strchr(tmp3+1,'_');
    strncpy(car->spz,tmp3+1,tmp4-tmp3-1);
    car->spz[tmp4-tmp3-1] = '\0';
    //printf("SPZ: %s\n",car.spz);
    
    char* tmp5 = strchr(tmp4+1,'_');
    strncpy(majitel->meno,tmp4+1,tmp5-tmp4-1);
    majitel->meno[tmp5-tmp4-1] = '\0';
    //printf("Meno: %s\n",majitel->meno);
    
    char* tmp6 = strchr(tmp5+1,'_');
    strncpy(majitel->priezvisko,tmp5+1,tmp6-tmp5-1);
    majitel->priezvisko[tmp6-tmp5-1] = '\0';
    //printf("Priezvysko: %s\n",majitel->priezvisko);
    
    char* tmp7 = strchr(tmp6+1,';');
    strncpy(majitel->rodnecislo,tmp6+1,tmp7-tmp6-1);
    majitel->rodnecislo[tmp7-tmp6-1]= '\0';
    //printf("RodneCislo: %s\n",majitel->rodnecislo);
      car->majitel = majitel;
      skusPridatAuto(zoznam,car);
    // printf("Adresa car: %p\n",car);
      //printCar(&car);
      //printf("Adresa majitel: %p\n",&majitel);
 }
    free(majitel);
    majitel = NULL;
    free(car);
    car = NULL;
    fclose(src);
    return true;
}
_Bool exportTxt(LIST* zoznam,const char* fileName) {
    FILE *src = fopen(fileName, "w+");
    if (src == NULL)
        return false;
    int kontrola;
    for (int i = 0; i < zoznam->pocetaut; i++) {
        if (zoznam->auta[i].farba == '0') {
            kontrola = fprintf(src, "%s%s%s%s%s%s", zoznam->auta[i].znacka, zoznam->auta[i].model, zoznam->auta[i].spz,
                zoznam->majitel[i].meno, zoznam->majitel[i].priezvisko, zoznam->majitel[i].rodnecislo);
        } else {
            kontrola = fprintf(src, "%s_%s_%s_%s_%s_%s_%s;\n", zoznam->auta[i].znacka, zoznam->auta[i].model, enumToStr(zoznam->auta[i].farba), zoznam->auta[i].spz,
                    zoznam->majitel[i].meno, zoznam->majitel[i].priezvisko, zoznam->majitel[i].rodnecislo);
        }
        }
    if (kontrola < 0) {
        printf("Nepodarilo sa ulozit do databazy\n");
        fclose(src);
        return false;
    }else
        fclose(src);
        return true;
}

void pridajDoDatabazy(LIST* zoznam, int socket) {
    CAR autoAdd;
    MAJITEL majitelAdd;
    _Bool correct = false;
    char* buffer = calloc(BUFF_SIZE, sizeof (char));
    write(socket, "Pridaj zaznam do databazy\nZnaèka: ", BUFF_SIZE);
        read(socket, buffer, BUFF_SIZE);
        strncpy(autoAdd.znacka, buffer, strlen(buffer)+1);
        toUpperString(autoAdd.znacka);
        bzero(buffer,BUFF_SIZE);
        
        write(socket,"Model: ",BUFF_SIZE);
        read(socket, buffer, BUFF_SIZE);
        strncpy(autoAdd.model, buffer, strlen(buffer)+1);
        bzero(buffer,BUFF_SIZE);
        
        write(socket, "Spz: ", BUFF_SIZE);
        while (correct != true) {
            
            
            read(socket, buffer, BUFF_SIZE);
            toUpperString(buffer);
            if (porovnajSpz(zoznam, buffer) == true) {
            strncpy(autoAdd.spz, buffer, strlen(buffer)+1);
            correct = true;
            } else {
            write(socket, "Zadana SPZ uz existuje, alebo ste zadali zlu dlzku spz(7)\nSpz:", BUFF_SIZE);
            }
            bzero(buffer,BUFF_SIZE);
        }
        correct = false;
        write(socket,"Moznosti farieb: zelena, modra, cervena, zlta, cierna, biela\nFarba: ",BUFF_SIZE);
        FARBA porovnaj;
        while(correct != true){
        //bzero(buffer,BUFF_SIZE);
        read(socket, buffer, BUFF_SIZE);
        toUpperString(buffer);
        for(int i= 0; i < ENUM_SIZE; i++){
            porovnaj = i;
            if(porovnaj == strToenum(buffer)){
                autoAdd.farba = strToenum(buffer);
                bzero(buffer,BUFF_SIZE);
                correct = true;
            }
            }
        if(correct == false){
                write(socket,"Takato moznost neexistuje\nMoznosti farieb: zelena, modra, cervena, zlta, cierna, biela\nFarba: ",BUFF_SIZE);
           }
            bzero(buffer,BUFF_SIZE);
        }
        
        write(socket,"Meno majitela: ",BUFF_SIZE);
        read(socket, buffer, BUFF_SIZE);
        toUpperString(buffer);
        strncpy(majitelAdd.meno,buffer,strlen(buffer)+1);
        bzero(buffer,BUFF_SIZE);
        
        write(socket,"Priezvysko majitela: ",BUFF_SIZE);
        read(socket, buffer, BUFF_SIZE);
        toUpperString(buffer);
        strncpy(majitelAdd.priezvisko,buffer,strlen(buffer)+1);
        
        correct = false;
        write(socket,"Rodne cislo majitela:",BUFF_SIZE);
        while(correct != true){
        read(socket, buffer, BUFF_SIZE);
        if((strlen(buffer)+1) == RODC_SIZE){
            strncpy(majitelAdd.rodnecislo,buffer,strlen(buffer)+1);
            correct = true;
        }else{
            write(socket,"Zla dlzka rodneho Cisla\nRodne cislo majitela:",BUFF_SIZE);
        }
        bzero(buffer,BUFF_SIZE);
        }
        autoAdd.majitel = &majitelAdd;
        skusPridatAuto(zoznam,&autoAdd);
        if(exportTxt(zoznam,"database.txt") == true)
             write(socket,"Zaznam uspesne pridany a zapisany do suboru\n",BUFF_SIZE);
        else
             write(socket,"Zaznam sa nepodarilo pridat",BUFF_SIZE);
        
            free(buffer);
            buffer = NULL;
}
void vymazMajitela(LIST* zoznam,int socket){
    char* buffer = calloc(BUFF_SIZE,sizeof(char));
    _Bool correct = false;
    MAJITEL pomocna;
    int pocitadlo = 0;
    write(socket,"Zadaj rodne cislo majitela: ",BUFF_SIZE);
    while (correct != true) {
    read(socket,buffer,BUFF_SIZE);
    if (strlen(zoznam->majitel[0].rodnecislo) == strlen(buffer)){
        strncpy(pomocna.rodnecislo,buffer,strlen(buffer)+1);
        correct = true;
    }else{
        write(socket,"\nZla dlzka rodneho cisla",BUFF_SIZE);
    }
    bzero(buffer,BUFF_SIZE);
    }
    for(int i=0; i< zoznam->pocetaut; i++){
        if(strcmp(zoznam->auta[i].majitel->rodnecislo,pomocna.rodnecislo) == 0){
            CAR pom = zoznam->auta[i];
            zoznam->auta[i] = zoznam->auta[i + 1];
            zoznam->auta[i + 1] = zoznam->auta[i];
            pocitadlo++;
          }
    }
    zoznam->pocetaut= zoznam->pocetaut-pocitadlo;
    for(int i=0; i< zoznam->pocetmajitelov; i++){
        if(strcmp(zoznam->majitel[i].rodnecislo,pomocna.rodnecislo) == 0){
            MAJITEL pom = zoznam->majitel[i];
            zoznam->majitel[i] = zoznam->majitel[i + 1];
            zoznam->majitel[i + 1] = zoznam->majitel[i];
        }
    }
        zoznam->pocetmajitelov--;
    write(socket,"Majitel aj s jeho autami uspesne vymazany\n",BUFF_SIZE);
    free(buffer);
    buffer = NULL;
    exportTxt(zoznam,"database.txt");
}
void hladajAuto(LIST* zoznam,int socket) {
    _Bool correct = false;
    _Bool found = false;
    char* buffer = calloc(BUFF_SIZE, sizeof (char));
    CAR hladajCar;
    write(socket, "Zadaj SPZ auta: ", BUFF_SIZE);
    while (correct != true) {
        read(socket, buffer, BUFF_SIZE);
        toUpperString(buffer);
        if (strlen(zoznam->auta[0].spz) == strlen(buffer)){
            strncpy(hladajCar.spz,buffer,SPZ_SIZE);
            correct = true;
        }else{
        write(socket, "Zadali ste zlu dlzku spz(7)\nZadaj SPZ auta:", BUFF_SIZE);
        }
        bzero(buffer,BUFF_SIZE);
    }
    for(int i=0; i<zoznam->pocetaut; i++){
        if(strcmp(zoznam->auta[i].spz,hladajCar.spz) == 0){
            hladajCar = zoznam->auta[i];
            write(socket,hladajAutoVypis(&hladajCar,buffer),BUFF_SIZE);
            found = true;
            break;
        }
    }
    if(found = false)
        write(socket, "Auto sa nepodarilo najst\n", BUFF_SIZE);
    free(buffer);
    buffer= NULL;
}
void hladajMajitela(LIST* zoznam,int socket){
    char* buffer = calloc(BUFF_SIZE,sizeof(char));      //bude lepsie ked vsetky vysledky nastackujem do buffera a potom naraz poslem
    _Bool correct = false;
    char pomocne[BUFF_SIZE];
    MAJITEL hladajM;
    CAR addMatch;
    write(socket,"Rodne cislo majitela:",BUFF_SIZE);
    while(correct != true){
        read(socket, buffer, BUFF_SIZE);
        if(strlen(buffer) == RODC_SIZE-1){
            strncpy(hladajM.rodnecislo,buffer,RODC_SIZE);
            correct = true;
        }else{
            write(socket,"Zla dlzka rodneho Cisla",BUFF_SIZE);
        }
         bzero(buffer,BUFF_SIZE);
        }
    for(int i=0; i < zoznam->pocetaut; i++){
        if(strcmp( zoznam->auta[i].majitel->rodnecislo , hladajM.rodnecislo ) == 0){
            addMatch =zoznam->auta[i];
            hladajAutoVypis(&addMatch,pomocne);
            strncat(buffer,pomocne,sizeof(pomocne));
        } 
    }
    write(socket,buffer,BUFF_SIZE);
    free(buffer);
    buffer = NULL;
}
void triedDatab(LIST* zoznam,int socket) {
    char* buffer = calloc(BUFF_SIZE,sizeof(char));
    write(socket,"\nTriedenie pod¾a:\n1 - Znaèka\n2 - Meno majite¾a\n->Zadaj atribút triedenia:  ",BUFF_SIZE);
    read(socket,buffer,BUFF_SIZE);
    int volba = atoi(buffer);
    CAR pomocna;
    switch (volba) {
        case 1:
            for (int i = 0; i < zoznam->pocetaut; ++i) {
                for (int j = i + 1; j < zoznam->pocetaut; ++j) {
                    if (strcmp(zoznam->auta[i].znacka, zoznam->auta[j].znacka) > 0) {
                        pomocna = zoznam->auta[i];
                        zoznam->auta[i] = zoznam->auta[j];
                        zoznam->auta[j]= pomocna;
                    }
                }
            }
            write(socket,"Utriedene podla Znacky\n",BUFF_SIZE);
            break;
        case 2:
            for (int i = 0; i < zoznam->pocetaut; ++i) {
                for (int j = i + 1; j < zoznam->pocetaut; ++j) {
                    if (strcmp(zoznam->auta[i].majitel->meno, zoznam->auta[j].majitel->meno) > 0) {
                        pomocna = zoznam->auta[i];
                        zoznam->auta[i] = zoznam->auta[j];
                        zoznam->auta[j] = pomocna;
                    }
                }
            }
            write(socket,"Utriedene podla majitela\n",BUFF_SIZE);
            break;
        default :
            write(socket,"Neznamy vstup",BUFF_SIZE);
    }
    free(buffer);
    buffer = NULL;
}
void databazaRun(LIST* zoznam,int socket, int volba){ 
    switch (volba) {
        case 0:
            printZoznamCars(zoznam, socket);
            break;
        case 1:
            printZoznamMajitelia(zoznam, socket);
            break;
        case 2:
            pridajDoDatabazy(zoznam,socket);
            break;
        case 3:
            hladajAuto(zoznam,socket);
            break;
        case 4:
            hladajMajitela(zoznam,socket);
            break;
        case 5:
            triedDatab(zoznam,socket);
            break;
        case 6:
            vymazMajitela(zoznam,socket);
            break;
        case 7:
            if(importTxt(zoznam,"database.txt")){
                write(socket,"Upesne nacitane",BUFF_SIZE);
            }
            else{
                write(socket,"Nastala chyba pri nacitani",BUFF_SIZE);
            }
            break;
        case 8:
            //write(socket,"Chybny vstup",BUFF_SIZE);
            return;
        default:
                write(socket,"ZLY VSTUP",BUFF_SIZE);
            
            break;
        }
}
//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void vypis(char* buffer) {
    strcpy(buffer, "\nType \"login\" if you want to log in:\nType \"signin\" to create account:\n");
}

void signinVypis(char* buffer) {
    strcpy(buffer, "Username:\nPassword:");
}

int login(const char* filename, char* input, int socket) {
    char buffer[BUFF_SIZE];
    char msge[BUFF_SIZE];
    char pomocne[30];
    char* msg;
    bzero(buffer, sizeof (buffer));
    FILE *src = fopen(filename, "r");
    while (fgets(buffer, BUFF_SIZE, src) != NULL && buffer[0] != '\n') {
        char* tmp1 = strchr(buffer, '\n');
        strncpy(pomocne, buffer, tmp1 - buffer);
        printf("%s\n", pomocne);
        if (strcmp(pomocne, input) == 0) {
            msg = concat("Login succesfull\n", databazaMenu(msge));     
            write(socket,msg,BUFF_SIZE);
            free(msg);
            msg = NULL;
            bzero(msge, sizeof (msge));
            return 1;
        }
        bzero(pomocne, sizeof (pomocne));
    }
    //write(socket,"Failed to login\0",BUFF_SIZE);
    fclose(src);
    return 0;
}
    void signin(const char* filename, char* input) {
        char str[60];
        FILE *src = fopen(filename, "a");
        fputs(input, src);
        fputc('\n', src);
        fclose(src);
    }
    
    int koniec=0;
    void spracujSignal(int signal){
    if(signal == SIGINT){
        //exit(0);
        koniec = 1;
    }
    }

    int main(int argc, char** argv) {
        int sockfd, ret;
        LIST* zoznam;
        zoznam = calloc(LIST_SIZE,sizeof(LIST));
        initList(zoznam);
        signal(SIGINT, spracujSignal);
        int volba;
        struct sockaddr_in serverAddr;
        int newSocket;
        struct sockaddr_in newAddr;
        socklen_t clientLenght;
        char buffer[BUFF_SIZE];
        pid_t childpid;
        _Bool isLogged = false;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            printf("Error in connection\n");
            return 1;
        }
        printf("Server Socket is created\n");

        memset(&serverAddr, '\0', sizeof (serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        ret = bind(sockfd, (struct sockaddr*) &serverAddr, sizeof (serverAddr));
        if (ret < 0) {
            printf("Error in binding\n");
            return 2;
        }
        printf("Bind to port %d\n", PORT);

        if (listen(sockfd, 10) == 0)
            printf("Listening...\n");
        else
            printf("Error in binding\n");
        while (!koniec) {
            newSocket = accept(sockfd, (struct sockaddr*) &newAddr, &clientLenght); //new socket prijme connection od klienta
            if (newSocket < 0)
                return 3;
            printf("Connection accepted from %s: %hu\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
            if ((childpid = fork()) > 0) {
                close(newSocket);
            }
            else if (childpid == 0) {
                //close(sockfd);
                printf("PID som dnu\n");
                while (1) {
                    recv(newSocket, buffer, sizeof (buffer), 0);

                    if (strcmp(buffer, "exit") == 0) {
                        printf("Client %s: %hu disconnected.\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                        close(newSocket);
                        exit(0);
                    }
                    
                    if (strcmp(buffer, "signin") == 0) {
                        if (recv(newSocket, buffer, sizeof (buffer), 0)) {
                            signin("loginsfile.txt", buffer);
                        }
                    }
                    
                    if (strcmp(buffer, "login") == 0) {
                        write(newSocket, "login_ack", 10);
                        printf("|%s|\n", buffer);
                        if (recv(newSocket, buffer, sizeof (buffer), 0)) {
                            isLogged = login("loginsfile.txt", buffer, newSocket);
                            bzero(buffer, sizeof (buffer)); 
                        }
                    }
                    
                    if (isLogged) {
                        break;
                    }
                    else {
                        bzero(buffer, sizeof (buffer));
                        vypis(buffer);
                        write(newSocket, buffer, strlen(buffer) + 1);
                        bzero(buffer, sizeof (buffer));
                    }                 
                }
                
                //databazaMenu(buffer);
                //write(newSocket, buffer, strlen(buffer) + 1);
                
                
                while (1) {
                    recv(newSocket, buffer, sizeof (buffer), 0);
                    
                    if (buffer[0] >= '0' && buffer[0] <= '8') {
                        volba = atoi(buffer);
                        databazaRun(zoznam, newSocket, volba);
                    }
                    
                    else if (strcmp(buffer, "exit") == 0) {
                        printf("Client %s: %hu disconnected.\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                        close(newSocket);
                        exit(0);
                    }
                    
                    else if (strcmp(buffer, "menu") == 0) {
                        databazaMenu(buffer);
                        write(newSocket,databazaMenu, strlen(buffer) + 1); 
                        bzero(buffer, sizeof (buffer));
                    }

                    else {
                        printf("Client:%s \n", buffer);
                        //strcpy(buffer,"\nUnknown command.\n");
                        write(newSocket, "Neznamy vstup", strlen(buffer) + 1); 
                        bzero(buffer, sizeof (buffer));
                    }
                }
                close(newSocket);
                exit(0);
            }
        }
        freeList(zoznam);
        close(sockfd);
        //close(newSocket)

        return (EXIT_SUCCESS);
    }