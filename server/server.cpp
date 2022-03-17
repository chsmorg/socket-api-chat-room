
//
//  main.c
//  server v1
//
//  Created by chase morgan on 3/15/22.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "winsock2.h"

#define SERVER_PORT   13388
#define MAX_PENDING   5
#define MAX_LINE      256



//login function 
int login(char* username, char* password) {  
    char usernames[MAX_LINE][32];   //2d array for usernames and passwords allows for 256 users
    char passwords[MAX_LINE][8];  
    int len;

    FILE* fp = fopen("users.txt", "r");  //opens users file
    if (!fp) {
        printf("Error connecting to users list.\n");  
            return 0;
    }
    for (len=0; !feof(fp); len++) {
        fscanf(fp, "%s %s\n", usernames[len], passwords[len]);     //gets each entry from file and put them into arrays
    }
    fclose(fp);

    for (int i = 0; i < len; i++) {         //loops through the arrays tokening out the useless characters and checking if the client inputs match an entry in the file
        if (strcmp(username, strtok(usernames[i],"(),")) == 0 && strcmp(password, strtok(passwords[i], "( ) , ")) == 0) { 
            return 1;      
        }
    }
    return 0; 
}
//new user function
int newuser(char* username, char* password) {
    FILE* fp = fopen("users.txt", "a"); //opens uesrs file
    if (!fp) {
        printf("Error connecting to users list.\n"); //file error
        return -1;
    }
   
    if (login(username, password) == 1) {  //checks if client username and password are already in the file returns 0 if it does
        fclose(fp);
        return 0;
    }
    else {

            fprintf(fp, "\n(%s, %s)", username, password); //writes new entry into file
            fclose(fp);
            return 1;
    }
    fclose(fp);
    return -1;
}


   void main() {

       int opt = true;
   
    // Initialize Winsock.
      WSADATA wsaData;
      int iResult = WSAStartup( MAKEWORD(2,2), &wsaData );
      if ( iResult != NO_ERROR ){
         printf("Error at WSAStartup()\n");
         return;
      }
   
    // Create a socket.
      SOCKET listenSocket;
      listenSocket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   
      if ( listenSocket == INVALID_SOCKET ) {
         printf( "Error at socket(): %ld\n", WSAGetLastError() );
         WSACleanup();
         return;
      }

      //allows for multiple connections not used for v1

      if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
          sizeof(opt)) < 0)
      {
          printf("opt failed.\n");
          closesocket(listenSocket);
          WSACleanup();
          return;
      }
   
    // Bind the socket.
      sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_addr.s_addr = INADDR_ANY; //use local address
      addr.sin_port = htons(SERVER_PORT);
      if ( bind( listenSocket, (SOCKADDR*) &addr, sizeof(addr) ) == SOCKET_ERROR ) {
         printf( "bind() failed.\n" );
         closesocket(listenSocket);
         WSACleanup();
         return;
      }
   
    // Listen on the Socket.
      if ( listen( listenSocket, MAX_PENDING ) == SOCKET_ERROR ){
         printf( "Error listening on socket.\n");
         closesocket(listenSocket);
         WSACleanup();
         return;
      }
   
    // Accept connections.
      SOCKET s;
      struct sockaddr_in caddr;
      //int addrlen;
      
   
      printf( "My chat room server. Version One.\n\n" );
      while(1){
         
         s = accept(listenSocket, (struct sockaddr*) &addr, NULL);
         if( s == SOCKET_ERROR){
            printf("accept() error \n");
            closesocket(listenSocket);
            WSACleanup();
            return;
         }
      
         //printf( "Client Connected.\n");
         char username[32];
         bool loggedin = false;
         
        
      
      	// Send and receive data.
         
         while (1) {                                                                //loops while there is a client connected 
            


             char buf[MAX_LINE];
             char m[MAX_LINE] = "\0";                                               //placeholder for message to be sent to socket s
             int len = recv(s, buf, MAX_LINE, 0); 
             if (len == -1) {                                                       //if user disconnects display the ip address and port number where the connection was established 
                 closesocket(s);
                 printf("timeout from %s on port %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
                 break;                                                         // break from loop
             }

                 buf[len] = 0;
                 strtok(buf, " ");                                            //toekns the buffer to get the first word from the client input
                 if (strcmp(buf, "login") == 0) {                           //if input is login and no user is logged in 
                     if (loggedin == false) { 


                         char* u = strtok(NULL, " ");        
                         char* p = strtok(NULL, " ");  //tokens the username and password form input
                         int lin = login(u, p);       //checks if the user is able to login with the input
                         if (lin == 0) {              //if check fails, set message to error message and send to client
                             strncpy(m, "Denied. User name or password incorrect.", sizeof(m));
                         }
                         else {
                             loggedin = true;    //if user exists in file, set login to true and send a confirmation message to sever and client
                             strncpy(m, "login confirmed", sizeof(m));

                             strcpy(username, u);
                           
                             printf("%s login.\n", username);
                         }
                     }
                     else {             //otherwise send an error message to client if they are already logged in
                         
                         strncpy(m, "already logged in.", sizeof(m));
                        

                     }

                 }
                 else if (strcmp(buf, "logout") == 0) {    //if logout and logged in
                     if (loggedin == true) {
                         
                         
                         printf("%s logout.\n", username);
                         strncat(username, " left", 5);
                         send(s, username, strlen(username), 0);  //send logout message to client and server 
                         
                         loggedin = false;
                         closesocket(s);      //close the client socket and break from the loop
                         break;
                     }
                     else {          //if not logged in, send diffrent error message and close the socket
                         
                         strncpy(m, "Not logged in. exiting", sizeof(m));
                         send(s, m, strlen(m), 0);
                         closesocket(s);
                         break;  //break from loop

                     }
                 }
                 else if (strcmp(buf, "newuser") == 0) { //if newuser
                     char* u = strtok(NULL, " ");
                     char* p = strtok(NULL, " ");  //token username and password from client input
                     

                     
                     switch (newuser(u, p)) {       //validate if user can be created and send error messages accordingly
                         
                     case 0:
                         strncpy(m, "Denied. User account already exists. ",sizeof(m));
                         break;
                     case 1:
                         
                         strncpy(m, "New user account created. Please login.", sizeof(m));  //prints to server if new user is created
                         printf("New user account created.\n");
                         break;
                     default:
                         strncpy(m, "User creation failed. Try again.", sizeof(m));
                         break;


                     }

                 }
                 else if (strcmp(buf, "send") == 0) {  //if send
                     if (loggedin) {

                     
                     char* message = strtok(NULL, "");  //tokens anything after the command into message
                     strcpy(m, username);                    //copies username into m
                     strncat(m, ": ", 1);                  //adds a : after username
                     strncat(m, message, strlen(message));   //adds the message sent by the client to m
                     printf("%s\n", m);                       //prints result to server and user
                     }
                     else {
                         strncpy(m, "Denied. Please login first.", sizeof(m));  //error messsage if client is not logged in
                     }
                 }
           send(s, m, strlen(m), 0); // sends resulting message to client  and repeats loop while client is still logged in
         }
      }
   
      closesocket(listenSocket);
   }

   
  
             


