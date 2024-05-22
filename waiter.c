#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define MAX_ITEMS 100
#define MAX_ORDERS 10

typedef struct {
    int serialnumber;
    int price;
} MenuItem;

key_t generateKey(const char *filename, int waiterID) {
    key_t key;
    if ((key = ftok(filename, waiterID)) == -1) {
        perror("Error in ftok");
        exit(1);
    }
    return key;
}

int createSharedMemory(key_t key) {
    int shmid;
    size_t size =1024;
    if ((shmid = shmget(key, size, 0644 | IPC_CREAT)) == -1) {
        perror("Error in shmget in creating/accessing shared memory");
        exit(1);
    }
    return shmid;
}

int *attachSharedMemory(int shmid) {
    int *shmPtr;
    if ((shmPtr = shmat(shmid, NULL, 0)) == (int *)-1) {
        perror("Error in attaching the memory segment");
        exit(1);
    }
    return shmPtr;
}

void readMenu(MenuItem menu[], int *numItems) {
    FILE *menuFile = fopen("menu.txt", "r");
    if (menuFile == NULL) {
        perror("Error opening menu file");
        exit(1);
    }
    *numItems = 0;
    char line[100];
    while (fgets(line, sizeof(line), menuFile) != NULL && *numItems < MAX_ITEMS) {
        char *token = strtok(line, " ");
        int foundSerial = 0;
        while (token != NULL) {
            int number;
            if (sscanf(token, "%d", &number) == 1) {
                if (!foundSerial) {
                    menu[*numItems].serialnumber = number;
                    foundSerial = 1;
                } else {
                    menu[*numItems].price = number;
                }
            }
            token = strtok(NULL, " ");
        }
        (*numItems)++;
    }
    fclose(menuFile);
}

int processOrders(int (*tableShmPtr)[MAX_ORDERS], MenuItem menu[], int numItems){
	int sum=0;
    	for (int i = 0; i < 5; i++) {
        	for (int j = 0; j < 10; j++) {
            	if(tableShmPtr[i][j] >= 0 && tableShmPtr[i][j] <=numItems){//number of items
            		for (int k = 0; k < numItems; k++) {
                		if (tableShmPtr[i][j] == menu[k].serialnumber)
                   		sum = sum + menu[k].price;
            		}
        	}
        	else{
        	 sum = -9999;
             return sum;
        	}

    	}
   	}
    //printf("BILL = %d\n", sum);
	return sum;
}

int main() {
	int waiterID;
    printf("Enter Waiter ID: ");
    scanf("%d", &waiterID);
while(1){


    key_t tableKey = generateKey("menu.txt", waiterID);
    key_t tableKey2 = generateKey("menu.txt", waiterID+100);
    key_t managerKey = generateKey("earnings.txt", waiterID);

    int tableShmid = createSharedMemory(tableKey);
    int tableShmid2 = createSharedMemory(tableKey2);
    int managerShmid = createSharedMemory(managerKey);

    int (*tableShmPtr)[MAX_ORDERS] = (int (*)[MAX_ORDERS])attachSharedMemory(tableShmid);
    int *flag_and_earnings = (int *)attachSharedMemory(tableShmid2);
    int *managerShmPtr = attachSharedMemory(managerShmid);
    *managerShmPtr=1;
    int max_waiters=*(managerShmPtr+1);
    //printf("MAX= %d\n", max_waiters);
    if(waiterID>max_waiters){
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < MAX_ORDERS; j++) {
                shmdt((void *)tableShmPtr[i][j]);
            }
    }
    for (int j = 0; j < 3; j++) {
                shmdt((void *)flag_and_earnings[j]);
            }
        #pragma GCC diagnostic pop
        if(shmctl(tableShmid, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
        if(shmctl(tableShmid2, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
         if(shmctl(managerShmid, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }

        printf("Waiter ID does not exist, please enter a valid ID");
        break;
    }

    MenuItem menu[MAX_ITEMS];
    int numItems;

    readMenu(menu, &numItems);
    //printf("FLAG = %d\n",flag_and_earnings[0]);
    //printf("NUMBER OF CUSTOMERS = %d\n",flag_and_earnings[2]);
    
    do{
        if(flag_and_earnings[2]==-1)
        {
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < MAX_ORDERS; j++) {
                shmdt((void *)tableShmPtr[i][j]);
            }
    }
    for (int j = 0; j < 3; j++) {
                shmdt((void *)flag_and_earnings[j]);
            }
        #pragma GCC diagnostic pop
        if(shmctl(tableShmid, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
        if(shmctl(tableShmid2, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
        printf("\n");
        printf("No more customers are there. ENDING!!\n");
        printf("\n");
            break;
        }
        printf("Waiting for Customer's Input\n");
        //*managerShmPtr = 1;
        sleep(2);
    }
    while (flag_and_earnings[0] != 0);
    //printf("FLAG before sum= %d\n",flag_and_earnings[2]);
    int sum = processOrders(tableShmPtr,menu, numItems);
    if(sum==0)
        *managerShmPtr = 1;
    if(sum>0)
    {
        printf("\n\nBill Amount for Table %d: %d INR\n\n", waiterID, sum);
        printf("\n");
        *managerShmPtr = sum;
        sleep(2);
    }
    flag_and_earnings[1] = sum;
	flag_and_earnings[0] = 1;

    if(flag_and_earnings[2]==-1){
    	*managerShmPtr =0;
        break;
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
        for (int i = 0; i < 10; i++) {
            for (int j = 0; j < MAX_ORDERS; j++) {
                shmdt((void *)tableShmPtr[i][j]);
            }
    }
    for (int j = 0; j < 3; j++) {
                shmdt((void *)flag_and_earnings[j]);
            }
    #pragma GCC diagnostic pop
    if(shmctl(tableShmid, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
    if(shmctl(tableShmid2, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
    if(shmdt(managerShmPtr)==-1){
            perror("Error in shmdt in detaching the memory segment\n");
			return 1;
     }

    }

    return 0;
}