
#ifndef _USAR_NVM_
#define _USAR_NVM_

//初始化
void User_NVM_Init();
//读到的数据内容和长度
void read_NVM_record(unsigned char *data, unsigned char *len);
//更新数据
void updata_NVM_Record(unsigned char *data, unsigned char len);


#endif