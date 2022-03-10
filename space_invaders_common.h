#ifndef SPACE_INVADERS_COMMON_H
#define SPACE_INVADERS_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "cpu.h"

memory_t* create_ram_memory();
void destroy_ram_memory(memory_t*);
void load_rom_memory_from_file(const char*, memory_t*, int);

#ifdef __cplusplus
} // extern "C"
#endif

#endif
