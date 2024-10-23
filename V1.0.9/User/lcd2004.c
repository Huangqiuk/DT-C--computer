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
 * ��������WR_DATA
 * ���ܣ�������������д���ض��� GPIO �˿ڣ�GPIOE����
 * ������uint32_t data��һ���޷��� 32 λ����������Ҫ���д�������ݡ�
 * ����ֵ����
 * ��ע���ú������Ƚ�������������� 8 λ��Ϊ����������ֵ���кϲ��ڳ��� 8 λ�ռ䡣
         Ȼ�󣬽����ƺ��������ȫ�ֽṹ�� dut_info �еĳ�Ա���� VLKFlag ���а�λ�������
         �� VLKFlag ��ֵ���������ݵĵ� 8 λ�ϡ���󣬽����������ݸ�ֵ�� GPIOE ��������ƼĴ�����
         ���� GPIO_OCTL ��һ������ߺ������ڷ��� GPIOE ���ض��Ĵ�����
**************************************************************/
void WR_DATA(uint32_t data) 
{ 
    data = data<<8;
    data |=  dut_info.VLKFlag;
    GPIO_OCTL(GPIOE) = (data);
}

/*************************************************************
 * ��������Init_Lcd
 * ���ܣ���ʼ��LCD��Ļ
 * ��������
 * ����ֵ����
 * ��ע���ú������ڳ�ʼ��LCD��Ļ��GPIO���źͷ���һϵ��LCD������г�ʼ�����á�
 *       ʹ���˼Ĵ�����GPIO���Ž������ã���ͨ������wr_lcd��������LCD���
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

    // ���⸺��
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);

    WR_DATA(0);


    E_1();
    WR_1();
    RS_1();

    E_0();
    wr_lcd(comm, 0x01); // ����
    wr_lcd(comm, 0x06); // ��ַ�Զ�+1��������ƣ�
    wr_lcd(comm, 0x0c); // ����ʾ �ع��
    wr_lcd(comm, 0x38); // ��λ���� ������ʾ

    // �򿪱���
    LCD_ON();
}


/*************************************************************
 * ��������wr_lcd
 * ���ܣ���LCD��Ļд�����ݻ�����
 * ������
 *     - dat_cmd������/�����־λ��Ϊ1��ʾд�����ݣ�Ϊ0��ʾд������
 *     - content��Ҫд������ݻ�����
 * ����ֵ����
 * ��ע���ú���������LCD��Ļд�����ݻ����
 *       ����dat_cmd����ָʾ��д�����ݻ������
 *       ����contentΪҪд������ݻ��������ݡ�
 *       �����߼���������RS��WR��E������״̬��д�����ݻ����������
**************************************************************/
void wr_lcd(uint8_t dat_cmd, uint8_t content)
{
    WR_DATA(content); // д�����ݻ�����
    if (dat_cmd)
    {
        RS_1(); // ����ģʽ
        WR_0(); // дʹ��
    }
    else
    {
        RS_0(); // ����ģʽ
        WR_0(); // дʹ��
    }

    E_1(); // ʹ��
    Delayms(1); // ��ʱ1ms
    E_0(); // ʧ��
}

/*************************************************************
 * ��������Display_Y
 * ���ܣ���LCD��Ļ��ָ��������ʾ�ַ���
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - code: Ҫ��ʾ���ַ���
 * ����ֵ����
 * ��ע���ú�����ָ�����ַ�����ʾ��LCD��Ļ��ָ���������ϡ�
 *       ����ַ������ȳ�����һ�е���󳤶ȣ�MAX_ONE_LINES_LENGTH�������ض϶��ಿ�֡�
**************************************************************/
void Display_Y(uint8_t y, const char *code)
{
    uint8_t len = 0;
    uint8_t i = 0;

    // ÿһ�е���ʼ��ַ
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};

    // ��ȡ�ַ����ĳ���
    len = strlen(code);

    // ������ȳ���һ�е���󳤶ȣ����нض�
    if (MAX_ONE_LINES_LENGTH < len)
    {
        len = MAX_ONE_LINES_LENGTH;
    }

    // ���ù��λ�õ�ָ������
    wr_lcd(comm, line[y] | 0x80);

    // ����ַ�д������
    for (i = 0; i < len; i++)
    {
        wr_lcd(dat, code[i]);
    }
}

/*************************************************************
 * ��������Display_MAX
 * ���ܣ���LCD��Ļ����ʾ�ַ������Զ�����
 * ������
 *     - code: Ҫ��ʾ���ַ���
 * ����ֵ����
 * ��ע���ú�����ָ�����ַ�����ʾ��LCD��Ļ�ϣ��Զ����л��С�
 *       ����ַ������ȳ���LCD��Ļ��ʾ����󳤶ȣ�MAX_ALL_LENGTH�������ض϶��ಿ�֡�
**************************************************************/
void Display_Max(const char *code)
{
    // ÿһ�е���ʼ��ַ
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;
    uint8_t i = 0;
    uint8_t len = strlen(code);

    // ������ȳ���LCD��Ļ��ʾ����󳤶ȣ����нض�
    if (MAX_ALL_LENGTH > len)
    {
        len = MAX_ALL_LENGTH;
    }

    for (i = 0; i < len; i++)
    {
        // ���ַ����Ϊһ����󳤶�ʱ����Ҫ�л�����һ��
        if ((MAX_ONE_LINES_LENGTH == i)
                || (MAX_TWO_LINES_LENGTH == i) || (MAX_THREE_LENGTH_LENGTH == i))
        {
            lineOffset = line[i / MAX_ONE_LINES_LENGTH] | 0x80;   // ������һ�е�LCD����ֵ
            wr_lcd(comm, lineOffset);           // ����LCD����ֵ�л�����һ��
        }

        wr_lcd(dat, code[i]);   // �����ַ����ݵ�LCD������ʾ
    }
}

/*************************************************************
 * ��������Display_YX_MAX
 * ���ܣ���LCD��Ļ����ʾ�ַ������Զ����У�ָ����ʼ�к���
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - x: ָ��������ȡֵ��ΧΪ0��19
 *     - code��Ҫ��ʾ���ַ���
 * ����ֵ����
 * ��ע���ú�����ָ�����ַ�����ʾ��LCD��Ļ�ϣ��Զ����л��С�
 *       ����ָ����ʼ�У�y�����У�x�����ַ�������ָ��λ�ÿ�ʼ��ʾ��
 *       ����ַ������ȳ���LCD��Ļ��ʾ����󳤶ȣ�MAX_ALL_LENGTH�������ض϶��ಿ�֡�
**************************************************************/
void Display_YX_Max(uint8_t y, uint8_t x, const char *code)
{
    char new_code[200];
    int spaces = 0;
    uint8_t len = 0;
    uint8_t i = 0;
    uint8_t lineOffset = 0;

    // ÿһ�е���ʼ��ַ
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};


    // ���������������ո���
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

    // ���ո�
    for (i = 0; i < spaces; i++)
    {
        new_code[i] = ' ';
    }

    // ���ַ��� code ���Ƶ� new_code �У���ʼλ��Ϊ new_code + spaces
    strcpy(new_code + spaces, code);
    len = strlen(new_code);

    // ������ȳ���LCD��Ļ��ʾ����󳤶ȣ����нض�
    if (MAX_ALL_LENGTH < len)
    {
        len = MAX_ALL_LENGTH;
    }

    for (i = 0; i < len; i++)
    {
        // ���ַ����Ϊһ����󳤶�ʱ����Ҫ�л�����һ��
        if ((MAX_ONE_LINES_LENGTH == i)
                || (MAX_TWO_LINES_LENGTH == i) || (MAX_THREE_LENGTH_LENGTH ==  i))
        {
            lineOffset = line[i / MAX_ONE_LINES_LENGTH] | 0x80;   // ������һ�е�LCD����ֵ
            wr_lcd(comm, lineOffset);           // ����LCD����ֵ�л�����һ��
        }

        wr_lcd(dat, new_code[i]);   // �����ַ����ݵ�LCD������ʾ
    }
}

/*************************************************************
 * ��������Display_YX
 * ���ܣ���LCD��Ļ����ʾ�ַ�����ָ����ʼ�к���
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - x: ָ��������ȡֵ��ΧΪ0��19
 *     - code��Ҫ��ʾ���ַ���
 * ����ֵ����
 * ��ע���ú�����ָ�����ַ�����ʾ��LCD��Ļ�ϡ�
 *       ����ָ����ʼ�У�y�����У�x�����ַ�������ָ��λ�ÿ�ʼ��ʾ��
 *       ����ַ������ȳ���LCD��Ļ��ʾһ�е���󳤶ȣ�MAX_ONE_LINES_LENGTH�������ض϶��ಿ�֡�
**************************************************************/
void Display_YX(uint8_t y, uint8_t x, const char *code)
{
    uint8_t i;
    uint8_t lineOffset = 0;

    // ��ƫ�Ƶ�ַ��ÿһ�ж�Ӧһ��ƫ��ֵ
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};

    // ������ƫ�Ƶ�ַ�����λ����Ϊ1����ʾ��������
    lineOffset = line[y] | 0x80;

    // ������x��ʼ���ַ���������Ƿ񳬹���Ļ��Χ�������ַ���������
    for (i = x; (MAX_ONE_LINES_LENGTH > i) && (code[i - x] != '\0'); i++)
    {
        wr_lcd(comm, lineOffset + i);    // ���Ϳ������������ʾλ��Ϊָ���еĵ�i��
        wr_lcd(dat, code[i - x]);     // �������ݣ���ָ���ַ���ʾ����Ļ��
    }
}

/*************************************************************
 * ��������Display_Centered
 * ���ܣ���LCD��Ļ�Ͼ�����ʾ�ַ���
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - code��Ҫ��ʾ���ַ���
 * ����ֵ����
 * ��ע���ú�����ָ�����ַ���������ʾ��LCD��Ļ�ϡ�
 *       �ַ�������ָ���У�y���Ͼ�����ʾ��
 *       ����ַ������ȳ���LCD��Ļ��ʾһ�е���󳤶ȣ�MAX_ONE_LINES_LENGTH�������ض϶��ಿ�֡�
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

    // ������е���ʼλ��
    x = (MAX_ONE_LINES_LENGTH - len) / 2;
    lineOffset = line[y] | 0x80;
    for (i = 0; i < len; i++)
    {
        wr_lcd(comm, lineOffset + x + i);
        wr_lcd(dat, code[i]);
    }
}


/*************************************************************
 * ��������Display_YX_Format
 * ���ܣ���ָ��������LCD��Ļ����ʾ��ʽ���ַ���
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - x: ָ��������ȡֵ��ΧΪ0��19
 *     - format: ��ʽ���ַ���
 *     - ...: �ɱ�����б�
 * ����ֵ����
 * ��ע���ú����Ὣ�ɱ�����б����ʽ���ַ������и�ʽ�������������ʾ��ָ�������LCD��Ļ�ϡ�
 *       ��ʾ���ַ����Ϊ20���ַ��������Ĳ��ֽ����ضϡ�
**************************************************************/
void Display_YX_Format(uint8_t y, uint8_t x, const char *format, ...)
{
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;
    uint8_t len = 0;
    uint8_t i = 0;
    int n = 0;

    // ��ʾ���ַ����Ϊ20���ַ�
    char code[MAX_ONE_LINES_LENGTH];

    // ���ɱ�����б�洢��va_list��
    va_list args;
    va_start(args, format);

    // ʹ��vsnprintf����ʽ���ɱ�����б�
    n = vsnprintf(code, sizeof(code), format, args);

    // ����va_list
    va_end(args);

    // ��ʽ��ʧ�ܻ����ַ������ȳ�������󳤶�
    if (n < 0 || n >= sizeof(code))
    {
        return;
    }

    // ����ָ�������������ʾ
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
 * Display_Centered_Format����������LCD��ʾ������ʾ���е��ַ�����
 *
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 *     - format    ��ʽ���ַ���������Ҫ��ʾ���ı��Լ���ѡ�ĸ�ʽ���Ʒ���
 *     - ...       �ɱ�����б����ڱ���ʽ���滻��ֵ��
 *
 * ע���������ÿɱ�����б�ķ�ʽ�����Ը�����Ҫ���벻ͬ���ͺ������Ĳ������������Ǹ�ʽ��Ϊ�ַ�����
 *    ͨ��ʹ��vsnprintf������ɱ�����б����Է�ֹ����ַ��������
 */
void Display_Centered_Format(uint8_t y, const char *format, ...)
{
    uint8_t len = 0;
    uint8_t line[] = {0x00, 0x40, 0x14, 0x54};
    int n = 0;
    uint8_t x = 0;
    uint8_t lineOffset = 0;
    uint8_t i = 0;

    // ���ɱ�����б�洢��va_list��
    va_list args;
    va_start(args, format);

    // ������ʾ���ַ����Ϊ20���ַ�
    char code[MAX_ONE_LINES_LENGTH + 1];

    // ʹ��vsnprintf����ʽ���ɱ�����б�
    n = vsnprintf(code, sizeof(code), format, args);
    len = strlen(code);

    // ����va_list
    va_end(args);

    // ��ʽ��ʧ�ܻ����ַ������ȳ�������󳤶�
    if ((n < 0) || (n >= sizeof(code)))
    {
        return;
    }

    if (MAX_ONE_LINES_LENGTH < len)
    {
        len = MAX_ONE_LINES_LENGTH;
    }

    // ������е���ʼλ��
    x = (MAX_ONE_LINES_LENGTH - len) / 2;
    lineOffset = line[y] | 0x80;

    for (i = 0; i < len; i++)
    {
        wr_lcd(comm, lineOffset + x + i);
        wr_lcd(dat, code[i]);
    }
}


/*************************************************************
 * ��������Clear_Line
 * ���ܣ����LCD��Ļ��ָ���е�����
 * ������
 *     - y: ָ��������ȡֵ��ΧΪ0��3
 * ����ֵ����
 * ��ע���ú�����LCD��Ļ��ָ���е�����ȫ�������
 *       ����y��ʾҪ������У�������0��ʼ������
 *       ��������ݽ����滻Ϊ�ո��ַ���
**************************************************************/
void Clear_Line(uint8_t y)
{
    uint8_t i;
    uint8_t line[5] = {0x00, 0x40, 0x14, 0x54};
    uint8_t lineOffset = 0;

    // ������ƫ�Ƶ�ַ�����λ����Ϊ1����ʾ��������
    lineOffset = line[y] | 0x80;

    for (i = 0; i < MAX_ONE_LINES_LENGTH; i++)
    {
        wr_lcd(comm, lineOffset + i);
        wr_lcd(dat, ' '); // �ÿո��ַ��滻����
    }
}

/*************************************************************
 * ��������Clear_All_Lines
 * ���ܣ����LCD��Ļ�ϵ�������
 * ��������
 * ����ֵ����
 * ��ע���ú�����LCD��Ļ�ϵ�����������ȫ����ա�
 *       ͨ��ѭ������Clear_Line������ÿһ�н�����ղ�����
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
 * ��������Vertical_Scrolling_Display
 * ���ܣ���ֱ������ʾ��������LCD��Ļ��������ʾ��Ϣ���������
 * ������
 *     - Msg: ��Ϣ���飬����Ҫ��LCD����ʾ�����ݣ�ÿ�����MAX_ONE_LINES_LENGTH���ַ�
 *     - numLines: ��Ϣ������ȡֵ��ΧΪ0��4
 *     - currentStartLine: ��ǰ��ʼ��������ȡֵ��ΧΪ0��3
 * ����ֵ����
 * ��ע���ú���ͨ��ѭ���ķ�ʽ��LCD��Ļ��������ʾ��Ϣ��������ݣ�
***********************************************/

void Vertical_Scrolling_Display(char (* const Msg)[MAX_ONE_LINES_LENGTH], uchar numLines, uchar currentStartLine)
{
    uchar startLine = currentStartLine;
    uchar lineIndex = 0;
    uchar i = 0; // ����ѭ������

    // ������ʾ��Ϣ
    for (i = 0; i < numLines; i++)
    {
        Clear_Line(i); // ���LCD��i������
        lineIndex = (startLine + i) % numLines;  // ������Ϣ�����е�������

        // ��LCD��i����ʾ��Ϣ
        Display_YX(i, 0, Msg[lineIndex]);  // ��i�е�0�п�ʼ��ʾ
    }
}

/***********************************************
 * ��������addString
 * ���ܣ�����Ϣ����ָ��λ������ַ���
 * ������
 *     - line: ��Ϣ�����е���������ȡֵ��ΧΪ0��3
 *     - column: ��Ϣ������ָ���е���������ȡֵ��ΧΪ0��19
 *     - str: Ҫ��ӵ��ַ���
 * ����ֵ����
 * ��ע���ú������ַ���str��ӵ���Ϣ����Msg��ָ��λ�ã�line�У�column�У���
 *       ���������lineС��MAX_LINES��������columnС��MAX_ONE_LINES_LENGTH����ִ����Ӳ�����
 *       ���ǰ�����ȱ���������Ϣ����Msg����ÿ��δʹ�õĲ����ÿո�����MAX_ONE_LINES_LENGTH���ȡ�
 *       ����ַ���str���ȳ���ʣ��ռ䣬��ֻ���Ʋ����ַ�����
*******************************************************/
void addString(uchar line, uchar column, const char *str, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH])
{
    int i = 0;
    int j = 0;

    if (line < MAX_LINES && column < MAX_ONE_LINES_LENGTH)
    {
        // ��Msg����ÿһ�н����ַ�����
        for (i = 0; i < MAX_LINES; i++)
        {
            int length = strlen(Msg[i]);
            for (j = length; j < MAX_ONE_LINES_LENGTH; j++)
            {
                Msg[i][j] = ' ';  // �ÿո����ַ�
            }
        }

        // �����ַ�����ָ��λ��
        strncpy(Msg[line] + column, str, MAX_ONE_LINES_LENGTH - column);
    }
}

void addFormattedString(uchar line, uchar column, const char *format, char Msg[MAX_LINES][MAX_ONE_LINES_LENGTH], ...)
{
    int i = 0;
    int j = 0;

    if (line < MAX_LINES && column < MAX_ONE_LINES_LENGTH)
    {
        // ��Msg����ÿһ�н����ַ�����
        for (i = 0; i < MAX_LINES; i++)
        {
            int length = strlen(Msg[i]);
            for (j = length; j < MAX_ONE_LINES_LENGTH; j++)
            {
                Msg[i][j] = ' ';  // �ÿո����ַ�
            }
        }

        // ��ʽ���ַ���
        va_list args;
        va_start(args, format);
        char formattedStr[MAX_ONE_LINES_LENGTH - column];
        vsnprintf(formattedStr, sizeof(formattedStr), format, args);

        // ���Ƹ�ʽ������ַ�����ָ��λ��
        strncpy(Msg[line] + column, formattedStr, MAX_ONE_LINES_LENGTH - column);

        // �����ɱ������ʹ��
        va_end(args);
    }
}


char startMsgBuff[][MAX_ONE_LINES_LENGTH] =
{
    "    Digiwise DTA",
    "   Test Equipment ",
};

/*************************************************************
 * ��������Power_On_Information
 * ���ܣ���ӡ������Ϣ
 * ��������
 * ����ֵ����
 * ��ע���ú���������LCD��Ļ�ϴ�ӡ������Ϣ��
 *       ����Display_Centered�����ֱ��ڵ�0�к͵�1�о�����ʾ�ַ�����
 *       �ַ�������Ϊ������Ϣ��
**************************************************************/
void POWER_ON_Information(void)
{
    Vertical_Scrolling_Display(startMsgBuff, 2, 0);
}
