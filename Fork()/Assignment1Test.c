//
//  Assignment1Test.c
//  
//
//  Created by Jonah Tuckman on 2/12/19.
//

#include "Assignment1Test.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>


int main(int argc, char *argv[]){
    printf("(%d) I am the parent\n", getpid());
    
    int rc;
    pid_t parent;
    parent = getpid();
    rc = fork();
    
    if(rc == 0) {
        printf("(%d) I am child A\n", getpid());
    }
    else {
        rc = fork();
        if (rc == 0) {
            printf("(%d) I am child B\n",getpid());
        }
        else {
            while(wait(0) > 0);
            printf("(%d) Huh, I'm awake? Who am I?\n", getpid());
        }
    }
    
    if (getpid() == parent){
        printf("(%d) I am still the parent\n", getpid());
    }
    rc = fork();
    if(rc == 0) {
        printf("(%d) I am child C\n", getpid());
    }
    else {
        wait(0);
    }
    
    if (getpid() == parent){
        printf("(%d) Race to the finish?\n", getpid());
        rc = fork();
        if(rc == 0) {
            printf("(%d) Racing Child Finished!\n", getpid());
        } else {
            printf("(%d) Parent Finished!\n", getpid());
        }
        return 0;
    }
    printf("(%d) I'm done! \n", getpid());
    return 0;
}
