#include "MPU6050.h"
#include "Delay.h"

/* Software I2C, independent of the OLED's PB8/PB9 bus.
 * Open-drain: writing 1 releases the line; pull-up resistors raise it.
 * Every wait is bounded, so a disconnected sensor cannot hang the MCU.
 */
#define SCL_PIN GPIO_Pin_10
#define SDA_PIN GPIO_Pin_11

static void SDA_Write(uint8_t high)
{
    GPIO_WriteBit(GPIOB, SDA_PIN, high ? Bit_SET : Bit_RESET);
    Delay_us(5);
}

static uint8_t SDA_Read(void)
{
    return GPIO_ReadInputDataBit(GPIOB, SDA_PIN);
}

static void SCL_Low(void)
{
    GPIO_ResetBits(GPIOB, SCL_PIN);
    Delay_us(5);
}

/* Wait at most approximately 1 ms for clock stretching / a stuck line. */
static uint8_t SCL_High(void)
{
    uint16_t remaining = 200;
    GPIO_SetBits(GPIOB, SCL_PIN);
    while (GPIO_ReadInputDataBit(GPIOB, SCL_PIN) == Bit_RESET) {
        if (remaining-- == 0) {
            return 0;
        }
        Delay_us(5);
    }
    Delay_us(5);
    return 1;
}

static uint8_t I2C_Start(void)
{
    SDA_Write(1);
    if (!SCL_High() || !SDA_Read()) {
        return 0;
    }
    SDA_Write(0);  /* SDA falling while SCL is high: START. */
    SCL_Low();
    return 1;
}

static uint8_t I2C_Stop(void)
{
    uint8_t ok;
    SCL_Low();
    SDA_Write(0);
    ok = SCL_High();
    SDA_Write(1);  /* SDA rising while SCL is high: STOP. */
    return (uint8_t)(ok && SDA_Read());
}

/* Send 8 bits, then sample the slave's ACK on the ninth clock. */
static uint8_t I2C_Send(uint8_t value)
{
    uint8_t bit;
    uint8_t acknowledged;
    for (bit = 0; bit < 8; bit++) {
        SDA_Write((uint8_t)(value & 0x80u));
        value <<= 1;
        if (!SCL_High()) {
            return 0;
        }
        SCL_Low();
    }
    SDA_Write(1);  /* Release SDA so the sensor can pull it low. */
    if (!SCL_High()) {
        return 0;
    }
    acknowledged = (uint8_t)(SDA_Read() == 0);
    SCL_Low();
    return acknowledged;
}

/* ack=1 requests another byte; ack=0 sends NACK for the last byte. */
static uint8_t I2C_Receive(uint8_t *value, uint8_t ack)
{
    uint8_t bit;
    uint8_t result = 0;
    SDA_Write(1);
    for (bit = 0; bit < 8; bit++) {
        if (!SCL_High()) {
            return 0;
        }
        result = (uint8_t)((result << 1) | SDA_Read());
        SCL_Low();
    }
    SDA_Write((uint8_t)!ack);
    if (!SCL_High()) {
        return 0;
    }
    SCL_Low();
    SDA_Write(1);
    *value = result;
    return 1;
}

static uint8_t WriteRegister(uint8_t reg, uint8_t value)
{
    if (!I2C_Start() ||
        !I2C_Send((uint8_t)(MPU6050_ADDRESS << 1)) ||
        !I2C_Send(reg) || !I2C_Send(value)) {
        (void)I2C_Stop();
        return 0;
    }
    return I2C_Stop();
}

static uint8_t ReadRegisters(uint8_t reg, uint8_t *buffer, uint8_t length)
{
    uint8_t i;
    if (!I2C_Start() ||
        !I2C_Send((uint8_t)(MPU6050_ADDRESS << 1)) ||
        !I2C_Send(reg) || !I2C_Start() ||
        !I2C_Send((uint8_t)((MPU6050_ADDRESS << 1) | 1u))) {
        (void)I2C_Stop();
        return 0;
    }
    for (i = 0; i < length; i++) {
        if (!I2C_Receive(&buffer[i], (uint8_t)(i + 1u < length))) {
            (void)I2C_Stop();
            return 0;
        }
    }
    return I2C_Stop();
}

MPU6050_Status MPU6050_ReadID(uint8_t *id)
{
    if (id == 0 || !ReadRegisters(0x75, id, 1)) {
        return MPU6050_BUS_ERROR;
    }
    return MPU6050_OK;
}

MPU6050_Status MPU6050_Init(uint8_t *id)
{
    GPIO_InitTypeDef gpio;
    if (id == 0) {
        return MPU6050_BUS_ERROR;
    }
    *id = 0;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Set output latches high before enabling open-drain outputs. */
    GPIO_SetBits(GPIOB, SCL_PIN | SDA_PIN);
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = SCL_PIN | SDA_PIN;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, &gpio);
    Delay_ms(100);

    if (MPU6050_ReadID(id) != MPU6050_OK) {
        return MPU6050_BUS_ERROR;
    }
    if (*id != 0x68) {
        return MPU6050_ID_ERROR;
    }
    /* Restore known settings, then wake the sensor using X gyro PLL. */
    if (!WriteRegister(0x6B, 0x80)) {
        return MPU6050_BUS_ERROR;
    }
    Delay_ms(100);
    if (!WriteRegister(0x6B, 0x01) ||  /* PWR_MGMT_1: awake, PLL clock */
        !WriteRegister(0x6C, 0x00) ||  /* PWR_MGMT_2: enable all axes */
        !WriteRegister(0x1A, 0x03) ||  /* CONFIG: DLPF enabled */
        !WriteRegister(0x19, 0x09) ||  /* 1000/(1+9) = 100 Hz */
        !WriteRegister(0x1B, 0x00) ||  /* GYRO_CONFIG: +/-250 deg/s */
        !WriteRegister(0x1C, 0x00)) {  /* ACCEL_CONFIG: +/-2 g */
        return MPU6050_BUS_ERROR;
    }
    Delay_ms(100);
    return MPU6050_OK;
}

static int16_t Signed16(uint8_t high, uint8_t low)
{
    uint16_t bits = (uint16_t)(((uint16_t)high << 8) | low);
    /* Explicit conversion of two's-complement data, including -32768. */
    int32_t value = (bits & 0x8000u) ? (int32_t)bits - 65536L : (int32_t)bits;
    return (int16_t)value;
}

MPU6050_Status MPU6050_ReadRaw(MPU6050_Raw *data)
{
    uint8_t bytes[14];
    if (data == 0 || !ReadRegisters(0x3B, bytes, 14)) {
        return MPU6050_BUS_ERROR;
    }
    data->ax = Signed16(bytes[0], bytes[1]);
    data->ay = Signed16(bytes[2], bytes[3]);
    data->az = Signed16(bytes[4], bytes[5]);
    /* bytes[6:8] are temperature, not gyroscope values. */
    data->gx = Signed16(bytes[8], bytes[9]);
    data->gy = Signed16(bytes[10], bytes[11]);
    data->gz = Signed16(bytes[12], bytes[13]);
    return MPU6050_OK;
}
