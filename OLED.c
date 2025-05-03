#include "stm32f10x.h"
#include "OLED.h"
#include "OLED_Font.h"

#define OLED_ADDRESS       0x78 // OLED 地址
#define OLED_WRITE_COMMAND 0x00
#define OLED_WRITE_DATA    0x40

// OLED 引脚
#define OLED_W_SCL(x) GPIO_WriteBit(GPIOB, GPIO_Pin_8, (BitAction)(x))
#define OLED_W_SDA(x) GPIO_WriteBit(GPIOB, GPIO_Pin_9, (BitAction)(x))

uint8_t OLED_DISPLAY_LINES = OLED_CHARS_VERTICAL;

/**
 * @brief  I2C 引脚初始化
 * @param  无
 * @retval 无
 */
void OLED_I2C_Init()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure = {
        .GPIO_Speed = GPIO_Speed_10MHz,
        .GPIO_Mode  = GPIO_Mode_Out_OD,
        .GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_9,
    };
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
 * @brief  I2C 开始
 * @param  无
 * @retval 无
 */
void OLED_I2C_Start()
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

/**
 * @brief  I2C 停止
 * @param  无
 * @retval 无
 */
void OLED_I2C_Stop()
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
 * @brief  I2C 发送一个字节
 * @param  data 要发送的字节
 * @retval 无
 */
void OLED_I2C_SendByte(char data)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        OLED_W_SDA(!!(data & (0x80 >> i)));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1); // 额外的一个时钟, 不处理应答信号
    OLED_W_SCL(0);
}

/**
 * @brief  写命令
 * @param  command 要写入的命令
 * @retval 无
 */
void OLED_WriteCommand(char command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDRESS);       // 从机地址
    OLED_I2C_SendByte(OLED_WRITE_COMMAND); // 写命令
    OLED_I2C_SendByte(command);
    OLED_I2C_Stop();
}

/**
 * @brief  写数据
 * @param  data 要写入的数据
 * @retval 无
 */
void OLED_WriteData(char data)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(OLED_ADDRESS);    // 从机地址
    OLED_I2C_SendByte(OLED_WRITE_DATA); // 写数据
    OLED_I2C_SendByte(data);
    OLED_I2C_Stop();
}

/**
 * @brief  设置光标位置
 * @param  y 以左上角为原点, 向下方向的坐标, 范围: 0 - 7
 * @param  x 以左上角为原点, 向右方向的坐标, 范围: 0 - 127
 * @retval 无
 */
void OLED_SetCursor(uint8_t y, uint8_t x)
{
    OLED_WriteCommand(0xB0 | y);                 // 设置Y位置
    OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4)); // 设置X位置高4位
    OLED_WriteCommand(0x00 | (x & 0x0F));        // 设置X位置低4位
}

/**
 * @brief  清屏
 * @param  无
 * @retval 无
 */
void OLED_Clear()
{
    uint8_t i, j;
    for (i = 0; i < 8; i++) {
        OLED_SetCursor(i, 0);
        for (j = 0; j < OLED_RES_HORIZONTAL; j++) {
            OLED_WriteData(0x00);
        }
    }
}

/**
 * @brief  清空行
 * @param  line 行位置, 范围: 0 - 3, 溢出归零
 * @param  columnStart 起始列位置
 * @param  columnEnd 结束列位置, 左闭右闭
 * @retval 无
 */
void OLED_ClearLine(uint8_t line, uint8_t columnStart, uint8_t columnEnd)
{
    if (columnStart > columnEnd) {
        columnStart ^= columnEnd;
        columnEnd ^= columnStart;
        columnStart ^= columnEnd;
    }
    columnEnd %= OLED_CHARS_HORIZONTAL;
    columnEnd++; // 闭区间
    line %= OLED_CHARS_VERTICAL;
    uint8_t ops = OLED_CHAR_WIDTH * (columnEnd - columnStart);
    uint8_t i;
    OLED_SetCursor(line * 2, columnStart * OLED_CHAR_WIDTH);
    for (i = 0; i < ops; i++) {
        OLED_WriteData(0x00);
    }
    OLED_SetCursor(line * 2 + 1, columnStart * OLED_CHAR_WIDTH);
    for (i = 0; i < ops; i++) {
        OLED_WriteData(0x00);
    }
}

/**
 * @brief  清空列
 * @param  column 列位置, 范围: 0 - 15, 溢出归零
 * @param  lineStart 起始行位置
 * @param  lineEnd 结束行位置, 左闭右闭
 * @retval 无
 */
void OLED_ClearColumn(uint8_t column, uint8_t lineStart, uint8_t lineEnd)
{
    if (lineStart > lineEnd) {
        lineStart ^= lineEnd;
        lineEnd ^= lineStart;
        lineStart ^= lineEnd;
    }
    lineEnd %= OLED_CHARS_VERTICAL;
    lineEnd++;
    lineEnd *= 2;
    column %= OLED_CHARS_HORIZONTAL;
    uint8_t i;
    for (; lineStart <= lineEnd; lineStart++) {
        OLED_SetCursor(lineStart, column * OLED_CHAR_WIDTH);
        for (i = 0; i < OLED_CHAR_WIDTH; i++) {
            OLED_WriteData(0x00);
        }
    }
}

/**
 * @brief  显示一个字符
 * @param  line 行位置, 范围: 0 - 3, 溢出归零
 * @param  column 列位置, 范围: 0 - 15, 溢出自动换行
 * @param  Char 要显示的字符
 * @retval 输出的字符数, 输出非可打印字符时不为 1
 */
uint8_t OLED_ShowChar(uint8_t line, uint8_t column, char Char)
{
    if (Char < 32 || Char > 126) { // 非可打印字符
        return OLED_ShowNumNoZero(line, column, Char, 0);
    }
    uint8_t overflow = column / OLED_CHARS_HORIZONTAL;
    if (overflow) { // 换行
        line += overflow;
        column -= overflow * OLED_CHARS_HORIZONTAL;
    }
    line %= OLED_DISPLAY_LINES;
    uint8_t i;
    OLED_SetCursor(line * 2, column * OLED_CHAR_WIDTH); // 设置光标位置在上半部分
    for (i = 0; i < OLED_CHAR_WIDTH; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]); // 显示上半部分内容
    }
    OLED_SetCursor(line * 2 + 1, column * OLED_CHAR_WIDTH); // 设置光标位置在下半部分
    for (i = 0; i < OLED_CHAR_WIDTH; i++) {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); // 显示下半部分内容
    }
    return 1;
}

/**
 * @brief  显示字符串
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  str 要显示的字符串, 支持换行符
 * @retval 输出的字符数, 支持换行符
 */
uint8_t OLED_ShowString(uint8_t line, uint8_t column, char *str)
{
    uint8_t n = 0;
    while (*str) {
        if (*str == '\n') {
            uint8_t i = (column + 15) & ~15;
            while (column < i) { // 清空换行位置
                /*n +=*/OLED_ShowChar(line, column++, ' ');
            }
            str++;
            continue;
        }
        n += OLED_ShowChar(line, column++, *str++);
    }
    return n;
}

/**
 * @brief  显示字符串
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  b 显示'T'/'F'
 * @retval 输出的字符数, 包括换行填充
 */
uint8_t OLED_ShowBool(uint8_t line, uint8_t column, uint8_t b)
{
    return OLED_ShowChar(line, column, b ? 'T' : 'F');
}

/**
 * @brief  次方函数
 * @retval X 的 Y 次方
 */
uint32_t OLED_Pow(uint32_t x, uint32_t y)
{
    if (x == 2) {
        return 1 << y;
    }
    uint32_t result = 1;
    while (y--) {
        result *= x;
    }
    return result;
}

/**
 * @brief  计算给定进制下显示给定数字需要多少位
 * @param  number 要计算的数字
 * @param  base 进制
 * @retval 字符串长度
 */
uint8_t OLED_GetNumLen(uint32_t number, uint32_t base)
{
    if (number == 0) {
        return 1;
    }
    uint8_t length = 0;
    do {
        length++;
        number /= base;
    } while (number > 0);
    return length;
}

/**
 * @brief  显示无符号十进制数
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  number 要显示的数字, 范围: 0 - 4294967295
 * @param  length 要显示数字的长度, 范围: 1 - 10, 0: auto
 * @retval 输出的字符数
 */
uint8_t OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    uint8_t n = 0;
    if (length == 0) {
        length = OLED_GetNumLen(number, 10);
    }
    for (; length; length--) {
        n += OLED_ShowChar(line, column++, number / OLED_Pow(10, length - 1) % 10 + '0');
    }
    return n;
}

/**
 * @brief  显示无符号十进制数, 隐藏多余的 0
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  number 要显示的数字, 范围: 0 - 4294967295
 * @param  length 要显示数字的长度, 范围: 1 - 10, 0: auto
 * @retval 输出的字符数
 */
uint8_t OLED_ShowNumNoZero(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    if (number == 0) {
        return OLED_ShowChar(line, column + length - 1, '0');
    }
    uint8_t n = 0;
    if (length == 0) {
        length = OLED_GetNumLen(number, 10);
    }
    char Char;
    uint8_t noZeroOnce = 1;
    for (; length; length--) {
        Char = number / OLED_Pow(10, length - 1) % 10 + '0';
        if (noZeroOnce) {
            if (Char == '0') {
                Char = ' ';
            } else {
                noZeroOnce = 0;
            }
        }
        n += OLED_ShowChar(line, column++, Char);
    }
    return n;
}

/**
 * @brief  显示有符号十进制数
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  number 要显示的数字, 范围: -2147483648 - 2147483647
 * @param  length 要显示数字的长度, 范围: 1 - 10, 0: auto, 不包括正负号
 * @retval 输出的字符数
 */
uint8_t OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length)
{
    uint8_t n = 0;
    if (number > 0) {
        n += OLED_ShowChar(line, column, '+');
    } else if (number < 0) {
        n += OLED_ShowChar(line, column, '-');
        number = -number;
    } // 0 隐藏正负号
    if (length == 0) {
        length = OLED_GetNumLen(number, 10);
    }
    return n + OLED_ShowNum(line, column + 1, number, length);
}

/**
 * @brief  显示十六进制数
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  number 要显示的数字, 范围: 0 - 0xFFFFFFFF
 * @param  length 要显示数字的长度, 范围: 1 - 8, 0: auto
 * @retval 输出的字符数
 */
uint8_t OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    uint8_t n = 0;
    if (length == 0) {
        length = OLED_GetNumLen(number, 16);
    }
    char Char;
    uint8_t single;
    for (; length; length--) {
        single = number / OLED_Pow(16, length - 1) % 16;
        if (single < 10) {
            Char = single + '0';
        } else {
            Char = single - 10 + 'A';
        }
        n += OLED_ShowChar(line, column++, Char);
    }
    return n;
}

/**
 * @brief  显示无符号二进制数
 * @param  line 起始行位置, 范围: 0 - 3
 * @param  column 起始列位置, 范围: 0 - 15
 * @param  number 要显示的数字, 范围: 0 - 0xFFFFFFFF
 * @param  length 要显示数字的长度, 范围: 1 - 32
 * @retval 无
 */
void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length)
{
    for (; length; length--) {
        OLED_ShowChar(line, column++, ((number >> (length - 1)) & 0x01) + '0');
    }
}

/**
 * @brief  初始化
 * @param  无
 * @retval 无
 */
void OLED_Init()
{
    uint32_t i, j;
    for (i = 0; i < 1000; i++) { // 上电延时
        for (j = 0; j < 1000; j++);
    }

    OLED_I2C_Init(); // 端口初始化

    OLED_WriteCommand(0xAE); // 关闭显示

    OLED_WriteCommand(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);

    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F);

    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00);

    OLED_WriteCommand(0x40); // 设置显示开始行

    OLED_WriteCommand(0xA1); // 设置左右方向, 0xA1正常 0xA0左右反置

    OLED_WriteCommand(0xC8); // 设置上下方向, 0xC8正常 0xC0上下反置

    OLED_WriteCommand(0xDA); // 设置COM引脚硬件配置
    OLED_WriteCommand(0x12);

    OLED_WriteCommand(0x81); // 设置对比度控制
    OLED_WriteCommand(0xCF);

    OLED_WriteCommand(0xD9); // 设置预充电周期
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB); // 设置VCOMH取消选择级别
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4); // 设置整个显示打开/关闭

    OLED_WriteCommand(0xA6); // 设置正常/倒转显示

    OLED_WriteCommand(0x8D); // 设置充电泵
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF); // 开启显示

    OLED_Clear(); // OLED清屏
}
