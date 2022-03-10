#ifndef __DEASSEMBLER_H__
#define __DEASSEMBLER_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "common_types.h"

#define DISAMMBLER_VERSION_MAJOR		0
#define DISAMMBLER_VERSION_MINOR		1

const char* dikke_kut(memory_t* rom, int offset);
void dissamble_curr_instr(memory_t* rom, int offset);
void dissamble(memory_t* rom);

#endif /* __DEASSEMBLER_H__ */
