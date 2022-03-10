#ifndef __CLI_PARSER_H__
#define __CLI_PARSER_H_

#include "common_types.h"

typedef enum cmd_type {
	e_idle_cmd = 0,
	e_read_ram_cmd,
	e_write_ram_cmd,
	e_clear_ram_cmd,
	e_read_reg_cmd,
	e_write_reg_cmd,
	e_show_cpu_state_cmd,
	e_opcode_cmd,
	e_breakpoint_cmd,
	e_run_cmd,
	e_step_cmd,
	e_show_stack_cmd
} cmd_type_t;

cmd_type_t get_cmd(char *cmd);
bool parse_opcode_cmd(memory_t *rom, char *cmd);
void parse_get_reg_cmd(char *cmd);
void parse_get_ram_cmd(char *cmd);

#endif
