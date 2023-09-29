/* Automatically generated nanopb header */
/* Generated by nanopb-0.4.7 */

#ifndef PB_MESSAGE_PB_H_INCLUDED
#define PB_MESSAGE_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 40
#error Regenerate this file with the current version of nanopb generator.
#endif

/* Struct definitions */
typedef struct _MyMessage {
    float acceleration_x;
    float acceleration_y;
    float acceleration_z;
    float rotation_x;
    float rotation_y;
    float rotation_z;
    double latitude;
    double longitude;
    double altitude;
} MyMessage;


#ifdef __cplusplus
extern "C" {
#endif

/* Initializer values for message structs */
#define MyMessage_init_default                   {0, 0, 0, 0, 0, 0, 0, 0, 0}
#define MyMessage_init_zero                      {0, 0, 0, 0, 0, 0, 0, 0, 0}

/* Field tags (for use in manual encoding/decoding) */
#define MyMessage_acceleration_x_tag             1
#define MyMessage_acceleration_y_tag             2
#define MyMessage_acceleration_z_tag             3
#define MyMessage_rotation_x_tag                 4
#define MyMessage_rotation_y_tag                 5
#define MyMessage_rotation_z_tag                 6
#define MyMessage_latitude_tag                   7
#define MyMessage_longitude_tag                  8
#define MyMessage_altitude_tag                   9

/* Struct field encoding specification for nanopb */
#define MyMessage_FIELDLIST(X, a) \
X(a, STATIC,   SINGULAR, FLOAT,    acceleration_x,    1) \
X(a, STATIC,   SINGULAR, FLOAT,    acceleration_y,    2) \
X(a, STATIC,   SINGULAR, FLOAT,    acceleration_z,    3) \
X(a, STATIC,   SINGULAR, FLOAT,    rotation_x,        4) \
X(a, STATIC,   SINGULAR, FLOAT,    rotation_y,        5) \
X(a, STATIC,   SINGULAR, FLOAT,    rotation_z,        6) \
X(a, STATIC,   SINGULAR, DOUBLE,   latitude,          7) \
X(a, STATIC,   SINGULAR, DOUBLE,   longitude,         8) \
X(a, STATIC,   SINGULAR, DOUBLE,   altitude,          9)
#define MyMessage_CALLBACK NULL
#define MyMessage_DEFAULT NULL

extern const pb_msgdesc_t MyMessage_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define MyMessage_fields &MyMessage_msg

/* Maximum encoded size of messages (where known) */
#define MyMessage_size                           57

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
