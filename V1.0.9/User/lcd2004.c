#include "Lcd2004.h"
#include "common.h"
#include "system.h"
#include "timer.h"
#include "state.h"
#include "systick.h"
#include "AvoPin.h"
#include "stdarg.h"
#include "DutInfo.h"

/*************************************************************
 * 函数名：WR_DATA
 * 功能：将处理后的数据写入特定的 GPIO 端口（GPIOE）。
 * 参数：uint32_t data，一个无符号 32 位整数，代表要进行处理的数据。
 * 返回值：无
 * 备注：该函数首先将传入的数据左移 8 位，为后续与其他值进行合并腾出低 8 位空间。
         然后，将左移后的数据与全局结构体 dut_info 中的成员变量 VLKFlag 进行按位或操作，
         将 VLKFlag 的值放置在数据的低 8 位上。最后，将处理后的数据赋值给 GPIOE 的输出控制寄存器，
         假设 GPIO_OCTL 是一个宏或者函数用于访问 GPIOE 的特定寄存器。
**************************************************************/
void WR_DATA(uint32_t data) 
{ 
    data = data<<8;
    data |=  dut_info.VLKFlag;
    GPIO_OCTL(GPIOE) = (data);
}

/*************************************************************
 * 函数名：Init_Lcd
 * 功能：初始化LCD屏幕
 * 参数：无
 * 返回值：无
 * 备注：该函数用于初始化LCD屏幕的GPIO引脚和发送一系列LCD命令进行初始化设置。
 *       使用了寄存器对GPIO引脚进行配置，并通过调用wr_lcd函数发送LCD命令。
**************************************************************/
void Init_Lcd(void)
{
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOE);

    // data
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_15);

    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
    gpio_init(GPIOE, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);

    // 背光负极
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);

    WR_DATA(0);


    E_1();
    WR_1();
    RS_1();

    E_0();
    wr_lcd(comm, 0x01); // 清屏
    wr_lcd(comm, 0x06); // 地址自动+1（光标右移）
    wr_lcd(comm, 0x0c); // 开显示 关光标
    wr_lcd(comm, 0x38); // 八位总线 两行显示

    // 打开背光
    LCD_ON();
}


/*************************************************************
 * 函数名：wr_lcd
 * 功能：向LCD屏幕写入数据或命令
 * 参数：
 *     - dat_cmd：数据/命令标志位，为1表示写入数据，为0表示写入命令
 *     - content：要写入的数据或命令
 * 返回值：无
 * 备注：该函数用于向LCD屏幕写入数据或命令。
 *       参数dat_cmd用于指示是写入数据还是命令。
 *       参数content为要写入的数据或命令内容。
 *       函数逻辑包括设置RS、WR和E等引脚状态、写入数据或命令操作。
**************************************************************/
void wr_lcd(uint8_t dat_cmd, uint8_t content)
{
    WR_DATA(content); // 写入数据或命令
    if (dat_cmd)
    {
        RS_1(); // 数据模式
        WR_0(); // 写使能
    }
    else
    {
        RS_0(); // 命令模式
        WR_0(); // 写使能
    }

    E_1(); // 使能
    Delayms(1); // 延时1ms
    E_0(); // 失能
}

/*************************************************************
 * 函数名：Display_Y
 * 功能：在LCD屏幕上指定行数显示字符串
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 *     - code: 要显示的字符串
 * 返回值：无
 * 备注：该函数将指定的字符串显示在LCD屏幕上指定的行数上。
 *       如果字符串长度超过了一行的最大长度（MAX_ONE_LINES_LENGTH），则会截断多余部分。
**************************************************************/
void Display_Y(uint8_t y, const char *code)
{
    uint8_t len = 0;
    uint8_t i = 0;

    // 每一行的起始地址
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};

    // 获取字符串的长度
    len = strlen(code);

    // 如果长度超过一行的最大长度，进行截断
    if (MAX_ONE_LINES_LENGTH < len)
    {
        len = MAX_ONE_LINES_LENGTH;
    }

    // 设置光标位置到指定行数
    wr_lcd(comm, line[y] | 0x80);

    // 逐个字符写入数据
    for (i = 0; i < len; i++)
    {
        wr_lcd(dat, code[i]);
    }
}

/*************************************************************
 * 函数名：Display_MAX
 * 功能：在LCD屏幕上显示字符串，自动换行
 * 参数：
 *     - code: 要显示的字符串
 * 返回值：无
 * 备注：该函数将指定的字符串显示在LCD屏幕上，自动进行换行。
 *       如果字符串长度超过LCD屏幕显示的最大长度（MAX_ALL_LENGTH），则会截断多余部分。
**************************************************************/
void Display_Max(const char *code)
{
    // 每一行的起始地址
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;
    uint8_t i = 0;
    uint8_t len = strlen(code);

    // 如果长度超过LCD屏幕显示的最大长度，进行截断
    if (MAX_ALL_LENGTH > len)
    {
        len = MAX_ALL_LENGTH;
    }

    for (i = 0; i < len; i++)
    {
        // 当字符序号为一行最大长度时，需要切换到下一行
        if ((MAX_ONE_LINES_LENGTH == i)
                || (MAX_TWO_LINES_LENGTH == i) || (MAX_THREE_LENGTH_LENGTH == i))
        {
            lineOffset = line[i / MAX_ONE_LINES_LENGTH] | 0x80;   // 计算下一行的LCD命令值
            wr_lcd(comm, lineOffset);           // 发送LCD命令值切换到下一行
        }

        wr_lcd(dat, code[i]);   // 发送字符数据到LCD进行显示
    }
}

/*************************************************************
 * 函数名：Display_YX_MAX
 * 功能：在LCD屏幕上显示字符串，自动换行，指定起始行和列
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 *     - x: 指定列数，取值范围为0到19
 *     - code：要显示的字符串
 * 返回值：无
 * 备注：该函数将指定的字符串显示在LCD屏幕上，自动进行换行。
 *       可以指定起始行（y）和列（x），字符串将从指定位置开始显示。
 *       如果字符串长度超过LCD屏幕显示的最大长度（MAX_ALL_LENGTH），则会截断多余部分。
**************************************************************/
void Display_YX_Max(uint8_t y, uint8_t x, const char *code)
{
    char new_code[200];
    int spaces = 0;
    uint8_t len = 0;
    uint8_t i = 0;
    uint8_t lineOffset = 0;

    // 每一行的起始地址
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};


    // 根据行数计算填充空格数
    if (y == 0)
    {
        spaces = x;
    }
    else if (y == 1)
    {
        spaces = 20 + x;
    }
    else if (y == 2)
    {
        spaces = 40 + x;
    }
    else if (y == 3)
    {
        spaces = 60 + x;
    }

    // 填充空格
    for (i = 0; i < spaces; i++)
    {
        new_code[i] = ' ';
    }

    // 将字符串 code 复制到 new_code 中，起始位置为 new_code + spaces
    strcpy(new_code + spaces, code);
    len = strlen(new_code);

    // 如果长度超过LCD屏幕显示的最大长度，进行截断
    if (MAX_ALL_LENGTH < len)
    {
        len = MAX_ALL_LENGTH;
    }

    for (i = 0; i < len; i++)
    {
        // 当字符序号为一行最大长度时，需要切换到下一行
        if ((MAX_ONE_LINES_LENGTH == i)
                || (MAX_TWO_LINES_LENGTH == i) || (MAX_THREE_LENGTH_LENGTH ==  i))
        {
            lineOffset = line[i / MAX_ONE_LINES_LENGTH] | 0x80;   // 计算下一行的LCD命令值
            wr_lcd(comm, lineOffset);           // 发送LCD命令值切换到下一行
        }

        wr_lcd(dat, new_code[i]);   // 发送字符数据到LCD进行显示
    }
}

/*************************************************************
 * 函数名：Display_YX
 * 功能：在LCD屏幕上显示字符串，指定起始行和列
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 *     - x: 指定列数，取值范围为0到19
 *     - code：要显示的字符串
 * 返回值：无
 * 备注：该函数将指定的字符串显示在LCD屏幕上。
 *       可以指定起始行（y）和列（x），字符串将从指定位置开始显示。
 *       如果字符串长度超过LCD屏幕显示一行的最大长度（MAX_ONE_LINES_LENGTH），则会截断多余部分。
**************************************************************/
void Display_YX(uint8_t y, uint8_t x, const char *code)
{
    uint8_t i;
    uint8_t lineOffset = 0;

    // 行偏移地址，每一行对应一个偏移值
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};

    // 计算行偏移地址，最高位设置为1，表示控制命令
    lineOffset = line[y] | 0x80;

    // 遍历从x开始的字符，并检查是否超过屏幕范围或遇到字符串结束符
    for (i = x; (MAX_ONE_LINES_LENGTH > i) && (code[i - x] != '\0'); i++)
    {
        wr_lcd(comm, lineOffset + i);    // 发送控制命令，设置显示位置为指定行的第i列
        wr_lcd(dat, code[i - x]);     // 发送数据，将指定字符显示在屏幕上
    }
}

/*************************************************************
 * 函数名：Display_Centered
 * 功能：在LCD屏幕上居中显示字符串
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 *     - code：要显示的字符串
 * 返回值：无
 * 备注：该函数将指定的字符串居中显示在LCD屏幕上。
 *       字符串将在指定行（y）上居中显示。
 *       如果字符串长度超过LCD屏幕显示一行的最大长度（MAX_ONE_LINES_LENGTH），则会截断多余部分。
**************************************************************/
void Display_Centered(uint8_t y, const char *code)
{
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    uint8_t x = 0;
    uint8_t lineOffset = 0;
    uint8_t i = 0;
    uint8_t len = 0;

    len = strlen(code);
    if (MAX_ONE_LINES_LENGTH < len)
    {
        len = MAX_ONE_LINES_LENGTH;
    }

    // 计算居中的起始位置
    x = (MAX_ONE_LINES_LENGTH - len) / 2;
    lineOffset = line[y] | 0x80;
    for (i = 0; i < len; i++)
    {
        wr_lcd(comm, lineOffset + x + i);
        wr_lcd(dat, code[i]);
    }
}


/*************************************************************
 * 函数名：Display_YX_Format
 * 功能：按指定坐标在LCD屏幕上显示格式化字符串
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 *     - x: 指定列数，取值范围为0到19
 *     - format: 格式化字符串
 *     - ...: 可变参数列表
 * 返回值：无
 * 备注：该函数会将可变参数列表与格式化字符串进行格式化，并将结果显示在指定坐标的LCD屏幕上。
 *       显示的字符串最长为20个字符，超过的部分将被截断。
**************************************************************/
void Display_YX_Format(uint8_t y, uint8_t x, const char *format, ...)
{
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;
    uint8_t len = 0;
    uint8_t i = 0;
    int n = 0;

    // 显示的字符串最长为20个字符
    char code[MAX_ONE_LINES_LENGTH];

    // 将可变参数列表存储在va_list中
    va_list args;
    va_start(args, format);

    // 使用vsnprintf来格式化可变参数列表
    n = vsnprintf(code, sizeof(code), format, args);

    // 清理va_list
    va_end(args);

    // 格式化失败或结果字符串长度超过了最大长度
    if (n < 0 || n >= sizeof(code))
    {
        return;
    }

    // 根据指定的坐标进行显示
    len = strlen(code);
    len = (len > (MAX_ONE_LINES_LENGTH - x)) ? (MAX_ONE_LINES_LENGTH - x) : len;
    lineOffset = line[y] | 0x80;

    for (i = 0; i < len; i++)
    {
        wr_lcd(comm, lineOffset + x + i);
        wr_lcd(dat, code[i]);
    }
}

/**
 * Display_Centered_Format函数用于在LCD显示屏上显示居中的字符串。
 *
 *     - y: 指定行数，取值范围为0到3
 *     - format    格式化字符串，包含要显示的文本以及可选的格式控制符。
 *     - ...       可变参数列表，用于被格式化替换的值。
 *
 * 注：函数采用可变参数列表的方式，可以根据需要插入不同类型和数量的参数，并将它们格式化为字符串。
 *    通过使用vsnprintf来处理可变参数列表，可以防止结果字符串溢出。
 */
void Display_Centered_Format(uint8_t y, const char *format, ...)
{
    uint8_t len = 0;
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    int n = 0;
    uint8_t x = 0;
    uint8_t lineOffset = 0;
    uint8_t i = 0;

    // 将可变参数列表存储在va_list中
    va_list args;
    va_start(args, format);

    // 假设显示的字符串最长为20个字符
    char code[MAX_ONE_LINES_LENGTH + 1];

    // 使用vsnprintf来格式化可变参数列表
    n = vsnprintf(code, sizeof(code), format, args);
    len = strlen(code);

    // 清理va_list
    va_end(args);

    // 格式化失败或结果字符串长度超过了最大长度
    if ((n < 0) || (n >= sizeof(code)))
    {
        return;
    }

    if (MAX_ONE_LINES_LENGTH < len)
    {
        len = MAX_ONE_LINES_LENGTH;
    }

    // 计算居中的起始位置
    x = (MAX_ONE_LINES_LENGTH - len) / 2;
    lineOffset = line[y] | 0x80;

    for (i = 0; i < len; i++)
    {
        wr_lcd(comm, lineOffset + x + i);
        wr_lcd(dat, code[i]);
    }
}


/*************************************************************
 * 函数名：Clear_Line
 * 功能：清除LCD屏幕上指定行的内容
 * 参数：
 *     - y: 指定行数，取值范围为0到3
 * 返回值：无
 * 备注：该函数将LCD屏幕上指定行的内容全部清除。
 *       参数y表示要清除的行，行数从0开始计数。
 *       清除的内容将被替换为空格字符。
**************************************************************/
void Clear_Line(uint8_t y)
{
    uint8_t i;
    uint8_t line[5] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;

    // 计算行偏移地址，最高位设置为1，表示控制命令
    lineOffset = line[y] | 0x80;

    for (i = 0; i < MAX_ONE_LINES_LENGTH; i++)
    {
        wr_lcd(comm, lineOffset + i);
        wr_lcd(dat, ' '); // 用空格字符替换内容
    }
}

/*************************************************************
 * 函数名：Clear_All_Lines
 * 功能：清空LCD屏幕上的所有行
 * 参数：无
 * 返回值：无
 * 备注：该函数将LCD屏幕上的所有行内容全部清空。
 *       通过循环调用Clear_Line函数对每一行进行清空操作。
**************************************************************/
void Clear_All_Lines(void)
{
    uint8_t y;

    for (y = 0; y < MAX_LINES; y++)
    {
        Clear_Line(y);
    }
}

/***************************************************************
 * 函数名：Vertical_Scrolling_Display
 * 功能：垂直滚动显示函数，在LCD屏幕上逐行显示消息数组的内容
 * 参数：
 *     - Msg: 消息数组，包含要在LCD上显示的内容，每行最多MAX_ONE_LINES_LENGTH个字符
 *     - numLines: 消息行数，取值范围为0到4
 *     - currentStartLine: 当前起始行索引，取值范围为0到3
 * 返回值：无
 * 备注：该函数通过循环的方式在LCD屏幕上逐行显示消息数组的内容，
***********************************************/

void Vertical_Scrolling_Display(char (* const Msg)[MAX_ONE_LINES_LENGTH], uchar numLines, uchar currentStartLine)
{
    uchar startLine = currentStartLine;
    uchar lineIndex = 0;
    uchar i = 0; // 用于循环计数

    // 逐行显示消息
    for (i = 0; i < numLines; i++)
    {
        Clear_Line(i); // 清空LCD第i行内容
        lineIndex = (startLine + i) % numLines;  // 计算消息数组中的行索引

        // 在LCD第i行显示消息
        Display_YX(i, 0, Msg[lineIndex]);  // 第i行第0列开始显示
    }
}

/***********************************************
 * 函数名：addString
 * 功能：向消息数组指定位置添加字符串
 * 参数：
 *     - line: 消息数组中的行索引，取值范围为0到3
 *     - column: 消息数组中指定行的列索引，取值范围为0到19
 *     - str: 要添加的字符串
 * 返回值：无
 * 备注：该函数将字符串str添加到消息数组Msg的指定位置（line行，column列）。
 *       如果行索引line小于MAX_LINES且列索引column小于MAX_ONE_LINES_LENGTH，则执行添加操作。
 *       添加前，会先遍历整个消息数组Msg，将每行未使用的部分用空格补齐至MAX_ONE_LINES_LENGTH长度。
 *       如果字符串str长度超过剩余空间，则只复制部分字符串。
*******************************************************/
void addString(uchar line, uchar column, const char *str, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH])
{
    int i = 0;
    int j = 0;

    if (line < MAX_LINES && column < MAX_ONE_LINES_LENGTH)
    {
        // 对Msg数组每一行进行字符补齐
        for (i = 0; i < MAX_LINES; i++)
        {
            int length = strlen(Msg[i]);
            for (j = length; j < MAX_ONE_LINES_LENGTH; j++)
            {
                Msg[i][j] = ' ';  // 用空格补齐字符
            }
        }

        // 复制字符串到指定位置
        strncpy(Msg[line] + column, str, MAX_ONE_LINES_LENGTH - column);
    }
}

void addFormattedString(uchar line, uchar column, const char *format, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH], ...)
{
    int i = 0;
    int j = 0;

    if (line < MAX_LINES && column < MAX_ONE_LINES_LENGTH)
    {
        // 对Msg数组每一行进行字符补齐
        for (i = 0; i < MAX_LINES; i++)
        {
            int length = strlen(Msg[i]);
            for (j = length; j < MAX_ONE_LINES_LENGTH; j++)
            {
                Msg[i][j] = ' ';  // 用空格补齐字符
            }
        }

        // 格式化字符串
        va_list args;
        va_start(args, format);
        char formattedStr[MAX_ONE_LINES_LENGTH - column];
        vsnprintf(formattedStr, sizeof(formattedStr), format, args);

        // 复制格式化后的字符串到指定位置
        strncpy(Msg[line] + column, formattedStr, MAX_ONE_LINES_LENGTH - column);

        // 结束可变参数的使用
        va_end(args);
    }
}


char startMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "    Digiwise DTA",
    "   Test Equipment ",
};

/*************************************************************
 * 函数名：Power_On_Information
 * 功能：打印开机信息
 * 参数：无
 * 返回值：无
 * 备注：该函数用于在LCD屏幕上打印开机信息。
 *       调用Display_Centered函数分别在第0行和第1行居中显示字符串。
 *       字符串内容为开机信息。
**************************************************************/
void POWER_ON_Information(void)
{
    Vertical_Scrolling_Display(startMsgBuff, 2, 0);
}
