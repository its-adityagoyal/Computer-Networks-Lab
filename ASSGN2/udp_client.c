#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

char *server_ip, *roll;

int main(int argc, char *argv[]){
    if(argc==3){
        server_ip=argv[1];
        roll=argv[2];
    }

    int sockfd;
    struct sockaddr_in server_addr;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(30067);
    inet_pton(AF_INET,server_ip, &server_addr.sin_addr);
    socklen_t len_server_addr = sizeof(server_addr);

    while(1){
        char input[100];
        printf("Enter your request: ");
        fgets(input,sizeof(input),stdin);
        input[strcspn(input,"\n")]='\0';
        char request[1024];
        sprintf(request,"%s#%s",roll,input);
        sendto(sockfd, request, 1024, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        if(!strcmp(input,"EXIT")){
            break;
        }

        //Receive
        char respond[1024];
        recvfrom(sockfd, respond, sizeof(respond), 0,(struct sockaddr *)&server_addr, &len_server_addr);
        printf("Received response: %s\n",respond);
    }
    close(sockfd);
    printf("Client closed\n");
    return 0;
}