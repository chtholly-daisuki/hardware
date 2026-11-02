//江协科技stm32 2.1
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include <stdio.h>

void class2_1(){
	//寄存器操作:需要查阅手册，对寄存器数据进行人工计算
	/*RCC->APB2ENR = 0x00000010;
	GPIOC->CRH=0X00300000;
	GPIOC->ODR=0X00002000;*/
	
	//RCC APB2时钟控制(选择外设，新的状态)，可右键函数go to definition of""
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_13;//右键.GPIO_Pin,转到定义，选择member,跳转到声明
	//选中@ref后，ctrlF查找，看到宏定义列表，通过原理图得知灯连的是13端口(13pin)
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	//将指定端口设置为高电平
//	GPIO_SetBits(GPIOC,GPIO_Pin_13);
	//将指定端口设置为低电平：亮灯
	GPIO_ResetBits(GPIOC,GPIO_Pin_13);
	//配置端口模式
}

void class3_2_1_SingleLight(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启A0口（电灯口）时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0;//A0引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//50MHz
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);//重置为低电平0：亮灯
	//GPIO_SetBits(GPIOA,GPIO_Pin_0);//设置为高电平1：熄灯
	//GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);//==GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	//GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);//==GPIO_SetBits(GPIOA,GPIO_Pin_0);
}
void class3_2_1_SingleLightTwinkle(){
	for(int i=1;i<=10;i++){
		GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);
		Delay_ms(i*10);
		GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);
		Delay_ms(i*10);
	}
}
/*---------------------------------------------------------------------------------------------------*/
void class3_2_1_MultipleLight(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//开启A0口（电灯口）时钟
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;//推挽输出
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_All;//A0引脚
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;//50MHz
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOA,GPIO_Pin_0);//重置为低电平0：亮灯
	//GPIO_SetBits(GPIOA,GPIO_Pin_0);//设置为高电平1：熄灯
	//GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_RESET);//==GPIO_ResetBits(GPIOA,GPIO_Pin_0);
	//GPIO_WriteBit(GPIOA,GPIO_Pin_0,Bit_SET);//==GPIO_SetBits(GPIOA,GPIO_Pin_0);
}
void class3_2_1_MultipleLightTwinkle(){
	for(int i=1;i<=10;i++){
		GPIO_Write(GPIOA,~0x0001);//0x16进制转为二进制机器编码: 0000 0000 0000 0001控制Pin[A0]
		//然后因为电路是stm32的gpio输出低电平时点亮，所以要~取反
		//ctrl cv:
		Delay_ms(i*10);
		GPIO_Write(GPIOA,~0x0002);//0x16进制转为二进制机器编码: 0000 0000 0000 0010控制Pin[A1]
		Delay_ms(i*10);
		GPIO_Write(GPIOA,~0x0004);//0x16进制转为二进制机器编码: 0000 0000 0000 0100控制Pin[A2]
		Delay_ms(i*10);
		GPIO_Write(GPIOA,~0x0008);//0x16进制转为二进制机器编码: 0000 0000 0000 1000控制Pin[A3]
		Delay_ms(i*10);
		GPIO_Write(GPIOA,~0x0010);//0x16进制转为二进制机器编码: 0000 0000 0001 0000控制Pin[A4]
		Delay_ms(i*10);
		//GPIO_Write(GPIOA,~0X0020);//0x16进制转为二进制机器编码: 0000 0000 0010 0000控制Pin[A5]
		//Delay_ms(i*10);
		//GPIO_Write(GPIOA,~0X0040);//0x16进制转为二进制机器编码: 0000 0000 0100 0000控制Pin[A6]
		//Delay_ms(i*10);
	}
}
void FSR_Init(void)
{
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;

    /* 1. 给 GPIOA 和 ADC1 开启时钟 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1,
        ENABLE
    );

    /* 2. 设置 ADC 时钟：常见 72MHz 配置下得到 12MHz */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    /* 3. PA0 用于接收模拟电压 */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &gpio);

    /* 4. 配置 ADC：每次由程序启动一次采样 */
    ADC_StructInit(&adc);
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    /* 5. PA0 对应 ADC 通道 0，使用较长的采样时间 */
    ADC_RegularChannelConfig(
        ADC1,
        ADC_Channel_0,
        1,
        ADC_SampleTime_239Cycles5
    );

    /* 6. 启用 ADC，等待稳定并完成校准 */
    ADC_Cmd(ADC1, ENABLE);
    Delay_us(10);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1) == SET)
    {
    }

    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1) == SET)
    {
    }
}

/* 读取一次 PA0 电压，返回 0～4095 */
uint16_t FSR_Read(void)
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    /* 等待这一次转换完成 */
    while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
    }

    return ADC_GetConversionValue(ADC1);
}
void class4_2_OLEDInit(){
	OLED_Init();
	OLED_ShowChar(1,1,'@');
	OLED_ShowString(1,3,"HelloWorld!");
	OLED_ShowNum(2,1,12345,5);
}

/* 初始化 USART1：PA9 发送，9600 波特率 */
void Bluetooth_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    /* 开启 GPIOA 和 USART1 的时钟 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1,
        ENABLE
    );

    /* PA9 交给 USART1 控制，作为串口发送引脚 */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;  // 复用推挽输出
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* 串口通信参数：9600、8位数据、1位停止、无校验 */
    USART_StructInit(&usart);
    usart.USART_BaudRate = 9600;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Tx;  // 本次只发送

    USART_Init(USART1, &usart);
    USART_Cmd(USART1, ENABLE);
}

/* 发送一个字节 */
void Bluetooth_SendByte(uint8_t data)
{
    /* 等待发送数据寄存器空闲 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }

    USART_SendData(USART1, data);
}

/* 依次发送字符串中的每个字符 */
void Bluetooth_SendString(const char *text)
{
    while (*text != '\0')
    {
        Bluetooth_SendByte((uint8_t)*text);
        text++;
    }

    /* 等待最后一个字符完全发出 */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
    }
}


/* MPU6050 demo: replace only the original main(), keep existing helpers. */
int main(void)
{
    uint16_t value;
    uint8_t id = 0;
    uint8_t ready = 0;
    uint8_t rows = 0;
    MPU6050_Status status;
    MPU6050_Raw motion;
    char message[80];

    OLED_Init();
    FSR_Init();
    Bluetooth_Init();
    OLED_ShowString(1, 1, "FSR ADC:");

    while (1)
    {
        value = FSR_Read();
        OLED_ShowNum(1, 9, value, 4);

        if (!ready)
        {
            status = MPU6050_Init(&id);
            if (status != MPU6050_OK)
            {
                if (status == MPU6050_ID_ERROR)
                {
                    sprintf(message, "ERR: WHO_AM_I=0x%02X, expected 0x68\r\n", (unsigned int)id);
                    OLED_ShowString(2, 1, "MPU ID mismatch ");
                }
                else
                {
                    sprintf(message, "ERR: MPU6050 I2C, check power/GND/PB10/PB11/AD0\r\n");
                    OLED_ShowString(2, 1, "MPU I2C ERROR   ");
                }
                Bluetooth_SendString(message);
                OLED_ShowString(3, 1, "Check wiring    ");
                OLED_ShowString(4, 1, "Retry in 1s     ");
                Delay_ms(1000);
                continue;
            }
            ready = 1;
            rows = 0;
        }

        if (MPU6050_ReadRaw(&motion) != MPU6050_OK)
        {
            Bluetooth_SendString("ERR: MPU6050 read failed\r\n");
            OLED_ShowString(2, 1, "MPU read failed ");
            ready = 0;
            Delay_ms(500);
            continue;
        }

        
        if (rows == 0)
        {
            sprintf(message, "MPU6050 ID: 0x%02X\r\n", (unsigned int)id);
            Bluetooth_SendString(message);
            Bluetooth_SendString("ax,ay,az,gx,gy,gz\r\n");
        }
        sprintf(message, "%d,%d,%d,%d,%d,%d\r\n",
                (int)motion.ax, (int)motion.ay, (int)motion.az,
                (int)motion.gx, (int)motion.gy, (int)motion.gz);
        Bluetooth_SendString(message);
        rows = (uint8_t)((rows + 1u) % 10u);

        
        sprintf(message, "AX:%6d       ", (int)motion.ax);
        OLED_ShowString(2, 1, message);
        sprintf(message, "AY:%6d       ", (int)motion.ay);
        OLED_ShowString(3, 1, message);
        sprintf(message, "AZ:%6d       ", (int)motion.az);
        OLED_ShowString(4, 1, message);

        
		
		/*Bluetooth_SendString("hello1\r\n");
        Delay_ms(500);*/
        Delay_ms(100);
    }
}
