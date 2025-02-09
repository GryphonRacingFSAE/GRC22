/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2019 Erik Moqvist
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * This file was generated by cantools version 39.4.5 Mon Jun  3 17:14:37 2024.
 */

#include <string.h>

#include "rlm.h"

static inline uint8_t pack_left_shift_u8(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value << shift) & mask);
}

static inline uint8_t pack_left_shift_u16(uint16_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value << shift) & mask);
}

static inline uint8_t pack_left_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value << shift) & mask);
}

static inline uint8_t pack_right_shift_u16(uint16_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value >> shift) & mask);
}

static inline uint8_t pack_right_shift_u32(uint32_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value >> shift) & mask);
}

static inline uint16_t unpack_left_shift_u16(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint16_t)((uint16_t)(value & mask) << shift);
}

static inline uint32_t unpack_left_shift_u32(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint32_t)((uint32_t)(value & mask) << shift);
}

static inline uint8_t unpack_right_shift_u8(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint8_t)((uint8_t)(value & mask) >> shift);
}

static inline uint16_t unpack_right_shift_u16(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint16_t)((uint16_t)(value & mask) >> shift);
}

static inline uint32_t unpack_right_shift_u32(uint8_t value, uint8_t shift, uint8_t mask) {
    return (uint32_t)((uint32_t)(value & mask) >> shift);
}

int rlm_rlm_accel_0_xf0_pack(uint8_t* dst_p, const struct rlm_rlm_accel_0_xf0_t* src_p, size_t size) {
    uint16_t x_accel;
    uint16_t y_accel;
    uint16_t z_accel;

    if (size < 6u) {
        return (-EINVAL);
    }

    memset(&dst_p[0], 0, 6);

    x_accel = (uint16_t)src_p->x_accel;
    dst_p[0] |= pack_left_shift_u16(x_accel, 0u, 0xffu);
    dst_p[1] |= pack_right_shift_u16(x_accel, 8u, 0xffu);
    y_accel = (uint16_t)src_p->y_accel;
    dst_p[2] |= pack_left_shift_u16(y_accel, 0u, 0xffu);
    dst_p[3] |= pack_right_shift_u16(y_accel, 8u, 0xffu);
    z_accel = (uint16_t)src_p->z_accel;
    dst_p[4] |= pack_left_shift_u16(z_accel, 0u, 0xffu);
    dst_p[5] |= pack_right_shift_u16(z_accel, 8u, 0xffu);

    return (6);
}

int rlm_rlm_accel_0_xf0_unpack(struct rlm_rlm_accel_0_xf0_t* dst_p, const uint8_t* src_p, size_t size) {
    uint16_t x_accel;
    uint16_t y_accel;
    uint16_t z_accel;

    if (size < 6u) {
        return (-EINVAL);
    }

    x_accel = unpack_right_shift_u16(src_p[0], 0u, 0xffu);
    x_accel |= unpack_left_shift_u16(src_p[1], 8u, 0xffu);
    dst_p->x_accel = (int16_t)x_accel;
    y_accel = unpack_right_shift_u16(src_p[2], 0u, 0xffu);
    y_accel |= unpack_left_shift_u16(src_p[3], 8u, 0xffu);
    dst_p->y_accel = (int16_t)y_accel;
    z_accel = unpack_right_shift_u16(src_p[4], 0u, 0xffu);
    z_accel |= unpack_left_shift_u16(src_p[5], 8u, 0xffu);
    dst_p->z_accel = (int16_t)z_accel;

    return (0);
}

int rlm_rlm_accel_0_xf0_init(struct rlm_rlm_accel_0_xf0_t* msg_p) {
    if (msg_p == NULL)
        return -1;

    memset(msg_p, 0, sizeof(struct rlm_rlm_accel_0_xf0_t));

    return 0;
}

int16_t rlm_rlm_accel_0_xf0_x_accel_encode(double value) {
    return (int16_t)(value / 0.000122);
}

double rlm_rlm_accel_0_xf0_x_accel_decode(int16_t value) {
    return ((double)value * 0.000122);
}

bool rlm_rlm_accel_0_xf0_x_accel_is_in_range(int16_t value) {
    (void)value;

    return (true);
}

int16_t rlm_rlm_accel_0_xf0_y_accel_encode(double value) {
    return (int16_t)(value / 0.000122);
}

double rlm_rlm_accel_0_xf0_y_accel_decode(int16_t value) {
    return ((double)value * 0.000122);
}

bool rlm_rlm_accel_0_xf0_y_accel_is_in_range(int16_t value) {
    (void)value;

    return (true);
}

int16_t rlm_rlm_accel_0_xf0_z_accel_encode(double value) {
    return (int16_t)(value / 0.000122);
}

double rlm_rlm_accel_0_xf0_z_accel_decode(int16_t value) {
    return ((double)value * 0.000122);
}

bool rlm_rlm_accel_0_xf0_z_accel_is_in_range(int16_t value) {
    (void)value;

    return (true);
}

int rlm_rlm_gyro_0_xf1_pack(uint8_t* dst_p, const struct rlm_rlm_gyro_0_xf1_t* src_p, size_t size) {
    uint16_t x_rot;
    uint16_t y_rot;
    uint16_t z_rot;

    if (size < 6u) {
        return (-EINVAL);
    }

    memset(&dst_p[0], 0, 6);

    x_rot = (uint16_t)src_p->x_rot;
    dst_p[0] |= pack_left_shift_u16(x_rot, 0u, 0xffu);
    dst_p[1] |= pack_right_shift_u16(x_rot, 8u, 0xffu);
    y_rot = (uint16_t)src_p->y_rot;
    dst_p[2] |= pack_left_shift_u16(y_rot, 0u, 0xffu);
    dst_p[3] |= pack_right_shift_u16(y_rot, 8u, 0xffu);
    z_rot = (uint16_t)src_p->z_rot;
    dst_p[4] |= pack_left_shift_u16(z_rot, 0u, 0xffu);
    dst_p[5] |= pack_right_shift_u16(z_rot, 8u, 0xffu);

    return (6);
}

int rlm_rlm_gyro_0_xf1_unpack(struct rlm_rlm_gyro_0_xf1_t* dst_p, const uint8_t* src_p, size_t size) {
    uint16_t x_rot;
    uint16_t y_rot;
    uint16_t z_rot;

    if (size < 6u) {
        return (-EINVAL);
    }

    x_rot = unpack_right_shift_u16(src_p[0], 0u, 0xffu);
    x_rot |= unpack_left_shift_u16(src_p[1], 8u, 0xffu);
    dst_p->x_rot = (int16_t)x_rot;
    y_rot = unpack_right_shift_u16(src_p[2], 0u, 0xffu);
    y_rot |= unpack_left_shift_u16(src_p[3], 8u, 0xffu);
    dst_p->y_rot = (int16_t)y_rot;
    z_rot = unpack_right_shift_u16(src_p[4], 0u, 0xffu);
    z_rot |= unpack_left_shift_u16(src_p[5], 8u, 0xffu);
    dst_p->z_rot = (int16_t)z_rot;

    return (0);
}

int rlm_rlm_gyro_0_xf1_init(struct rlm_rlm_gyro_0_xf1_t* msg_p) {
    if (msg_p == NULL)
        return -1;

    memset(msg_p, 0, sizeof(struct rlm_rlm_gyro_0_xf1_t));

    return 0;
}

int16_t rlm_rlm_gyro_0_xf1_x_rot_encode(double value) {
    return (int16_t)(value / 0.00763);
}

double rlm_rlm_gyro_0_xf1_x_rot_decode(int16_t value) {
    return ((double)value * 0.00763);
}

bool rlm_rlm_gyro_0_xf1_x_rot_is_in_range(int16_t value) {
    return ((value >= -32765) && (value <= 32765));
}

int16_t rlm_rlm_gyro_0_xf1_y_rot_encode(double value) {
    return (int16_t)(value / 0.00763);
}

double rlm_rlm_gyro_0_xf1_y_rot_decode(int16_t value) {
    return ((double)value * 0.00763);
}

bool rlm_rlm_gyro_0_xf1_y_rot_is_in_range(int16_t value) {
    return ((value >= -32765) && (value <= 32765));
}

int16_t rlm_rlm_gyro_0_xf1_z_rot_encode(double value) {
    return (int16_t)(value / 0.00763);
}

double rlm_rlm_gyro_0_xf1_z_rot_decode(int16_t value) {
    return ((double)value * 0.00763);
}

bool rlm_rlm_gyro_0_xf1_z_rot_is_in_range(int16_t value) {
    return ((value >= -32765) && (value <= 32765));
}

int rlm_rlm_position_0_xf2_pack(uint8_t* dst_p, const struct rlm_rlm_position_0_xf2_t* src_p, size_t size) {
    uint32_t latitude;
    uint32_t longitude;

    if (size < 8u) {
        return (-EINVAL);
    }

    memset(&dst_p[0], 0, 8);

    latitude = (uint32_t)src_p->latitude;
    dst_p[0] |= pack_left_shift_u32(latitude, 0u, 0xffu);
    dst_p[1] |= pack_right_shift_u32(latitude, 8u, 0xffu);
    dst_p[2] |= pack_right_shift_u32(latitude, 16u, 0xffu);
    dst_p[3] |= pack_right_shift_u32(latitude, 24u, 0xffu);
    longitude = (uint32_t)src_p->longitude;
    dst_p[4] |= pack_left_shift_u32(longitude, 0u, 0xffu);
    dst_p[5] |= pack_right_shift_u32(longitude, 8u, 0xffu);
    dst_p[6] |= pack_right_shift_u32(longitude, 16u, 0xffu);
    dst_p[7] |= pack_right_shift_u32(longitude, 24u, 0xffu);

    return (8);
}

int rlm_rlm_position_0_xf2_unpack(struct rlm_rlm_position_0_xf2_t* dst_p, const uint8_t* src_p, size_t size) {
    uint32_t latitude;
    uint32_t longitude;

    if (size < 8u) {
        return (-EINVAL);
    }

    latitude = unpack_right_shift_u32(src_p[0], 0u, 0xffu);
    latitude |= unpack_left_shift_u32(src_p[1], 8u, 0xffu);
    latitude |= unpack_left_shift_u32(src_p[2], 16u, 0xffu);
    latitude |= unpack_left_shift_u32(src_p[3], 24u, 0xffu);
    dst_p->latitude = (int32_t)latitude;
    longitude = unpack_right_shift_u32(src_p[4], 0u, 0xffu);
    longitude |= unpack_left_shift_u32(src_p[5], 8u, 0xffu);
    longitude |= unpack_left_shift_u32(src_p[6], 16u, 0xffu);
    longitude |= unpack_left_shift_u32(src_p[7], 24u, 0xffu);
    dst_p->longitude = (int32_t)longitude;

    return (0);
}

int rlm_rlm_position_0_xf2_init(struct rlm_rlm_position_0_xf2_t* msg_p) {
    if (msg_p == NULL)
        return -1;

    memset(msg_p, 0, sizeof(struct rlm_rlm_position_0_xf2_t));

    return 0;
}

int32_t rlm_rlm_position_0_xf2_latitude_encode(double value) {
    return (int32_t)(value / 1e-07);
}

double rlm_rlm_position_0_xf2_latitude_decode(int32_t value) {
    return ((double)value * 1e-07);
}

bool rlm_rlm_position_0_xf2_latitude_is_in_range(int32_t value) {
    return ((value >= -900000000) && (value <= 900000000));
}

int32_t rlm_rlm_position_0_xf2_longitude_encode(double value) {
    return (int32_t)(value / 1e-07);
}

double rlm_rlm_position_0_xf2_longitude_decode(int32_t value) {
    return ((double)value * 1e-07);
}

bool rlm_rlm_position_0_xf2_longitude_is_in_range(int32_t value) {
    return ((value >= -1800000000) && (value <= 1800000000));
}

int rlm_rlm_trajectory_0_xf3_pack(uint8_t* dst_p, const struct rlm_rlm_trajectory_0_xf3_t* src_p, size_t size) {
    uint16_t direction;
    uint16_t speed;

    if (size < 4u) {
        return (-EINVAL);
    }

    memset(&dst_p[0], 0, 4);

    speed = (uint16_t)src_p->speed;
    dst_p[0] |= pack_left_shift_u16(speed, 0u, 0xffu);
    dst_p[1] |= pack_right_shift_u16(speed, 8u, 0xffu);
    direction = (uint16_t)src_p->direction;
    dst_p[2] |= pack_left_shift_u16(direction, 0u, 0xffu);
    dst_p[3] |= pack_right_shift_u16(direction, 8u, 0xffu);

    return (4);
}

int rlm_rlm_trajectory_0_xf3_unpack(struct rlm_rlm_trajectory_0_xf3_t* dst_p, const uint8_t* src_p, size_t size) {
    uint16_t direction;
    uint16_t speed;

    if (size < 4u) {
        return (-EINVAL);
    }

    speed = unpack_right_shift_u16(src_p[0], 0u, 0xffu);
    speed |= unpack_left_shift_u16(src_p[1], 8u, 0xffu);
    dst_p->speed = (int16_t)speed;
    direction = unpack_right_shift_u16(src_p[2], 0u, 0xffu);
    direction |= unpack_left_shift_u16(src_p[3], 8u, 0xffu);
    dst_p->direction = (int16_t)direction;

    return (0);
}

int rlm_rlm_trajectory_0_xf3_init(struct rlm_rlm_trajectory_0_xf3_t* msg_p) {
    if (msg_p == NULL)
        return -1;

    memset(msg_p, 0, sizeof(struct rlm_rlm_trajectory_0_xf3_t));

    return 0;
}

int16_t rlm_rlm_trajectory_0_xf3_speed_encode(double value) {
    return (int16_t)(value / 0.01);
}

double rlm_rlm_trajectory_0_xf3_speed_decode(int16_t value) {
    return ((double)value * 0.01);
}

bool rlm_rlm_trajectory_0_xf3_speed_is_in_range(int16_t value) {
    (void)value;

    return (true);
}

int16_t rlm_rlm_trajectory_0_xf3_direction_encode(double value) {
    return (int16_t)(value / 0.01);
}

double rlm_rlm_trajectory_0_xf3_direction_decode(int16_t value) {
    return ((double)value * 0.01);
}

bool rlm_rlm_trajectory_0_xf3_direction_is_in_range(int16_t value) {
    return (value >= 0);
}

int rlm_rlm_bus_load_0_xf4_pack(uint8_t* dst_p, const struct rlm_rlm_bus_load_0_xf4_t* src_p, size_t size) {
    if (size < 1u) {
        return (-EINVAL);
    }

    memset(&dst_p[0], 0, 1);

    dst_p[0] |= pack_left_shift_u8(src_p->bus_load, 0u, 0xffu);

    return (1);
}

int rlm_rlm_bus_load_0_xf4_unpack(struct rlm_rlm_bus_load_0_xf4_t* dst_p, const uint8_t* src_p, size_t size) {
    if (size < 1u) {
        return (-EINVAL);
    }

    dst_p->bus_load = unpack_right_shift_u8(src_p[0], 0u, 0xffu);

    return (0);
}

int rlm_rlm_bus_load_0_xf4_init(struct rlm_rlm_bus_load_0_xf4_t* msg_p) {
    if (msg_p == NULL)
        return -1;

    memset(msg_p, 0, sizeof(struct rlm_rlm_bus_load_0_xf4_t));

    return 0;
}

uint8_t rlm_rlm_bus_load_0_xf4_bus_load_encode(double value) {
    return (uint8_t)(value / 0.5);
}

double rlm_rlm_bus_load_0_xf4_bus_load_decode(uint8_t value) {
    return ((double)value * 0.5);
}

bool rlm_rlm_bus_load_0_xf4_bus_load_is_in_range(uint8_t value) {
    return (value <= 200u);
}
