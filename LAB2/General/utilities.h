#include <string.h>

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