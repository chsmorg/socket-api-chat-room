
//
//  main.c
//  client v1 
//
//  Created by chase morgan on 3/15/22.
//

#include <stdio.h>
#include "winsock2.h"

#define SERVER_PORT  13388
#define MAX_LINE      256
//help command to assist users with commands list, no impact on project.
void help() {
    printf("> command list: (all inputs must be 256 characters or less)\n"
           "> 'newuser', format: newuser 'username' 'password', username must be 3-32 characters long and password must be 4-8 characters\n"
           "> 'login', format: login 'username' 'password', username must be 3-32 characters long and password must be 4-8 characters\n"
           "> 'send', format: send 'message', message must contain a message body\n"
           "> 'logout' exits the program regardless of login status\n");
}

void main(int argc, char** argv) {

    if (argc < 2) {
        printf("\nUseage: client serverName\n");
        return;
    }

    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("Error at WSAStartup()\n");
        return;
    }

    //translate the server name or IP address (128.90.54.1) to resolved IP address
    unsigned int ipaddr;
    // If the user input is an alpha name for the host, use gethostbyname()
    // If not, get host by addr (assume IPv4)
    if (isalpha(argv[1][0])) {   // host address is a name  
        hostent* remoteHost = gethostbyname(argv[1]);
        if (remoteHost == NULL) {
            printf("Host not found\n");
            WSACleanup();
            return;
        }
        ipaddr = *((unsigned long*)remoteHost->h_addr);
    }
    else //"128.90.54.1"
        ipaddr = inet_addr(argv[1]);


    // Create a socket.
    SOCKET s;
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        WSACleanup();
        return;
    }

    // Connect to a server.
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = ipaddr;
    addr.sin_port = htons(SERVER_PORT);
    if (connect(s, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        printf("Failed to connect.\n");
        WSACleanup();
        return;
    }

    // Send and receive data.
    char buf[MAX_LINE];
    char input[MAX_LINE]; //user input string
    bool logout = false; //user logout status
    printf("My chat room client. Version One. \n\n");
      
      while (!logout) { // loops while user is not logged out
          bool sendserver = true;  //checks if user input will be sent to the server, only variable


          printf("> ");
          gets_s(buf);                         //used to get the input from the user
          strncpy(input, buf, sizeof(buf));   //copies that input into the buffer 
          strtok(input, " ");                //tokens the input string from a space input, input would only curretly hold the first word of the string
          
          if (strcmp(input, "logout") == 0) {
              logout = true;                   //if the user input = logout, sets the logout check to true ending the loop
          }
          else if (strlen(input)==0) {       //checks if the user input contains atleast one character, otherwise send an error message, not sent to server
              printf("> Please enter a command. (Command list: 'login', 'newuser', 'send', 'logout') or 'help' for more info.\n");
              continue;
          }
          else if (strcmp(input, "help") == 0) {  //used for help command, prints the command list along with formatting, not sent to server
              help();
              continue;
          }
          else {
              
              if (strcmp(input, "newuser") == 0  || strcmp(input, "login") == 0) { //checks for the newuser and login formatting, if input is one of these commands 
                  char* u = strtok(NULL, " ");       //gets the next word after command
                  char* p = strtok(NULL, " ");      //gets word after that, cannot contain spaces
                  sendserver = false;    
                  if (p != NULL) {                  //checks for NULL 
                      int userlen = strlen(u);
                      int passlen = strlen(p);
                      if (userlen >= 3 && userlen <= 32 && passlen >= 4 && passlen <= 8) sendserver = true; // checks length of username and password, if it is a vaild input, message will be sent to server
                          
                     
                  }
                  if (!sendserver) {  //if it did not pass the checks, sends error message to user and will not be sent to the server
                      printf("> Useage of command incorrect. please try again. (use 'help' for more info)\n");
                  }
                  
                  
              }
              else if (strcmp(input, "send") == 0) { //checks for send command
                  char* m = strtok(NULL, "");

                  
                  if (!m) {   //checks if the message body is empty and sends an error message, will not be sent to server
                      printf("> Must contain a message body.\n");
                      sendserver = false;
                          
                  }
                  
                  
              }
              else {
                  printf("> Please enter a vaild command. (Command list: 'login', 'newuser', 'send', 'logout') or use 'help' for more info.\n"); // if input was not a command. sends error message along with command list
                  sendserver = false;
              }
          }
          if (sendserver) {  //sends the input to the server if it passed all checks 
              send(s, buf, strlen(buf), 0);
              int len = recv(s, buf, MAX_LINE, 0); //waits for a message to be received from server then prints to the user
              buf[len] = 0;
              printf("%> %s\n", buf); 
          }
         
          
      }
      
   
      closesocket(s); // if loop is exited, close the socket
   }
