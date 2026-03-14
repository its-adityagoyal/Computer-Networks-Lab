#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

char* server_ip, *username;
int port;

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

int main(int argc, char *argv[]){
    if(argc==3){
        server_ip=argv[1];
        port=atoi(argv[2]);
    }else{
        printf("./%s {server_ip} {port}\n",argv[0]);
        return 1;
    }

    username = "Unknown";
    //Socket
    struct sockaddr_in server_addr;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) perror("socket failed\n");

    //Server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET,server_ip, &server_addr.sin_addr);

    //Connect
    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

    while(1){
        char command[100];
        printf("Enter your request: ");
        fgets(command,sizeof(command),stdin);

        if(strncmp(command, "SUBMIT",6)==0){
            send_all(sockfd, command, strlen(command));
            while(1){
                char line[100];
                printf(">> ");
                fflush(stdout);
                fgets(line,sizeof(line),stdin);
                if(strcmp(line, ".\n")==0){
                    line[0]='\0';
                    send_all(sockfd, line, 1);
                    break;
                }
                send_all(sockfd, line, strlen(line));  //The last character sent is '\n'
            }
        }else{
            command[strcspn(command,"\n")]='\0';
            if(!strncmp(command,"HELLO",5)){
                username=&(command[6]);
            }
            send_all(sockfd, command, strlen(command)+1);
            if(!strncmp(command, "QUIT", 4)){
                break;
            }
        }

        //Receive
        char respond[1024];
        recv_until_null(sockfd, respond, sizeof(respond));
        printf("Received response: ");
        int i=0;
        while (respond[i] != '\0') {
            printf("%c",respond[i]);
            i++;
        }
    }
    printf("Client %s closed!\n",username);
    close(sockfd);
    return 0;
}