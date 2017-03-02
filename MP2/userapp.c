#include "userapp.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

long int factorial(int n)
{
        if(n<=1) return 1;
        return n*factorial(n-1);
}

void user_registration()
{
	pid_t pid = getpid();
	char* status = NULL;
	//long int n =15000;

	//printf("The pid is %d \n",pid);
	if(asprintf(&status, "sudo echo %d >/proc/mp1/status", "R" )<0){
		fputs("Fail to register", stderr);
		return;
	}
    
    if(asprintf(&status, "sudo echo %d >/proc/mp1/status", "Y" )<0){
        fputs("Fail to register", stderr);
        return;
    }
    
    if(asprintf(&status, "sudo echo %d >/proc/mp1/status", "D" )<0){
        fputs("Fail to register", stderr);
        return;
    }

	/*if(system(status)<0) {
		fputs( "Fail to execute",stderr);
		return;
	}
	printf("%d calculating \n",pid);
	long int  res =0;
	for(int i=0;i<200000;i++) // Add a delay to make cpu runtime visible
		 res=factorial(n);
//	printf("The factorial of n is %ld \n", res);
	printf("The %d call the cat: \n",pid);	
	system ("cat /proc/mp1/status"); // Print out the recorded time and pid in status
	sleep(1);*/
	return; 

}


int main(int argc, char const *argv[])
{

	user_registration();
	return 0;
}
