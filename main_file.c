#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cpu.h"
#include "cli_parser.h"
#include "deassembler.h"

#define EMULATOR_VERSION_MAJOR		0
#define EMULATOR_VERSION_MINOR		1

#define RAM_SIZE			(16*1024)
#define DEBUG_SIZE			60

#define CLEAR_SCREEN			"\e[1;1H\e[2J"

memory_t memory = {0, NULL};

typedef struct bp {
	uint16_t pc[DEBUG_SIZE];
	uint8_t pc_cnt;
} bp_t;

typedef struct call_stack {
	cpu_model_t cpu[DEBUG_SIZE];
	uint8_t curr_index;
} call_stack_t;

bp_t dbg_bp = {0};
call_stack_t cst = {0};



static void show_cpu_state(memory_t* mem, cpu_model_t *cpu);

void add_to_stack(cpu_model_t *cpu) {
	uint8_t index = cst.curr_index;

	cst.cpu[index] = *cpu;
	cst.curr_index = (++index < DEBUG_SIZE) ? index : 0;
}

void show_stack(memory_t *mem, cpu_model_t *cpu, uint8_t index) {
	
	printf("--------------------------------------------------------------\n");
	printf("Call Stack: %d\n", index);
	printf("Instruction: ");
	dissamble_curr_instr(mem, cpu->core.pc);
	printf("PC: 0x%04x\t", cpu->core.pc, cpu->core.pc);
	printf("Stack: 0x%04x\t", cpu->core.stack);
	printf("Reg B: 0x%02x\tReg C 0x%02x\t", cpu->core.b, cpu->core.c);
	printf("Reg D: 0x%02x\tReg E 0x%02x\t", cpu->core.d, cpu->core.e);
	printf("Reg H: 0x%02x\tReg L 0x%02x\n", cpu->core.h, cpu->core.l);
	printf("Reg A: 0x%02x\tReg M 0x%02x\n", cpu->core.a, cpu->core.m);
	printf("Status: C: %d P: %d AC: %d z: %d s: %d\n", cpu->core.status.bits.c,
							   cpu->core.status.bits.p,
							   cpu->core.status.bits.ac,
							   cpu->core.status.bits.z,
							   cpu->core.status.bits.s);
 
}

void show_stack_cmd(cpu_model_t *cpu, memory_t *mem) {
	uint8_t index = (cst.curr_index != 0) ? (cst.curr_index -1) : (DEBUG_SIZE - 1);
	for(int i = 0 ; i < DEBUG_SIZE ; i++) {
		cpu_model_t *cpu = &cst.cpu[index];
		show_stack(mem, cpu, i);

		index = (index == 0) ? (DEBUG_SIZE-1) : index-1;		
	}
}

// ---------------------------------------------------------
static inline size_t get_file_size(FILE* pFile)
{
   fseek(pFile, 0, SEEK_END);
   size_t size = ftell(pFile);
   
   rewind(pFile);

   return size;
}

static void create_ram_memory()
{
	memory.memory = (uint8_t*) calloc(RAM_SIZE, sizeof(uint8_t));
	memory.memory_size = RAM_SIZE;

	if(memory.memory == NULL) {
		printf("ERROR: Failed to allocate RAM memory\n");
		exit(0);
	}
}

static void destroy_ram_memory()
{
	if(memory.memory != NULL) {
		free(memory.memory);	
	}
}

static void load_rom_memory_from_file(const char* rom_file, memory_t* rom_mem, int offset)
{
	FILE* pInputFile = fopen(rom_file, "rb");
	
	if(pInputFile == NULL) {
		printf("ERROR: Rom file not found\n");
		exit(0);
	}
	
	size_t size = get_file_size(pInputFile);
   
	fread(rom_mem->memory + offset, 1, size, pInputFile);
}


static bool get_opcode(memory_t* mem, char* ass_code)
{
	memset(mem->memory, 0, 4);

	mem->memory[0] = 0x04;
	mem->memory_size = 1;

	return 1;
}

static void write_to_reg(char *str, cpu_model_t *cpu)
{
	char *cmd = strtok(str, " ");
	char *reg = strtok(NULL, " ");
	char *val = strtok(NULL, " ");

	uint8_t reg_val = (uint8_t) atoi(val);

	cpu_set_reg_value(cpu, *reg, reg_val);
}

static void breakpoint_cmd(char *str, cpu_model_t *cpu, memory_t *rom)
{
	int index = 0;
	char *cmd = strtok(str, " ");
	char *bp = strtok(NULL, " ");

	uint16_t pc = (uint16_t) atoi(bp);

	for(index = 0 ; index < dbg_bp.pc_cnt ; index++) {
		if(dbg_bp.pc[index] == pc) {
			printf("BP already exists");
			return;
		}
	}

	if(index < 10) {
		dbg_bp.pc[index] = pc;
		dbg_bp.pc_cnt++;
		printf("Breakpoint added at the following address: 0x%x\n", pc);
	}

	else {
		printf("Unable to add break point. FULL STACK!!!\n");
	}
}

static void step_cmd(char *str, cpu_model_t *cpu, memory_t *rom)
{
	uint16_t step_cnt = 1;
	char *cmd = strtok(str, " ");
	char *steps = strtok(NULL, " ");

	if(steps != NULL)
		step_cnt = (uint16_t) atoi(steps);

	for(int i = 0 ; i < step_cnt ; i++) {
		add_to_stack(cpu);
		execute_single_cpu_8080_cycle(rom, cpu);
	}

	dissamble_curr_instr(rom, cpu->core.pc);
	show_cpu_state(rom, cpu);
}

static bool is_not_bp(uint16_t pc_temp) {

	for(int i = 0 ; i < dbg_bp.pc_cnt ; i++) {
		if(dbg_bp.pc[i] == pc_temp) {
			return false;
		}
	}

	return true;
}

static void run_cmd(char *str, cpu_model_t *cpu, memory_t *rom)
{
	char *cmd = strtok(str, " ");

	while(is_not_bp(cpu->core.pc)) {
		add_to_stack(cpu);
		execute_single_cpu_8080_cycle(rom, cpu);
	}

	dissamble_curr_instr(rom, cpu->core.pc);
	show_cpu_state(rom, cpu);

}

static void opcode_cmd(char *str, cpu_model_t *cpu, memory_t *rom)
{
/*
	if (parse_opcode_cmd(rom, str) == true) {
		execute_interrupt_opcode_cmd(&memory, rom, cpu);
	}

	else {
		printf("Unrecognizable assembly command\n");
	}
*/
}

static void show_cpu_state(memory_t* mem, cpu_model_t *cpu)
{
	display_curr_cpu_8080_status(mem, &cpu->core);
}

static void write_to_ram(char* str, cpu_model_t *cpu)
{
}

void print_screen(char* str) {
	printf(CLEAR_SCREEN);
	
	printf("8080 Emulator - Version %d.%02d\n", EMULATOR_VERSION_MAJOR, EMULATOR_VERSION_MINOR);

	printf("8080 Emulator Step Mode\n");
	printf("\nCMD Executed << %s\n", str);
	printf("\n--------------------------------\n");
}

static void execute_cmd(cmd_type_t cmd, char *str, cpu_model_t *cpu, memory_t* rom)
{
	print_screen(str);

	switch(cmd) {
		case e_write_reg_cmd:
			write_to_reg(str, cpu);
			break;
		case e_write_ram_cmd:
			write_to_ram(str, cpu);
			break;
		case e_show_cpu_state_cmd:
			show_cpu_state(rom, cpu);
			break;
		case e_opcode_cmd:
			opcode_cmd(str, cpu, rom);
			break;
		case e_breakpoint_cmd:
			breakpoint_cmd(str, cpu, rom);
			break;
		case e_run_cmd:
			run_cmd(str, cpu, rom);
			break;
		case e_step_cmd:
			step_cmd(str, cpu, rom);
			break;
		case e_show_stack_cmd:
			show_stack_cmd(cpu, rom);
			break;
		default:
			break;
	}
}

static void enter_bin(char* rom_file, int offset)
{
	cpu_model_t cpu = {0};
	cpu.is_running = true;
	
	load_rom_memory_from_file(rom_file, &memory, offset);

	execute_cpu_8080(&memory, &cpu);
}


static void show_step_info(cpu_model_t* cpu, memory_t* mem) {
	char str[80] = {0};

	int ret = scanf(" %[^\n]s", str);

	execute_cmd(get_cmd(str), str, cpu, mem);

	printf("\nCMD << ");
}

static void enter_step(char *rom_file, int offset)
{
	bool bLoop = true;
	
	cpu_model_t cpu = {0};
	
	load_rom_memory_from_file(rom_file, &memory, offset);

	printf(CLEAR_SCREEN);
	printf("8080 Emulator - Version %d.%02d\n", EMULATOR_VERSION_MAJOR, EMULATOR_VERSION_MINOR);
	printf("8080 Emulator Step Mode\n");
	printf("\nCMD << ");

	while( bLoop == true) {
		show_step_info(&cpu, &memory);
	}
}

static void execute_code(char exec, int argc, char* argv[])
{
	if(exec == 'b' && argc >= 3) {
		printf("8080 Emulator Normal Mode\n");
		enter_bin(argv[2], atoi(argv[3]));
	}

	else if(exec == 's' && argc >= 3) {
		printf("8080 Emulator Step Mode\n");
		enter_step(argv[2], atoi(argv[3]));
	}

	else {
		printf("Invalid arguments provided\n");
		exit(0);
	}
}

int main(int argc, char* argv[])
{
	printf(CLEAR_SCREEN);
	printf("8080 Emulator - Version %d.%02d\n", EMULATOR_VERSION_MAJOR, EMULATOR_VERSION_MINOR);

	if(argc >= 2)   {
	
		if(argv[1][0] != '-') {
			printf("Error No arguments provided\n");
			exit(0);
		}

		create_ram_memory();
		execute_code(argv[1][1], argc, argv);
	}

	destroy_ram_memory();

	return 0;
}
