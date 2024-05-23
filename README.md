# Problem Statement
Use the different OS concepts learnt such as Linux commands, process creation, inter-process communication, wait(), sleep(), etc. to create a hotel management system application
## System Overview
The system simulates a hotel environment where the different entities like an Admin, a Hotel Manager, Tables, Customers and Waiters are represented as processes. There are table processes (maximum number of concurrent table processes will be 10) in this system such that each table process represents a table of this hotel. As we all know, customers enter a hotel,  sit at the different tables and order food items from a predefined menu. We will simulate this using processes as well. Each table process will create one or more child processes, each child process representing a customer sitting at that particular table.  We will call each such child process as a customer process. Note that each table process needs to create at least one customer process and can create a maximum of five concurrent customer processes at a time. In this hotel system, we will also have waiter processes. Each waiter process takes orders for a particular table and calculates the total bill amount for that table. Each table has a Number and each waiter has a Waiter ID. A waiter having ID x will only attend table number x. Note that the number of table processes will always be equal to the number of waiter processes. Moreover, the table processes and the waiter processes do not have any parent-child relationship. This implies that during execution, you should run each table process in a different terminal and each waiter process should run in a separate terminal. Additionally, there is a hotel manager process responsible for overseeing the total earnings, calculating the total earnings of all the waiters and handling termination, upon receiving a termination intimation from the admin process. The overall workflow is depicted in the diagram below. Note that the arrows in the figure depict communications between the relevant processes. In the rest of the problem statement, the term user is used to refer to a person who will be executing this application.
![Screenshot 2024-05-22 214212](https://github.com/ani-91/Hotel-Management-System/assets/141425684/e11ffc38-187a-467c-b776-884ff7ce2f83)
### 1. Posix complaint Table.c
- On execution, each instance of this program creates a separate table process, i.e., if the executable file corresponding to table.c is table.out, then each time table.out is executed on a separate terminal, a separate table process is created indicating a new table along with customers (customer process creation is mentioned later) who have entered the hotel and are sitting at that particular table. For every instance of table.out executed on the terminal, the same source code should be used. Multiple source files should not be created for different instances of table processes. 
- When a table process is run, first, it will ask the user to enter a positive integer as its unique table number. The prompt message should be as follows: Enter Table Number: Out of the total table processes being concurrently executed, it is guaranteed that the table number will be sequentially assigned by the user, will be monotonically increasing, will be a positive integer lying in the range 1 to 10 (1 and 10 inclusive) and will  be unique for each table. You can assume that the user always gives a valid input for the table number. 
- Next, the table process requests to know the number of customers sitting at that table by asking the user to enter a positive integer. The prompt message should be as follows: Enter Number of Customers at Table (maximum no. of customers can be 5): For each table process being concurrently executed, this value will be input by the user and will be within the bounds from 1 to 5, both 1 and 5 inclusive. Each customer process will be a child of that table process. Thus, for each customer sitting at the table, the table process will create a child process and a pipe for IPC between the table process and the corresponding customer. It is to be noted that a separate pipe is used for communication between a table process and each one of its customer processes. Thus, if there are four customers at a table, the table process will create four pipes. Note that you will be creating POSIX compliant ordinary pipes here. 
- The table process will read the menu from the menu.txt file which is a pre-created file and will have the format as below. 1. Veg Burger 30 INR 2. Chicken Burger 40 INR 3. Ostrich Eggs 25 INR 4. Egg Frankie 30 INR This menu will be displayed to the user on the terminal in the above format after the user has entered the number of customers at that table. Assume that the file menu.txt is present in the same directory as your assignment source files and is already created. 
- Each customer process will take the input from the user regarding the food item(s) s/he wants to order using the prompt message below. Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: The user will enter the corresponding integer value(s) for each customer process. A customer process is allowed to order multiple food items as well as multiple instances of a single food item (like two veg burgers). For each customer process, the end of order is represented with an input of -1. A customer process may not order anything at all. However, some input(s) has/have to be provided for every customer. All the customer processes will communicate the order items to the corresponding table process using the pipes mentioned in 1.(c). Note that a customer process may try to order (by mistake) an item which is not present in the menu. Handling of this scenario is explained later. 
- Only after gathering the orders from all the customers at the table, the table process communicates the entire order to the waiter process assigned for this table using shared memory. Note that a separate shared memory segment is used for communication between a distinct table process-waiter process pair. Thus, if there are four table processes, there will be four waiter processes and four corresponding shared memory segments. 
- Subsequently, the waiter process will communicate the total bill amount for the order (a valid one) placed to the table process via the shared memory segment. On receiving the total bill amount from the waiter via shared memory (bill calculation is explained later), the table process displays the bill amount in the following format. The total bill amount is X INR.  Once the bill amount has been displayed, it implies that the current set of customers leave the hotel. You are free to handle the termination of the customer processes in an appropriate manner. 
- After this, the table process asks the user if s/he wishes to seat a new set of customers at the table (since the previous set has left) by re-displaying the message in 1.(c). If the value entered by the user is -1, the table process understands that no more customers will be sitting at this table and informs the corresponding waiter  process to terminate. After this communication, the table process terminates as well. If the value entered by the user is between 1 and 5 (both values inclusive), resume normal execution from part 1.(c) onwards. Note that it is guaranteed that the user never enters zero as the number of customers for a table. 
- It is implicit that a table process is never terminated as long as there are customers sitting at the table. 
- You may use your own logic regarding the termination of each customer process. However, note that in case any customer from a given set of customers sitting at a table places an invalid order (by mentioning one or more non-existent food items), all the customers will be asked to place the entire order again.
### 2. Posix complaint waiter.c
- On execution, each instance of this program creates a separate waiter process, i.e., if the executable file corresponding to waiter.c is waiter.out, then each time waiter.out is executed on a separate terminal, a separate waiter process is created representing a new waiter for the corresponding table. For every instance of the waiter.out executed on the terminal, the same source code should be used. Multiple source files should not be created for different instances of waiter processes. -
- When a waiter process is run, first, it will ask the user to enter a positive integer as its unique Waiter ID. The prompt message should be as follows: Enter Waiter ID: Out of the total waiter processes being concurrently executed, the Waiter ID will be sequentially assigned by the user, will be monotonically increasing, will be a positive integer lying in the range 1 to 10 (1 and 10 inclusive) and will  be unique for each waiter. You can assume that the user always gives a valid input for waiter ID, i.e., numbers in the range 1 to 10. There will always be a one-to-one mapping between the number of waiters and the number of tables. A waiter having ID X will only attend table having number X. Note that the number of waiters is exactly equal to the number of tables.  
- Each waiter process receives the order from its corresponding table process using a shared memory segment. Note that for each table-waiter pair, a separate shared memory segment is to be used. These segments have been mentioned in 1.(f) as well. 
- Each waiter process checks whether the order is a valid order i.e., the serial number(s) of all the item(s) in the order exist in the menu as well. If not, the waiter process communicates the same to the corresponding table process via the shared memory segment. The table process then has to retake the entire order (across all customers for that table) by showing the prompt message in 1.(e) again. 
- Once the correct order is received by the waiter, s/he calculates the total bill amount of the table, displays it to the user on the terminal as per the format given below and communicates the amount to the hotel manager process as well as the table process using two different shared memory segments.  Bill Amount for Table X: 60 INR Note that the shared memory segment between the waiter and the table is the same that has been mentioned above in 1.(f). Moreover, each waiter process communicates the bill amount to the hotel manager process using a unique shared memory segment.
- Each waiter process will terminate when the corresponding table process informs it regarding the termination as mentioned in 1.(h) via the shared memory segment. 
- It is implicit that a waiter process is never terminated as long as there are customers sitting at the corresponding table.
### 3. Posix complaint hotelmanager.c
- Only one instance of this program is executed in the entire application.
-  The hotel manager receives the total number of tables at the restaurant from the user by displaying the below prompt message. Enter the Total Number of Tables at the Hotel: Note that this number is guaranteed to be equal to the number of instances of table (as well as waiter) processes to be executed. If the user enters X as the total number of tables, exactly X tables and X waiters should be created. However, not all X instances of tables and waiters are guaranteed to be executed in the beginning itself, i.e., initially, Y out of the X instances may be executed and after sometime, the remaining X - Y instances can be executed. 
- The hotel manager process receives the earnings for a particular table from the corresponding waiter for a single set of customers using shared memory and writes the same into an output file called earnings.txt in the format below. Earning from Table X: 60 INR Earning from Table Y: 50 INR Thus, the file earnings.txt will contain the earnings from different tables of the hotel in separate lines. Note that there should be only one file recording all the earnings. Also, with each waiter process, the hotel manager process communicates using a separate shared memory segment (as mentioned in 2.(e)). Each table will have an earning for each set of customers.
- On receiving intimation of termination (discussed below) from admin process and only when there are no customers at the hotel sitting at any table, the hotel manager calculates the total earnings of the hotel (sum of the individual earnings), the total wages of all the waiters which is 40% of the total earnings of the hotel and the profit made by the hotel which is the difference between the total earnings and total wages. These values are displayed to the user in the below format. Total Earnings of Hotel: 500 INR Total Wages of Waiters: 200 INR Total Profit: 300 INR These values should also be added in the same format to the output file earnings.txt, i.e., first, all individual earnings should be listed and then total earnings, total wages and total profit should be recorded. 
- Before the hotel manager process exits, it should display the terminating message as below (after displaying the earnings, wages and profit message). Thank you for visiting the Hotel! 
- You need to figure out your own logic regarding how the hotel manager process will determine that there are no customers left at the hotel.
### 4. Posix complaint admin.c
- This admin process will keep running along with the table, customer, waiter and hotel manager processes and only one instance of this program is executed. This process will keep displaying a message as: Do you want to close the hotel? Enter Y for Yes and N for No.
- If N is given as input, the admin process keeps running as usual and will not communicate with any other process. If Y is given as input, the admin process will inform the hotel manager process using shared memory that the hotel needs to close. After passing the termination information to the hotel manager, the admin process will terminate. When the hotel manager process receives the termination request from the admin process, it begins the termination task only when there are no more customers at the hotel. The termination task involves 3.(c), 3.(d), cleaning up all the IPC constructs, terminating all the processes and other cleanup tasks as required. 
