#ifndef PEN_MPU6050_H
#define PEN_MPU6050_H

#include "stm32f10x.h"

/* 7-bit address. AD0 must be connected to GND. */
#define MPU6050_ADDRESS 0x68u
#define MPU6050_DEVICE_ID 0x68u
#define MPU6500_DEVICE_ID 0x70u

typedef enum {
    MPU6050_OK = 0,
    MPU6050_BUS_ERROR = 1,
    MPU6050_ID_ERROR = 2,
    MPU6050_CONFIG_ERROR = 3
} MPU6050_Status;

typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
} MPU6050_Raw;

/* Supports MPU6050 (0x68) and MPU6500 (0x70) identity values.
 * PB10=SCL, PB11=SDA. External/on-board pull-ups are required. */
MPU6050_Status MPU6050_Init(uint8_t *id);
MPU6050_Status MPU6050_ReadID(uint8_t *id);
MPU6050_Status MPU6050_ReadRaw(MPU6050_Raw *data);

#endif
