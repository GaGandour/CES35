#ifndef __GENERAL_H__
#define __GENERAL_H__

#define SERVER_PORT 8080 /* arbitrary, but client & server must agree */
#define BUF_SIZE 4096  /* block transfer size */
#define MESSAGE_LENGTH 15
#define MAX_CONSECUTIVE_ERRORS 5

#define WHO_AND_WHERE 1
#define ME_AND_HERE 2
#define WHAT_SPEED 3
#define THIS_SPEED 4
#define MOVE_TO_THERE 5
#define ON_MY_WAY 6

#define DRONE_SPEED 1.0


typedef struct drone_info {
    int id;
    float x;
    float y;
    float z;

    float vx;
    float vy;
    float vz;

    int status;
} drone_info;

#endif