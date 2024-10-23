#include "lcd.h"
#include "common.h" 
#include "timer.h"
#include "delay.h"
#include "spiflash.h"
#include "font.h"



LCD_CB lcdCB;

//***************************************************************
#define LCD_WriteColor(color)				do{\
													LCD_DAT=(color>>8);\
													LCD_DAT=color;\
												}while(0)

#define LCD_WriteGRAM()						LCD_CMD=0x002C

// 图片数据在Flash中的首地址
uint32 g_PIC_StartAddr[] = {
	0x00000100, 	0x00000498, 	0x0000185C, 	0x0000A45C, 	0x0000B518, 	0x00014118, 	0x00015518, 	0x00015560, 	0x00015628, 	0x00015640, 	0x000171C0, 	0x0001724C, 	0x0001748E, 	0x000176D0, 	0x00017912, 	0x00017B54, 
	0x00017D96, 	0x0001826E, 	0x00018386, 	0x0001A4FE, 	0x0001C676, 	0x0001E7EE, 	0x00020966, 	0x00022ADE, 	0x00024C56, 	0x00026DCE, 	0x00028F46, 	0x0002B0BE, 	0x0002D236, 	0x0002F3AE, 	0x00031526, 	0x0003369E, 
	0x00035816, 	0x0003798E, 	0x00039B06, 	0x0003BC7E, 	0x0003DDF6, 	0x0003FF6E, 	0x000420E6, 	0x0004425E, 	0x000463D6, 	0x000467C6, 	0x0004893E, 	0x0004AAB6, 	0x0004CC2E, 	0x0004EDA6, 	0x0004F1C6, 	0x0004F5B6, 
	0x0005172E, 	0x000538A6, 	0x00055A1E, 	0x00055F96, 	0x000563B6, 	0x0005852E, 	0x0005A6A6, 	0x0005ACC6, 	0x0005B2E6, 	0x0005BBE6, 	0x0005DD5E, 	0x0005FED6, 	0x0006204E, 	0x000641C6, 	0x0006633E, 	0x000684B6, 
	0x0006A62E, 	0x0006C7A6, 	0x0006E91E, 	0x00070A96, 	0x00071316, 	0x000715BE, 	0x00071A96, 	0x0007228E, 	0x00072A86, 	0x0007327E, 	0x00073A76, 	0x0007426E, 	0x00074A66, 	0x0007525E, 	0x00075A56, 	0x0007624E, 
	0x00076A46, 	0x0007723E, 	0x00077DA2, 	0x00078906, 	0x0007946A, 	0x00079FCE, 	0x0007AB32, 	0x0007B696, 	0x0007C1FA, 	0x0007CD5E, 	0x0007D8C2, 	0x0007E426, 	0x0007E76E, 	0x0007EAB6, 	0x0007EDFE, 	0x0007F146, 
	0x0007F48E, 	0x0007F7D6, 	0x0007FB1E, 	0x0007FE66, 	0x000801AE, 	0x000804F6, 	0x000806C4, 	0x00080892, 	0x00080A60, 	0x00080C2E, 	0x00080DFC, 	0x00080FCA, 	0x00081198, 	0x00081366, 	0x00081534, 	0x00081702, 
	0x000818D0, 	0x00081948, 	0x000819C0, 	0x00081A38, 	0x00081AB0, 	0x00081B28, 	0x00081BA0, 	0x00081C18, 	0x00081C90, 	0x00081D08, 	0x00081D80, 	0x00082230, 	0x00083154, 	0x00084078, 	0x00084F9C, 	0x00085EC0, 
	0x00086DE4, 	0x00087D08, 	0x00088C2C, 	0x00089B50, 	0x0008AA74, 	0x0008B998, 	0x0008C8BC, 	0x0008D7E0, 	0x0008E704, 	0x0008E7CC, 	0x0008E894, 	0x00098894, 	0x00098A38, 	0x000A2A38, 	0x000ACA38, 	0x000ACEE8, 
	0x000AD398, 	0x000AD848, 	0x000ADCF8, 	0x000AE1A8, 	0x000AE658, 	0x000AEB08, 	0x000AEF26, 	0x000AF344, 	0x000AF762, 	0x000AFB80, 	0x000AFF9E, 	0x000B03BC, 	0x000B0C54, 	0x000B14EC, 	0x000B1D84, 	0x000B261C, 
	0x000B2EB4, 	0x000B374C, 	0x000B3FE4, 	0x000B487C, 	0x000B5114, 	0x000B59AC, 	0x000B6244, 	0x000B6ADC, 	0x000B7374, 	0x000B7C0C, 	0x000B84A4, 	0x000B8D3C, 	0x000B95D4, 
};

// 各图片的高度
uint16 g_PIC_Height[] = {
	0x002E, 	0x002E, 	0x00A0, 	0x0066, 	0x00A0, 	0x00A0, 	0x0003, 	0x000A, 	0x0003, 	0x00A0, 	0x0007, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 
	0x001F, 	0x000E, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 
	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0024, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0016, 	0x0024, 	0x0099, 
	0x0099, 	0x0099, 	0x0023, 	0x0016, 	0x0099, 	0x0099, 	0x0007, 	0x0007, 	0x0030, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 
	0x0099, 	0x0099, 	0x0099, 	0x0040, 	0x0022, 	0x001F, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 
	0x0022, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 
	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 
	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x001E, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 
	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x000A, 	0x000A, 	0x00A0, 	0x000F, 	0x00A0, 	0x00A0, 	0x003C, 	0x003C, 
	0x003C, 	0x003C, 	0x003C, 	0x003C, 	0x003C, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 
	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 
};

// 各图片的宽度
uint16 g_PIC_Width[] = {
	0x000A, 	0x0037, 	0x0070, 	0x0015, 	0x0070, 	0x0010, 	0x000C, 	0x000A, 	0x0004, 	0x0016, 	0x000A, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 
	0x0014, 	0x000A, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x000E, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x0018, 	0x000E, 	0x001C, 
	0x001C, 	0x001C, 	0x0014, 	0x0018, 	0x001C, 	0x001C, 	0x0070, 	0x0070, 	0x0018, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x0011, 	0x000A, 	0x0014, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 
	0x001E, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 
	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x0014, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 
	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x000A, 	0x000A, 	0x0080, 	0x000E, 	0x0080, 	0x0080, 	0x000A, 	0x000A, 
	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 
	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 
};


/*
// 图片在Flash中的首地址
uint32 g_PIC_StartAddr[] = {
	0x00000100, 	0x00000498, 	0x0000185C, 	0x0000A45C, 	0x0000B518, 	0x00014118, 	0x00015518, 	0x00015560, 	0x00015628, 	0x00015640, 	0x000171C0, 	0x0001724C, 	0x0001748E, 	0x000176D0, 	0x00017912, 	0x00017B54, 
	0x00017D96, 	0x0001826E, 	0x00018386, 	0x0001A4FE, 	0x0001C676, 	0x0001E7EE, 	0x00020966, 	0x00022ADE, 	0x00024C56, 	0x00026DCE, 	0x00028F46, 	0x0002B0BE, 	0x0002D236, 	0x0002F3AE, 	0x00031526, 	0x0003369E, 
	0x00035816, 	0x0003798E, 	0x00039B06, 	0x0003BC7E, 	0x0003DDF6, 	0x0003FF6E, 	0x000420E6, 	0x0004425E, 	0x000463D6, 	0x000467C6, 	0x0004893E, 	0x0004AAB6, 	0x0004CC2E, 	0x0004EDA6, 	0x0004F1C6, 	0x0004F5B6, 
	0x0005172E, 	0x000538A6, 	0x00055A1E, 	0x00055F96, 	0x000563B6, 	0x0005852E, 	0x0005A6A6, 	0x0005ACC6, 	0x0005B2E6, 	0x0005BBE6, 	0x0005DD5E, 	0x0005FED6, 	0x0006204E, 	0x000641C6, 	0x0006633E, 	0x000684B6, 
	0x0006A62E, 	0x0006C7A6, 	0x0006E91E, 	0x00070A96, 	0x00071316, 	0x000715BE, 	0x00071A96, 	0x0007228E, 	0x00072A86, 	0x0007327E, 	0x00073A76, 	0x0007426E, 	0x00074A66, 	0x0007525E, 	0x00075A56, 	0x0007624E, 
	0x00076A46, 	0x0007723E, 	0x00077DA2, 	0x00078906, 	0x0007946A, 	0x00079FCE, 	0x0007AB32, 	0x0007B696, 	0x0007C1FA, 	0x0007CD5E, 	0x0007D8C2, 	0x0007E426, 	0x0007E76E, 	0x0007EAB6, 	0x0007EDFE, 	0x0007F146, 
	0x0007F48E, 	0x0007F7D6, 	0x0007FB1E, 	0x0007FE66, 	0x000801AE, 	0x000804F6, 	0x000806C4, 	0x00080892, 	0x00080A60, 	0x00080C2E, 	0x00080DFC, 	0x00080FCA, 	0x00081198, 	0x00081366, 	0x00081534, 	0x00081702, 
	0x000818D0, 	0x00081948, 	0x000819C0, 	0x00081A38, 	0x00081AB0, 	0x00081B28, 	0x00081BA0, 	0x00081C18, 	0x00081C90, 	0x00081D08, 	0x00081D80, 	0x00082230, 	0x00083154, 	0x00084078, 	0x00084F9C, 	0x00085EC0, 
	0x00086DE4, 	0x00087D08, 	0x00088C2C, 	0x00089B50, 	0x0008AA74, 	0x0008B998, 	0x0008C8BC, 	0x0008D7E0, 	0x0008E704, 	0x0008E7CC, 	0x0008E894, 	0x00098894, 	0x00098D44, 	0x000991F4, 	0x000996A4, 	0x00099B54, 
	0x0009A004, 	0x0009A4B4, 	0x0009A964, 	0x0009AD82, 	0x0009B1A0, 	0x0009B5BE, 	0x0009B9DC, 	0x0009BDFA, 	0x0009C218, 	0x0009CAB0, 	0x0009D348, 	0x0009DBE0, 	0x0009E478, 	0x0009ED10, 	0x0009F5A8, 	0x0009FE40, 
	0x000A06D8, 	0x000A0F70, 	0x000A1808, 	0x000A20A0, 	0x000A2938, 	0x000A31D0, 	0x000A3A68, 	0x000A4300, 	0x000A4B98, 	0x000A5430, 
};

// 各图片的高度
uint16 g_PIC_Height[] = {
	0x002E, 	0x002E, 	0x00A0, 	0x0066, 	0x00A0, 	0x00A0, 	0x0003, 	0x000A, 	0x0003, 	0x00A0, 	0x0007, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 
	0x001F, 	0x000E, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 
	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0024, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0016, 	0x0024, 	0x0099, 
	0x0099, 	0x0099, 	0x0023, 	0x0016, 	0x0099, 	0x0099, 	0x0007, 	0x0007, 	0x0030, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 	0x0099, 
	0x0099, 	0x0099, 	0x0099, 	0x0040, 	0x0022, 	0x001F, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 
	0x0022, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x001B, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 
	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000F, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 	0x000B, 
	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x0006, 	0x001E, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 
	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x0022, 	0x000A, 	0x000A, 	0x00A0, 	0x003C, 	0x003C, 	0x003C, 	0x003C, 	0x003C, 
	0x003C, 	0x003C, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x001F, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 
	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 	0x0032, 
};

// 各图片的宽度
uint16 g_PIC_Width[] = {
	0x000A, 	0x0037, 	0x0070, 	0x0015, 	0x0070, 	0x0010, 	0x000C, 	0x000A, 	0x0004, 	0x0016, 	0x000A, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 
	0x0014, 	0x000A, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x000E, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x0018, 	0x000E, 	0x001C, 
	0x001C, 	0x001C, 	0x0014, 	0x0018, 	0x001C, 	0x001C, 	0x0070, 	0x0070, 	0x0018, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x0011, 	0x000A, 	0x0014, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 	0x001E, 
	0x001E, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x0036, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 
	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x001C, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 	0x0015, 
	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x0014, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 
	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x0039, 	0x000A, 	0x000A, 	0x0080, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 	0x000A, 
	0x000A, 	0x000A, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0011, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 
	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 	0x0016, 
};
*/
// 设定起始扫描坐标
void LCD_SetCursor(uint16 x, uint16 y)
{
	LCD_CMD = 0x002A;
	LCD_DAT = x>>8;
	LCD_DAT = x&0XFF;

	LCD_CMD = 0x002B;
	LCD_DAT = y>>8;
	LCD_DAT = y&0XFF;

	LCD_WriteGRAM();
}

// 设定操作区域，输入参数为起始x坐标、结束x坐标、起始y坐标、结束y坐标
void LCD_SetAreaBy2Point(uint16 startX, uint16 startY, uint16 endX, uint16 endY)
{
	LCD_CMD = 0x002A;
	LCD_DAT = startX>>8;
	LCD_DAT = startX&0XFF;
	LCD_DAT = endX>>8;
	LCD_DAT = endX&0XFF;

	LCD_CMD = 0x002B;
	LCD_DAT = startY>>8;
	LCD_DAT = startY&0XFF;
	LCD_DAT = endY>>8;
	LCD_DAT = endY&0XFF;

	LCD_WriteGRAM();
}

// 设定操作区域，输入参数为起始x坐标、起始y坐标、区域宽、区域高
void LCD_SetAreaByPointAndSize(uint16 startX, uint16 startY, uint16 width, uint16 height)
{
	LCD_CMD = 0x002A;
	LCD_DAT = startY>>8;
	LCD_DAT = startY&0XFF;
	LCD_DAT = (startY+width-1)>>8;
	LCD_DAT = (startY+width-1)&0XFF;

	LCD_CMD = 0x002B;
	LCD_DAT = startX>>8;
	LCD_DAT = startX&0XFF;
	LCD_DAT = (startX+height-1)>>8;
	LCD_DAT = (startX+height-1)&0XFF;

	LCD_WriteGRAM();
}

// 颜色翻转开关
void LCD_ColorInverse(BOOL onoff)
{
	if(onoff)
	{
		LCD_CMD=0x21;
	}
	else
	{
		LCD_CMD=0x20;
	}
}

// 显示开关
void LCD_DisplayOnOff(BOOL onoff)
{
	if(onoff)
	{
		LCD_CMD=0x29;
	}
	else
	{
		LCD_CMD=0x28;
	}
}

// 绘制一个像素
void LCD_DrawPixel(uint16 x,uint16 y,uint16 color)
{	   
	LCD_SetCursor(x, y);

	LCD_CMD = 0x002C;
	LCD_WriteColor(color);
}	

// 填充指定的颜色
void LCD_FillColor(uint16 color)
{
	int i,j;

	// 从原点开始扫描
	LCD_SetAreaByPointAndSize(0, 0, LCD_WIDTH, LCD_HEIGHT);

	for (i=0;i<LCD_HEIGHT;i++)
	{
		for (j=0;j<LCD_WIDTH;j++)
		{
			LCD_WriteColor(color);
		}
	}
}

// LCD驱动芯片初始化
void LCD_DriverInit(void)
{
	LCD_RST_H();
	Delayms(1);
	LCD_RST_L();
	Delayms(10);
	LCD_RST_H();
	Delayms(120);

	LCD_CMD = 0x11; //Sleep out 
	Delayms (120);         //Delay 120ms  

	LCD_CMD = 0xB1; 
	LCD_DAT = 0x01; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x05; 

	LCD_CMD = 0xB2;
	LCD_DAT = 0x01; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x05; 

	LCD_CMD = 0xB3; 
	LCD_DAT = 0x01; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x05; 
	LCD_DAT = 0x05; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x05; 

	LCD_CMD = 0xB4; //Dot inversion 
	LCD_DAT = 0x03; 

	LCD_CMD = 0xC0; 
	LCD_DAT = 0x28; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x04; 

	LCD_CMD = 0xC1;
	LCD_DAT = 0XC0; 

	LCD_CMD = 0xC2; 
	LCD_DAT = 0x0D; 
	LCD_DAT = 0x00; 

	LCD_CMD = 0xC3; 
	LCD_DAT = 0x8D; 
	LCD_DAT = 0x2A; 

	LCD_CMD = 0xC4; 
	LCD_DAT = 0x8D; 
	LCD_DAT = 0xEE;

	LCD_CMD = 0xC5; //VCOM 
	LCD_DAT = 0x1a;  

	// 设置扫描方向
	LCD_CMD = 0x36; //MX, MY, RGB mode 
	LCD_DAT = 0x40;

	LCD_CMD = 0xE0; 
	LCD_DAT = 0x07; 
	LCD_DAT = 0x18; 
	LCD_DAT = 0x0c; 
	LCD_DAT = 0x15; 
	LCD_DAT = 0x2E; 
	LCD_DAT = 0x2a; 
	LCD_DAT = 0x23; 
	LCD_DAT = 0x28; 
	LCD_DAT = 0x28; 
	LCD_DAT = 0x28; 
	LCD_DAT = 0x2e; 
	LCD_DAT = 0x39; 
	LCD_DAT = 0x00; 
	LCD_DAT = 0x03;   
	LCD_DAT = 0x02; 
	LCD_DAT = 0x10; 
//	LCD_DAT = 0x13; 

	LCD_CMD = 0xE1; 
	LCD_DAT = 0x06; 
	LCD_DAT = 0x23; 
	LCD_DAT = 0x0d; 
	LCD_DAT = 0x17; 
	LCD_DAT = 0x35; 
	LCD_DAT = 0x30; 
	LCD_DAT = 0x2a; 
	LCD_DAT = 0x2d; 
	LCD_DAT = 0x2c; 
	LCD_DAT = 0x29; 
	LCD_DAT = 0x31; 
	LCD_DAT = 0x3B; 
	LCD_DAT = 0x00; 
	LCD_DAT = 0x02; 
	LCD_DAT = 0x03; 
	LCD_DAT = 0x12;  

	LCD_CMD = 0x3A; //65k mode 
	LCD_DAT = 0x05; 

	LCD_CMD = 0x29; //Display on

/*	// 初始化
	LCD_CMD = 0x00CF;  
	LCD_DAT = 0x00; 
	LCD_DAT = 0xD9; 
	LCD_DAT = 0X30; 

	LCD_CMD = 0x00ED;  
	LCD_DAT = 0x64; 
	LCD_DAT = 0x03; 
	LCD_DAT = 0X12; 
	LCD_DAT = 0X81; 

	LCD_CMD = 0x00E8;  
	LCD_DAT = 0x85; 
	LCD_DAT = 0x10; 
	LCD_DAT = 0x78; 

	LCD_CMD = 0x00CB;  
	LCD_DAT = 0x39; 
	LCD_DAT = 0x2C; 
	LCD_DAT = 0x00; 
	LCD_DAT = 0x34; 
	LCD_DAT = 0x02; 

	LCD_CMD = 0x00F7;  
	LCD_DAT = 0x20; 

	LCD_CMD = 0x00EA;  
	LCD_DAT = 0x00; 
	LCD_DAT = 0x00; 

	LCD_CMD = 0x00C0;    //Power control 
	LCD_DAT = 0x21;   //VRH[5:0] 

	LCD_CMD = 0x00C1;    //Power control 
	LCD_DAT = 0x12;   //SAP[2:0];BT[3:0] 

	LCD_CMD = 0x00C5;    //VCM control 
	LCD_DAT = 0x32; 
	LCD_DAT = 0x3C; 

	LCD_CMD = 0x00C7;    //VCM control2 
	LCD_DAT = 0XC1; 

	// 设置行列扫描方向，设定左上方为原点
	LCD_CMD = 0x0036;    // Memory Access Control 
	LCD_DAT = 0x08;		// D7控制水平镜像，D6控制垂直镜像

	LCD_CMD = 0x003A;   
	LCD_DAT = 0x55; 

	LCD_CMD = 0x00B1;   
	LCD_DAT = 0x00;   
	LCD_DAT = 0x18; 

	LCD_CMD = 0x00B6;    // Display Function Control 
	LCD_DAT = 0x0A; 
	LCD_DAT = 0xA2; 

	LCD_CMD = 0x00F2;    // 3Gamma Function Disable 
	LCD_DAT = 0x00; 

	LCD_CMD = 0x0026;    //Gamma curve selected 
	LCD_DAT = 0x01; 

	LCD_CMD = 0x00E0;    //Set Gamma 
	LCD_DAT = 0x0F; 
	LCD_DAT = 0x20; 
	LCD_DAT = 0x1E; 
	LCD_DAT = 0x09; 
	LCD_DAT = 0x12; 
	LCD_DAT = 0x0B; 
	LCD_DAT = 0x50; 
	LCD_DAT = 0XBA; 
	LCD_DAT = 0x44; 
	LCD_DAT = 0x09; 
	LCD_DAT = 0x14; 
	LCD_DAT = 0x05; 
	LCD_DAT = 0x23; 
	LCD_DAT = 0x21; 
	LCD_DAT = 0x00; 

	LCD_CMD = 0XE1;    //Set Gamma 
	LCD_DAT = 0x00; 
	LCD_DAT = 0x19; 
	LCD_DAT = 0x19; 
	LCD_DAT = 0x00; 
	LCD_DAT = 0x12; 
	LCD_DAT = 0x07; 
	LCD_DAT = 0x2D; 
	LCD_DAT = 0x28; 
	LCD_DAT = 0x3F; 
	LCD_DAT = 0x02; 
	LCD_DAT = 0x0A; 
	LCD_DAT = 0x08; 
	LCD_DAT = 0x25; 
	LCD_DAT = 0x2D; 
	LCD_DAT = 0x0F; 

	LCD_CMD = 0x0011;    //Exit Sleep 
	Delayms(120); 
	LCD_CMD = 0x0029;    //Display on 
*/
}

// LCD控制端口初始化
void LCD_CtrlPortInit(void)
{
	// 使能端口时钟
	rcu_periph_clock_enable(RCU_GPIOA);
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOD);
	rcu_periph_clock_enable(RCU_GPIOE);

	// LCD背光控制端口设置为输出
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
	
	// LCD复位信号控制端口设置为输出
	gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0); 

	// 设置 PD.00(D2), PD.01(D3), PD.04(NOE-RD), PD.05(NWE-WR), PD.07(NE1-CS)，PD.08(D13), PD.09(D14),
	// PD.10(D15), PD.12(A17-RS), PD.14(D0), PD.15(D1) 为复用推挽输出
	gpio_init(GPIOD, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | 
														GPIO_PIN_1 |
														GPIO_PIN_4 |
														GPIO_PIN_5 |
														GPIO_PIN_7 |
														GPIO_PIN_11 |
														GPIO_PIN_14 |
														GPIO_PIN_15);

	gpio_init(GPIOE, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_7 |
														GPIO_PIN_8 |
														GPIO_PIN_9 |
														GPIO_PIN_10);
}

/*******************************************************************************
*    函数名: LCD_FSMCInit
*    参  数: 无
*    返  回: 无
*    功  能: 配置FSMC驱动TFT配置
********************************************************************************/ 
void LCD_FSMCInit(void)
{
	// GD32
    exmc_norsram_parameter_struct lcd_init_struct;
    exmc_norsram_timing_parameter_struct lcd_timing_init_struct;	

	// 使能FSMC时钟
	rcu_periph_clock_enable(RCU_EXMC);
	exmc_norsram_deinit(EXMC_BANK0_NORSRAM_REGION0);

    /*-- FSMC Configuration ------------------------------------------------------*/
    /* FSMC_Bank1_NORSRAM1 configuration */
	lcd_timing_init_struct.asyn_address_setuptime = 1;
	lcd_timing_init_struct.asyn_address_holdtime = 0;
	lcd_timing_init_struct.asyn_data_setuptime = 2;
	lcd_timing_init_struct.bus_latency = 0;
	lcd_timing_init_struct.syn_clk_division = EXMC_SYN_CLOCK_RATIO_DISABLE;
	lcd_timing_init_struct.syn_data_latency = EXMC_DATALAT_2_CLK;
	lcd_timing_init_struct.asyn_access_mode = EXMC_ACCESS_MODE_B;

    /* Color LCD configuration ------------------------------------
     LCD configured as follow:
        - Data/Address MUX = Disable
        - Memory Type = SRAM
        - Data Width = 16bit
        - Write Operation = Enable
        - Extended Mode = Enable
        - Asynchronous Wait = Disable */
	lcd_init_struct.norsram_region = EXMC_BANK0_NORSRAM_REGION0;
	lcd_init_struct.address_data_mux = DISABLE;
   	lcd_init_struct.memory_type = EXMC_MEMORY_TYPE_PSRAM;	
	lcd_init_struct.asyn_wait = DISABLE;
	lcd_init_struct.databus_width = EXMC_NOR_DATABUS_WIDTH_16B;
	lcd_init_struct.burst_mode = DISABLE;
	lcd_init_struct.nwait_polarity = EXMC_NWAIT_POLARITY_LOW;
	lcd_init_struct.wrap_burst_mode = DISABLE;	
	lcd_init_struct.nwait_config = EXMC_NWAIT_CONFIG_BEFORE;	
	lcd_init_struct.memory_write = ENABLE;
    lcd_init_struct.nwait_signal = DISABLE;
	lcd_init_struct.extended_mode = DISABLE;
	lcd_init_struct.write_mode = EXMC_ASYN_WRITE;								// 异步写
    lcd_init_struct.read_write_timing = &lcd_timing_init_struct;
    lcd_init_struct.write_timing = &lcd_timing_init_struct;
	exmc_norsram_init(&lcd_init_struct);

	/* - BANK 1 (of NOR/SRAM Bank 0~3) is enabled */
    exmc_norsram_enable(EXMC_BANK0_NORSRAM_REGION0);
}

// LCD驱动模块初始化入口
void LCD_Init(void)
{
	LCD_CtrlPortInit();

	LCD_FSMCInit();

	LCD_DriverInit();

	LCD_FillColor(LCD_COLOR_BLACK);
	
	// LCD背光
	LCD_BL_OFF();

//	LCD_BL_ON();
//	
//	LCD_FillColor(LCD_COLOR_RED);
//	LCD_FillColor(LCD_COLOR_GREEN);
//	LCD_FillColor(LCD_COLOR_BLUE);	
}

// LCD处理过程函数
void LCD_Process(void)
{
}

// 在指定位置显示指定的图片
void LCD_DrawPic(uint16 x, uint16 y, const uint16* pData)
{
	uint16 width=0, height=0;
	uint32 i;
	uint32 size;
	
	if(NULL == pData)
	{
		return;
	}

	width = pData[0];
	height = pData[1];
	size = width;
	size *= height;

	LCD_SetAreaByPointAndSize(x, y, width, height);

	for(i=0; i<size; i++)
	{
		LCD_WriteColor(pData[2+i]);
	}
}

// 在指定区域显示指定的颜色
void LCD_DrawAreaColor(uint16 x, uint16 y, uint16 w, uint16 h, uint16 data)
{
	uint32 i;
	uint32 size = w * h;	

	/* 需要添加参数判断 */
	CHECK_PARAM_OVER_RETURN(w,LCD_WIDTH);
	CHECK_PARAM_OVER_RETURN(h,LCD_HEIGHT);

	LCD_SetAreaByPointAndSize(x, y, w, h);

	for(i=0; i<size; i++)
	{
		LCD_WriteColor(data);
	}
}

// 画直线
void LCD_DrawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2, uint16 usColor, uint16 usLineWidth)
{

	uint16 t; 

	int xerr=0,yerr=0,delta_x,delta_y,distance; 

	int incx,incy,uRow,uCol; 

	delta_x=x2-x1; //计算坐标增量 

	delta_y=y2-y1; 

	uRow=x1; 

	uCol=y1; 

	if(delta_x>0)incx=1; //设置单步方向 

	else if(delta_x==0)incx=0;//垂直线 

	else {incx=-1;delta_x=-delta_x;} 

	if(delta_y>0)incy=1; 

	else if(delta_y==0)incy=0;//水平线 

	else{incy=-1;delta_y=-delta_y;} 

	if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴 

	else distance=delta_y; 

	for(t=0;t<=distance+1;t++ )//画线输出 

	{  

		LCD_DrawAreaColor(uRow,uCol, 3, 3, usColor);//画点 

		xerr+=delta_x ; 

		yerr+=delta_y ; 

		if(xerr>distance) 

		{ 

			xerr-=distance; 

			uRow+=incx; 

		} 

		if(yerr>distance) 

		{ 

			yerr-=distance; 

			uCol+=incy; 

		} 

	}  

}  


// 显示指定位置的图片数据，在参数中给出要显示的图片数据在SPI Flash中的首地址
void LCD_DrawPicFromAddress(uint16 x, uint16 y, uint16 w, uint16 h, uint32 addr)
{
#if 0	// NO DMA
	uint32 size		= 0;
	uint8  data[2];

	/* 设定操作区域 */
	LCD_SetAreaByPointAndSize(x, y, w, h);

	/* 根据图片尺寸与数据作图 */
	size = w * h;	

	// 片选
	SPI_FLASH_CS_L();

	// 发送读取命令
	(void)SPI_FLASH_SendByte(CMD_READ_BYTE);

	// 发送24位起始地址
	(void)SPI_FLASH_SendByte((addr & 0xFF0000) >> 16);
	(void)SPI_FLASH_SendByte((addr& 0xFF00) >> 8);
	(void)SPI_FLASH_SendByte(addr & 0xFF);

	// 一直读取完指定的数量为止
	while(size--)
	{
		// 读取一个字节
		data[1] = SPI_FLASH_SendByte(0);
		data[0] = SPI_FLASH_SendByte(0);

		LCD_WriteColor( *((uint16*)data) );
	}

	// 释放
	SPI_FLASH_CS_H();
#else	// DMA
	uint32 size = 0, offset = 0;
	uint32 srcAddr = addr;

	/* 设定操作区域 */
	LCD_SetAreaByPointAndSize( x, y, w, h );

	/* 根据图片尺寸与数据作图 */
	size = w;
	size *= h;
	size *= 2;
	offset = y;
	offset *= LCD_WIDTH;
	offset += x;

#define _DMA_MOVE_BYTES_COUNT_TOP_			0xD000
	while(size > 0)
	{
		// 超过搬运上限，分多次
		if(size > _DMA_MOVE_BYTES_COUNT_TOP_)
		{
			SPIx_DMAy_Read(srcAddr, 0x60020000+offset, _DMA_MOVE_BYTES_COUNT_TOP_);
			srcAddr += _DMA_MOVE_BYTES_COUNT_TOP_;
			size -= _DMA_MOVE_BYTES_COUNT_TOP_;
			offset += _DMA_MOVE_BYTES_COUNT_TOP_;
		}
		// 不超过搬运上限，一次完成
		else
		{
			SPIx_DMAy_Read(srcAddr, 0x60020000+offset, size);
			break;
		}
	}
#undef _DMA_MOVE_BYTES_COUNT_TOP_
#endif
}

// 在指定位置显示指定ID的图片
void LCD_DrawBmpByID(uint16 x, uint16 y, uint8 id, uint8 mode)
{
	// 显示背景
	uint32 ulDataAddress = 0;
	uint16 usBackgroundColor;

	if(id >= 50)
	{
		return;
	}

	// 取出数据的首地址
	ulDataAddress = g_PIC_StartAddr[id];

	switch(mode)
	{
		// 直接绘制
		case LCD_OPERATOR_MODE_DRAW:
			LCD_DrawPicFromAddress(x, y, g_PIC_Width[id], g_PIC_Height[id], ulDataAddress);
			break;

		// 背景色
		case LCD_OPERATOR_MODE_BACKGROUND:
			usBackgroundColor = LCD_COLOR_BACKGROUND;
			LCD_DrawAreaColor(x, y, g_PIC_Width[id], g_PIC_Height[id], usBackgroundColor);
			break;

		// 全白
		case LCD_OPERATOR_MODE_ALL_WHITE:
			usBackgroundColor = LCD_COLOR_WHITE;
			LCD_DrawAreaColor(x, y, g_PIC_Width[id], g_PIC_Height[id], usBackgroundColor);
			break;

		// 全黑
		case LCD_OPERATOR_MODE_ALL_BLACK:
			usBackgroundColor = LCD_COLOR_BLACK;
			LCD_DrawAreaColor(x, y, g_PIC_Width[id], g_PIC_Height[id], usBackgroundColor);
			break;

		default:
			break;
	}
}

// 在指定坐标位置打印一个字符
void LCD_PrintChar(uint16 x, uint16 y, uint8 ch, uint16 color, LCD_OPERATOR_MODE_E mode)
{
	uint16 width=0, height=0;
	uint32 i, j;
	uint32 size;
	uint16* pData = FONT_GetAsciiDataPtr(ch);
	uint16 mask=0x0001;
	
	width = 16;//FONT_WIDTH();
	height = 8;//FONT_HEIGHT();

	size = width;
	size *= height;

	LCD_SetAreaByPointAndSize(x, y, width, height);

	for(i=0; i<height; i++)
	{
		for(j=0; j<width; j++)
		{
			if((*(pData+j)) & mask)
			{
				LCD_WriteColor(color);
			}
			else
			{
				LCD_WriteColor(0);
			}
		}

		mask <<= 1;
	}
}

// 从指定位置开始，打印一个字符串
void LCD_PrintString(uint16 x, uint16 y, char* pStr, uint16 color, LCD_OPERATOR_MODE_E mode)
{
	uint16 xPos=x, yPos=y;
	
	if(NULL == pStr)
	{
		return;
	}

	if((x >= LCD_WIDTH)||(y >= LCD_HEIGHT))
	{
		return;
	}

	while(0 != *pStr)
	{
		LCD_PrintChar(xPos, yPos, *pStr, color, mode);

		xPos += FONT_WIDTH();
		pStr ++;
	}
}

// 在指定位置绘制正方形
void LCD_DrawRect(uint16 x, uint16 y, uint8 width, uint16 color)
{
	uint8 i, j;
	uint8 xPos, yPos;
	
	for(i=0; i<width; i++)
	{
		xPos = x;
		yPos = y+i;
		for(j=0; j<width; j++)
		{
			LCD_SetCursor(xPos++, yPos);

			LCD_CMD = 0x002C;

			LCD_WriteColor(color);
		}
	}
}

// .动态显示
void LCD_DotDynamicShow(uint32 param)
{
	static uint8 count;
	
	if(4 == count)
	{
		count = 0;
	}
	if(0 == count)
	{
		LCD_PrintString(107, 50, "         ", LCD_COLOR_WHITE, LCD_OPERATOR_MODE_DRAW);
	}
	if(1 == count)
	{
		LCD_PrintString(107, 50, ".        ", LCD_COLOR_WHITE, LCD_OPERATOR_MODE_DRAW);
	}
	if(2 == count)
	{
		LCD_PrintString(107, 50, "..       ", LCD_COLOR_WHITE, LCD_OPERATOR_MODE_DRAW);
	}
	if(3 == count)
	{
		LCD_PrintString(107, 50, "...      ", LCD_COLOR_WHITE, LCD_OPERATOR_MODE_DRAW);
	}
	count++;
}
