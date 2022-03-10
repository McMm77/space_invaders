#include "deassembler.h"

#ifdef dgconfig_DEASSEMBLER
	memory_t rom_memory = {0, 0, NULL};
#endif

static const char* mov_instr(memory_t*, int*);
static const char* ora_instr(memory_t*, int*);
static const char* rz_instr(memory_t*, int*);
static const char* ldax_instr(memory_t*, int*);
static const char* inx_instr(memory_t*, int*);
static const char* dcx_instr(memory_t*, int*);
static const char* jnz_instr(memory_t*, int*);
static const char* ret_instr(memory_t*, int*);
static const char* nop_instr(memory_t*, int*);
static const char* stax_instr(memory_t*, int*);
static const char* inr_instr(memory_t*, int*);
static const char* dcr_instr(memory_t*, int*);
static const char* mvi_instr(memory_t*, int*);
static const char* rlc_instr(memory_t*, int*);
static const char* lxi_instr(memory_t*, int*);
static const char* dcr_instr(memory_t*, int*);
static const char* dad_instr(memory_t*, int*);
static const char* rrc_instr(memory_t*, int*);
static const char* ral_instr(memory_t*, int*);
static const char* rar_instr(memory_t*, int*);
static const char* shld_instr(memory_t*, int*);
static const char* daa_instr(memory_t*, int*);
static const char* lhld_instr(memory_t*, int*);
static const char* cma_instr(memory_t*, int*);
static const char* sta_instr(memory_t*, int*);
static const char* stc_instr(memory_t*, int*);
static const char* lda_instr(memory_t*, int*);
static const char* cmc_instr(memory_t*, int*);
static const char* hlt_instr(memory_t*, int*);
static const char* add_instr(memory_t*, int*);
static const char* adc_instr(memory_t*, int*);
static const char* sub_instr(memory_t*, int*);
static const char* sbb_instr(memory_t*, int*);
static const char* ana_instr(memory_t*, int*);
static const char* xra_instr(memory_t*, int*);
static const char* cmp_instr(memory_t*, int*);
static const char* rnz_instr(memory_t*, int*);
static const char* pop_instr(memory_t*, int*);
static const char* jmp_instr(memory_t*, int*);
static const char* cnz_instr(memory_t*, int*);
static const char* push_instr(memory_t*, int*);
static const char* adi_instr(memory_t*, int*);
static const char* rst_instr(memory_t*, int*);
static const char* jz_instr(memory_t*, int*);
static const char* cz_instr(memory_t*, int*);
static const char* call_instr(memory_t*, int*);
static const char* aci_instr(memory_t*, int*);
static const char* rnc_instr(memory_t*, int*);
static const char* jnc_instr(memory_t*, int*);
static const char* out_instr(memory_t*, int*);
static const char* cnc_instr(memory_t*, int*);
static const char* sui_instr(memory_t*, int*);
static const char* rc_instr(memory_t*, int*);
static const char* jc_instr(memory_t*, int*);
static const char* in_instr(memory_t*, int*);
static const char* cc_instr(memory_t*, int*);
static const char* sbi_instr(memory_t*, int*);
static const char* rpo_instr(memory_t*, int*);
static const char* jpo_instr(memory_t*, int*);
static const char* xthl_instr(memory_t*, int*);
static const char* cpo_instr(memory_t*, int*);
static const char* ani_instr(memory_t*, int*);
static const char* rpe_instr(memory_t*, int*);
static const char* pchl_instr(memory_t*, int*);
static const char* jpe_instr(memory_t*, int*);
static const char* xchg_instr(memory_t*, int*);
static const char* cpe_instr(memory_t*, int*);
static const char* xri_instr(memory_t*, int*);
static const char* rp_instr(memory_t*, int*);
static const char* jp_instr(memory_t*, int*);
static const char* di_instr(memory_t*, int*);
static const char* cp_instr(memory_t*, int*);
static const char* ori_instr(memory_t*, int*);
static const char* rm_instr(memory_t*, int*);
static const char* sphl_instr(memory_t*, int*);
static const char* jm_instr(memory_t*, int*);
static const char* ei_instr(memory_t*, int*);
static const char* cm_instr(memory_t*, int*);
static const char* cpi_instr(memory_t*, int*);

/* Table taken from http://pastraiser.com/cpu/i8080/i8080_opcodes.html */
static const char* (*assembly_instr[]) (memory_t*, int*) = 
{
	nop_instr,  lxi_instr,  stax_instr,  inx_instr,   inr_instr,  dcr_instr,   mvi_instr,  rlc_instr,
	nop_instr,  dad_instr,  ldax_instr,  dcx_instr,   inr_instr,  dcr_instr,   mvi_instr,  rrc_instr,	/* \n */

	nop_instr,  lxi_instr,  stax_instr,  inx_instr,   inr_instr,  dcr_instr,   mvi_instr,  ral_instr,
	nop_instr,  dad_instr,  ldax_instr,  dcx_instr,   inr_instr,  dcr_instr,   mvi_instr,  rar_instr,	/* \n */

	nop_instr,  lxi_instr,  shld_instr,  inx_instr,   inr_instr,  dcr_instr,   mvi_instr,  daa_instr,
	nop_instr,  dad_instr,  lhld_instr,  dcx_instr,   inr_instr,  dcr_instr,   mvi_instr,  cma_instr,	/* \n */

	nop_instr,  lxi_instr,  sta_instr,   inx_instr,   inr_instr,  dcr_instr,   mvi_instr,  stc_instr,
	nop_instr,  dad_instr,  lda_instr,   dcx_instr,   inr_instr,  dcr_instr,   mvi_instr,  cmc_instr,

	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,
	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,

	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,
	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,

	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,
	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,

	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   hlt_instr,  mov_instr,
	mov_instr,  mov_instr,  mov_instr,   mov_instr,   mov_instr,  mov_instr,   mov_instr,  mov_instr,

	add_instr,  add_instr,  add_instr,   add_instr,   add_instr,  add_instr,   add_instr,  add_instr,
	adc_instr,  adc_instr,  adc_instr,   adc_instr,   adc_instr,  adc_instr,   adc_instr,  adc_instr,

	sub_instr,  sub_instr,  sub_instr,   sub_instr,   sub_instr,  sub_instr,   sub_instr,  sub_instr,
	sbb_instr,  sbb_instr,  sbb_instr,   sbb_instr,   sbb_instr,  sbb_instr,   sbb_instr,  sbb_instr,

	ana_instr,  ana_instr,  ana_instr,   ana_instr,   ana_instr,  ana_instr,   ana_instr,  ana_instr,
	xra_instr,  xra_instr,  xra_instr,   xra_instr,   xra_instr,  xra_instr,   xra_instr,  xra_instr,

	ora_instr,  ora_instr,  ora_instr,   ora_instr,   ora_instr,  ora_instr,   ora_instr,  ora_instr,
	cmp_instr,  cmp_instr,  cmp_instr,   cmp_instr,   cmp_instr,  cmp_instr,   cmp_instr,  cmp_instr,

	rnz_instr,  pop_instr,  jnz_instr,   jmp_instr,   cnz_instr,  push_instr,  adi_instr,  rst_instr,
	rz_instr,   ret_instr,  jz_instr,    jmp_instr,   cz_instr,   call_instr,  aci_instr,  rst_instr,

	rnc_instr,  pop_instr,  jnc_instr,   out_instr,   cnc_instr,  push_instr,  sui_instr,  rst_instr,
	rc_instr,   ret_instr,  jc_instr,    in_instr,    cc_instr,   call_instr,  sbi_instr,  rst_instr,

	rpo_instr,  pop_instr,  jpo_instr,   xthl_instr,  cpo_instr,  push_instr,  ani_instr,  rst_instr,
	rpe_instr,  pchl_instr, jpe_instr,   xchg_instr,  cpe_instr,  call_instr,  xri_instr,  rst_instr,

	rp_instr,   pop_instr,  jp_instr,    di_instr,    cp_instr,   push_instr,  ori_instr,  rst_instr,
	rm_instr,   sphl_instr, jm_instr,    ei_instr,    cm_instr,   call_instr,  cpi_instr,  rst_instr};


static unsigned char* asm_names[] = { "NOP", "LXI B", "STAX B", "INX B", "INR B", "DCR B", "MVI B", "RLC",\
       				      "NOP", "DAD B", "LDAX_B", "DCX B", "INR C", "DCR C", "MVI C", "RRC",\
				      "NOP", "LXI D", "STAX D", "INX D", "INR D", "DCR D", "MVI D", "RAL",\
				      "NOP", "DAD D", "LDAX D", "DCX D", "INR E", "DCR E", "MVI E", "RAR",\
				      "NOP", "LXI H", "SHLD",   "INX H", "INR H", "DCR H", "MVI H", "DAA",\
				      "NOP", "DAD H", "LHLD",   "DCX H", "INR L", "DCR L", "MVI L", "CMA",\
				      "NOP", "LXI SP", "STA", "INX SP", "INR M", "DCR M", "MVI M", "STC",\
				      "NOP", "DAD SP", "LDA", "DCX SP", "INR A", "DCR A",  "MVI A", "CMC",\

				      "MOV B, B", "MOV B, C", "MOV B, D", "MOV B, E", "MOV B, H", "MOV B, L", "MOV B, M", "MOV B, A",
				      "MOV C, B", "MOV C, C", "MOV C, D", "MOV C, E", "MOV C, H", "MOV C, L", "MOV C, M", "MOV C, A",
				      "MOV D, B", "MOV D, C", "MOV D, D", "MOV D, E", "MOV D, H", "MOV D, L", "MOV D, M", "MOV D, A",
				      "MOV E, B", "MOV E, C", "MOV E, D", "MOV E, E", "MOV E, H", "MOV E, L", "MOV E, M", "MOV E, A",
				      "MOV H, B", "MOV H, C", "MOV H, D", "MOV H, E", "MOV H, H", "MOV H, L", "MOV H, M", "MOV H, A",
				      "MOV L, B", "MOV L, C", "MOV L, D", "MOV L, E", "MOV L, H", "MOV L, L", "MOV L, M", "MOV L, A",
				      "MOV M, B", "MOV M, C", "MOV M, D", "MOV M, E", "MOV M, H", "MOV M, L", "HLT", 	  "MOV M, A",
				      "MOV A, B", "MOV A, C", "MOV A, D", "MOV A, E", "MOV A, H", "MOV A, L", "MOV A, M", "MOV A, A",

				      "ADD B", "ADD C", "ADD D", "ADD E", "ADD H", "ADD L", "ADD M", "ADD A",
				      "ADC B", "ADC C", "ADC D", "ADC E", "ADC H", "ADC L", "ADC M", "ADC A",
				      "SUB B", "SUB C", "SUB D", "SUB E", "SUB H", "SUB L", "SUB M", "SUB A",
				      "SBB B", "SBB C", "SBB D", "SBB E", "SBB H", "SBB L", "SBB M", "SBB A",
				      "ANA B", "ANA C", "ANA D", "ANA E", "ANA H", "ANA L", "ANA M", "ANA A",
				      "XRA B", "XRA C", "XRA D", "XRA E", "XRA H", "XRA L", "XRA M", "XRA A",
				      "ORA B", "ORA C", "ORA D", "ORA E", "ORA H", "ORA L", "ORA M", "ORA A",
				      "CMP B", "CMP C", "CMP D", "CMP E", "CMP H", "CMP L", "CMP M", "CMP A",

				      "RNZ", "POP B",   "JNZ", 	"JMP", 	"CNZ", 	"PUSH B",   "ADI", "RST 0",
				      "RZ",  "RET",	"JZ", 	"JMP",  "CZ", 	"CALL",     "ACI", "RST 1",
				      "RNC", "POP D", 	"JNC", 	"OUT",  "CNC", 	"PUSH D",   "SUI", "RST 2",
				      "RC",  "RET", 	"JC", 	"IN",   "CC",   "CALL",     "SBI", "RST 3",
				      "RPO", "POP H",   "JPO",  "XTHL", "CPO", 	"PUSH H",   "ANI", "RST 4",
				      "RPE", "PCHL",    "JPE",  "XCHG", "CPE", 	"CALL",     "XRI", "RST 5",
				      "RP",  "POP PSW", "JP", 	"DI",   "CP", 	"PUSH PSW", "ORI", "RST 6",
				      "RM",  "SPHL",    "JM", 	"EI",   "CM", 	"CALL",     "CPI", "RST 7"};   	

/* -------------------------------------------------------------- */
static const char* write_extended_instr(uint16_t counter, const char* ass_cmd, const char* param, uint8_t  is_value)
{
	static char compl_instr[16] = {0};
	memset(compl_instr, 0, 16);

	if(is_value)
  	   sprintf(compl_instr, "0x%04x\t%s,#%s", counter, ass_cmd, param);
	else
	   sprintf(compl_instr, "0x%04x\t%s %s", counter,  ass_cmd, param);

	return compl_instr;
}


static const char* write_instr(uint16_t counter, const char* ass_cmd)
{
	static char compl_instr[16] = {0};
	memset(compl_instr, 0, 16);

	sprintf(compl_instr, "0x%04x\t%s", counter, ass_cmd);

	return compl_instr;
}

static const char* mov_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;

	return asm_code;
}

static const char* ora_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;

	return asm_code;
}

static const char* rz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;

	return asm_code;
}

static const char* ldax_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}

static const char* inx_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}

static const char* dcx_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}

static const char* jnz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}

static const char* ret_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}

static const char* nop_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}

static const char* stax_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* inr_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* dcr_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* mvi_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[8] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rlc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* lxi_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];
	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 3;
	return asm_code;
}


static const char* dad_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* rrc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* ral_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* rar_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* shld_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];
	char parameter[12] = {0};

	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);
	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* daa_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* lhld_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* cma_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* sta_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];
	char parameter[12] = {0};

	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]); 
	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* stc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* lda_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];
	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* cmc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* hlt_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* add_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* adc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* sub_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* sbb_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* ana_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* xra_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* cmp_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* rnz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* pop_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jmp_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* cnz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* push_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* adi_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rst_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* cz_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* call_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}



static const char* aci_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rnc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jnc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* out_instr(memory_t* rom, int* offset)
{	
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* cnc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* sui_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);


	*offset += 2;
	return asm_code;
}


static const char* rc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);


	*offset += 3;
	return asm_code;
}


static const char* in_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* cc_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* sbi_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);


	*offset += 2;
	return asm_code;
}


static const char* rpo_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jpo_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* xthl_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* cpo_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* ani_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rpe_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* pchl_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jpe_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* xchg_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* cpe_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* xri_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rp_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jp_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* di_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* cp_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* ori_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}


static const char* rm_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* sphl_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* jm_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* ei_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	const char* asm_code = write_instr(*offset, asm_names[opcode]);

	*offset += 1;
	return asm_code;
}


static const char* cm_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[12] = {0};
	sprintf(parameter, "$%02x%02x", rom->memory[*offset+2], rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 0);

	*offset += 3;
	return asm_code;
}


static const char* cpi_instr(memory_t* rom, int* offset)
{
	uint8_t opcode = rom->memory[*offset];

	char parameter[6] = {0};
	sprintf(parameter, "$%02x", rom->memory[*offset+1]);

	const char* asm_code = write_extended_instr(*offset, asm_names[opcode], parameter, 1);

	*offset += 2;
	return asm_code;
}

static void print_dissambled_code(const char* instr)
{
	printf("%s\n", instr);
}

const char* dikke_kut(memory_t* rom, int offset)
{
	int opcode = rom->memory[offset];

	const char* instr = (assembly_instr[opcode](rom, &offset));

	return instr;
}
void dissamble_curr_instr(memory_t* rom, int offset)
{
	int opcode = rom->memory[offset];

	const char* instr = (assembly_instr[opcode](rom, &offset));
	print_dissambled_code(instr);
}

void dissamble(memory_t* rom)
{
	int offset = 0;
	while(offset < rom->memory_size)   {
		int opcode = rom->memory[offset];

		if(opcode > 0xFF || opcode < 0)
			exit(0);

		const char* instr = (assembly_instr[opcode](rom, &offset));
		print_dissambled_code(instr);
	}
}
