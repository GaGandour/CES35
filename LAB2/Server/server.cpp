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

#include "../General/general.h"
#include "../General/utilities.h"

#define QUEUE_SIZE 10

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

    /* Build address structure to bind to socket. */
    memset(channel, 0, sizeof(*channel)); /* zero channel */
    channel->sin_family = AF_INET; /* different hosts connected by IPV4 */
    channel->sin_addr.s_addr = htonl(INADDR_ANY);
    channel->sin_port = htons(SERVER_PORT);
    /* Passive open. Wait for connection. */

    /* Socket creation */
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
    {
        printf("socket call failed");
        return -1;
    }

    /* Prevent errors, helps with reuse of address and port */
    int on = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));

    /* Binds the socket to the address and port number */
    int b = bind(s, (struct sockaddr *)channel, sizeof(*channel));
    if (b < 0)
    {
        printf("bind failed");
        return -1;
    }

    /* Puts the server socket in a passive mode */
    int l = listen(s, QUEUE_SIZE); /* specify queue size */
    if (l < 0)
    {
        printf("listen failed");
        return -1;
    }

    return s;
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

void deal_with_response(
    int socket_fd, 
    int num_bytes_received,
    char *received_response,
    drone_info * drone
) {
    static char expected_response = ME_AND_HERE;

    if (num_bytes_received != MESSAGE_LENGTH) {
        printf("ERROR: Number of bytes received (%d) != expected (%d).\n", num_bytes_received, MESSAGE_LENGTH);
        return;
    }
    if (received_response[0] != expected_response) {
        printf("ERROR: Expected response id (%c) != received response id (%c).\n", expected_response, received_response[0]);
        return;
    }

    char message[MESSAGE_LENGTH];
    // set message type
    message[0] = message_type_to_response(expected_response);
    // set drone id in message
    set_id_in_message(message, get_id_from_message(received_response));

    if (received_response[0] == ME_AND_HERE) {
        // Update client's drone position
        // ...
        // Send WHAT_SPEED
    }
    else if (received_response[0] == THIS_SPEED) {
        // Update client's drone speed
        // ...
        // Calculate new direction order
        // ...
        // Send MOVE_TO_THERE
        set_float_in_message(message, 3, 1);
        set_float_in_message(message, 7, 0);
        set_float_in_message(message, 11, 0);
    }
    else if (received_response[0] == ON_MY_WAY) {
        // Update client's drone new speed
        // ...
        expected_response = next_expected_response_type(expected_response);
        // Return before sending a direct message, 
        // next one should be broadcast!
        return;
    }

    printf("Writing message: %s\n", message);
    write(socket_fd, message, MESSAGE_LENGTH);
    expected_response = next_expected_response_type(expected_response);
    return;
}

int main(int argc, char *argv[])
{
    if (argc != 1)
    {
        printf("Usage: ./server");
        return -1;
    }

    int socket_fd, num_bytes;

    char buf[BUF_SIZE];         /* buffer for outgoing file */
    struct sockaddr_in channel; /* holds IP address */

    int s = open_tcp_connection(socket_fd, &channel);
    if (s < 0) {
        return -1;
    }
    // Connection is now opened.

    drone_info server_drone, client_drone;
    set_initial_drone_info(&server_drone);

    char expected_message = WHO_AND_WHERE;
    
    /* Socket is now set up and bound. Wait for connection and process it. */
    while (1)
    {
        socket_fd = accept(s, 0, 0); /* block for connection request */
        if (socket_fd < 0)
        {
            printf("accept failed");
            exit(-1);
        }

        while(1) {
            /* Send initial message */
            char not_really_a_broadcast[MESSAGE_LENGTH];
            not_really_a_broadcast[0] = WHO_AND_WHERE;
            printf("\nWriting first message: %s\n", not_really_a_broadcast);
            write(socket_fd, not_really_a_broadcast, MESSAGE_LENGTH); 

            /* Read responses and send messages according to custom protocol */
            for(int message_counter = 0; message_counter < 3; message_counter++) {
                num_bytes = read(socket_fd, buf, BUF_SIZE); /* read from socket */
                if (num_bytes > 0) {
                    // deal with data
                    printf("\nRead response: %s\n", buf);
                    deal_with_response(socket_fd, num_bytes, buf, &client_drone);
                }
            }
            sleep(5);
        }
        
        /* close connection */
        // close(socket_fd); 
    }
}
