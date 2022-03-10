#ifndef __CPU_H__
#define __CPU_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include "common_types.h"


typedef union status_reg {

	struct {
		uint8_t c; //       : 1;
		uint8_t p; //       : 1;
		uint8_t ac; //      : 1;
		uint8_t z; //       : 1;
		uint8_t s; //       : 1;
	} bits;

	uint8_t reg;

} status_reg_t;

typedef struct cpu_core {
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint8_t m;
	
	uint16_t stack;
	uint16_t pc;
	status_reg_t status;
	
} cpu_core_t;

typedef uint8_t (*input_handler) (uint8_t);
typedef void (*output_handler) (uint8_t, uint8_t);

typedef struct cpu_model {
	uint8_t		is_running;
	bool		interrupt_enable;	
	input_handler   cb_in;
	output_handler  cb_out;
	cpu_core_t	core;
} cpu_model_t;	

bool single_cpu_8080_cycle(FILE*, memory_t*, cpu_model_t*);

bool execute_single_cpu_8080_cycle(memory_t*, cpu_model_t*);

void execute_cpu_8080(memory_t*, cpu_model_t*);

void execute_interrupt(FILE*, int, memory_t*, cpu_model_t*);

void execute_interrupt_opcode_cmd(memory_t *ram, memory_t* cmd, cpu_model_t* cpu_8080);

void display_curr_cpu_8080_status(memory_t *mem, cpu_core_t* core);

void cpu_set_reg_value(cpu_model_t *cpu_8080, char reg, uint8_t val);


/*
void cpu_set_reg_value(cpu_model_t*, char, uint8_t);

void display_cpu_status(cpu_model_t*);

void reset_cpu(cpu_model_t*);

bool execute_single_cpu_cycle(memory_t*, memory_t*, cpu_model_t*);

void execute_cpu(memory_t*, memory_t*, cpu_model_t*);

uint8_t *common_func_get_reg(cpu_model_t*, uint8_t);

void display_curr_cpu_status(cpu_core_t* core);

void common_func_reg_zero_status_bit(cpu_model_t*, uint8_t);

void common_func_reg_parity_status_bit(cpu_model_t*, uint8_t);

void common_func_reg_sign_status_bit(cpu_model_t*, uint8_t);

void common_func_mem_zero_status_bit(cpu_model_t*, uint16_t);

void common_func_mem_parity_status_bit(cpu_model_t*, uint16_t);

void common_func_mem_sign_status_bit(cpu_model_t*, uint16_t);
*/

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __CPU_H__ */

