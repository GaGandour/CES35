/* This is the client code */

#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <math.h>
#include <netinet/in.h>
#include <netdb.h>

#include "../General/general.h"
#include "../General/utilities.h"

// #define DEBUG

void set_initial_drone_info(drone_info *drone) {
    drone->id = 1;
    drone->x = 5;
    drone->y = 4;
    drone->z = 7;

    drone->vx = 1;
    drone->vy = 0;
    drone->vz = 0;

    drone->status = 0;
}

int establish_tcp_connection(
    int &socket_fd,
    struct sockaddr_in *channel,
    struct hostent *host,
    char *server_name
) {

    /* Look up host’s IP address */
    host = gethostbyname(server_name); 
    if (!host) {
        printf("Failed to locate %s using gethostbyname.\n", server_name); 
        return -1;
    }
  
    /* Open socket file descriptor */
    socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        printf("Socket call failed.\n");
        return -1;
    }

    /* Set socket receive (connect, read, ...) timeout */
    struct timeval timeout;
    timeout.tv_sec = 10; /* Timeout in seconds*/
    timeout.tv_usec = 0; /* Prevent strange behavior from microseconds */
    if(setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout))) {
        printf("Error setting timeout!\n");
        return -1;
    } else {
        printf("Timeout should be working!\n");
    }
    
    /* Build address structure to bind to socket */
    memset(channel, 0, sizeof(*channel)); /* Zero channel */
    channel->sin_family= AF_INET; /* Different hosts connected by IPV4 */
    memcpy(&channel->sin_addr.s_addr, host->h_addr, host->h_length);
    channel->sin_port= htons(SERVER_PORT);
    
    int connection = connect(socket_fd, (struct sockaddr *) channel, sizeof(*channel));
    if (connection < 0) {
        printf("Connection failed.\n"); 
        return -1;
    }

    return 0;
}

char next_expected_message_type(char current_expected_message) {
    if (current_expected_message == WHO_AND_WHERE)
        return WHAT_SPEED;
    if (current_expected_message == WHAT_SPEED)
        return MOVE_TO_THERE;
    if (current_expected_message == MOVE_TO_THERE)
        return WHO_AND_WHERE;
    return -1;
}

char response_type_to_message(char message) {
    if (message == WHO_AND_WHERE)
        return ME_AND_HERE;
    if (message == WHAT_SPEED)
        return THIS_SPEED;
    if (message == MOVE_TO_THERE)
        return ON_MY_WAY;
    return -1;
}

void deal_with_message(
    int socket_fd,
    int num_bytes_received,
    char *received_message,
    char *response,
    drone_info * drone
) {
    static char expected_message = WHO_AND_WHERE;

    if (num_bytes_received != MESSAGE_LENGTH) {
        printf("ERROR: Number of bytes received (%d) != expected (%d).\n", num_bytes_received, MESSAGE_LENGTH);
        return;
    }
    if (received_message[0] != expected_message) {
        printf("ERROR: Expected message type (%d) != received message type (%d).\n", expected_message, received_message[0]);
        return;
    }

    /* Set response type and drone id */
    response[0] = response_type_to_message(expected_message);
    set_id_in_message(response, drone->id);

    if (received_message[0] == WHO_AND_WHERE) {
        /* Compose ME_AND_HERE with drone's position before sending it */
        set_float_in_message(response, 3, drone->x);
        set_float_in_message(response, 7, drone->y);
        set_float_in_message(response, 11, drone->z);
    }
    else if (received_message[0] == WHAT_SPEED) {
        if (get_id_from_message(received_message) != drone->id) {
            printf("ERROR: message's target ID (%d) != drone ID (%d)\n", get_id_from_message(received_message), drone->id);
            return;
        } 
        /* Compose THIS_SPEED with drone's speed before sending it */
        set_float_in_message(response, 3, drone->vx);
        set_float_in_message(response, 7, drone->vy);
        set_float_in_message(response, 11, drone->vz);
    }
    else if (received_message[0] == MOVE_TO_THERE) {
        if (get_id_from_message(received_message) != drone->id) {
            printf("ERROR: message's target ID (%d) != drone ID (%d)\n", get_id_from_message(received_message), drone->id);
            return;
        }
        /* Update drone's speed based on new target position */
        float target_x = get_float_from_message(received_message, 3);
        float target_y = get_float_from_message(received_message, 7);
        float target_z = get_float_from_message(received_message, 11);
        float dx = target_x - drone->x;
        float dy = target_y - drone->y;
        float dz = target_z - drone->z;
        float distance = sqrt(dx*dx + dy*dy + dz*dz);
        drone->vx = DRONE_SPEED * dx / distance;
        drone->vy = DRONE_SPEED * dy / distance;
        drone->vz = DRONE_SPEED * dz / distance;
        /* Compose ON_MY_WAY with drone's new speed before sending it */
        set_float_in_message(response, 3, drone->vx);
        set_float_in_message(response, 7, drone->vy);
        set_float_in_message(response, 11, drone->vz);
    }   
    /* Update to next expected message type */
    expected_message = next_expected_message_type(expected_message);
}

void send_response(int &socket_fd, char *response, int show_in_terminal = 1) {
    if (show_in_terminal) {
        #ifdef DEBUG
        fflush(stdin);
        getchar();
        fflush(stdin);
        #endif
        printf("(->) Sending response:\n");
        print_message(response);
        printf("---\n\n");
    }
    if (write(socket_fd, response, MESSAGE_LENGTH) < 0) {
        printf("Error writing to socket!\n");
    }
}

void read_message(int &socket_client_fd, int &num_bytes, char *buf) {
    num_bytes = read(socket_client_fd, buf, MESSAGE_LENGTH);
    if (num_bytes > 0) {
        printf("(<-) Received message:\n");
        print_message(buf);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: ./client <server-name>"); 
        return -1;
    }

    int socket_fd;              /* Socket file descriptor */
    int num_bytes_read;         /* Number of bytes received */
    char buf[MESSAGE_LENGTH];   /* Buffer for incoming data */
    struct hostent * host;      /* Info about server */
    struct sockaddr_in channel; /* Holds IP address */

    int tcp_connection_status = establish_tcp_connection(socket_fd, &channel, host, argv[1]);
    if (tcp_connection_status < 0) {
        return -1;
    } else {
        printf("Connection with server established.\n\n");
        printf("----------------------------------\n\n");
    }
    /* Connection is now established */
    
    drone_info drone;
    set_initial_drone_info(&drone);

    int error_counter = 0;
    char response[MESSAGE_LENGTH];

    /* Read messages and send responses according to protocol */
    while (error_counter < MAX_CONSECUTIVE_ERRORS) {
        read_message(socket_fd, num_bytes_read, buf);
        if (num_bytes_read > 0) {
            error_counter = 0; /* Successfull read, reset counter */
            deal_with_message(socket_fd, num_bytes_read, buf, response, &drone);
            send_response(socket_fd, response, 1);
        } else {
            error_counter++; /* Error reading, increase counter */
            printf("(!) Did not receive a message, sending response again. [%d]\n\n", error_counter);
            send_response(socket_fd, response, 1);
        }
    }
    close(socket_fd); /* Close connection */
    printf("Too many consecutive reading errors. Ending connection.\n\n");
    printf("----------------------------------\n\n");
}