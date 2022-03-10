#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "space_invaders_common.h"

#define RAM_SIZE	(16*1024)

memory_t memory = {0, NULL};

static inline size_t get_file_size(FILE* pFile)
{
   fseek(pFile, 0, SEEK_END);
   size_t size = ftell(pFile);
   
   rewind(pFile);

   return size;
}

memory_t* create_ram_memory()
{
	memory.memory = (uint8_t*) calloc(RAM_SIZE, sizeof(uint8_t));
	memory.memory_size = RAM_SIZE;

	if(memory.memory == NULL) {
		printf("ERROR: Failed to allocate RAM memory\n");
		exit(0);
	}

	return &memory;
}

void destroy_ram_memory(memory_t* mem)
{
	if(mem->memory != NULL) {
		free(mem->memory);	
	}
}

void load_rom_memory_from_file(const char* rom_file, memory_t* mem, int offset)
{
	FILE* pInputFile = fopen(rom_file, "rb");
	
	if(pInputFile == NULL) {
		printf("ERROR: Rom file not found\n");
		exit(0);
	}
	
	size_t size = get_file_size(pInputFile);
   
	fread(mem->memory + offset, 1, size, pInputFile);
}


