#include <stdio.h>         // perror, snprintf
#include <stdlib.h>        // strtol
#include <unistd.h>        // close, write
#include <string.h>        // strlen
#include <time.h>          // time, ctime
#include <sys/socket.h>    // socket, AF_INET, SOCK_STREAM, bind, listen, accept
#include <netinet/in.h>    // servaddr, INADDR_ANY, htons
#include <arpa/inet.h>
#include <pthread.h>
#include <ncurses.h>
#include "Queue.h"
#include "Macros.h"
#include "Colors.h"
#include "Server.h"

// Global Variables
int sockfd = -1;
int connected_clients = 0;
MyQueue* message_q;
Client_ID* client_table[CHATROOM_SIZE];

WINDOW *NOTIFICATIONS_WINDOW;
WINDOW *ERROR_WINDOW;
int X_MAX_SIZE;
int Y_MAX_SIZE;
int NOTIFICATION_WINDOW_MAX_HEIGHT;

// Concurrency
pthread_mutex_t queue_mutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_table_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t client_count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condc              = PTHREAD_COND_INITIALIZER;
pthread_cond_t  condp              = PTHREAD_COND_INITIALIZER;

// Messages
const char* room_filled = "Chat room has exceeded the max number of users.\n";

void init_windows() {
    // Start it up
    initscr();
    start_color();
    INIT_COLOR_PAIRS;
    refresh();

    // Get current size of the screen/terminal
    getmaxyx(stdscr, Y_MAX_SIZE, X_MAX_SIZE);

    NOTIFICATION_WINDOW_MAX_HEIGHT = Y_MAX_SIZE * 0.90;

    ERROR_WINDOW = newwin(Y_MAX_SIZE - NOTIFICATION_WINDOW_MAX_HEIGHT, X_MAX_SIZE, NOTIFICATION_WINDOW_MAX_HEIGHT, 0);
    NOTIFICATIONS_WINDOW = newwin(NOTIFICATION_WINDOW_MAX_HEIGHT, X_MAX_SIZE, 0, 0);

    scrollok(ERROR_WINDOW, true);
    scrollok(NOTIFICATIONS_WINDOW, true);

    wbkgd(NOTIFICATIONS_WINDOW, COLOR_PAIR(GREEN_ON_BLACK));
    wbkgd(ERROR_WINDOW, COLOR_PAIR(RED_ON_BLACK));

    wrefresh(NOTIFICATIONS_WINDOW);
    wrefresh(ERROR_WINDOW);
    refresh();
}

int run_c_server(int argc, char **argv) {
    init_windows();
    int port_number = PORT_NUM;
    message_q = init_queue();
    if (!message_q) {
        wprintw(ERROR_WINDOW, "Could not allocate memory for the message queue.\n");
        wprintw(ERROR_WINDOW,"Press any key to exit server.");
        wrefresh(ERROR_WINDOW);
        clean_up_windows();
        exit(EXIT_FAILURE);
    }

    // Process arguments, if there are any
    if (argc == 2) {;
        port_number = strtol(argv[1], NULL, 10);
    }
    createServer(port_number);
    connectServer();
    free(message_q);
}

void createServer(int port_number) { 
    struct sockaddr_in	servaddr;
    // 1. Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        wprintw(ERROR_WINDOW, "Failed to create socket.\n");
        wprintw(ERROR_WINDOW,"Press any key to exit server.");
        wrefresh(ERROR_WINDOW);
        clean_up_windows();
        exit(EXIT_FAILURE);
    }

    // 2. Set up the sockaddr_in
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;               // Specify the family
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);     // use any available
    servaddr.sin_port        = htons(port_number);	  // port to listen on

    // 3. "Bind" that address object to our listening file descriptor
    if ((bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
        wprintw(ERROR_WINDOW, "Failed to bind.\n");
        wprintw(ERROR_WINDOW,"Press any key to exit server.");
        wrefresh(ERROR_WINDOW);
        clean_up_windows();
        exit(EXIT_FAILURE);
    }

    // 4. Tell the system that we are going to use this sockect for listening
    if (listen(sockfd, 0) < 0) {
        wprintw(ERROR_WINDOW, "Failed to listen.\n");
        wprintw(ERROR_WINDOW,"Press any key to exit server.");
        wrefresh(ERROR_WINDOW);
        clean_up_windows();
        exit(EXIT_FAILURE);
    }
    wprintw(NOTIFICATIONS_WINDOW, "Chat room has been created.\n");
    wprintw(NOTIFICATIONS_WINDOW, "---------------------------\n");
    wrefresh(NOTIFICATIONS_WINDOW);
}

void connectServer() {
    char cli_username[USERNAME_LEN];
    int connfd;
    struct sockaddr_in	cliaddr;
    socklen_t clisize = sizeof(cliaddr);
    // Create thread for the consumer
    pthread_t tid;
    pthread_create(&tid, NULL, consumer, NULL); 
    pthread_detach(tid);   

    // Loop forever
    for ( ; ; ) {
        memset(&cliaddr, 0, sizeof(cliaddr));
        if ((connfd =  accept(sockfd, (struct sockaddr*) &cliaddr,  &clisize)) < 0) {
            wprintw(ERROR_WINDOW, "Failed to accept incoming connection.\n");
            wprintw(ERROR_WINDOW,"Press any key to exit server.");
            wrefresh(ERROR_WINDOW);
            clean_up_windows();
            exit(EXIT_FAILURE);
        }
        else {
            // Create a new thread for the new connection
            pthread_t tid;
            memset(&cli_username, 0, USERNAME_LEN);
            if (read(connfd, cli_username, USERNAME_LEN) < 0) {
                wprintw(ERROR_WINDOW, "Failed to read username of new client\n");
                wrefresh(ERROR_WINDOW);
                continue;
            }
            Client_ID* new_client = (Client_ID*) malloc(sizeof(Client_ID));
            strcpy(new_client->username, cli_username);
            new_client->connfd = connfd;
            pthread_create(&tid, NULL, producer, (void*) new_client);
            pthread_detach(tid);
        }
    }
}

void* producer(void* arg) {
    Client_ID* new_client = (Client_ID*) arg;
    pthread_mutex_lock(&client_count_mutex);
    pthread_mutex_lock(&client_table_mutex);
    if (connected_clients > CHATROOM_SIZE) {
        printf("%s",room_filled);
        if (write(new_client->connfd, room_filled, strlen(room_filled)) < 0) {
            wprintw(ERROR_WINDOW, "Failed to write to client.\n");
            wrefresh(ERROR_WINDOW);
        }
        close(sockfd);
        free(new_client);
        pthread_mutex_unlock(&client_table_mutex);    
        pthread_mutex_unlock(&client_count_mutex); 
        pthread_exit(0);
    } else {
        wattron(NOTIFICATIONS_WINDOW, COLOR_PAIR(CYAN_ON_BLACK));
        wprintw(NOTIFICATIONS_WINDOW, "(+) %s has entered the chat.\n", new_client->username);
        wrefresh(NOTIFICATIONS_WINDOW);
        // Add the next client to the client_table
        for (size_t i = 0; i < CHATROOM_SIZE; ++i) {
            if (!client_table[i]) {
                client_table[i] = new_client;
                break;
            }
        }
        connected_clients++; 
    }
    pthread_mutex_unlock(&client_table_mutex);    
    pthread_mutex_unlock(&client_count_mutex);

    // Alert everyone that there is a new client online
    alert_users(new_client);

    // Before handling input, alert the new client of everyone who is online
    get_all_online(new_client);  

    // Handle input from the client
    char buffer[BUFFSIZE];
    memset(&buffer, 0, BUFFSIZE);
    while (read(new_client->connfd, buffer, MAXLINE) > 0) {
        if (strstr(buffer, ": signing off\n")) break;
        pthread_mutex_lock(&queue_mutex);
        while (is_full(message_q)) {
            pthread_cond_wait(&condp, &queue_mutex);
        }
        enqueue(message_q, buffer);
        memset(&buffer, 0, BUFFSIZE);
        pthread_cond_signal(&condc);
        pthread_mutex_unlock(&queue_mutex);
    }
    remove_client(new_client);
    pthread_exit(0);
}

void* consumer(void* arg) {
    char buffer[MAXLINE];
    for (;;) {
        pthread_mutex_lock(&queue_mutex);
        while (is_empty(message_q)) {
            pthread_cond_wait(&condc, &queue_mutex);
        }
        while (!(is_empty(message_q))) {
            memset(&buffer, 0, MAXLINE);
            strcpy(buffer, dequeue(message_q));
            pthread_mutex_lock(&client_table_mutex);
            // Could probably optimize this part with a nice STL container O_O
            for (size_t i = 0; i < CHATROOM_SIZE; ++i) {
                if (client_table[i]) {
                    if (write(client_table[i]->connfd, buffer, strlen(buffer)) < 0) {
                        wprintw(ERROR_WINDOW, "Failed to write to consumer thread.\n");
                        wrefresh(ERROR_WINDOW);
                        continue;
                    }
                }
            }
            pthread_mutex_unlock(&client_table_mutex);
        }
        pthread_cond_signal(&condp);
        pthread_mutex_unlock(&queue_mutex);
    }
    pthread_exit(0);
}

void remove_client(Client_ID* client) {
    char buffer[MAXLINE];
    pthread_mutex_lock(&client_count_mutex);
    pthread_mutex_lock(&client_table_mutex);
    // Remove from the table
    for (size_t i = 0; i < CHATROOM_SIZE; ++i) {
        if (client_table[i] == client) {
            wattron(NOTIFICATIONS_WINDOW, COLOR_PAIR(MAGENTA_ON_BLACK));
            wprintw(NOTIFICATIONS_WINDOW, "(-) %s has left the chat.\n",client_table[i]->username);
            wrefresh(NOTIFICATIONS_WINDOW);
            client_table[i] = NULL;
        } 
        else if (client_table[i]) {
            memset(buffer, 0, MAXLINE);
            strcat(buffer, "3$23%#$^$21@");
            strcat(buffer, client->username);
            if (write(client_table[i]->connfd, buffer, strlen(buffer)) < 0) {
                wprintw(ERROR_WINDOW,"Failed to write when removing client.\n");
                wrefresh(ERROR_WINDOW);
                continue;
            }
        }
    }
    free(client); 
    connected_clients--;  
    pthread_mutex_unlock(&client_table_mutex);    
    pthread_mutex_unlock(&client_count_mutex);
}


void get_all_online(Client_ID* new_client) {
    pthread_mutex_lock(&client_table_mutex);
    char buffer[MAXLINE];
    memset(buffer, 0, MAXLINE);

    strcat(buffer, "@##$%23443$2");

    for (size_t i = 0; i < CHATROOM_SIZE; ++i) {
        if (client_table[i]) {
           strcat(buffer, "(+) "); 
           strcat(buffer, client_table[i]->username);
           strcat(buffer, "\n");
        }
    }

    if (write(new_client->connfd, buffer, strlen(buffer)) < 0) {
        wprintw(ERROR_WINDOW, "Failed to write to new user.\n");
        wrefresh(ERROR_WINDOW);
    }
    pthread_mutex_unlock(&client_table_mutex);
}

void alert_users(Client_ID* new_client) {
    pthread_mutex_lock(&client_table_mutex);
    char buffer[MAXLINE];
        for (size_t i = 0; i < CHATROOM_SIZE; ++i) {
        if (client_table[i] && client_table[i] != new_client) {
            memset(buffer, 0, MAXLINE);
            strcat(buffer, "#$?!39#21@#$");
            strcat(buffer, new_client->username);
            if (write(client_table[i]->connfd, buffer, strlen(buffer)) < 0) {
                wprintw(ERROR_WINDOW, "Failed to write to current user of new user.\n");
                wrefresh(ERROR_WINDOW);
            }
        }
    }
    pthread_mutex_unlock(&client_table_mutex);
}

void clean_up_windows() {
    if (message_q) {
        free(message_q);
    }
    delwin(NOTIFICATIONS_WINDOW);
    delwin(ERROR_WINDOW);
    getch();
    endwin();
}