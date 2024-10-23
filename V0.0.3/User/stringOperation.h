#ifndef 	__STRING_OPERATION_H_
#define 	__STRING_OPERATION_H_


// ��������
//=============================================================
// ������ת���ַ���
void STRING_IntToStr(uint32 param, uint8 *str);

// �ַ���ת��ֵ
uint32 STRING_StrToVal(uint8 *str);

// �������ַ���������һ��
void STRING_CatStr(uint8 *dst, uint8 *src);

// ��ָ�����ַ����������Ӵ��������򷵻��ַ������״γ����Ӵ��ĵ�ַ�����򷵻ؿ�ָ��
int16 STRING_Find(const uint8 *mStr, const uint8 *subStr);

// �����ַ�����ָ���ַ����ɹ����ظ��ַ��״γ��ֵ��±�λ�ã����򷵻�-1
int16 STRING_FindCh(const uint8 *str, uint8 ch);

// �����ַ�����ָ���ַ��ĸ���
int16 STRING_CalculateCh(const uint8 *str, uint8 ch);

// �ж������ַ����Ƿ����
BOOL STRING_CmpStr(const uint8 *str1, const uint8 *str2);

// ɾ��ָ���ַ����е�ָ���ַ������سɹ�ɾ���ĸ���
uint16 STRING_DeleteCh(uint8 str[], uint8 ch);

// ��ָ�����ַ�����ĩβ1��ʼ��ȡ��������ǰ count ���ַ�
void STRING_Right(const char *str, uint8 count, char arr[]);

// ��ָ�����ַ����Ŀ�ʼλ�ã���ȡcount���ַ�Ϊ�Ӵ�
void STRING_Left(const uint8 *str, uint8 count, uint8 arr[]);

// ��ָ�����ַ�����ָ���±�λ�ÿ�ʼ����ȡ�м�count���ַ���Ϊ�Ӵ�
void STRING_Mid(const uint8 *str, uint8 startNum, int8 count, uint8 arr[]);

// ��ָ�����ַ����ĵ�һ���ַ����ڶ����ַ���Ϊһ���µ��ִ�
BOOL STRING_FromTo(const uint8 *str, uint8 firstChar, uint8 secondChar, uint8 arr[]);

// �õ�ָ����ŵĶ���λ��
int16 STRING_GPS_GetCommaPosition(const uint8 *str, uint16 num);

// �õ�ָ����ŵĵ�m�����ź�n������֮����ַ���Ϊ���ַ���
BOOL STRING_GPS_FormCommaStr(const uint8 *str, uint16 mNum, uint16 nNum, uint8 arr[]);

uint8 STRING_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len);


#endif

