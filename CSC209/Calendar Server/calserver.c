// IMPORTANT
// PLEASE NOTE CODE WHICH WAS TAKEN FROM muffinman.c GIVEN IN THE STARTER CODE.
// ALL CODE TAKEN FROM THIS FILE IS NOTED WHERE NECESSARY
//
// I HAVE COMMENTED VARIOUS SECTIONS OF THE muffinman.c CODE IN THIS FILE IN ORDER TO
// SHOW ANY POTENTIAL MARKER THAT I UNDERSTAND WHAT THE CODE DOES

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lists.h"
//Server-related headers
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/signal.h>

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 8
#define DELIM " \r\n"

#ifndef PORT
  #define PORT 53926
#endif

// Global variable to our listen file descriptor for access between functions
static int listenfd;
// Global variable to always know how many people are connected
int numClients = 1;

// Create the heads of the two empty data-structures 
Calendar *calendar_list = NULL;
User *user_list = NULL;

// A client struct is needed to manage multiple clients' connections, 
// usernames, and other pertinent information
struct client {
    int fd;
    char *userName;
    char *buf;
    int inbuf;
    struct in_addr ipaddr;
    struct client *next;
} *top = NULL;

static char greeting[] = "What is your user name?\r\n";
static char prompt[] = "Go ahead and enter calendar commands>\r\n";

//Functions take from muffinman.c for server fucntionality
void bindandlisten();
void newconnection();
void whatsup(struct client *p);
static struct client *addclient(int fd, struct in_addr addr);
static void removeclient(int fd);
static void broadcast(struct client *client, char *s, int size);

int find_network_newline(char *buf, int inbuf);
static void broadcastToName(char *name, char *message, int size);

/* 
 * FROM ORIGINAL calendar.c
 * Return a calendar time representation of the time specified
 *  in local_args. 
 *    local_argv[0] must contain hour. 
 *    local_argv[1] may contain day, otherwise use current day
 *    local_argv[2] may contain month, otherwise use current month
 *    local_argv[3] may contain day, otherwise use current year
 */
time_t convert_time(int local_argc, char** local_argv) {

   time_t rawtime;
   struct tm * info;  

   // Instead of expicitly setting the time, use the current time and then
   // change some parts of it.
   time(&rawtime);            // get the time_t represention of the current time 
   info = localtime(&rawtime);   

   // The user must set the hour.
   info->tm_hour = atof(local_argv[0]);
   
   // We don't want the current minute or second. 
   info->tm_min = 0;
   info->tm_sec = 0;

   if (local_argc > 1) {
       info->tm_mday = atof(local_argv[1]);
   }
   if (local_argc > 2) {
       // months are represented counting from 0 but most people count from 1
       info->tm_mon = atof(local_argv[2])-1;
   }
   if (local_argc > 3) {
       // tm_year is the number of years after the epoch but users enter the year AD
       info->tm_year = atof(local_argv[3])-1900;
   }

   // start off by assuming we won't be in daylight savings time
   info->tm_isdst = 0;
   mktime(info);
   // need to correct if we guessed daylight savings time incorrect since
   // mktime changes info as well as returning a value. Check it out in gdb.
   if (info->tm_isdst == 1) {
       // this means we guessed wrong about daylight savings time so correct the hour
       info->tm_hour--;
   }
   return  mktime(info);
}


/* 
 * FROM ORIGINAL calendar.c
 * Read and process calendar commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(struct client *p, int cmd_argc, char **cmd_argv, Calendar **calendar_list_ptr, User **user_list_ptr) {

    char *message;
    char message2[INPUT_BUFFER_SIZE];
    Calendar *calendar_list = *calendar_list_ptr; 
    User *user_list = *user_list_ptr;

    fprintf(stderr, "%s\n", cmd_argv[0]);

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
        
    } else if (strcmp(cmd_argv[0], "add_calendar") == 0 && cmd_argc == 2) {
        if (add_calendar(calendar_list_ptr, cmd_argv[1]) == -1) {
            message = "Calendar by this name already exists\r\n";
            broadcast(p, message, strlen(message));
        }
        
    } else if (strcmp(cmd_argv[0], "list_calendars") == 0 && cmd_argc == 1) {
        message = list_calendars(calendar_list);
        broadcast(p, message, strlen(message));
        
    } else if (strcmp(cmd_argv[0], "add_event") == 0 && cmd_argc >= 4) {
        // Parameters for convert_time are the values in the array
        // cmd_argv but starting at cmd_argv[3]. The first 3 are not
        // part of the time.
        // So subtract these 3 from the count and pass the pointer 
        // to element 3 (where the first is element 0).
        time_t time = convert_time(cmd_argc-3, &cmd_argv[3]);

        if (add_event(calendar_list, cmd_argv[1], cmd_argv[2], time) == -1) {
           message = "Calendar by this name does not exist\r\n";
           broadcast(p, message, strlen(message));
        }

    } else if (strcmp(cmd_argv[0], "list_events") == 0 && cmd_argc == 2) {
        message = list_events(calendar_list, cmd_argv[1]);
        broadcast(p, message, strlen(message));

    } else if (strcmp(cmd_argv[0], "add_user") == 0 && cmd_argc == 2) {
        if (add_user(user_list_ptr, cmd_argv[1]) == -1) {
                message = "User by this name already exists\r\n";
                broadcast(p, message, strlen(message));
        } else {
            sprintf(message2, "User %s added.\r\n", cmd_argv[1]);
            broadcast(p, message2, strlen(message2));
        }

    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        message = list_users(user_list);
        broadcast(p, message, strlen(message));
        
    } else if (strcmp(cmd_argv[0], "subscribe") == 0 && cmd_argc == 3) {
        int return_code = subscribe(user_list, calendar_list, cmd_argv[1], cmd_argv[2]);
        if (return_code == -1) {
            message = "User by this name does not exist\r\n";
            broadcast(p, message, strlen(message));
        } else if (return_code == -2) {
            message = "Calendar by this name does not exist\r\n";
            broadcast(p, message, strlen(message));
        } else if (return_code == -3) {
            message = "This user is already subscribed to this calendar\r\n";
            broadcast(p, message, strlen(message));
        }
        sprintf(message2, "You have been subscribed to calendar %s \r\n", cmd_argv[2]);
        broadcastToName(cmd_argv[1], message2, strlen(message));
      
    } else {
        message = "Incorrect syntax\r\n";
        broadcast(p, message, strlen(message));
    }
    return 0;
}


int main(int argc, char* argv[]) {
    struct client *p;

    /* Bind and listen on a socket.
     * By binding we associate a particular address to this server.
     * By listening to wait for incoming connections on this particular socket.
     */
    bindandlisten();

    //We establish the server and it will only close when the process is killed
    while (1) {
        fd_set fdlist;
        //Initialize mxfd to listenfd every while iteration (having no clients will keep it this way)
        int maxfd = listenfd;
        FD_ZERO(&fdlist);
        FD_SET(listenfd, &fdlist);
        //Iterate across all clients until we get to the most recent client. It becomes maxfd
        for (p = top; p; p = p->next) {
            FD_SET(p->fd, &fdlist);
            if (p->fd > maxfd) {
                maxfd = p->fd;
            }
        }
        //Select over our file descriptor list and get any ready file descriptors
        if (select(maxfd + 1, &fdlist, NULL, NULL, NULL) < 0) {
            perror("select");
        } else {
            for (p = top; p; p = p->next) {
                if (FD_ISSET(p->fd, &fdlist)) {
                    break;
                }
            }      
            if (p) {
                whatsup(p);  // might remove p from list, so can't be in the loop
            }
            // If someone is not in our list but has connected then
            // we must request them to become a client
            if (FD_ISSET(listenfd, &fdlist)) {
                newconnection();
            }
        }
    }
    return 0;
 }

//Bind and listen on a socket; abort on error
//TAKEN FROM muffinman.c IN THE STARTER CODE
void bindandlisten()  {
    struct sockaddr_in r;

    //Assign a file descriptor to the socket that clients will connect to
    //Essentially: Establish a socket that listens to the internet and from all interfaces
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    //This is the code provided from the assignment page to release the server port
    int on = 1;
    int status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
                            (const char *) &on, sizeof(on));

    if(status == -1) {
        perror("setsockopt -- REUSEADDR");
    }

    //Clear the memory at the sockaddr_in for security
    memset(&r, '\0', sizeof r);

    //Set the values of the sockaddr_in:
    //Stores address for the internet address family
    r.sin_family = AF_INET;
    //Accepts connections from all interfaces
    r.sin_addr.s_addr = INADDR_ANY;
    //Accepts connections specifically from this port
    r.sin_port = htons(PORT);

    //Bind the address of 'r' to our socket 'listenfd'
    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
        perror("bind");
        exit(1);
    }

    //Enables the socket to accept connections
    if (listen(listenfd, 5)) {
        perror("listen");
        exit(1);
    }
}

//TAKEN FROM muffinman.c IN THE STARTER CODE
//Accept a new connection and then instantiate the client's details in struct
void newconnection() {
    int fd;
    struct sockaddr_in r;
    socklen_t socklen = sizeof r;
    // Accept the connection on our socket from the new client
    if ((fd = accept(listenfd, (struct sockaddr *)&r, &socklen)) < 0) {
        perror("accept");
    } else {
        printf("connection from %s\n", inet_ntoa(r.sin_addr));
        addclient(fd, r.sin_addr);
    }
    // Issue them a greeting and empty a buffer for them to use
    write(fd, greeting, sizeof greeting);
}

//Check activity in the select() call
void whatsup(struct client *p) {
    char buf[INPUT_BUFFER_SIZE];
    int len;
    int where;
    memset(buf, '\0', sizeof(buf));
    // Read whatever a single client gave us and stick it in their buffer
    if ((len = read(p->fd, p->buf, INPUT_BUFFER_SIZE)) < 0) {
        perror("read");
    } else if (len == 0) {
        removeclient(p->fd);
    }
    p->inbuf += len;
    // If there is a network new line then it's time to process a command
    where = find_network_newline(p->buf, p->inbuf);
    // Here we will handle all interaction with a network newline
    // Also, we process user commands and user name input
    if (where) {
        // If they have a username then they entered a legitimate command
        if (p->userName) {
            char *next_token = strtok(p->buf, DELIM);
            // for holding arguments to individual commands passed to sub-procedure
            char *cmd_argv[INPUT_ARG_MAX_NUM];
            memset(cmd_argv, '\0', sizeof(cmd_argv));
            int cmd_argc = 0;
            while (next_token != NULL) {
                if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
                    char *message = "Too many arguments!";
                    broadcast(p, message,strlen(message));
                    cmd_argc = 0;
                    break;
                }
            cmd_argv[cmd_argc] = next_token;
            cmd_argc++;
            next_token = strtok(NULL, DELIM);
            }
            if (cmd_argc > 0 && process_args(p, cmd_argc, cmd_argv, &calendar_list, &user_list) == -1) {
                printf("Disconnect from %s\r\n", inet_ntoa(p->ipaddr));
                removeclient(p->fd);
            }
            broadcast(p, ">", 2);
        // If the user did not have a name they just gave use their name
        } else {
            // Try to allocate room for their name
            if ((p->userName = malloc((sizeof(buf)))) == NULL) {
                perror("malloc");
                exit(1);
            }
            broadcast(p, prompt, strlen(prompt));
            broadcast(p, ">\r\n", 2);
        }
        // Move all the memory from the left over received data
        memmove(p->buf, (p->buf + where + 4), (INPUT_BUFFER_SIZE - (p->inbuf)));
        p->inbuf -= (where + 2);
    }
}

//TAKEN FROM muffinman.c IN THE STARTER CODE
static struct client *addclient(int fd, struct in_addr addr) {
    struct client *p;
    if ((p = malloc(sizeof(struct client))) == NULL) {
        perror("malloc");
        exit(1);
    }
    p->inbuf = 0;
    if ((p->buf = malloc(INPUT_BUFFER_SIZE)) == NULL) {
        perror("malloc");
        exit(1);
    }
    memset(p->buf, '\0', sizeof(p->buf));
    p->userName = NULL;
    printf("Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    top = p;
    numClients++;
    return p;
}

//TAKEN FROM muffinman.c IN THE STARTER CODE
static void removeclient(int fd) {
    struct client **p;
    // Iterate across all clients starting from top, checking to see which
    // client we must free and shutdown
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    if (*p) {
        struct client *t = (*p)->next;
        if ((shutdown(fd, 2)) < 0) {
            perror("shutdown");
            exit(1);
        }
        printf("Removing client %s\n", inet_ntoa((*p)->ipaddr));
        fflush(stdout);
        free((*p)->buf);
        free((*p)->userName);
        free(*p);
        *p = t;
        numClients--;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
        fflush(stderr);
    }
}

//TAKEN FROM muffinman.c IN THE STARTER CODE
//Iterate over all of the clients and senda a message to all clients
//With a similar name
static void broadcast(struct client *client, char *s, int size) {
    struct client *p;
    for (p = top; p; p = p->next) {
        if (p->userName == client->userName) {
            write(p->fd, s, size);
        }
    }
}

//Iterate over all of the clients and find a client with a name char *s
static void broadcastToName(char *name, char *message, int size) {
    struct client *p;
    for (p = top; p; p = p->next) {
        if ((strcmp(p->userName, name)) == 0) {
            broadcast(p, message, size);
        }
    }
}

int find_network_newline(char *buf, int inbuf) {
  // Step 1: write this function
  int count = 0;
  while (count < inbuf) {
    if (buf[count] == '\r') {
      if (count < inbuf && buf[count+1] == '\n') {
          return count;
      }
    }
  count++;
  }
  return -1; // return the location of '\r' if found
}