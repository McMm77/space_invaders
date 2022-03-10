#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "simple_assembler.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)	(sizeof(x)/(sizeof(x[0])))
#endif /* ARRAY_SIZE */

typedef struct ass_opcode {
	uint8_t opcode;
	char* ass_cmd;
} ass_opcode_t;

static const char* single_instr_tab[] = { "NOP" , "STAX", "INX", "INR", "DCR", "RLC", "RAL", "DAA",
       				          "STC", "DAD", "LDAX", "DCX", "RRC", "RAR", "CMA", "CMC",
					  "MOV", "HLT", "ADD", "ADC", "SUB", "SBB", "ANA", "XRA",
					  "ORA", "CMP", "RNZ", "RNC", "RPO", "RP", "POP", "XTHL",
					  "DI",  "PUSH", "RST", "RZ", "RC", "RPE", "RM", "RET",
					  "PCHL", "SPHL", "XCHG", "EI", NULL };
static const char* single_param_instr_tab[] = {"MVI", "OUT", "ADI", "SUI", "ANI", "ORI", "IN", "ACI", "SBI", "XRI", "CPI", NULL };
static const char* dual_param_instr_tab[] = {"LXI", "SHLD", "STA", "LHLD", "LDA", "JNZ", "JNC", "JPO", "JP" "JMP", "CNZ", "CNC", "CPO",
	                                     "CP", "JZ", "JC", "JPE", "JM", "CZ", "CC", "CPE", "CM", "CALL", NULL};
static const ass_opcode_t opcode_table[] = 
	{{0x00, "NOP"}, {0x01, "LXIB"},  {0x02, "STAXB"}, {0x03, "INXB"},  {0x04, "INRB"}, {0x05, "DCRB"}, {0x06, "MVIB"}, {0x07, "RLC"},
	 {0x08, "NOP"}, {0x09, "DADB"},  {0x0A, "LDAXB"}, {0x0B, "DCXB"},  {0x0C, "INRC"}, {0X0D, "DCRC"}, {0x0E, "MVIC"}, {0x0F, "RRC"},

	 {0x10, "NOP"}, {0x11, "LXID"},  {0x12, "STAXD"}, {0x13, "INXD"},  {0x14, "INRD"}, {0x15, "DCRD"}, {0x16, "MVID"}, {0x17, "RAL"},
	 {0x18, "NOP"}, {0x19, "DADD"},  {0x1A, "LDAXD"}, {0x1B, "DCXD"},  {0x1C, "INRE"}, {0X1D, "DCRE"}, {0x1E, "MVIE"}, {0x1F, "RAR"},

	 {0x20, "NOP"}, {0x21, "LXIH"},  {0x22, "SHLD"},  {0x23, "INXH"},  {0x24, "INRH"}, {0x25, "DCRH"}, {0x26, "MVIH"}, {0x27, "DAA"},
	 {0x28, "NOP"}, {0x29, "DADH"},  {0x2A, "LHLD"},  {0x2B, "DCXH"},  {0x2C, "INRL"}, {0X2D, "DCRL"}, {0x2E, "MVIL"}, {0x2F, "CMA"},

	 {0x30, "NOP"}, {0x31, "LXISP"}, {0x32, "STA"},   {0x33, "INXSP"}, {0x34, "INRM"}, {0x35, "DCRM"}, {0x36, "MVIM"}, {0x37, "STC"},
	 {0x38, "NOP"}, {0x39, "DADSP"}, {0x3A, "LDA"},   {0x3B, "DCXSP"}, {0x3C, "INRA"}, {0x3D, "DCRA"}, {0x3E, "MVIA"}, {0x3F, "CMC"},
	 
	 {0x40, "MOVBB"}, {0x41, "MOVBC"},  {0x42, "MOVBD"}, {0x43, "MOVBE"},  {0x44, "MOVBH"}, {0x45, "MOVBL"}, {0x46, "MOVBM"}, {0x47, "MOVBA"},
	 {0x48, "MOVCB"}, {0x49, "MOVCC"},  {0x4A, "MOVCD"}, {0x4B, "MOVCE"},  {0x4C, "MOVCH"}, {0X4D, "MOVCL"}, {0x4E, "MOVCM"}, {0x4F, "MOVCA"},

	 {0x50, "MOVDB"}, {0x51, "MOVDC"},  {0x52, "MOVDD"},  {0x53, "MOVDE"},  {0x54, "MOVDH"}, {0x55, "MOVDL"}, {0x56, "MOVDM"}, {0x57, "MOVDA"},
	 {0x58, "MOVEB"}, {0x59, "MOVEC"},  {0x5A, "MOVED"},  {0x5B, "MOVEE"},  {0x5C, "MOVEH"}, {0X5D, "MOVEL"}, {0x5E, "MOVEM"}, {0x5F, "MOVEA"},
	 
	 {0x60, "MOVHB"}, {0x61, "MOVHC"},  {0x62, "MOVHD"}, {0x63, "MOVHE"},  {0x64, "MOVHH"}, {0x65, "MOVHL"}, {0x66, "MOVHM"}, {0x67, "MOVHA"},
	 {0x68, "MOVLB"}, {0x69, "MOVLC"},  {0x6A, "MOVLD"}, {0x6B, "MOVLE"},  {0x6C, "MOVLH"}, {0X6D, "MOVLL"}, {0x6E, "MOVLM"}, {0x6F, "MOVLA"},

	 {0x70, "MOVMB"}, {0x71, "MOVMC"},  {0x72, "MOVMD"},  {0x73, "MOVME"},  {0x74, "MOVMH"}, {0x75, "MOVML"}, {0x76, "HLT"}, {0x77, "MOVMA"},
	 {0x78, "MOVAB"}, {0x79, "MOVAC"},  {0x7A, "MOVAD"},  {0x7B, "MOVAE"},  {0x7C, "MOVAH"}, {0X7D, "MOVAL"}, {0x7E, "MOVAM"}, {0x7F, "MOVAA"},

	 {0x80, "ADDB"}, {0x81, "ADDC"},  {0x82, "ADDD"},  {0x83, "ADDE"},  {0x84, "ADDH"}, {0x85, "ADDL"}, {0x86, "ADDM"}, {0x87, "ADDA"},
	 {0x88, "ADCB"}, {0x89, "ADCC"},  {0x8A, "ADCD"},  {0x8B, "ADCE"},  {0x8C, "ADCH"}, {0X8D, "ADCL"}, {0x8E, "ADCM"}, {0x8F, "ADCA"},

	 {0x90, "SUBB"}, {0x91, "SUBC"},  {0x92, "SUBD"},  {0x93, "SUBE"},  {0x94, "SUBH"}, {0x95, "SUBL"}, {0x96, "SUBM"}, {0x97, "SUBA"},
	 {0x98, "SBBB"}, {0x99, "SBBC"},  {0x9A, "SBBD"},  {0x9B, "SBBE"},  {0x9C, "SBBH"}, {0X9D, "SBBL"}, {0x9E, "SBBM"}, {0x9F, "SBBA"},

	 {0xA0, "ANAB"}, {0xA1, "ANAC"},  {0xA2, "ANAD"},  {0xA3, "ANAE"},  {0xA4, "ANAH"}, {0xA5, "ANAL"}, {0xA6, "ANAM"}, {0xA7, "ANAA"},
	 {0xA8, "XRAB"}, {0xA9, "XRAC"},  {0xAA, "XRAD"},  {0xAB, "XRAE"},  {0xAC, "XRAH"}, {0XAD, "XRAL"}, {0xAE, "XRAM"}, {0xAF, "XRAA"},

	 {0xB0, "ORAA"}, {0xB1, "ORAC"},  {0xB2, "ORAD"},  {0xB3, "ORAE"},  {0xB4, "ORAH"}, {0xB5, "ORAL"}, {0xB6, "ORAM"}, {0xB7, "ORAA"},
	 {0xB8, "CMPB"}, {0xB9, "CMPC"},  {0xBA, "CMPD"},  {0xBB, "CMPE"},  {0xBC, "CMPH"}, {0XBD, "CMPL"}, {0xBE, "CMPM"}, {0xBF, "CMPA"},

	 {0xC0, "RNZ"},  {0xC1, "POP"},   {0xC2, "JNZ"},  {0xC3, "JMP"},   {0xC4, "CNZ"}, {0xC5, "PUSHB"},  {0xC6, "ADI"},  {0xC7, "RST0"},
	 {0xC8, "RZ"},   {0xC9, "RET"},   {0xCA, "JZ"},   {0xCB, "JMP"},   {0xCC, "CZ"},  {0XCD, "CALL"},   {0xCE, "ACI"},  {0xCF, "RST1"},

	 {0xD0, "RNC"},  {0xD1, "POP"},   {0xD2, "JNC"},  {0xD3, "OUT"},   {0xD4, "CNC"}, {0xD5, "PUSHD"},   {0xD6, "SUI"}, {0xD7, "RST2"},
	 {0xD8, "RC"},   {0xD9, "RET"},   {0xDA, "JC"},   {0xDB, "IN"},    {0xDC, "CC"},  {0XDD, "CALL"},    {0xDE, "SBI"}, {0xDF, "RST3"},

	 {0xE0, "RPO"},  {0xE1, "POP"},   {0xE2, "JPO"},  {0xE3, "XTHL"},  {0xE4, "CPO"}, {0xE5, "PUSHH"},   {0xE6, "ANI"}, {0xE7, "RST4"},
	 {0xE8, "RPE"},  {0xE9, "PCHL"},  {0xEA, "JPE"},  {0xEB, "XCHG"},  {0xEC, "CPE"}, {0XED, "CALL"},    {0xEE, "XRI"}, {0xEF, "RST5"},

	 {0xF0, "RP"},   {0xF1, "POP"},   {0xF2, "JP"},   {0xF3, "DI"},    {0xF4, "CP"},  {0xF5, "PUSHPSW"}, {0xF6, "ORI"}, {0xF7, "RST6"},
	 {0xF8, "RM"},   {0xF9, "SPHL"},  {0xFA, "JM"},   {0xFB, "EI"},    {0xFC, "CM"},  {0XFD, "CALL"},    {0xFE, "CPI"}, {0xFF, "RST7"}
};

static int search_for_assembly_table_entry(char *full_assembly)
{
	int count = 0;
	int array_size = ARRAY_SIZE(opcode_table);

	printf("ARRAY SIZE: %d\n", array_size);
	for (count = 0 ; count < array_size ;  count++) {
		if( strcmp(full_assembly, opcode_table[count].ass_cmd) == 0)
			return count;
	}

	return -1;
}

static bool extract_single_instr(char* ass_cmd, char *str, memory_t* rom)
{
	char *s = NULL;
	char temp_str[10] = {0};
	char *p = strcat(temp_str, ass_cmd);

	do {
		s = strtok(NULL, " ,");

		if (s != NULL)
			p = strcat(p, s);

	} while (s!=NULL);

	int index = search_for_assembly_table_entry(temp_str);

	if (index != -1) {
		rom->memory[0] = opcode_table[index].opcode;
		rom->memory_size = 1;
	}

	return (index != -1);
}

static bool extract_single_param(char* ass_cmd, char *str, memory_t* rom)
{
	char temp_str[10] = {0};
	char * p = strcat(temp_str, ass_cmd);
	char * s = strtok(NULL, " ,");
	uint8_t data = 0;

	if (strcmp(ass_cmd, "MVI") == 0)
	{
		p = strcat(p,s);
		s = strtok(NULL, " ,");
	}

	char *i = strstr(s, "H");

	if (i != 0) {
		i = 0;
		data = strtol(s, NULL, 16);
	} else {
		data = atoi(s);
	}

	int index = search_for_assembly_table_entry(temp_str);

	if (index != -1) {
		rom->memory[0] = opcode_table[index].opcode;
		rom->memory[1] = data;
	}

	return (index != -1);
}

static bool extract_dual_param(char *ass_cmd, char *str, memory_t *rom)
{
	char temp_str[10] = {0};
	char *p = strcat(temp_str, ass_cmd);
	char *s = strtok(NULL, " ,");
	char *i = strstr(s, "H");
	uint16_t data = 0;

	if (i!= 0) {
		i = 0;
		data = strtol(s, NULL, 16);
	} else {
		data = atoi(s);
	}

	int index = search_for_assembly_table_entry(temp_str);

	if (index != -1) {
		rom->memory[0] = opcode_table[index].opcode;
		rom->memory[1] = (data & 0xFF);
		rom->memory[2] = (data >> 8) & 0xFF;
	}

	return (index != -1);
}

static bool is_valid_instr(char *ass_cmd, const char* instr_table[])
{
	int count = 0;

	while (instr_table[count] != NULL) {
		if (strcmp(instr_table[count], ass_cmd) == 0)
			return true;

		count++;
	}

	return false;
}

bool assemble(memory_t *rom, char *cmd)
{
	bool is_valid_cmd = false;

	char *ass_cmd = strtok(cmd, " ");
	if (is_valid_instr(ass_cmd, single_instr_tab)) {
		is_valid_cmd = extract_single_instr(ass_cmd, cmd, rom);
	}

	else if (is_valid_instr(ass_cmd, single_param_instr_tab)) {
		is_valid_cmd = extract_single_param(ass_cmd, cmd, rom);
	}

	else if (is_valid_instr(ass_cmd, dual_param_instr_tab)) {
		is_valid_cmd = extract_dual_param(ass_cmd, cmd, rom);
	}

	else
	{}

	return is_valid_cmd;
}

