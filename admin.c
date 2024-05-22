#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>


key_t generateManagerKey(int x) {
    	key_t key;
    	if ((key = ftok("earnings.txt", x)) == -1) {
        	perror("Error in ftok\n");
        	exit(1);
    	}
    	return key;
}

int createSharedMemory(key_t key) {
    	int shmid;
    	if ((shmid = shmget(key,1024, 0644 | IPC_CREAT)) == -1) {
        	perror("Error in shmget in creating/accessing shared memory\n");
        	exit(1);
    	}
    	return shmid;
}

int *attachSharedMemory(int shmid) {
    	int *shmPtr;
    	if ((shmPtr = shmat(shmid, NULL, 0)) == (void*)-1) {
        	perror("Error in attaching the memory segment\n");
        	exit(1);
    	}
    	return shmPtr;
}
int main() {
	key_t adminKey = generateManagerKey(0);
	int adminShmid = createSharedMemory(adminKey);
	int *adminShmPtr = attachSharedMemory(adminShmid);
	char choice;
	
	*adminShmPtr = 1;
	printf("Do you want to close the hotel? Enter Y for Yes and N for No - ");
	while(1){
	scanf("%c",&choice);
	if(choice == 'N'){
		printf("Do you want to close the hotel? Enter Y for Yes and N for No - ");
		continue;
	}
	else if(choice == 'Y'){
		*adminShmPtr = 0;
		break;
		}
	sleep(2);
	}
	
	if (shmdt(adminShmPtr) == -1) {
        perror("Error detaching shared memory\n");
    	}
    	
	
	return 0;
}