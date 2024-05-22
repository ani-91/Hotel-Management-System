#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include<sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#define MAX_TABLES 10
#define BUF_SIZE 1024


void generate_keys(key_t keys[],int num_tables){
    for(int i=0;i<num_tables;i++){
        keys[i]=ftok("earnings.txt",i+1);
        if (keys[i]==-1){
            perror("ftok() Failed! Could not generate keys for Hotel Manager.");
            exit(EXIT_FAILURE);
        }
    }
}


void generate_shm_ids(key_t keys[],int shmids[],int num_tables){
    for(int i=0;i<num_tables;i++){
        shmids[i]=shmget(keys[i],BUF_SIZE,0666|IPC_CREAT);
        if(shmids[i]==-1){
            perror("shmget() Failed! Could not create shared memory segment for Hotel Manager.");
            exit(EXIT_FAILURE);
        }
    }
}


int main(){

    //connecting to admin and retrieving admin_cmd

    key_t admin_key;
    int admin_shmid;
    int* cmd_ptr;

    admin_key=ftok("earnings.txt",0);
    if (admin_key==-1){
            perror("ftok() Failed! Could not generate key for Hotel Manager-Admin communication.");
            exit(EXIT_FAILURE);
    }

    admin_shmid=shmget(admin_key,BUF_SIZE,0666|IPC_CREAT);
    if(admin_shmid==-1){
            perror("shmget() Failed! Could not create shared memory segment for Hotel Manager and Admin.");
            exit(EXIT_FAILURE);
    }

    cmd_ptr=(int*)shmat(admin_shmid,NULL,0);
    if (*cmd_ptr==-1){
            perror("Error in shmat(). Hotel Manager could not communicate with Admin");
            exit(EXIT_FAILURE);
    }

    int admin_cmd=*cmd_ptr;//command by admin: 0 for close, 1 for open

    if(admin_cmd==0){
        printf("\nThe hotel is not open yet. Please wait for the admin to open the hotel!\n");
        return 0;
    }

    //hotel opens here!
    printf("\n----------WELCOME TO OUR HOTEL----------\n");

    int total_tables;
    printf("\nEnter the Total Number of Tables at the Hotel: ");
    scanf("%d", &total_tables);

    if (total_tables<1 || total_tables>MAX_TABLES){
        printf("You did not select valid number of tables. We have maximum 10 tables.\n");
        return 1;
    }

    key_t keys[total_tables];
    int shm_ids[total_tables];

    generate_keys(keys,total_tables);
    generate_shm_ids(keys,shm_ids,total_tables);

    int *table_earnings[total_tables];

    for (int i=0;i<total_tables;i++){
        table_earnings[i]=(int*)shmat(shm_ids[i],NULL,0);
        if (*table_earnings[i]==-1){
            printf("Error in shmat(). Hotel Manager could not retrieve total bill amount for Table %d\n",i);
            return 1;
        }
    }

    for (int i=0;i<total_tables;i++){
        *(table_earnings[i]+1)=total_tables;
    }


    //hotel manager starts working from here!

    FILE *f=fopen("earnings.txt","w");
    if (f==NULL){
        perror("ERROR! Could not open earnings.txt file.");
        return 1;
    }
    fclose(f);

    int total_earnings=0;
    int terminate=0;
    while(terminate==0){

        sleep(2);

        int allTablesEmpty[total_tables];

        for(int k=0;k<total_tables;k++)
            allTablesEmpty[k]=1;

        for(int i=0;i<total_tables;i++){
            allTablesEmpty[i]=1;

            //customers have left the table and we have some earnings from that table
            if(*table_earnings[i]>1){

                printf("\nReceived %d INR from Table %d.\n",*table_earnings[i],i+1);
                printf(" \n");

                FILE *f=fopen("earnings.txt","a");
                if (f==NULL){
                    perror("\nERROR! Could not open earnings.txt file.\n");
                    return 1;
                }
                fprintf(f,"Earning from Table %d: %d INR\n",i+1,*table_earnings[i]);
                fclose(f);

                allTablesEmpty[i]=0;
                total_earnings+=*table_earnings[i];
                *table_earnings[i]=0;
            }

            //customers are still ordering
            else if(*table_earnings[i]==1){
                allTablesEmpty[i]=0;
                continue;
            }
        }

        admin_cmd=*cmd_ptr;
        int ctr=0;

        for(int k=0;k<total_tables;k++)
            if(allTablesEmpty[k]==1)
                ctr++;

        //when admin says to close and all tables are empty
        if (ctr==total_tables && admin_cmd==0){
            FILE *f=fopen("earnings.txt","a");
            if (f==NULL){
                perror("ERROR! Could not open earnings.txt file.");
                return 1;
            }

            fprintf(f,"\nTotal Earnings of the Day: %d INR\n",total_earnings);
            fprintf(f,"\nTotal Wages of Waiters: %d INR\n",(int)(0.4*total_earnings));
            fprintf(f,"\nTotal Profit: %d INR\n",total_earnings-(int)(0.4*total_earnings));

            fclose(f);
            terminate=1;
        }
    }

    printf("\n----------Thank you for Visiting the Hotel----------\n");

    //hotel manager's work is over

    //hotel closes here!

    for(int i=0;i<total_tables;i++){
        if(shmdt(table_earnings[i])==-1){
            perror("Error in shmdt() in detaching the memory segment related to Waiter.\n");
            return 1;
        }
    }

    for(int i=0;i<total_tables;i++){
        if(shmctl(shm_ids[i],IPC_RMID,0)==-1){
            perror("Error in shmctl(). Could not remove memory segment related to Waiter.\n");
            return 1;
        }
    }

    //removing admin-manager shared memory segment

    if(shmdt(cmd_ptr)==-1){
			perror("Error in shmdt() in detaching the memory segment related to Admin\n");
			return 1;
		}

    if(shmctl(admin_shmid,IPC_RMID,0)==-1){
            perror("Error in shmctl(). Could not remove memory segment related to Admin.\n");
			return 1;
        }

    return 0;

}