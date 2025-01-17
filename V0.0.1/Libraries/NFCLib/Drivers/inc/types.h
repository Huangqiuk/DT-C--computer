/***********************************************************************************************
 *   type.h:  Type definition Header file for NXP Family 
 *   Microprocessors
 *
 *   Copyright(C) 2006, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.04.01  ver 1.00    Preliminary version, first Release
 *
 ***********************************************************************************************/
#ifndef __TYPES_H__
#define __TYPES_H__

// CodeRed - ifdef for GNU added to avoid potential clash with stdint.h
#if defined   (  __GNUC__  )
#include <stdint.h>
#else

/* exact-width signed integer types */
typedef   signed           char int8_t;
typedef   signed short     int int16_t;
typedef   signed           int int32_t;
typedef   signed           __int64 int64_t;

/* exact-width unsigned integer types */
typedef unsigned           char uint8_t;
typedef unsigned short     int uint16_t;
typedef unsigned           int uint32_t;
typedef unsigned           __int64 uint64_t;

#endif // __GNUC__ 

//#include <lpc_types.h>

#define BOOLEAN uint8_t

#endif  /* __TYPE_H__ */

/***********************************************************************************************
 **                            End Of File
 ***********************************************************************************************/

