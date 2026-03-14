#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> 

int port, process_no=1;
int listen_fd, client_sock;

typedef enum{ 
    QUEUED, 
    PRINTING, 
    DONE, 
    CANCELED 
}job_state_t; 

typedef struct{ 
    int id; 
    char title[128]; 
    job_state_t state;  
    struct timespec submitted_at; // monotonic timestamp at submit 
    int canceled; 
}job_t; 

static double elapsed_seconds (struct timespec start, struct timespec now) { 
    double s = (double)(now.tv_sec - start.tv_sec); 
    double ns = (double)(now.tv_nsec - start.tv_nsec) / 1e9; 
    return s + ns; 
}

job_state_t compute_state(job_t *j) { 
    if (j->canceled){
        j->state=CANCELED;
        return CANCELED; 
    }
    struct timespec now; 
    clock_gettime(CLOCK_MONOTONIC, &now); 
    double dt = elapsed_seconds(j->submitted_at, now); 
    if (dt < 10.0){
        j->state=QUEUED;
        return QUEUED; 
    }
    if (dt < 30.0){
        j->state=PRINTING;
        return PRINTING; 
    }
    j->state=DONE;
    return DONE; 
} 

char *state_str(job_state_t state){
    if(state==QUEUED) return "QUEUED";
    if(state==PRINTING) return "PRINTING";
    if(state==DONE) return "DONE";
    else return "CANCELED";
    
}

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
    close(client_sock);
    printf("\nServer Process %d shutting down...\n",process_no);
    exit(0);
}
void parent_handler(int sig){
    close(listen_fd);
    printf("\nServer shutting down...\n");
    exit(0);
}

void server(){
    signal(SIGINT,myhandler);
    char *username;
    job_t jobs[20];
    int next_job_id=1;
    int i=0;

    while(1){
        char buffer[1024];
        if (recv_until_null(client_sock, buffer, sizeof(buffer)) < 0) break;

        if(!strncmp(buffer, "HELLO",5)){
            username = &(buffer[6]);
            char respond[1024];
            sprintf(respond,"HI! %s\n",username);
            send_all(client_sock, respond, strlen(respond)+1);

        }else if(!strncmp(buffer, "SUBMIT",6)){
            buffer[strcspn(buffer,"\n")]='\0';
            strcpy(jobs[i].title, &(buffer[7]));
            jobs[i].id=next_job_id;
            next_job_id++;
            jobs[i].state=QUEUED;
            clock_gettime(CLOCK_MONOTONIC, &jobs[i].submitted_at); 
            jobs[i].canceled = 0;

            //Respond
            char respond[1024];
            sprintf(respond,"ID: %d\n",jobs[i].id);
            send_all(client_sock, respond, strlen(respond)+1);
            i++;
        }else if(!strncmp(buffer, "STATUS",6)){
            char* jobid_str = &(buffer[7]);
            int jobid = atoi(jobid_str);
            
            if(jobid<next_job_id){
                char respond[1024];
                sprintf(respond,"%s\n",state_str(compute_state(&jobs[jobid-1])));
                send_all(client_sock, respond, strlen(respond)+1);
            }else{
                char respond[1024];
                sprintf(respond,"%s\n","404");
                send_all(client_sock, respond, strlen(respond)+1);
            }
        }else if(!strncmp(buffer, "LIST",4)){
            char respond[1024]="";
            for(int it=0; it<i; it++){
                char line[100];
                sprintf(line,"Title: %s | ID: %d | STATE: %s\n",jobs[it].title, jobs[it].id, state_str(compute_state(&jobs[it])));
                strcat(respond,line);
            }
            send_all(client_sock, respond, strlen(respond)+1);
        }else if(!strncmp(buffer, "CANCEL",6)){
            char* jobid_str = &(buffer[7]);
            int jobid = atoi(jobid_str);
            char* respond;
            if(jobid<next_job_id){
                if(jobs[jobid-1].canceled==1){
                    respond="409 ALREADY_DONE\n";
                }else{
                    jobs[jobid-1].canceled=1;
                    jobs[jobid-1].state=CANCELED;
                    respond="DONE\n";
                }
            }else{
                respond="404\n";
            }
            send_all(client_sock, respond, strlen(respond)+1);
        }else if(!strncmp(buffer, "QUIT",4)){
            char respond[1024];
            sprintf(respond,"GOODBYE %s\n",username);
            send_all(client_sock, respond, strlen(respond)+1);
            close(client_sock);
            printf("Server Process %d closed\n",process_no);
            exit(0);
        }else{
            char respond[1024];
            sprintf(respond,"INVALID COMMAND\n");
            send_all(client_sock, respond, strlen(respond)+1);
        }
    }

}

int main(int argc, char *argv[]){
    if(argc==1){
        perror("Enter port number!\n");
        return 1;
    }else{
        port=atoi(argv[1]);
    }
    printf("Server running on port %d\n",port);
    signal(SIGCHLD, SIG_IGN);
    

    //Define socket
    listen_fd= socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) perror("socket failed\n");
    signal(SIGINT, parent_handler);

    //Define address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    //Bind
    if(bind(listen_fd,(struct sockaddr *)&server_addr,sizeof(server_addr))<0) perror("bind failed\n");

    //Listen
    listen(listen_fd, 5);
    
    while(1){
        //Accept
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        client_sock = accept(listen_fd, (struct sockaddr*) &client_addr, &len);
        if (client_sock < 0) perror("socket failed\n");

        pid_t pid=fork();
        if(pid==0){ //Inside child
            close(listen_fd);
            printf("Client Socket fd: %d & Server Process No.: %d\n",client_sock, process_no);
            printf("Client IP: %s & Client Port: %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));
            server();
            //Do something
            exit(0);
        }else{ //Inside parent
            close(client_sock);
            process_no++;
        }
    }
    return 0;
}