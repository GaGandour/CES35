/* This is the server code */

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>

#include "../General/general.h"
#include "../General/utilities.h"

#define QUEUE_SIZE 10

// #define DEBUG

void set_initial_drone_info(drone_info *drone) {
    drone->id = 0;
    drone->x = 0;
    drone->y = 0;
    drone->z = 0;

    drone->vx = 1;
    drone->vy = 0;
    drone->vz = 0;

    drone->status = 0;
}

int open_tcp_connection(
    int &socket_fd,
    struct sockaddr_in *channel
) {
    /* Build address structure to bind to socket */
    memset(channel, 0, sizeof(*channel)); /* Zero channel */
    channel->sin_family = AF_INET; /* Different hosts connected by IPV4 */
    channel->sin_addr.s_addr = htonl(INADDR_ANY);
    channel->sin_port = htons(SERVER_PORT);

    /* Open socket file descriptor */
    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        printf("Socket call failed.\n");
        return -1;
    }

    /* Prevent errors, helps with reuse of address and port */
    int on = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on))) {
        printf("Error preventing reuse of address!\n");
        return -1;
    } else {
        printf("Reuse of address should be working!\n");
    }

    /* Set socket receive (accept, read, ...) timeout */
    struct timeval timeout;
    timeout.tv_sec = 10; /* Timeout in seconds*/
    timeout.tv_usec = 0; /* Prevent strange behavior from microseconds */
    if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout))) {
        printf("Error setting timeout!\n");
        return -1;
    } else {
        printf("Timeout should be working!\n");
    }

    /* Prevent killing server when client closes connection */
    signal(SIGPIPE, SIG_IGN); 

    /* Binds the socket to the address and port number */
    if (bind(socket_fd, (struct sockaddr *)channel, sizeof(*channel)) < 0) {
        printf("Bind failed.\n");
        return -1;
    }

    /* Puts the server socket in a passive mode */
    if (listen(socket_fd, QUEUE_SIZE) < 0) { /* specify queue size */
        printf("Listen failed.\n");
        return -1;
    }

    return socket_fd;
}

char next_expected_response_type(char current_expected_response) {
    if (current_expected_response == ME_AND_HERE)
        return THIS_SPEED;
    if (current_expected_response == THIS_SPEED)
        return ON_MY_WAY;
    if (current_expected_response == ON_MY_WAY)
        return ME_AND_HERE;
    return -1;
}

char message_type_to_response(char response) {
    if (response == ME_AND_HERE)
        return WHAT_SPEED;
    if (response == THIS_SPEED)
        return MOVE_TO_THERE;
    if (response == ON_MY_WAY)
        return WHO_AND_WHERE;
    return -1;
}

void send_first_message(int &socket_fd) {
    char first_message[MESSAGE_LENGTH];
    first_message[0] = WHO_AND_WHERE;
    printf("(->): ");
    print_message(first_message);
    if (write(socket_fd, first_message, MESSAGE_LENGTH) < 0) {
        printf("Error writing to socket!\n");
    }
}

void send_message(int &socket_fd, char *message, int show_in_terminal = 1) {
    if (show_in_terminal) {
        printf("(->): ");
        print_message(message);
    }
    if (write(socket_fd, message, MESSAGE_LENGTH) < 0) {
        printf("Error writing to socket!\n");
    }
}

void read_response(int &socket_client_fd, int &num_bytes, char *buf) {
    #ifdef DEBUG
    fflush(stdin);
    getchar();
    fflush(stdin);
    #endif
    num_bytes = read(socket_client_fd, buf, MESSAGE_LENGTH);
    if (num_bytes > 0) {
        printf("(<-): ");
        print_message(buf);
    }
}

int deal_with_response(
    int socket_fd, 
    int num_bytes_received,
    char *expected_response_type,
    char *received_response,
    char *message,
    drone_info *drone
) {
    if (num_bytes_received != MESSAGE_LENGTH) {
        printf("ERROR: Number of bytes received (%d) != expected (%d).\n", num_bytes_received, MESSAGE_LENGTH);
        return -1;
    }
    if (received_response[0] != *expected_response_type) {
        printf("ERROR: Expected response type (%d) != received response type (%d).\n", *expected_response_type, received_response[0]);
        return -1;
    }

    /* Set message type and drone id */
    message[0] = message_type_to_response(*expected_response_type);
    set_id_in_message(message, get_id_from_message(received_response));

    /* Update to next expected response type */
    *expected_response_type = next_expected_response_type(*expected_response_type);

    if (received_response[0] == ME_AND_HERE) {
        /* Update client's drone position */
        // ...
    }
    else if (received_response[0] == THIS_SPEED) {
        /* Update client's drone speed */
        // ...
        /* Calculate new direction order */
        // ...
        /* Compose MOVE_TO_THERE before sending it */
        set_float_in_message(message, 3, 10);
        set_float_in_message(message, 7, 10);
        set_float_in_message(message, 11, 10);
    }
    else if (received_response[0] == ON_MY_WAY) {
        /* Update client's drone new speed */
        // ...
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 1) {
        printf("Usage: ./server");
        return -1;
    }

    int socket_fd;              /* Socket file descriptor */
    int num_bytes_read;         /* Number of bytes received */
    char buf[MESSAGE_LENGTH];   /* Buffer for incoming data */
    struct sockaddr_in channel; /* Holds IP address */

    socket_fd = open_tcp_connection(socket_fd, &channel);
    if (socket_fd < 0) {
        return -1;
    }
    /* Connection is now opened */

    drone_info server_drone, client_drone; 
    set_initial_drone_info(&server_drone);
    
    /* Socket is now set up and bound, wait for connection and process it */
    while (1) {
        int socket_client_fd = accept(socket_fd, 0, 0); /* Block for connection request */
        if (socket_client_fd < 0) { /* Connection request timeout */
            printf("No connection found! Keep looking...\n");
            continue;
        } 
        printf("Connection found!\n\n");
        printf("-----------------\n\n");

        send_first_message(socket_client_fd);
        
        int error_counter = 0;
        char message[MESSAGE_LENGTH];
        message[0] = WHO_AND_WHERE;
        char expected_response_type = ME_AND_HERE;

        /* Read responses and send messages according to protocol */
        while(error_counter < MAX_CONSECUTIVE_ERRORS) {
            read_response(socket_client_fd, num_bytes_read, buf);
            if(num_bytes_read > 0) {
                error_counter = 0; /* Successfull read, reset counter */
                if (deal_with_response(socket_client_fd, num_bytes_read, &expected_response_type, buf, message, &client_drone) < 0) /* Compose new message */
                    continue;
                printf("---\n\n");
                #ifndef DEBUG
                if (message[0] == WHO_AND_WHERE) sleep(5);
                #endif
                send_message(socket_client_fd, message, 1); /* Send message */
            } else {
                error_counter++; /* Error reading, increase counter */
                printf("(!) Did not receive a response, sending message again. [%d]\n\n", error_counter);
                send_message(socket_client_fd, message, 0);  /* Send message again */
            }
        }
        close(socket_client_fd); /* Close connection */
        printf("Too many consecutive errors. Ending connection.\n\n");
        printf("-----------------\n\n");
    }
}
