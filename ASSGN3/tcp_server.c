// Roll: <23EC30067> Service Type: <B> d = <1>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h> 

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

int main(){
    char roll[] = "23EC30067";
    int port=30067;
    printf("Server running on port %d\n",port);

    //Define socket
    int sockfd= socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) perror("socket failed\n");

    //Define address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    //Bind
    if(bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0) perror("bind failed\n");

    //Listen
    listen:
    bool return_listen = false;
    listen(sockfd, 5);

    //Accept
    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
    int client_sock = accept(sockfd, (struct sockaddr*) &client_addr, &len);
    if (client_sock < 0) perror("socket failed\n");

    while(1){
        char buffer[1024];
        if (recv_until_null(client_sock, buffer, sizeof(buffer)) < 0) break;

        char *prefix = strtok(buffer, "#");
        char *request= strtok(NULL,"#");
        
        printf("Client IP: %s & Port: %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
        printf("Received message: %s\n",request);

        if(strcmp(prefix,roll)){
            char respond[1024]="Unauthorized client";
            send_all(client_sock, respond, strlen(respond)+1);
        }else{
            char *OP=strtok(request,"|");
            if(!strcmp(OP,"EXIT")){
                break;
            }else if(!strcmp(OP,"CONTINUE")){
                return_listen=true;
                break;
            }else if(!strcmp(OP,"SUM") || !strcmp(OP,"MIN") || !strcmp(OP,"MAX") || !strcmp(OP,"AVG")){

                int n = atoi(strtok(NULL, "|"));
                char *num = strtok(NULL, "|");
                int arr[n];
                arr[0]=atoi(strtok(num," "));
                for(int i=0;i<n;i++){
                    if(i) arr[i]=atoi(strtok(NULL," "));
                }

                if(!strcmp(OP,"SUM")){
                    char respond[1024];
                    int sum=0;
                    for(int i=0;i<n;i++) sum+=arr[i];
                    sprintf(respond, "%d", sum);
                    send_all(client_sock, respond, strlen(respond)+1);
                }else if(!strcmp(OP,"MAX")){
                    char respond[1024];
                    int maximum=arr[0];
                    for(int i=0;i<n;i++){
                        if(maximum<arr[i]){
                            maximum=arr[i];
                        }
                    }
                    sprintf(respond, "%d", maximum);
                    send_all(client_sock, respond, strlen(respond)+1);         
                }else if(!strcmp(OP,"MIN")){
                    char respond[1024];
                    int minimum=arr[0];
                    for(int i=0;i<n;i++){
                        if(minimum>arr[i]){
                            minimum=arr[i];
                        }
                    }
                    sprintf(respond, "%d", minimum);
                    send_all(client_sock, respond, strlen(respond)+1);          
                }else if(!strcmp(OP,"AVG")){
                    char respond[1024];
                    float avg=0;
                    for(int i=0;i<n;i++) avg+=(float)arr[i];
                    avg=avg/(float)n;
                    sprintf(respond, "%.2f", avg);
                    send_all(client_sock, respond, strlen(respond)+1);           
                }
            }else{
                char respond[1024]="Wrong Operation!";
                send_all(client_sock, respond, strlen(respond)+1);
            }
        }
    }
    close(client_sock);
    if(return_listen){
        goto listen;
    }
    close(sockfd);
    printf("Server closed!\n");
    return 0;
}