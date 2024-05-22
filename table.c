#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<stdbool.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define MAX_CUSTOMERS 5
#define MAX_ORDERS 10

int main() {
    int num_customers=0, table_number, earnings=0;
    printf("Enter table number: ");
    scanf("%d", &table_number);
    while(true){
    if(earnings!=-9999){
    printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
    scanf("%d", &num_customers);}
    if(num_customers>5){
        printf("Sorry, a table cannot accomodate more than 5 Customers\n");
        continue;
    }
    if(num_customers!=-1){
    int fileDescriptor = open("menu.txt", O_RDONLY);

    if (fileDescriptor == -1) {
        write(2, "Error opening file\n", 20); // write to stderr
        return 1;
    }

    // Read and print the contents of the file
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;

    while ((bytesRead = read(fileDescriptor, buffer, BUFFER_SIZE)) > 0) {
        write(1, buffer, bytesRead); // write to stdout
    }

    if (bytesRead == -1) { 
        write(2, "Error reading file\n", 19); // write to stderr
        close(fileDescriptor);
        return 1;
        // Close the file
    }
    close(fileDescriptor);}
    printf("\n");
    int customer_orders[MAX_CUSTOMERS][MAX_ORDERS];
    if(num_customers!=-1){}
     for (int i = 0; i < num_customers; i++) {
        pid_t pid;
        int pipefd[2];
        pipe(pipefd);

        pid = fork();

        if (pid == 0) {
            printf("\n");
            printf("\nPlease enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done:\n");

            int cust_ord[MAX_ORDERS];
            int in, size = 0;

            while (1) {
                if (size == MAX_ORDERS) {
                    printf("\nSorry, you cannot order more than %d items, finalizing your order...\n", MAX_ORDERS);
                    break;
                }

                scanf("%d", &in);

                if (in == -1) {
                    printf("Finalizing your order...\n");
                    break;
                }

                cust_ord[size++] = in;
            }
            while (size < MAX_ORDERS) {
                cust_ord[size++] = 0;
            }
            write(pipefd[1], cust_ord, sizeof(int) * MAX_ORDERS);
            exit(EXIT_SUCCESS);
        } else if (pid > 0) {
            waitpid(pid, NULL, 0); 
            read(pipefd[0], customer_orders[i], sizeof(int) * MAX_ORDERS);
        } else {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }
    for(int i=num_customers;i<MAX_CUSTOMERS;i++){
        for(int j=0;j<10;j++)
        customer_orders[i][j]=0;
    }
    

    int shmid1, shmid2;
    key_t key1 = ftok("menu.txt", table_number);
    key_t key2 = ftok("menu.txt", table_number + 100);
    size_t size = 1024;

    shmid1 = shmget(key1, size, 0664 | IPC_CREAT);
    shmid2 = shmget(key2, size , 0664 | IPC_CREAT);

    if(shmid1 == -1 || shmid2 == -1) {
        perror("Error in shmget in creating/accessing shared memory\n");
        return 1;
    }

    int (*shm)[MAX_ORDERS] = (int (*)[MAX_ORDERS])shmat(shmid1, NULL, 0);
    int *flag_and_earnings = (int *)shmat(shmid2, NULL, 0);

    flag_and_earnings[0] = 1; 
    flag_and_earnings[2] = num_customers;
    if(num_customers==-1)
        break;
    if (flag_and_earnings[0] == 1) {
        for (int i = 0; i < MAX_CUSTOMERS; i++) {
            for (int j = 0; j < MAX_ORDERS; j++) {
                shm[i][j] = customer_orders[i][j];
            }
        }
        flag_and_earnings[0] = 0;  
    }
    while(flag_and_earnings[0] != 1){
        printf("Waiting for waiter\n");
        sleep(2);
    }

    earnings = flag_and_earnings[1];
    if(earnings!=-9999){
        printf("\n");
        printf("\nThe total bill amount is %d INR.\n", earnings);
        printf("\n");
    }
    else{
        printf("\n");
        printf("Invalid Order, Everyone please Order again\n");
        printf("\n");
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
            for (int j = 0; j < MAX_ORDERS; j++) {
                shmdt((void *)shm[i][j]);
            }
    }
    for(int i=0;i<3;i++)
        shmdt((void *)flag_and_earnings[i]);
    #pragma GCC diagnostic pop
   // shmctl(shmid1, IPC_RMID, NULL);
    //shmctl(shmid2, IPC_RMID, NULL);
    if(shmctl(shmid1, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
    if(shmctl(shmid2, IPC_RMID, NULL)==-1){
            perror("Error in shmctl()\n");
			return 1;
        }
    }
    return 0;
}