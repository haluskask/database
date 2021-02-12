/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: matej
 *
 * Created on Utorok, 2019, januára 8, 12:50
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#define PORT 1234
#define BUFF_SIZE 1024
 char username[20];
void signin(char* buffer){
    bzero(buffer,sizeof(buffer));
    char pomocne[20];
    printf("Username: ");
    scanf("%s",&pomocne);
    strncpy(username,pomocne,strlen(pomocne)+1); // pre username pomenovanie
    char* pom = strchr(pomocne,'\0');
    *pom = '_';
    strncpy(buffer,pomocne,strlen(pomocne)+1);
    bzero(pomocne,sizeof(pomocne));
    printf("Password: ");
    scanf("%s",&pomocne);
    char* pom1 = strchr(buffer,'_');
    strncpy(pom1+1,pomocne,strlen(pomocne)+1);
    char* pom2 = strchr(buffer,'\0');
        *pom2 = ';';
        *(pom2+1) = '\0';
   // printf("%s",buffer);
}
int main(int argc, char** argv) {
    int clientSocket,ret;
    char client[10]={"Client"};
    //strcpy(username,"Client");
    struct sockaddr_in serverAddr; 
    char buffer[BUFF_SIZE];
    struct hostent* server;
    clientSocket = socket(AF_INET,SOCK_STREAM,0);
    if(clientSocket < 0){
        printf("Error in connection\n");
        return 1;
    }
    printf("Client Socket is created\n");
    
    server = gethostbyname(argv[1]);
    if (server == NULL){
		printf("Error, no such host\n");
		return 2;
	}
    memset(&serverAddr, '\0',sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    bcopy((char*)server->h_addr_list[0],(char*)&serverAddr.sin_addr.s_addr,server->h_length);
    
    ret = connect(clientSocket, (struct sockaddr*)&serverAddr,sizeof(serverAddr));
    if(ret < 0){
        printf("Error in connecting\n");
        return 2;
    }
        printf("Connected to server\n");
        
    while (1) {
        printf("%s:",client);
        scanf("%s", &buffer);
        send(clientSocket, buffer, strlen(buffer), 0);

        if (strcmp(buffer, "exit") == 0) {
            close(clientSocket);
            printf("Disconnected from server\n");
            return 3;
        }
        if (strcmp(buffer, "signin") == 0) {
            signin(buffer);
            send(clientSocket, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof (buffer));
        }
        if (strcmp(buffer, "login") == 0) {
            signin(buffer);
            send(clientSocket, buffer, strlen(buffer), 0);
            bzero(buffer, sizeof (buffer));
        }
        if (recv(clientSocket, buffer, BUFF_SIZE, 0) < 0) {
            printf("Error in receiving data\n");
        }else{
            if (strncmp(buffer, "Login succesfull",16) == 0 ) {
            strcpy(client,username);
        }
            printf("Server: %s \n", buffer);
        //printf("%d\n",strlen(buffer));
            bzero(buffer, sizeof (buffer));
        }
    }
    return (EXIT_SUCCESS);
}

