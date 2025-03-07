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

#ifndef RLM_H
#define RLM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef EINVAL
#define EINVAL 22
#endif

/* Frame ids. */
#define RLM_RLM_ACCEL_0_XF0_FRAME_ID (0xf0u)
#define RLM_RLM_GYRO_0_XF1_FRAME_ID (0xf1u)
#define RLM_RLM_POSITION_0_XF2_FRAME_ID (0xf2u)
#define RLM_RLM_TRAJECTORY_0_XF3_FRAME_ID (0xf3u)
#define RLM_RLM_BUS_LOAD_0_XF4_FRAME_ID (0xf4u)

/* Frame lengths in bytes. */
#define RLM_RLM_ACCEL_0_XF0_LENGTH (6u)
#define RLM_RLM_GYRO_0_XF1_LENGTH (6u)
#define RLM_RLM_POSITION_0_XF2_LENGTH (8u)
#define RLM_RLM_TRAJECTORY_0_XF3_LENGTH (4u)
#define RLM_RLM_BUS_LOAD_0_XF4_LENGTH (1u)

/* Extended or standard frame types. */
#define RLM_RLM_ACCEL_0_XF0_IS_EXTENDED (0)
#define RLM_RLM_GYRO_0_XF1_IS_EXTENDED (0)
#define RLM_RLM_POSITION_0_XF2_IS_EXTENDED (0)
#define RLM_RLM_TRAJECTORY_0_XF3_IS_EXTENDED (0)
#define RLM_RLM_BUS_LOAD_0_XF4_IS_EXTENDED (0)

/* Frame cycle times in milliseconds. */

/* Signal choices. */

/* Frame Names. */
#define RLM_RLM_ACCEL_0_XF0_NAME "RLM_ACCEL_0XF0"
#define RLM_RLM_GYRO_0_XF1_NAME "RLM_GYRO_0XF1"
#define RLM_RLM_POSITION_0_XF2_NAME "RLM_POSITION_0XF2"
#define RLM_RLM_TRAJECTORY_0_XF3_NAME "RLM_TRAJECTORY_0XF3"
#define RLM_RLM_BUS_LOAD_0_XF4_NAME "RLM_BUS_LOAD_0XF4"

/* Signal Names. */
#define RLM_RLM_ACCEL_0_XF0_X_ACCEL_NAME "X_ACCEL"
#define RLM_RLM_ACCEL_0_XF0_Y_ACCEL_NAME "Y_ACCEL"
#define RLM_RLM_ACCEL_0_XF0_Z_ACCEL_NAME "Z_ACCEL"
#define RLM_RLM_GYRO_0_XF1_X_ROT_NAME "X_ROT"
#define RLM_RLM_GYRO_0_XF1_Y_ROT_NAME "Y_ROT"
#define RLM_RLM_GYRO_0_XF1_Z_ROT_NAME "Z_ROT"
#define RLM_RLM_POSITION_0_XF2_LATITUDE_NAME "LATITUDE"
#define RLM_RLM_POSITION_0_XF2_LONGITUDE_NAME "LONGITUDE"
#define RLM_RLM_TRAJECTORY_0_XF3_SPEED_NAME "SPEED"
#define RLM_RLM_TRAJECTORY_0_XF3_DIRECTION_NAME "DIRECTION"
#define RLM_RLM_BUS_LOAD_0_XF4_BUS_LOAD_NAME "BUS_LOAD"

/**
 * Signals in message RLM_ACCEL_0XF0.
 *
 * All signal values are as on the CAN bus.
 */
struct rlm_rlm_accel_0_xf0_t {
    /**
     * Range: -32787..32787 (-4..4 g)
     * Scale: 0.000122
     * Offset: 0
     */
    int16_t x_accel;

    /**
     * Range: -32787..32787 (-4..4 g)
     * Scale: 0.000122
     * Offset: 0
     */
    int16_t y_accel;

    /**
     * Range: -32787..32787 (-4..4 g)
     * Scale: 0.000122
     * Offset: 0
     */
    int16_t z_accel;
};

/**
 * Signals in message RLM_GYRO_0XF1.
 *
 * All signal values are as on the CAN bus.
 */
struct rlm_rlm_gyro_0_xf1_t {
    /**
     * Range: -32765..32765 (-250..250 degree/s)
     * Scale: 0.00763
     * Offset: 0
     */
    int16_t x_rot;

    /**
     * Range: -32765..32765 (-250..250 degree/s)
     * Scale: 0.00763
     * Offset: 0
     */
    int16_t y_rot;

    /**
     * Range: -32765..32765 (-250..250 degree/s)
     * Scale: 0.00763
     * Offset: 0
     */
    int16_t z_rot;
};

/**
 * Signals in message RLM_POSITION_0XF2.
 *
 * All signal values are as on the CAN bus.
 */
struct rlm_rlm_position_0_xf2_t {
    /**
     * Range: -900000000..900000000 (-90..90 degree)
     * Scale: 1e-07
     * Offset: 0
     */
    int32_t latitude;

    /**
     * Range: -1800000000..1800000000 (-180..180 degree)
     * Scale: 1e-07
     * Offset: 0
     */
    int32_t longitude;
};

/**
 * Signals in message RLM_TRAJECTORY_0XF3.
 *
 * All signal values are as on the CAN bus.
 */
struct rlm_rlm_trajectory_0_xf3_t {
    /**
     * Range: -
     * Scale: 0.01
     * Offset: 0
     */
    int16_t speed;

    /**
     * Range: 0..36000 (0..360 degree)
     * Scale: 0.01
     * Offset: 0
     */
    int16_t direction;
};

/**
 * Signals in message RLM_BUS_LOAD_0XF4.
 *
 * All signal values are as on the CAN bus.
 */
struct rlm_rlm_bus_load_0_xf4_t {
    /**
     * Range: 0..200 (0..100 %)
     * Scale: 0.5
     * Offset: 0
     */
    uint8_t bus_load;
};

/**
 * Pack message RLM_ACCEL_0XF0.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int rlm_rlm_accel_0_xf0_pack(uint8_t* dst_p, const struct rlm_rlm_accel_0_xf0_t* src_p, size_t size);

/**
 * Unpack message RLM_ACCEL_0XF0.
 *
 * @param[out] dst_p Object to unpack the message into.
 * @param[in] src_p Message to unpack.
 * @param[in] size Size of src_p.
 *
 * @return zero(0) or negative error code.
 */
int rlm_rlm_accel_0_xf0_unpack(struct rlm_rlm_accel_0_xf0_t* dst_p, const uint8_t* src_p, size_t size);

/**
 * Init message fields to default values from RLM_ACCEL_0XF0.
 *
 * @param[in] msg_p Message to init.
 *
 * @return zero(0) on success or (-1) in case of nullptr argument.
 */
int rlm_rlm_accel_0_xf0_init(struct rlm_rlm_accel_0_xf0_t* msg_p);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_accel_0_xf0_x_accel_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_accel_0_xf0_x_accel_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_accel_0_xf0_x_accel_is_in_range(int16_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_accel_0_xf0_y_accel_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_accel_0_xf0_y_accel_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_accel_0_xf0_y_accel_is_in_range(int16_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_accel_0_xf0_z_accel_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_accel_0_xf0_z_accel_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_accel_0_xf0_z_accel_is_in_range(int16_t value);

/**
 * Pack message RLM_GYRO_0XF1.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int rlm_rlm_gyro_0_xf1_pack(uint8_t* dst_p, const struct rlm_rlm_gyro_0_xf1_t* src_p, size_t size);

/**
 * Unpack message RLM_GYRO_0XF1.
 *
 * @param[out] dst_p Object to unpack the message into.
 * @param[in] src_p Message to unpack.
 * @param[in] size Size of src_p.
 *
 * @return zero(0) or negative error code.
 */
int rlm_rlm_gyro_0_xf1_unpack(struct rlm_rlm_gyro_0_xf1_t* dst_p, const uint8_t* src_p, size_t size);

/**
 * Init message fields to default values from RLM_GYRO_0XF1.
 *
 * @param[in] msg_p Message to init.
 *
 * @return zero(0) on success or (-1) in case of nullptr argument.
 */
int rlm_rlm_gyro_0_xf1_init(struct rlm_rlm_gyro_0_xf1_t* msg_p);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_gyro_0_xf1_x_rot_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_gyro_0_xf1_x_rot_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_gyro_0_xf1_x_rot_is_in_range(int16_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_gyro_0_xf1_y_rot_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_gyro_0_xf1_y_rot_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_gyro_0_xf1_y_rot_is_in_range(int16_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_gyro_0_xf1_z_rot_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_gyro_0_xf1_z_rot_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_gyro_0_xf1_z_rot_is_in_range(int16_t value);

/**
 * Pack message RLM_POSITION_0XF2.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int rlm_rlm_position_0_xf2_pack(uint8_t* dst_p, const struct rlm_rlm_position_0_xf2_t* src_p, size_t size);

/**
 * Unpack message RLM_POSITION_0XF2.
 *
 * @param[out] dst_p Object to unpack the message into.
 * @param[in] src_p Message to unpack.
 * @param[in] size Size of src_p.
 *
 * @return zero(0) or negative error code.
 */
int rlm_rlm_position_0_xf2_unpack(struct rlm_rlm_position_0_xf2_t* dst_p, const uint8_t* src_p, size_t size);

/**
 * Init message fields to default values from RLM_POSITION_0XF2.
 *
 * @param[in] msg_p Message to init.
 *
 * @return zero(0) on success or (-1) in case of nullptr argument.
 */
int rlm_rlm_position_0_xf2_init(struct rlm_rlm_position_0_xf2_t* msg_p);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int32_t rlm_rlm_position_0_xf2_latitude_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_position_0_xf2_latitude_decode(int32_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_position_0_xf2_latitude_is_in_range(int32_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int32_t rlm_rlm_position_0_xf2_longitude_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_position_0_xf2_longitude_decode(int32_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_position_0_xf2_longitude_is_in_range(int32_t value);

/**
 * Pack message RLM_TRAJECTORY_0XF3.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int rlm_rlm_trajectory_0_xf3_pack(uint8_t* dst_p, const struct rlm_rlm_trajectory_0_xf3_t* src_p, size_t size);

/**
 * Unpack message RLM_TRAJECTORY_0XF3.
 *
 * @param[out] dst_p Object to unpack the message into.
 * @param[in] src_p Message to unpack.
 * @param[in] size Size of src_p.
 *
 * @return zero(0) or negative error code.
 */
int rlm_rlm_trajectory_0_xf3_unpack(struct rlm_rlm_trajectory_0_xf3_t* dst_p, const uint8_t* src_p, size_t size);

/**
 * Init message fields to default values from RLM_TRAJECTORY_0XF3.
 *
 * @param[in] msg_p Message to init.
 *
 * @return zero(0) on success or (-1) in case of nullptr argument.
 */
int rlm_rlm_trajectory_0_xf3_init(struct rlm_rlm_trajectory_0_xf3_t* msg_p);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_trajectory_0_xf3_speed_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_trajectory_0_xf3_speed_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_trajectory_0_xf3_speed_is_in_range(int16_t value);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
int16_t rlm_rlm_trajectory_0_xf3_direction_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_trajectory_0_xf3_direction_decode(int16_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_trajectory_0_xf3_direction_is_in_range(int16_t value);

/**
 * Pack message RLM_BUS_LOAD_0XF4.
 *
 * @param[out] dst_p Buffer to pack the message into.
 * @param[in] src_p Data to pack.
 * @param[in] size Size of dst_p.
 *
 * @return Size of packed data, or negative error code.
 */
int rlm_rlm_bus_load_0_xf4_pack(uint8_t* dst_p, const struct rlm_rlm_bus_load_0_xf4_t* src_p, size_t size);

/**
 * Unpack message RLM_BUS_LOAD_0XF4.
 *
 * @param[out] dst_p Object to unpack the message into.
 * @param[in] src_p Message to unpack.
 * @param[in] size Size of src_p.
 *
 * @return zero(0) or negative error code.
 */
int rlm_rlm_bus_load_0_xf4_unpack(struct rlm_rlm_bus_load_0_xf4_t* dst_p, const uint8_t* src_p, size_t size);

/**
 * Init message fields to default values from RLM_BUS_LOAD_0XF4.
 *
 * @param[in] msg_p Message to init.
 *
 * @return zero(0) on success or (-1) in case of nullptr argument.
 */
int rlm_rlm_bus_load_0_xf4_init(struct rlm_rlm_bus_load_0_xf4_t* msg_p);

/**
 * Encode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to encode.
 *
 * @return Encoded signal.
 */
uint8_t rlm_rlm_bus_load_0_xf4_bus_load_encode(double value);

/**
 * Decode given signal by applying scaling and offset.
 *
 * @param[in] value Signal to decode.
 *
 * @return Decoded signal.
 */
double rlm_rlm_bus_load_0_xf4_bus_load_decode(uint8_t value);

/**
 * Check that given signal is in allowed range.
 *
 * @param[in] value Signal to check.
 *
 * @return true if in range, false otherwise.
 */
bool rlm_rlm_bus_load_0_xf4_bus_load_is_in_range(uint8_t value);

#ifdef __cplusplus
}
#endif

#endif
