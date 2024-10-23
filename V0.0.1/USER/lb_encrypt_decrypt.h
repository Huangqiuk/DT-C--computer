#ifndef __LB_ENCRYPT_DECRYPT_H__
#define __LB_ENCRYPT_DECRYPT_H__
#include <stdint.h>

//#define LB_DEBUG 1
 
#define LB_ENCRIPTION_VER "lime-encryption-v0.1"

void lime_crypt_ver(char* data);

/* brief encryption for CCU-ECU
   
*   @data  unsigned char data 
*   @len length of data
*   @dataout output of encryption
*   @rand is rand int number from ECU
*/
int32_t lime_encrypt(uint8_t* data, uint32_t len, uint8_t* dataout, uint32_t rand);
int32_t lime_decrypt(uint8_t* data, uint32_t len, uint8_t* dataout, uint32_t rand);

/*  brief encryption for CCU-LCM
*
*   @data  unsigned char data  
*   @len length of data
*   @dataout output of encryption
*/
int32_t lime_encrypt_simple(uint8_t* data, uint32_t len, uint8_t* dataout);
int32_t lime_decrypt_simple(uint8_t* data, uint32_t len, uint8_t* dataout);

#endif //__LB_ENCRYPT_DECRYPT_H__

