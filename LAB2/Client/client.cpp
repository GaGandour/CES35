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

    // look up host’s IP address
    host = gethostbyname(server_name); 
    if (!host) {
        printf("gethostbyname failed to locate %s", server_name); 
        return -1;
    }
  
    // Open socket file descriptor
    socket_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd < 0) {
        printf("socket call failed");
        return -1;
    }
    
    memset(channel, 0, sizeof(*channel));
    channel->sin_family= AF_INET;
    memcpy(&channel->sin_addr.s_addr, host->h_addr, host->h_length);
    channel->sin_port= htons(SERVER_PORT);
    
    int connection = connect(socket_fd, (struct sockaddr *) channel, sizeof(*channel));
    if (connection < 0) {
        printf("connect failed"); 
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
    int socket_fd, // will be used to send response
    int num_bytes_received,
    char *received_message,
    drone_info * drone
) {
    static char expected_message = WHO_AND_WHERE;

    if (num_bytes_received != MESSAGE_LENGTH) {
        printf("ERROR: Number of bytes received (%d) != expected (%d).\n", num_bytes_received, MESSAGE_LENGTH);
        return;
    }
    if (received_message[0] != expected_message) {
        printf("ERROR: Expected message id (%c) != received message id (%c).\n", expected_message, received_message[0]);
        return;
    }

    char response[MESSAGE_LENGTH];
    // set response type
    response[0] = response_type_to_message(expected_message);
    // set drone id in message
    set_id_in_message(response, drone->id);

    if (received_message[0] == WHO_AND_WHERE) {
        // Send ME_AND_HERE
        set_float_in_message(response, 3, drone->x);
        set_float_in_message(response, 7, drone->y);
        set_float_in_message(response, 11, drone->z);
    }
    else if (received_message[0] == WHAT_SPEED) {
        if (get_id_from_message(received_message) != drone->id) {
            printf("ERROR: Id from message (%d) != id from this drone (%d)", get_id_from_message(received_message), drone->id);
            return;
        } 
        // Send THIS_SPEED
        set_float_in_message(response, 3, drone->vx);
        set_float_in_message(response, 7, drone->vy);
        set_float_in_message(response, 11, drone->vz);
    }
    else if (received_message[0] == MOVE_TO_THERE) {
        if (get_id_from_message(received_message) != drone->id) {
            printf("ERROR: Id from message (%d) != id from this drone (%d)", get_id_from_message(received_message), drone->id);
            return;
        }
        // Update drone info
        float target_x, target_y, target_z;
        target_x = get_float_from_message(received_message, 3);
        target_y = get_float_from_message(received_message, 7);
        target_z = get_float_from_message(received_message, 11);
        float dx = target_x - drone->x;
        float dy = target_y - drone->y;
        float dz = target_z - drone->z;
        float distance = sqrt(dx*dx + dy*dy + dz*dz);
        drone->vx = DRONE_SPEED * dx / distance;
        drone->vy = DRONE_SPEED * dy / distance;
        drone->vz = DRONE_SPEED * dz / distance;
        // Send ON_MY_WAY
        set_float_in_message(response, 3, drone->vx);
        set_float_in_message(response, 7, drone->vy);
        set_float_in_message(response, 11, drone->vz);
    }

    printf("Writing response: %s\n", response);
    write(socket_fd, response, MESSAGE_LENGTH);
    expected_message = next_expected_message_type(expected_message);
    return;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        printf("Usage: ./client <server-name>"); 
        return -1;
    }

    int num_bytes, socket_fd;
    char buf[BUF_SIZE];  /* buffer for incoming file */
    struct hostent * host;  /* info about server */
    struct sockaddr_in channel; /* holds IP address */

    int tcp_connection_status = establish_tcp_connection(socket_fd, &channel, host, argv[1]);
    if (tcp_connection_status < 0) {
        return -1;
    }
    // Connection is now established.
    
    drone_info drone;
    set_initial_drone_info(&drone);

    char expected_message = WHO_AND_WHERE;
    
    while (1) {
        // Enter state machine. Forever wait for a server message,
        // deal with it and then expect the next one.
        num_bytes = read(socket_fd, buf, BUF_SIZE); /* read from socket */
        if (num_bytes > 0) {
            // deal with data
            printf("\nRead message: %s\n", buf);
            deal_with_message(socket_fd, num_bytes, buf, &drone);
        }
        // continue indefinetely
    }
}