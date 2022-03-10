#include "deassembler.h"

extern memory_t rom_memory;

static inline size_t get_file_size(FILE* pFile)
{
   fseek(pFile, 0, SEEK_END);
   size_t size = ftell(pFile);
   
   rewind(pFile);

   return size;
}

static int load_rom_memory_from_file(const char* rom_file, memory_t* rom_mem)
{
	FILE* pInputFile = fopen(rom_file, "rb");
	
	if(pInputFile == NULL) {
		printf("ERROR: Rom file not found\n");
		return -1;
	}

	size_t file_size_1 = get_file_size(pInputFile);
	size_t file_size = file_size_1 + 0x200;
   
	rom_mem->memory = (uint8_t*) malloc(file_size);
	
	if(rom_mem->memory == NULL) {
		printf("ERROR: Failed to allocate ROM memory\n");
		return -1;
	}
	
	rom_mem->memory_size = file_size;
	
	fread(rom_mem->memory, 1, file_size, pInputFile);
	
	return 0;
}

int main(int argc, char* argv[])
{
	int flag = 1;
	printf("8080 disammbler - Version %d.%02d\n", DISAMMBLER_VERSION_MAJOR, DISAMMBLER_VERSION_MINOR);

	if(argc >= 2)
	{
		if(load_rom_memory_from_file(argv[1], &rom_memory) != 0)   {
			exit(0);
		}
	}

	dissamble(&rom_memory);

	return 0;
}
