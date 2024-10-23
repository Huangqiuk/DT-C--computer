#include "Common.h"
#include "stringOperation.h"

// ===================================================================================================
// ��������
// ===================================================================================================

// ������ת���ַ���
void STRING_IntToStr(uint32 param, uint8 *str)
{
	uint32 localParam = param;
	
	// paramΪ1���ֽڵ����������ȣ�Ԥ�����һ���ֽڴ洢'\0'������ڻ�����+1
	uint8 tempStr[(sizeof(param)*8+1)] = {0};
	
	// ��ʱ�ַ����������һ��Ϊ0�������ַ�����β�����յ���ʽ���ӵ����ڶ���λ�ÿ�ʼ���±��С�ķ�������ַ�
	// �������Ϊsizeof(param)*8+1ʱ���±�Ϊ[0,sizeof(param)*8]����˵����ڶ���λ����[sizeof(param)*8+1-2]
	int8 index = (sizeof(param)*8+1) - 2;
	
	// ��������
	if (NULL == str)
	{
		return;
	}

	// �ӵ����ڶ���λ�ÿ�ʼ�����±��С�ķ����������
	do{
		tempStr[index--] = (localParam%10) + '0';
		localParam /= 10;
	}while((0 != localParam)&&(index > -1));

	// ���ھ����������������indexʵ��ָ������ǰ���һ����λ�ã��������++������ָ���һ���ǿ��ַ�
	index++;
	while(0 != tempStr[index])
	{
		*str++ = tempStr[index++];
	}
	*str = '\0';	// ��ӽ�����
}

// �ַ���ת��ֵ
uint32 STRING_StrToVal(uint8 *str)
{
	uint32 value = 0;
	uint8 *localStr = str;
	
	// ��������
	if (NULL == str)
	{
		return 0;
	}
	
	while ('\0' != *localStr)
	{
		if(((*localStr) < '0') || ((*localStr) > '9'))
		{
			return 0;
		}

		value *= 10;
		value += (*localStr)-'0';
		localStr ++;
	}
	
	return value;
}


// �������ַ���������һ��
void STRING_CatStr(uint8 *dst, uint8 *src)
{
	uint16 i = 0;
	uint16 j = 0;
	
	// ��������
	if ((NULL == dst) || (NULL == src))
	{
		return;
	}

	while (dst[i] != '\0')
	{
		i++;
	}

	while (src[j] != '\0')
	{
		dst[i++] = src[j++];
	}
	dst[i+j] = '\0';	// ����ַ���������־'\0'
	
}


// ��ָ�����ַ����������Ӵ��������򷵻��ַ������״γ����Ӵ����±��ַ�����򷵻�-1
int16 STRING_Find(const uint8 *mStr, const uint8 *subStr)
{
	uint8 *pCh = (uint8*)mStr;		
	uint8 *str1, *str2;
	volatile uint16 i = 0;	 // �Ӵ���ĸ���е��±�

	// ��������
	if ((NULL == mStr) || (NULL == subStr))
	{
		return -1;
	}

	if ('\0' == *mStr)
	{
		return -1;
	}

	if ('\0' == *subStr)
	{
		return -1;
	}

	while (*pCh != '\0')
	{				 
		str1 = pCh; 			   
		str2 = (uint8*)subStr;
		
		while ((*str1 != '\0') && (*str2 != '\0') && (*str1 == *str2))
		{
			str1++; 
			str2++;
		}
		if (*str2 == '\0') 
		{
			i = pCh - mStr;

			return i;
		}
		
		pCh++;		  
	}
	
	return -1;
}

uint8 STRING_FindLimitSpeedh(uint8 *str, uint8 ch)
{
	uint8 i = 0;

	for (i = 0;i < 100;i++)
	{
		if (str[i] == ch)
		{
			return i;
		}
		i++;
	}
	return 0;
}

// �����ַ�����ָ���ַ����ɹ����ظ��ַ��״γ��ֵ��±�λ�ã����򷵻�-1
int16 STRING_FindCh(const uint8 *str, uint8 ch)
{
	uint16 i = 0;
	
	// ��������
	if (NULL == str)
	{
		return -1;
	}

	while (str[i] != '\0')
	{
		if (str[i] == ch)
		{
			return i;
		}
		i++;
	}

	return -1;
}

// �����ַ�����ָ���ַ��ĸ���
int16 STRING_CalculateCh(const uint8 *str, uint8 ch)
{
	uint16 i = 0;
	uint16 cnt = 0;
	
	// ��������
	if (NULL == str)
	{
		return -1;
	}

	while (str[i] != '\0')
	{
		if (str[i] == ch)
		{
			cnt++;
		}
		i++;
	}

	return cnt;
}

// �ж������ַ����Ƿ����
BOOL STRING_CmpStr(const uint8 *str1, const uint8 *str2)
{
	uint8 *localStr1 = (uint8*)str1;
	uint8 *localStr2 = (uint8*)str2;
	
	// ��������
	if ((NULL == str1) || (NULL == str2))
	{
		return FALSE;
	}

	while ((*localStr1 != '\0') && (*localStr2 != '\0'))
	{
		// �����ִ�Сд���������ĸ����ͳһת��Ϊ��д
		if ((*localStr1 >= 'a')&&(*localStr1 <= 'z'))
		{
			*localStr1 = *localStr1 - 32;
		}

		if ((*localStr2 >= 'a')&&(*localStr2 <= 'z'))
		{
			*localStr2 = *localStr2 - 32;
		}

		if (*localStr1++ != *localStr2++)
		{
			return FALSE;
		} 
	}

	if ((*localStr1 == '\0') && (*localStr2 == '\0'))
	{
		return TRUE;
	}

	return FALSE;
}

uint8 STRING_CmpareN(const uint8 *str1, const uint8 *str2, uint8 len)
{
	uint16 i = 0;

	while(i < len)
	{
		if(*(str1 +i) != *(str2 +i))
		{
			return 0;
		}
		i++;
	}
	return 1;
}


// ɾ��ָ���ַ����е�ָ���ַ������سɹ�ɾ���ĸ���
uint16 STRING_DeleteCh(uint8 str[], uint8 ch)
{
	uint16 i, j;
	uint16 cnt = 0;
	
	for (i = 0; str[i] != '\0'; i++)
	{
		if (str[i] == ch) 
		{
			for(j = i; str[j] != '\0'; j++) 
			{
				str[j] = str[j+1];
			}
			cnt++;
			i--;
		}		 
	}

	return cnt;
}

// ��ָ�����ַ�����ĩβ1��ʼ��ȡ��������ǰ count ���ַ�
void STRING_Right(const char *str, uint8 count, char arr[])
{
	uint8 *localStr = (uint8*)str;
	uint8 length = strlen((char*)str);
	
	// ��������
	if (NULL == str)
	{
		return;
	}

	// ���Ȳ���Խ��
	if (count > length)
	{
		count = length;
	}

	localStr += (length - count);	// ���ұߵ�n���ַ���ʼ����'\0'���� 

	while (*localStr != '\0')
	{
		*arr++ = *localStr++;
	}
	*arr = '\0';

}

// ��ָ�����ַ����Ŀ�ʼλ�ã���ȡcount���ַ�Ϊ�Ӵ�
void STRING_Left(const uint8 *str, uint8 count, uint8 arr[])
{
	uint8 *localStr = (uint8*)str;
	uint8 length = strlen((char*)str);
	uint8 i;
	
	// ��������
	if (NULL == str)
	{
		return;
	}

	// ���Ȳ���Խ��
	if (count > length)
	{
		count = length;
	}

	for (i = 0; i < count; i++)
	{
	   arr[i] = *localStr++; 
	}
	
	arr[i] = '\0';
}


// ��ָ�����ַ�����ָ���±�λ�ÿ�ʼ����ȡ�м�count���ַ���Ϊ�Ӵ�
void STRING_Mid(const uint8 *str, uint8 startNum, int8 count, uint8 arr[])
{
	uint8 *localStr = (uint8*)str;
	uint8 length = strlen((char*)str);
	
	// ��������
	if ((NULL == str) || (startNum > length))
	{
		return;
	}

	// ���Ȳ���Խ��
	if (count > length)
	{
		count = length - startNum; // �ӵ�m�������
	}

	localStr += startNum;

	if (count > 0)
	{
		while ((count--)&&((*localStr) != '\0'))
		{
		   *arr++ = *localStr++; 
		}
	}
	else
	{
		while ((*localStr) != '\0')
		{
		   *arr++ = *localStr++; 
		}
	}
	
	*arr = '\0';
}

// ��ָ�����ַ����ĵ�һ���ַ����ڶ����ַ���Ϊһ���µ��ִ�
BOOL STRING_FromTo(const uint8 *str, uint8 firstChar, uint8 secondChar, uint8 arr[])
{
	uint8 *localStr = (uint8*)str;
	uint8 *localarr = arr;
	uint8 length = strlen((char*)str);
	int16 j, k;
	uint16 i;

	// ��������
	if ((NULL == localStr) || ('\0' == firstChar) || ('\0' == secondChar))
	{
		return FALSE;
	}

	if (firstChar == secondChar)
	{
		// ��������2��
		if (STRING_CalculateCh(localStr, firstChar) < 2)
		{
			return FALSE;
		}
	}

	j = STRING_FindCh(localStr, firstChar);
	k = STRING_FindCh(localStr, secondChar);
	if ((-1 == j) || (-1 == k))
	{
		return FALSE;
	}
	
	// ȡ�ڶ����ַ�֮ǰ���ַ���Ϊ���ַ���
	for (i = 0, ++j; localStr[j] != secondChar; i++, j++)
	{
		localarr[i] = localStr[j];
	}
	localarr[i] = '\0';

	return TRUE;
}

// �õ�ָ����ŵĶ���λ��
int16 STRING_GPS_GetCommaPosition(const uint8 *str, uint16 num)
{
	uint16 i, j;
	uint16 len = strlen((char*)str);

	// ��������
	if ((NULL == str) || (num > len))
	{
		return -1;
	}
	
	for (i = 0, j = 0; i < len; i++)
	{
		if (str[i] == ',')
		{
			 j++;
		}

		if (j == num)
		{
			return i;
		}
	}
	
	return -1;
}

// �õ�ָ����ŵĵ�m�����ź�n������֮����ַ���Ϊ���ַ���
BOOL STRING_GPS_FormCommaStr(const uint8 *str, uint16 mNum, uint16 nNum, uint8 arr[])
{
	int16 i, j; 
	uint16 len = strlen((char*)str);

	// ��������
	if ((NULL == str) || (mNum > len) || (nNum > len) || (mNum > nNum))
	{
		return FALSE;
	}

	// �Ҳ�������
	i = STRING_GPS_GetCommaPosition(str, mNum);
	j = STRING_GPS_GetCommaPosition(str, nNum);
	if ((i < 0) || (j < 0))
	{
		return FALSE;
	}

	// ��������֮��������
	if (1 == (j - i))
	{
		return FALSE;
	}

	// ��ȡ��������֮����ַ�������ַ���
	if (STRING_FromTo(&str[i], ',', ',', arr))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}



// 16����תchar
unsigned char HexToChar(unsigned char bChar)
{
	if((bChar>=0x30)&&(bChar<=0x39))
	{
		bChar -= 0x30;
	}
	else if((bChar>=0x41)&&(bChar<=0x46)) // Capital
	{
		bChar -= 0x37;
	}
	else if((bChar>=0x61)&&(bChar<=0x66)) //littlecase
	{
		bChar -= 0x57;
	}
	else 
	{
		bChar = 0xff;
	}
	return bChar;
}





