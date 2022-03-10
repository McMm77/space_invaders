#ifndef __COMMON_TYPES_H__
#define __COMMON_TYPES_H__

#include <stdint.h>
#include <stdbool.h>

/*
#define true			((uint8_t) 1)
#define false			((uint8_t) 0)
/*
typedef unsigned char		bool;

typedef signed char		int8_t;
typedef unsigned char		uint8_t;

typedef signed short		int16_t;
typedef unsigned short		uint16_t;

typedef unsigned long		uint32_t;
*/
typedef struct memory {
	size_t memory_size;
#if dgconfig_DEASSEMBLER
	int index;
#endif	
	uint8_t* memory;
} memory_t;

#endif
