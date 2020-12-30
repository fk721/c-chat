#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <ncurses.h>
#include "Queue.h"
#include "Macros.h"

/// Struct to represent a client.
/// A client contains a username, as well as a file descriptor that we write to.
typedef struct {
    int connfd;
    char username[USERNAME_LEN];
} Client_ID; 

/* ------------------- GLOBAL VARIABLES ------------------- */

/// sockfd defines the file descriptor of the server that is created.
extern int sockfd;

/// The number of connected clients currently in the chat room.
extern int connected_clients;

/// Queue which contains user messages, see `Queue.h` and `Queue.c` for more details.
extern MyQueue* message_q;

/// Static array (max size defined in `Macros.h`) that contain the clients currently in the chat room.
extern Client_ID* client_table[CHATROOM_SIZE];

/// ncurses window to display new clients.
extern WINDOW *NOTIFICATIONS_WINDOW;

/// ncurses window to display any errors (perror).
extern WINDOW *ERROR_WINDOW;

/// Max width size of the terminal.
extern int X_MAX_SIZE;

/// Max length of the terminal.
extern int Y_MAX_SIZE;

/// Max height of the notification window.
extern int NOTIFICATION_WINDOW_MAX_HEIGHT;

/* ------------------- THREAD RELATED ------------------- */

/// Mutex to protect `message_q`, a global variable.
extern pthread_mutex_t queue_mutex;

/// Mutext to protect `client_table`, a global variable.
extern pthread_mutex_t client_table_mutex;

/// Mutext to protect `connected clients`, a global variable.
extern pthread_mutex_t client_count_mutex;

/// Condition variable, waits when the message queue is empty.
/// Singals `condp` when all the messages have been dequeued and the queue is empty.
extern pthread_cond_t  condc;             

/// Condition variable, waits when the message queue is full.
/// Singals `condc` when a message has been added to the queue.
extern pthread_cond_t  condp; 

/* ------------------- SOME STRINGS ------------------- */

/// Messages to display in the instance in which the chat server is filled to capacity (size defined in `Macros.h`)
/// or when a new client logs in.
extern const char* room_filled;
extern const char* logged_in;

/* ------------------- PROTOTYPES ------------------- */

/// Initializes the two ncurses windows, `NOTIFICATIONS_WINDOW` as well as `ERROR_WINDOW`
void init_windows();

/// Extracts the port number for the server (if there is one provided).
/// Initializes the message queue.
int run_c_server(int argc, char **argv);

/// Basic networking, the server is created.
/// Create a socket, set up the sockaddr_in, bind that address, and have that socket ready for listening.
void createServer(int port_number);

/// Accept connections from incoming clients, indefinitely.
/// Creates one consumer, but MANY producers (depending on the number of clients).
/// One producer thread per client.
void connectServer();

/// Adding a new client to our client_table.
/// User input is handled by the producer, which puts user messages into the message queue.
/// In the queue is full, the consumer thread is prompted to run. 
void* producer(void* arg);

/// The consumer dequeues messages from the queue, and writes the dequeued message to each client that is online.
/// If the queue is empty, the producer thread is prompted to run.
void* consumer(void* arg);

/// After a client leave the chat, remove that client from the table.
void remove_client(Client_ID* client);

/// Alert the user who has just entered the chat room of those who are already online.
void get_all_online(Client_ID* new_client);

/// Alert all users logged in of the user who just entered the chat.
void alert_users(Client_ID* new_client);

/// Deallocate memory for ncurses windows.
void clean_up_windows();


#endif