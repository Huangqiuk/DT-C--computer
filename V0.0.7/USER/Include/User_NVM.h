
#ifndef _USAR_NVM_
#define _USAR_NVM_

//��ʼ��
void User_NVM_Init();
//�������������ݺͳ���
void read_NVM_record(unsigned char *data, unsigned char *len);
//��������
void updata_NVM_Record(unsigned char *data, unsigned char len);


#endif