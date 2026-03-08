#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

char *server_ip, *roll;
int sockfd;

void myhandler(int sig){
    char request[1024];
    sprintf(request,"%s#CONTINUE",roll);
    write(sockfd, request, sizeof(request));
    close(sockfd);
    printf("Client closed\n");
    exit(0);
}
int main(int argc, char *argv[]){
    if(argc==3){
        server_ip=argv[1];
        roll=argv[2];
    }

    //Socket
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    //Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(30067);
    inet_pton(AF_INET,server_ip, &server_addr.sin_addr);

    //Connect
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    signal(SIGINT,myhandler);
    while(1){
        char input[100];
        printf("Enter your request: ");
        fgets(input,sizeof(input),stdin);
        input[strcspn(input,"\n")]='\0';
        char request[1024];
        sprintf(request,"%s#%s",roll,input);

        //Write
        write(sockfd, request, sizeof(request));
        if(!strcmp(input,"EXIT")){
            break;
        }else if(!strcmp(input,"CONTINUE")){
            break;
        }

        //Receive
        char respond[1024];
        read(sockfd, respond, sizeof(respond));
        printf("Received response: %s\n",respond);
    }
    close(sockfd);
    printf("Client closed!\n");
    return 0;
}