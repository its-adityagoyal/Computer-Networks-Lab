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

int send_all(int sock, char *buf, int len){
    int total = 0;
    while(total < len){
        int n = write(sock, buf + total, len - total);
        if(n <= 0) return -1;
        total += n;
    }
    return total;
}

int recv_until_null(int sock, char *buf, int maxlen) {
    int total = 0;
    while (total < maxlen - 1) {
        int n = read(sock, buf + total, maxlen - 1 - total);
        if (n <= 0) return -1;   
        for (int i = 0; i < n; i++) {
            if (buf[total + i] == '\0') {
                return total + i;   
            }
        }
        total += n;
    }
    buf[maxlen - 1] = '\0';
    return total;
}

void myhandler(int sig){
    char request[1024];
    sprintf(request,"%s#CONTINUE",roll);
    send_all(sockfd, request, strlen(request)+1);
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
    if (sockfd < 0) perror("socket failed\n");

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
        send_all(sockfd, request, strlen(request)+1);
        if(!strcmp(input,"EXIT")){
            break;
        }else if(!strcmp(input,"CONTINUE")){
            break;
        }

        //Receive
        char respond[1024];
        recv_until_null(sockfd, respond, sizeof(respond));
        printf("Received response: %s\n",respond);
    }
    close(sockfd);
    printf("Client closed!\n");
    return 0;
}