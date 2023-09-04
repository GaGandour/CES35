#include <string.h>
#include "./general.h"

#define MESSAGE_TYPE_STRING_LENGTH 20

float get_float_from_message(char *message, int offset) {
    float result;
    memcpy(&result, message+offset, sizeof(result));
    return result;
}

void set_float_in_message(char *message, int offset, float value) {
    memcpy(message+offset, &value, sizeof(value));
}

int get_id_from_message(char *message) {
    return message[1] << 8 | message[2];
}

void set_id_in_message(char *message, int id) {
    message[1] = id >> 8;
    message[2] = id & 0xFF;
}

void print_message(char *message) {
    char message_type = message[0];
    int drone_id = get_id_from_message(message);
    float x = get_float_from_message(message, 3);
    float y = get_float_from_message(message, 7);
    float z = get_float_from_message(message, 11);

    // if (strlen(message) != MESSAGE_LENGTH) {
    //     printf("ERROR: Message had wrong length: %d. Expected length: %d", strlen(message), MESSAGE_LENGTH);
    //     return;
    // }

    switch (message_type)
    {
    case WHO_AND_WHERE:
        printf("WHO_AND_WHERE?");
        break;
    case ME_AND_HERE:
        printf("ME_AND_HERE! ID: %d, X: %f, Y: %f, Z: %f", drone_id, x, y, z);
        break;
    case WHAT_SPEED:
        printf("WHAT_SPEED? Asking ID %d", drone_id);
        break;
    case THIS_SPEED:
        printf("THIS_SPEED! ID: %d, Vx: %f, Vy: %f, Vz: %f", drone_id, x, y, z);
        break;
    case MOVE_TO_THERE:
        printf("MOVE_TO_THERE! Commanding ID: %d, X: %f, Y: %f, Z: %f", drone_id, x, y, z);
        break;
    case ON_MY_WAY:
        printf("ON_MY_WAY! ID: %d, Updated Velocity: Vx: %f, Vy: %f, Vz: %f", drone_id, x, y, z);
        break;
    default:
        printf("ERROR: Unknown message type: %d", message_type);
        break;
    }
    printf("\n\n");
}