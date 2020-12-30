#ifndef CHAT_H
#define CHAT_H

#include <ncurses.h>

/* Global Variables */
extern WINDOW *USER_INPUT_WINDOW;
extern WINDOW *CHAT_WINDOW;
extern WINDOW *ERROR_WINDOW;
extern WINDOW *STATUS_WINDOW;

extern int X_MAX_SIZE;
extern int Y_MAX_SIZE;

/// Extracts user arguments (username, client name/host number, port number) in that order.
/// At minimum, the username must be provided.
void run_c_chat(int argc, char **argv);

/// Initializes the four ncurses windows.
void init_windows();

/// Given a string, get the IP address.
void getHostbyName(char* name);

/// Connect to our chat server.
void connectToServer(int port_num);

/// Where the back and forth between the server and client occurs.
void chat(char* username);

/// Helper function to produce cleaner looking messages.
/// Adds "username: " to the beggining of the buffer.
void add_message_padding(char* buffer, char* username);

/// Close the fd of the socket properly
void clean();

/// Deallocate memory for ncurses windows.
void cleanup_windows();

#endif