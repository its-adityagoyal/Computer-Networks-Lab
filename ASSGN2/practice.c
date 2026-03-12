#include <stdio.h>
#include <string.h>  // for strtok
#include <stdlib.h> // for atoi

int main(){
    char buffer[]="23EC30067#SUM|5|5 3 6 2 5";
    char *prefix = strtok(buffer, "#");

    char *OP=strtok(NULL,"|");
    int n = atoi(strtok(NULL, "|"));
    char *num = strtok(NULL, "\0");
    printf("%s %s\n%d\n%s\n",prefix,OP,n,num);

    int arr[n];
    int i=1;
    arr[0]=atoi(strtok(num," "));
    while(i<n){
        arr[i]=atoi(strtok(NULL," "));
        i++;
    }
    for(int i=0;i<n;i++){
        printf("%d",arr[i]);
    }
}