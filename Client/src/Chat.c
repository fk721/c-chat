#include <stdlib.h>         // exit
#include <stdlib.h>         // strtol
#include <stdio.h>          // printf, fprintf, stderr
#include <unistd.h>         // read
#include <string.h>         // memset
#include <sys/socket.h>     // socket, AF_INET, SOCK_STREAM
#include <arpa/inet.h>      // inet_pton
#include <netinet/in.h>     // servaddr
#include <netdb.h>          // getaddrinfo
#include <arpa/inet.h>      // inet_ntop
#include <sys/select.h>     // fd_set
#include <ncurses.h>
#include "Chat.h"
#include "Macros.h"
#include "Colors.h"

/* Global Variables */
WINDOW *USER_INPUT_WINDOW;
WINDOW *CHAT_WINDOW;
WINDOW *ERROR_WINDOW;
WINDOW *STATUS_WINDOW;

int X_MAX_SIZE;
int Y_MAX_SIZE;
int sockfd = -1;


void run_c_chat(int argc, char **argv) {
    freopen("/dev/tty", "rw", stdin);
    // Initialize the screen
    initscr();
    start_color();
    INIT_COLOR_PAIRS;
    refresh();

    // Function to initalize the windows
    init_windows();

    char* client_name;
    int port_number = PORT_NUM;

    // Process arguments
    if (argc == 1) {
        wprintw(ERROR_WINDOW,"Cannot enter chatroom without providing a username.\n");
        wprintw(ERROR_WINDOW,"Press any key to continue.\n");
        wrefresh(ERROR_WINDOW);
        cleanup_windows();
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        client_name = argv[1];
    } if (argc == 4) {
        client_name = argv[1];
        getHostbyName(argv[2]);
        port_number = strtol(argv[3], NULL, 10);
    }

    connectToServer(port_number);
    write(sockfd, client_name, strlen(client_name));
    int chat_window_max_height = Y_MAX_SIZE * MAX_CHAT_PROP_H;
    move(chat_window_max_height, 0);
    refresh();
    chat(client_name);
}


void init_windows() {
    // Get current size of the screen/terminal
    getmaxyx(stdscr, Y_MAX_SIZE, X_MAX_SIZE);
    
    int chat_window_max_height = Y_MAX_SIZE * MAX_CHAT_PROP_H;
    int chat_window_max_width  = X_MAX_SIZE * MAX_CHAT_PROP_W;
    int status_window_max_width = X_MAX_SIZE * (1 - MAX_CHAT_PROP_W);
    int input_window_max_height = Y_MAX_SIZE * MAX_INPUT_PROP_H;
    int error_window_max_height = Y_MAX_SIZE - chat_window_max_height - input_window_max_height;

    CHAT_WINDOW = newwin(chat_window_max_height, chat_window_max_width, 0, 0);
    STATUS_WINDOW = newwin(chat_window_max_height, status_window_max_width, 0, chat_window_max_width);
    USER_INPUT_WINDOW = newwin(input_window_max_height, X_MAX_SIZE, chat_window_max_height, 0);
    ERROR_WINDOW = newwin(error_window_max_height, X_MAX_SIZE, chat_window_max_height + input_window_max_height, 0);
    
    // box(CHAT_WINDOW, 0, 0);
    // box(STATUS_WINDOW, 0, 0);
    // box(USER_INPUT_WINDOW, 0, 0);
    // box(ERROR_WINDOW, 0, 0);
    
    wbkgd(CHAT_WINDOW, COLOR_PAIR(WHITE_ON_BLUISH));
    wbkgd(STATUS_WINDOW, COLOR_PAIR(GREEN_ON_BLACK));
    wbkgd(USER_INPUT_WINDOW, COLOR_PAIR(WHITE_ON_GREY));
    wbkgd(ERROR_WINDOW, COLOR_PAIR(RED_ON_BLACK));

    scrollok(CHAT_WINDOW, true);
    scrollok(STATUS_WINDOW, true);
    scrollok(USER_INPUT_WINDOW, true);
    scrollok(ERROR_WINDOW, true);

    wrefresh(CHAT_WINDOW);
    wrefresh(STATUS_WINDOW);
    wrefresh(USER_INPUT_WINDOW);
    wrefresh(ERROR_WINDOW);

    // move(chat_window_max_height + 1, 1);
    refresh();
}

void getHostbyName(char* name) {
    struct addrinfo* results;
    struct addrinfo hint;
    char ADDR_P[INET_ADDRSTRLEN] = LOCAL_HOST;

    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_family = AF_INET;

    getaddrinfo(name, NULL, &hint, &results);
    struct addrinfo *rp = results;
    struct sockaddr_in*  p = (struct sockaddr_in*)rp->ai_addr;
    inet_ntop(AF_INET, &p->sin_addr.s_addr, ADDR_P, INET_ADDRSTRLEN);
}

void connectToServer(int port_num) {
    struct sockaddr_in	servaddr;
    char ADDR_P[INET_ADDRSTRLEN] = LOCAL_HOST;
    
    // 1. Create the socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        wprintw(ERROR_WINDOW,"Failed to create socket.\n");
        wprintw(ERROR_WINDOW,"Press any key to continue.\n");
        wrefresh(ERROR_WINDOW);
        cleanup_windows();
        exit(EXIT_FAILURE);
    }

    // 2. Set up the sockaddr_in
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;           // Specify the family
    servaddr.sin_port        = htons(port_num);	  // port to listen on
    if (inet_pton(AF_INET, ADDR_P, &servaddr.sin_addr.s_addr) <= 0) {
        wprintw(ERROR_WINDOW,"Failed to setup sockaddrin.\n");
        wprintw(ERROR_WINDOW,"Press any key to continue.\n");
        wrefresh(ERROR_WINDOW);
        cleanup_windows();
        exit(EXIT_FAILURE);
    }

    // 3. Connect
    if ((connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))) < 0) {
        wprintw(ERROR_WINDOW,"Failed to connect to server.\n");
        wprintw(ERROR_WINDOW,"Press any key to continue.\n");
        wrefresh(ERROR_WINDOW);
        cleanup_windows();
        exit(EXIT_FAILURE);
    }
    wprintw(STATUS_WINDOW, "Connected!\n");
    wrefresh(STATUS_WINDOW);
}

void chat(char* username) {
    char buffer[BUFFSIZE];
    fd_set fds;
    int n;
    int chat_window_max_height = Y_MAX_SIZE * MAX_CHAT_PROP_H;
    while (1) {
        move(chat_window_max_height, 0);
        refresh();
        // Set up our set of file descriptiors
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(sockfd, &fds);
        if (select(FD_SETSIZE, &fds, NULL, NULL, NULL) > 0) {
            // Clear our buffer
            memset(buffer, 0, BUFFSIZE);
            // Can we read from standard input?
            if(FD_ISSET(STDIN_FILENO, &fds)) {
                // wgetnstr(USER_INPUT_WINDOW, buffer, MAXLINE);
                // if ((n = read(STDIN_FILENO, buffer, MAXLINE)) < 0) {
                if ((n = wgetnstr(USER_INPUT_WINDOW, buffer, MAXLINE) < 0)) {
                    wprintw(ERROR_WINDOW,"Failed to read from standard input.\n");
                    wprintw(ERROR_WINDOW,"Press any key to continue.\n");
                    wrefresh(ERROR_WINDOW);
                    cleanup_windows();
                    exit(EXIT_FAILURE);
                }
                if (strncmp(buffer, "signing off", 11) == 0) {
                    cleanup_windows();
                    clean();
                    break;
                };

                werase(USER_INPUT_WINDOW);
                wrefresh(USER_INPUT_WINDOW);

                add_message_padding(buffer, username);

                if ((n = write(sockfd, buffer, strlen(buffer))) < 0) {
                    wprintw(ERROR_WINDOW,"Failed to write to server.\n");
                    wprintw(ERROR_WINDOW,"Press any key to continue.\n");
                    wrefresh(ERROR_WINDOW);
                    cleanup_windows();
                    exit(EXIT_FAILURE);
                }
            }

            move(chat_window_max_height, 0);

            // Clear our buffer
            memset(buffer, 0, BUFFSIZE);

            // Can we read from the server?
            if(FD_ISSET(sockfd, &fds)) {
                if ((n = read(sockfd, buffer, MAXLINE)) < 0) {
                    wprintw(ERROR_WINDOW,"Failed to read from the server.\n");
                    wprintw(ERROR_WINDOW,"Press any key to continue.\n");
                    wrefresh(ERROR_WINDOW);
                    cleanup_windows();
                    exit(EXIT_FAILURE);
                } if (n == 0) {
                    wprintw(ERROR_WINDOW,"Chatroom has shut down.\n");
                    wprintw(ERROR_WINDOW,"Press any key to exit..\n");
                    wrefresh(ERROR_WINDOW);
                    cleanup_windows();
                    clean();
                    break;
                }

                // Filter messages from the server accordingly, using 'magic numbers'?

                // This indicates a new user has entered the chat.
                if (strncmp(buffer, "#$?!39#21@#$", 12) == 0) {
                    wattron(STATUS_WINDOW, COLOR_PAIR(CYAN_ON_BLACK));
                    wprintw(STATUS_WINDOW, "(+) %s\n", buffer + 12);
                    wrefresh(STATUS_WINDOW);
                }
                // This indicates a user has left the chat.
                else if (strncmp(buffer, "3$23%#$^$21@", 12) == 0) {
                    wattron(STATUS_WINDOW, COLOR_PAIR(MAGENTA_ON_BLACK));
                    wprintw(STATUS_WINDOW, "(-) %s\n", buffer + 12);
                    wrefresh(STATUS_WINDOW);
                }
                // Display all currently logged on users
                else if (strncmp(buffer, "@##$%23443$2",12) == 0) {
                    wattron(STATUS_WINDOW, COLOR_PAIR(CYAN_ON_BLACK));
                    wprintw(STATUS_WINDOW, "%s", buffer + 12);
                    wrefresh(STATUS_WINDOW);
                }
                // For regular messages
                else {
                    wprintw(CHAT_WINDOW, "%s\n", buffer);
                    wrefresh(CHAT_WINDOW);
                }
            }  
        }
    }
}

void add_message_padding(char* buffer, char* username) {
    size_t username_len = strlen(username);
    memmove(buffer + username_len + 2, buffer, strlen(buffer));
    for (size_t i = 0; i < username_len; ++i) {
        buffer[i] = username[i];
    }
    buffer[username_len] = ':' ;
    buffer[username_len + 1] = ' ';
}

void clean() {
    if (sockfd > 0) {
        close(sockfd);
        shutdown(sockfd, SHUT_RDWR);
        printf("Chat has ended.\n");
    }
    exit(EXIT_SUCCESS);
}

void cleanup_windows() {
    getch();
    delwin(CHAT_WINDOW);
    delwin(STATUS_WINDOW);
    delwin(USER_INPUT_WINDOW);
    delwin(ERROR_WINDOW);
    endwin();
}