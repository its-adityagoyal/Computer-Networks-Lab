// Roll: <23EC30067> Service Type: <B> d = <1>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>  // for strtok
#include <stdlib.h> // for atoi

int main()
{
    char *roll="23EC30067";
    int port = 30067;
    printf("Server running on port %d\n",port);

    //Define socket
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    //Define address
    struct sockaddr_in server_addr, client_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Binding the socket with a port
    bind(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr));
    
    socklen_t len = sizeof(client_addr);
    while(1){
        char buffer[1024];
        int x=recvfrom(sockfd, buffer, sizeof(buffer), 0,(struct sockaddr *)&client_addr, &len);
        buffer[x]='\0';
        char *prefix = strtok(buffer, "#");
        char *request= strtok(NULL,"#");
        
        printf("Client IP: %d & Port:%d\n", ntohs(client_addr.sin_port),port);
        printf("Received message: %s\n",request);

        if(strcmp(prefix,roll)){
            sendto(sockfd, "Unauthorized client", 19, 0,(struct sockaddr *)&client_addr, len);
        }else{
            char *OP=strtok(request,"|");
            if(!strcmp(OP,"EXIT")){
                break;
            }
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
                sendto(sockfd,respond,sizeof(respond), 0,(struct sockaddr *)&client_addr, len);
            }else if(!strcmp(OP,"MAX")){
                char respond[1024];
                int maximum=0;
                for(int i=0;i<n;i++){
                    if(maximum<arr[i]){
                        maximum=arr[i];
                    }
                }
                sprintf(respond, "%d", maximum);
                sendto(sockfd,respond,sizeof(respond), 0,(struct sockaddr *)&client_addr, len);            
            }else if(!strcmp(OP,"MIN")){
                char respond[1024];
                int minimum=1e9;
                for(int i=0;i<n;i++){
                    if(minimum>arr[i]){
                        minimum=arr[i];
                    }
                }
                sprintf(respond, "%d", minimum);
                sendto(sockfd,respond,sizeof(respond), 0,(struct sockaddr *)&client_addr, len);            
            }else if(!strcmp(OP,"AVG")){
                char respond[1024];
                float avg=0;
                for(int i=0;i<n;i++) avg+=(float)arr[i];
                avg=avg/(float)n;
                sprintf(respond, "%.2f", avg);
                sendto(sockfd,respond,sizeof(respond), 0,(struct sockaddr *)&client_addr, len);            
            }else{
                char respond[1024]="Wrong Operation!";
                sendto(sockfd,respond,sizeof(respond), 0,(struct sockaddr *)&client_addr, len);
            }
        }
    }
    close(sockfd);
    printf("Server closed!\n");
    return 0;
}