#include "cpu.h"
#include "deassembler.h"

typedef unsigned long uint32_t;

#define OPCODE_INSTR		(0xFF)
#define OPCODE_FUNC(a)		static void a

#define INCR_PC_CNT(cpu_ptr)        (cpu_ptr->core.pc++)
#define INCR_PC_X_CNT(cpu_ptr, x)   (cpu_ptr->core.pc += x)
#define SET_PC_CNT(cpu_ptr, x)      (cpu_ptr->core.pc = x)

static void mov_instr(memory_t*, memory_t*, cpu_model_t*);
static void ora_instr(memory_t*, memory_t*, cpu_model_t*);
static void rz_instr(memory_t*, memory_t*, cpu_model_t*);
static void ldax_instr(memory_t*, memory_t*, cpu_model_t*);
static void inx_instr(memory_t*, memory_t*, cpu_model_t*);
static void dcx_instr(memory_t*, memory_t*, cpu_model_t*);
static void jnz_instr(memory_t*, memory_t*, cpu_model_t*);
static void ret_instr(memory_t*, memory_t*, cpu_model_t*);
static void nop_instr(memory_t*, memory_t*, cpu_model_t*);
static void stax_instr(memory_t*, memory_t*, cpu_model_t*);
static void inr_instr(memory_t*, memory_t*, cpu_model_t*);
static void dcr_instr(memory_t*, memory_t*, cpu_model_t*);
static void mvi_instr(memory_t*, memory_t*, cpu_model_t*);
static void rlc_instr(memory_t*, memory_t*, cpu_model_t*);
static void lxi_instr(memory_t*, memory_t*, cpu_model_t*);
static void dcr_instr(memory_t*, memory_t*, cpu_model_t*);
static void dad_instr(memory_t*, memory_t*, cpu_model_t*);
static void rrc_instr(memory_t*, memory_t*, cpu_model_t*);
static void ral_instr(memory_t*, memory_t*, cpu_model_t*);
static void rar_instr(memory_t*, memory_t*, cpu_model_t*);
static void shld_instr(memory_t*, memory_t*, cpu_model_t*);
static void daa_instr(memory_t*, memory_t*, cpu_model_t*);
static void lhld_instr(memory_t*, memory_t*, cpu_model_t*);
static void cma_instr(memory_t*, memory_t*, cpu_model_t*);
static void sta_instr(memory_t*, memory_t*, cpu_model_t*);
static void stc_instr(memory_t*, memory_t*, cpu_model_t*);
static void lda_instr(memory_t*, memory_t*, cpu_model_t*);
static void cmc_instr(memory_t*, memory_t*, cpu_model_t*);
static void hlt_instr(memory_t*, memory_t*, cpu_model_t*);
static void add_instr(memory_t*, memory_t*, cpu_model_t*);
static void adc_instr(memory_t*, memory_t*, cpu_model_t*);
static void sub_instr(memory_t*, memory_t*, cpu_model_t*);
static void sbb_instr(memory_t*, memory_t*, cpu_model_t*);
static void ana_instr(memory_t*, memory_t*, cpu_model_t*);
static void xra_instr(memory_t*, memory_t*, cpu_model_t*);
static void cmp_instr(memory_t*, memory_t*, cpu_model_t*);
static void rnz_instr(memory_t*, memory_t*, cpu_model_t*);
static void pop_instr(memory_t*, memory_t*, cpu_model_t*);
static void jmp_instr(memory_t*, memory_t*, cpu_model_t*);
static void cnz_instr(memory_t*, memory_t*, cpu_model_t*);
static void push_instr(memory_t*, memory_t*, cpu_model_t*);
static void adi_instr(memory_t*, memory_t*, cpu_model_t*);
static void rst_instr(memory_t*, memory_t*, cpu_model_t*);
static void jz_instr(memory_t*, memory_t*, cpu_model_t*);
static void cz_instr(memory_t*, memory_t*, cpu_model_t*);
static void call_instr(memory_t*, memory_t*, cpu_model_t*);
static void aci_instr(memory_t*, memory_t*, cpu_model_t*);
static void rnc_instr(memory_t*, memory_t*, cpu_model_t*);
static void jnc_instr(memory_t*, memory_t*, cpu_model_t*);
static void out_instr(memory_t*, memory_t*, cpu_model_t*);
static void cnc_instr(memory_t*, memory_t*, cpu_model_t*);
static void sui_instr(memory_t*, memory_t*, cpu_model_t*);
static void rc_instr(memory_t*, memory_t*, cpu_model_t*);
static void jc_instr(memory_t*, memory_t*, cpu_model_t*);
static void in_instr(memory_t*, memory_t*, cpu_model_t*);
static void cc_instr(memory_t*, memory_t*, cpu_model_t*);
static void sbi_instr(memory_t*, memory_t*, cpu_model_t*);
static void rpo_instr(memory_t*, memory_t*, cpu_model_t*);
static void jpo_instr(memory_t*, memory_t*, cpu_model_t*);
static void xthl_instr(memory_t*, memory_t*, cpu_model_t*);
static void cpo_instr(memory_t*, memory_t*, cpu_model_t*);
static void ani_instr(memory_t*, memory_t*, cpu_model_t*);
static void rpe_instr(memory_t*, memory_t*, cpu_model_t*);
static void pchl_instr(memory_t*, memory_t*, cpu_model_t*);
static void jpe_instr(memory_t*, memory_t*, cpu_model_t*);
static void xchg_instr(memory_t*, memory_t*, cpu_model_t*);
static void cpe_instr(memory_t*, memory_t*, cpu_model_t*);
static void xri_instr(memory_t*, memory_t*, cpu_model_t*);
static void rp_instr(memory_t*, memory_t*, cpu_model_t*);
static void jp_instr(memory_t*, memory_t*, cpu_model_t*);
static void di_instr(memory_t*, memory_t*, cpu_model_t*);
static void cp_instr(memory_t*, memory_t*, cpu_model_t*);
static void ori_instr(memory_t*, memory_t*, cpu_model_t*);
static void rm_instr(memory_t*, memory_t*, cpu_model_t*);
static void sphl_instr(memory_t*, memory_t*, cpu_model_t*);
static void jm_instr(memory_t*, memory_t*, cpu_model_t*);
static void ei_instr(memory_t*, memory_t*, cpu_model_t*);
static void cm_instr(memory_t*, memory_t*, cpu_model_t*);
static void cpi_instr(memory_t*, memory_t*, cpu_model_t*);

static void empty_instr(memory_t*, memory_t*, cpu_model_t*);

/* Table taken from http://pastraiser.com/cpu/i8080/i8080_opcodes.html */
static void (*assembly_instr[]) (memory_t*, memory_t*, cpu_model_t*) = 
{
	nop_instr,  inx_instr,  stax_instr,  inx_instr,   inr_instr,  dcr_instr,   mvi_instr,  rlc_instr,
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


// ---------------------------------------------------------------
void display_curr_cpu_status(cpu_core_t* core)
{
	printf("\n--------------------------");
	printf("------ CPU STATUS --------\n");
	
	printf("PC: %d\t 0x%04x\n", core->pc, core->pc);
	printf("Stack: 0x%04x\n", core->stack);
	printf("Reg B: 0x%02x\t\tReg C 0x%02x\n", core->b, core->c);
	printf("Reg D: 0x%02x\t\tReg E 0x%02x\n", core->d, core->e);
	printf("Reg H: 0x%02x\t\tReg L 0x%02x\n", core->h, core->l);
	printf("Reg A: 0x%02x\t\tReg M 0x%02x\n", core->a, core->m);
	printf("Status: C: %d P: %d AC: %d z: %d s: %d\n", core->status.bits.c,
							   core->status.bits.p,
							   core->status.bits.ac,
							   core->status.bits.z,
							   core->status.bits.s);
        printf("----------------------------\n");	
}

// ---------------------------------------------------------------
void display_cpu_status(cpu_model_t *cpu)
{
	display_curr_cpu_status(&cpu->core);
}

// ---------------------------------------------------------------
//                 CARRY BIT INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(cmc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->core.status.bits.c = (cpu->core.status.bits.c == 0) ? 1 : 0;
	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(stc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->core.status.bits.c = 1;
	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//                 SINGLE REGISTER INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(inr_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint8_t *reg = common_func_get_reg(cpu, opcode);

	if (reg == &cpu->core.m) {
		uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
		uint16_t val = ram->memory[addr]++;

		common_func_reg_zero_status_bit(cpu, val);
		common_func_reg_parity_status_bit(cpu, val);
		common_func_reg_sign_status_bit(cpu, val);

	}

	else {
		(*reg)++;
		common_func_reg_zero_status_bit(cpu, *reg);
		common_func_reg_parity_status_bit(cpu, *reg);
		common_func_reg_sign_status_bit(cpu, *reg);
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(dcr_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint8_t *reg = common_func_get_reg(cpu, opcode);

	if (reg == &cpu->core.m) {
		uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
		uint16_t val = ram->memory[addr]--;

		common_func_reg_zero_status_bit(cpu, val);
		common_func_reg_parity_status_bit(cpu, val);
		common_func_reg_sign_status_bit(cpu, val);

	}

	else {
		(*reg)--;
		common_func_reg_zero_status_bit(cpu, *reg);
		common_func_reg_parity_status_bit(cpu, *reg);
		common_func_reg_sign_status_bit(cpu, *reg);
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(cma_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->core.a = ~(cpu->core.a);

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(daa_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t lb_acc = cpu->core.a & 0x0F;
	uint8_t hb_acc = cpu->core.a & 0xF0;

	if (lb_acc > 9 || cpu->core.status.bits.ac == 1) {
		cpu->core.a += 6;

		if (hb_acc != cpu->core.a & 0xF0)
			cpu->core.status.bits.ac = 1;
	}

	hb_acc = (cpu->core.a >> 4) & 0x0F;

	if (hb_acc > 9 || cpu->core.status.bits.c == 1) {
		hb_acc += 9;
		cpu->core.a = ((hb_acc << 4) & 0xF0) | cpu->core.a & 0x0F;
	}

	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//                 NOP INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(nop_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//                 DATA TRANSFER INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(mov_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = 0x00;

	switch(opcode) {
		case 0x40:
			break;
		case 0x41:
			cpu->core.b = cpu->core.c;
			break;
		case 0x42:
			cpu->core.b = cpu->core.d;
			break;
		case 0x43:
			cpu->core.b = cpu->core.e;
			break;
		case 0x44:
			cpu->core.b = cpu->core.h;
			break;
		case 0x45:
			cpu->core.b = cpu->core.l;
			break;
		case 0x46:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.b = ram->memory[addr];
			break;
		case 0x47:
			cpu->core.b = cpu->core.a;
			break;
		case 0x48:
			cpu->core.c = cpu->core.b;
			break;
		case 0x49:
			break;
		case 0x4A:
			cpu->core.c = cpu->core.d;
			break;
		case 0x4B:
			cpu->core.c = cpu->core.e;
			break;
		case 0x4C:
			cpu->core.c = cpu->core.h;
			break;
		case 0x4D:
			cpu->core.c = cpu->core.l;
			break;
		case 0x4E:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.c = ram->memory[addr];
			break;
		case 0x4F:
			cpu->core.c = cpu->core.a;
			break;
		case 0x50:
			cpu->core.d = cpu->core.b;
			break;
		case 0x51:
			cpu->core.d = cpu->core.c;
			break;
		case 0x52:
			cpu->core.d = cpu->core.d;
			break;
		case 0x53:
			cpu->core.d = cpu->core.e;
			break;
		case 0x54:
			cpu->core.d = cpu->core.h;
			break;
		case 0x55:
			cpu->core.d = cpu->core.l;
			break;
		case 0x56:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.d = ram->memory[addr];
			break;
		case 0x57:
			cpu->core.d = cpu->core.a;
			break;
		case 0x58:
			cpu->core.e = cpu->core.b;
			break;
		case 0x59:
			cpu->core.e = cpu->core.c;
			break;
		case 0x5A:
			cpu->core.e = cpu->core.d;
			break;
		case 0x5B:
			cpu->core.e = cpu->core.e;
			break;
		case 0x5C:
			cpu->core.e = cpu->core.h;
			break;
		case 0x5D:
			cpu->core.e = cpu->core.l;
			break;
		case 0x5E:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.e = ram->memory[addr];
			break;
		case 0x5F:
			cpu->core.e = cpu->core.a;
			break;
		case 0x60:
			cpu->core.h = cpu->core.b;
			break;
		case 0x61:
			cpu->core.h = cpu->core.c;
			break;
		case 0x62:
			cpu->core.h = cpu->core.d;
			break;
		case 0x63:
			cpu->core.h = cpu->core.e;
			break;
		case 0x64:
			cpu->core.h = cpu->core.h;
			break;
		case 0x65:
			cpu->core.h = cpu->core.l;
			break;
		case 0x66:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.h = ram->memory[addr];
			break;
		case 0x67:
			cpu->core.h = cpu->core.a;
			break;
		case 0x68:
			cpu->core.l = cpu->core.b;
			break;
		case 0x69:
			cpu->core.l = cpu->core.c;
			break;
		case 0x6A:
			cpu->core.l = cpu->core.d;
			break;
		case 0x6B:
			cpu->core.l = cpu->core.e;
			break;
		case 0x6C:
			cpu->core.l = cpu->core.h;
			break;
		case 0x6D:
			cpu->core.l = cpu->core.l;
			break;
		case 0x6E:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.l = ram->memory[addr];
			break;
		case 0x6F:
			cpu->core.l = cpu->core.a;
			break;
		case 0x70:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.b;
			break;
		case 0x71:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.c;
			break;
		case 0x72:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.d;
			break;
		case 0x73:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.e;
			break;
		case 0x74:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.h;
			break;
		case 0x75:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.l;
			break;
		case 0x77:
			addr = (cpu->core.h << 8) | cpu->core.l;
			ram->memory[addr] = cpu->core.a;
			break;
		case 0x78:
			cpu->core.a = cpu->core.b;
			break;
		case 0x79:
			cpu->core.a = cpu->core.c;
			break;
		case 0x7A:
			cpu->core.a = cpu->core.d;
			break;
		case 0x7B:
			cpu->core.a = cpu->core.e;
			break;
		case 0x7C:
			cpu->core.a = cpu->core.h;
			break;
		case 0x7D:
			cpu->core.a = cpu->core.l;
			break;
		case 0x7E:
			addr = (cpu->core.h << 8) | cpu->core.l;
			cpu->core.a = ram->memory[addr];
			break;
		case 0x7F:
			cpu->core.a = cpu->core.a;
			break;
	
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(stax_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t address = 0x00;

	if (opcode == 0x02) {
		address = cpu->core.b << 8 | cpu->core.c;
	} else {
		address = cpu->core.d << 8 | cpu->core.e;
	}

	ram->memory[address] = cpu->core.a;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(ldax_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t address = 0x00;

	if (opcode == 0x0A) {
		address = cpu->core.b << 8 | cpu->core.c;
	} else {
		address = cpu->core.d << 8 | cpu->core.e;
	}

	cpu->core.a = ram->memory[address];
	
	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//        REGISTER OR MEMORY TO ACCUMULATOR INSTRUCTIONS
// ---------------------------------------------------------------
static void add_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	uint8_t ac_prod = (cpu->core.a & 0x0F) + (reg_val & 0x0F);
	cpu->core.status.bits.ac = ((ac_prod & 0x10) != 0);

	uint16_t c_prod = cpu->core.a + reg_val;
	cpu->core.status.bits.c = ((c_prod & 0x0100) != 0);

	cpu->core.a = (uint8_t) (c_prod & 0xFF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

static void adc_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	uint8_t cbit = cpu->core.status.bits.c != 0;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (reg_val & 0x0F) + cbit;
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0x00);

	uint16_t res = cpu->core.a  + reg_val + cbit;
	cpu->core.status.bits.c = ((res & 0x0100) != 0x00);

	cpu->core.a = (uint8_t) (res & 0x00FF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

static void sbb_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	uint8_t cbit = cpu->core.status.bits.c != 0;
	uint8_t two_qu = ~(reg_val + cbit) + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (reg_val & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0x00);

	uint16_t c_res = cpu->core.a + two_qu;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0x00);

	cpu->core.a = (uint8_t) (c_res & 0x00FF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

static void sub_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	uint8_t two_qu = ~reg_val + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (reg_val & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0x00);

	uint16_t c_res = cpu->core.a + two_qu;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0x00);

	cpu->core.a = (uint8_t) (c_res & 0x00FF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

}

static void cmp_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	uint8_t two_q = ~reg_val + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (two_q & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0);

	uint16_t c_res = cpu->core.a + two_q;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0);

	uint8_t result = cpu->core.a + two_q;

	common_func_reg_zero_status_bit(cpu, result);
	common_func_reg_parity_status_bit(cpu, result);
	common_func_reg_sign_status_bit(cpu, result);
}

static void ora_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	cpu->core.c = 0;

	cpu->core.a |= reg_val;

	cpu->core.status.bits.c = 0;
	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

static void ana_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	cpu->core.c = 0;

	cpu->core.a &= reg_val;

	cpu->core.status.bits.c = 0;
	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

static void xra_register_value(uint8_t reg_val, cpu_model_t *cpu)
{
	cpu->core.c = 0;

	cpu->core.a ^= reg_val;

	cpu->core.status.bits.c = 0;
	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);
}

OPCODE_FUNC(ora_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0xB0:
			ora_register_value(cpu->core.b, cpu);
			break;
		case 0xB1:
			ora_register_value(cpu->core.c, cpu);
			break;
		case 0xB2:
			ora_register_value(cpu->core.d, cpu);
			break;
		case 0xB3:
			ora_register_value(cpu->core.e, cpu);
			break;
		case 0xB4:
			ora_register_value(cpu->core.h, cpu);
			break;
		case 0xB5:
			ora_register_value(cpu->core.l, cpu);
			break;
		case 0xB6:
			ora_register_value(ram->memory[addr], cpu);
			break;
		case 0xB7:
			ora_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(add_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0x80:
			add_register_value(cpu->core.b, cpu);
			break;
		case 0x81:
			add_register_value(cpu->core.c, cpu);
			break;
		case 0x82:
			add_register_value(cpu->core.d, cpu);
			break;
		case 0x83:
			add_register_value(cpu->core.e, cpu);
			break;
		case 0x84:
			add_register_value(cpu->core.h, cpu);
			break;
		case 0x85:
			add_register_value(cpu->core.l, cpu);
			break;
		case 0x86:
			add_register_value(ram->memory[addr], cpu);
			break;
		case 0x87:
			add_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(adc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0x88:
			adc_register_value(cpu->core.b, cpu);
			break;
		case 0x89:
			adc_register_value(cpu->core.c, cpu);
			break;
		case 0x8A:
			adc_register_value(cpu->core.d, cpu);
			break;
		case 0x8B:
			adc_register_value(cpu->core.e, cpu);
			break;
		case 0x8C:
			adc_register_value(cpu->core.h, cpu);
			break;
		case 0x8D:
			adc_register_value(cpu->core.l, cpu);
			break;
		case 0x8E:
			adc_register_value(ram->memory[addr], cpu);
			break;
		case 0x8F:
			adc_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(sub_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0x90:
			sub_register_value(cpu->core.b, cpu);
			break;
		case 0x91:
			sub_register_value(cpu->core.c, cpu);
			break;
		case 0x92:
			sub_register_value(cpu->core.d, cpu);
			break;
		case 0x93:
			sub_register_value(cpu->core.e, cpu);
			break;
		case 0x94:
			sub_register_value(cpu->core.h, cpu);
			break;
		case 0x95:
			sub_register_value(cpu->core.l, cpu);
			break;
		case 0x96:
			sub_register_value(ram->memory[addr], cpu);
			break;
		case 0x97:
			sub_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);

}

OPCODE_FUNC(sbb_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0x98:
			sbb_register_value(cpu->core.b, cpu);
			break;
		case 0x99:
			sbb_register_value(cpu->core.c, cpu);
			break;
		case 0x9A:
			sbb_register_value(cpu->core.d, cpu);
			break;
		case 0x9B:
			sbb_register_value(cpu->core.e, cpu);
			break;
		case 0x9C:
			sbb_register_value(cpu->core.h, cpu);
			break;
		case 0x9D:
			sbb_register_value(cpu->core.l, cpu);
			break;
		case 0x9E:
			sbb_register_value(ram->memory[addr], cpu);
			break;
		case 0x9F:
			sbb_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);

}

OPCODE_FUNC(ana_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0xA0:
			ana_register_value(cpu->core.b, cpu);
			break;
		case 0xA1:
			ana_register_value(cpu->core.c, cpu);
			break;
		case 0xA2:
			ana_register_value(cpu->core.d, cpu);
			break;
		case 0xA3:
			ana_register_value(cpu->core.e, cpu);
			break;
		case 0xA4:
			ana_register_value(cpu->core.h, cpu);
			break;
		case 0xA5:
			ana_register_value(cpu->core.l, cpu);
			break;
		case 0xA6:
			ana_register_value(ram->memory[addr], cpu);
			break;
		case 0xA7:
			ana_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);

}

OPCODE_FUNC(xra_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0xA8:
			xra_register_value(cpu->core.b, cpu);
			break;
		case 0xA9:
			xra_register_value(cpu->core.c, cpu);
			break;
		case 0xAA:
			xra_register_value(cpu->core.d, cpu);
			break;
		case 0xAB:
			xra_register_value(cpu->core.e, cpu);
			break;
		case 0xAC:
			xra_register_value(cpu->core.h, cpu);
			break;
		case 0xAD:
			xra_register_value(cpu->core.l, cpu);
			break;
		case 0xAE:
			xra_register_value(ram->memory[addr], cpu);
			break;
		case 0xAF:
			xra_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(cmp_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0xB8:
			cmp_register_value(cpu->core.b, cpu);
			break;
		case 0xB9:
			cmp_register_value(cpu->core.c, cpu);
			break;
		case 0xBA:
			cmp_register_value(cpu->core.d, cpu);
			break;
		case 0xBB:
			cmp_register_value(cpu->core.e, cpu);
			break;
		case 0xBC:
			cmp_register_value(cpu->core.h, cpu);
			break;
		case 0xBD:
			cmp_register_value(cpu->core.l, cpu);
			break;
		case 0xBE:
			cmp_register_value(ram->memory[addr], cpu);
			break;
		case 0xBF:
			cmp_register_value(cpu->core.a, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//              ROTATE ACCUMULATOR INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(rlc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t hbit = ((cpu->core.a & 0x80) != 0);

	cpu->core.a <<= 1;
	cpu->core.a |= hbit;
	cpu->core.status.bits.c = hbit;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(rrc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t lbit = ((cpu->core.a & 0x01) != 0);

	cpu->core.a >>= 1;
	cpu->core.a |= (lbit << 7);
	cpu->core.status.bits.c = lbit;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(ral_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t hbit = ((cpu->core.a & 0x80) != 0);
	uint8_t cbit = cpu->core.status.bits.c;
			
	cpu->core.a <<= 1;
	cpu->core.a |= cbit;
	cpu->core.status.bits.c = hbit;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(rar_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t lbit = ((cpu->core.a & 0x01) != 0);
	uint8_t cbit = cpu->core.status.bits.c;

	cpu->core.a >>= 1;
	cpu->core.a |= (cbit << 7);
	cpu->core.status.bits.c = lbit;

	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//              REGISTER PAIR INSTRUCTIONS
// ---------------------------------------------------------------
static inline void push_on_the_stack(uint8_t hbit, uint8_t lbit, memory_t *ram, cpu_model_t *cpu)
{
	ram->memory[--(cpu->core.stack)] = hbit;
	ram->memory[--(cpu->core.stack)] = lbit;
}

static inline void pop_from_the_stack(uint8_t *hbit, uint8_t *lbit, memory_t *ram, cpu_model_t *cpu)
{
	*lbit = ram->memory[(cpu->core.stack)++];
	*hbit = ram->memory[(cpu->core.stack)++];
}

OPCODE_FUNC(push_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	switch (opcode) {
		case 0xC5:
			push_on_the_stack(cpu->core.b, cpu->core.c, ram, cpu);
			break;
		case 0xD5:
			push_on_the_stack(cpu->core.d, cpu->core.e, ram, cpu);
			break;
		case 0xE5:
			push_on_the_stack(cpu->core.h, cpu->core.l, ram, cpu);
			break;
		case 0xF5:
			push_on_the_stack(cpu->core.a, cpu->core.status.reg, ram, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(pop_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];

	switch (opcode) {
		case 0xC1:
			pop_from_the_stack(&cpu->core.b, &cpu->core.c, ram, cpu);
			break;
		case 0xD1:
			pop_from_the_stack(&cpu->core.b, &cpu->core.c, ram, cpu);
			break;
		case 0xE1:
			pop_from_the_stack(&cpu->core.h, &cpu->core.l, ram, cpu);
			break;
		case 0xF1:
			pop_from_the_stack(&cpu->core.a, &cpu->core.status.reg, ram, cpu);
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(dad_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t hl = (cpu->core.h << 8) | cpu->core.l;
	uint16_t src = 0;
	uint32_t prod = 0;

	switch (opcode) {
		case 0x09:
			src = (cpu->core.b << 8) | cpu->core.c;
			prod = hl + src;
			break;
		case 0x19:
			src = (cpu->core.d << 8) | cpu->core.e;
			prod = hl + src;
			break;
		case 0x29:
			prod = hl << 1;
			break;
		case 0x39:
			prod = hl + cpu->core.stack;
			break;
	}

	cpu->core.status.bits.c = (prod & 0x00010000) ? 1 : 0;

	cpu->core.h = (uint8_t) ((prod >> 8) && 0xFF);
	cpu->core.l = (uint8_t) (prod & 0xFF);

	INCR_PC_CNT(cpu);
}

static void increase_register_pair(uint8_t *hbit, uint8_t *lbit, cpu_model_t *cpu)
{
	uint16_t prod = (*hbit << 8) | *lbit;
	prod++;
	
	*hbit = (prod >> 8) & 0xFF;
	*lbit = prod & 0xFF;
}

OPCODE_FUNC(inx_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t prod = 0;

	switch (opcode) {
		case 0x03:
			increase_register_pair(&cpu->core.b, &cpu->core.c, cpu);
			break;
		case 0x13:
			increase_register_pair(&cpu->core.d, &cpu->core.e, cpu);
			break;
		case 0x23:
			increase_register_pair(&cpu->core.h, &cpu->core.l, cpu);
			break;
		case 0x33:
			cpu->core.stack++;
			break;
	}

	INCR_PC_CNT(cpu);
}

static void decrease_register_pair(uint8_t *hbit, uint8_t *lbit, cpu_model_t *cpu)
{
	uint16_t prod = (*hbit << 8) | *lbit;
	prod--;

	*hbit = (prod >> 8) & 0xFF;
	*lbit = prod & 0xFF;
}

OPCODE_FUNC(dcx_instr)(memory_t* ram, memory_t* rom, cpu_model_t *cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint16_t prod = 0;

	switch (opcode) {
		case 0x0B:
			decrease_register_pair(&cpu->core.b, &cpu->core.c, cpu);
			break;
		case 0x1B:
			decrease_register_pair(&cpu->core.d, &cpu->core.e, cpu);
			break;
		case 0x2B:
			decrease_register_pair(&cpu->core.h, &cpu->core.l, cpu);
			break;
		case 0x3B:
			cpu->core.stack--;
			break;
	}

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(xchg_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t hbit = cpu->core.d;
	uint8_t lbit = cpu->core.e;

	cpu->core.d = cpu->core.h;
	cpu->core.e = cpu->core.l;
	cpu->core.h = hbit;
	cpu->core.l = lbit;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(xthl_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	ram->memory[cpu->core.stack] = cpu->core.l;
	ram->memory[cpu->core.stack +1] = cpu->core.h;

	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(sphl_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->core.stack = (cpu->core.h << 8) | cpu->core.l;
	INCR_PC_CNT(cpu);
}

// ---------------------------------------------------------------
//              IMMEDIATE INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(mvi_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];
	uint8_t data = rom->memory[cpu->core.pc+1];
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	switch (opcode) {
		case 0x06:
			cpu->core.b = data;
			break;
		case 0x16:
			cpu->core.d = data;
			break;
		case 0x26:
			cpu->core.h = data;
			break;
		case 0x36:
			ram->memory[addr] = data;
			break;
		case 0x0E:
			cpu->core.c = data;
			break;
		case 0x1E:
			cpu->core.e = data;
			break;
		case 0x2E:
			cpu->core.l = data;
			break;
		case 0x3E:
			cpu->core.a = data;
	}

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(adi_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data  = rom->memory[cpu->core.pc + 1];

	common_func_reg_c_status_bit(cpu, cpu->core.a, data);
	common_func_reg_ac_status_bit(cpu, cpu->core.a, data);

	cpu->core.a += data;

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(aci_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];
	uint8_t cbit = cpu->core.status.bits.c != 0;

	common_func_reg_c_status_bit(cpu, cpu->core.a, (data + cbit));
	common_func_reg_ac_status_bit(cpu, cpu->core.a, (data + cbit));

	cpu->core.a += data;

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(sui_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];
	uint8_t two_qu = ~data + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (data & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0x00);

	uint16_t c_res = cpu->core.a + two_qu;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0x00);

	cpu->core.a = (uint8_t) (c_res & 0x00FF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(sbi_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];
	uint8_t cbit = cpu->core.status.bits.c != 0;
	uint8_t two_qu = ~(data + cbit) + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (data & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0x00);

	uint16_t c_res = cpu->core.a + two_qu;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0x00);

	cpu->core.a = (uint8_t) (c_res & 0x00FF);

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(ani_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];

	cpu->core.a &= data;

	cpu->core.status.bits.c = 0;

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(xri_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];

	cpu->core.a ^= data;

	cpu->core.status.bits.c = 0;

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(ori_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];

	cpu->core.a |= data;

	cpu->core.status.bits.c = 0;

	common_func_reg_zero_status_bit(cpu, cpu->core.a);
	common_func_reg_parity_status_bit(cpu, cpu->core.a);
	common_func_reg_sign_status_bit(cpu, cpu->core.a);

	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(cpi_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t data = rom->memory[cpu->core.pc + 1];
	uint8_t two_q = ~data + 1;

	uint8_t ac_res = (cpu->core.a & 0x0F) + (two_q & 0x0F);
	cpu->core.status.bits.ac = ((ac_res & 0x10) != 0);

	uint16_t c_res = cpu->core.a + two_q;
	cpu->core.status.bits.c = ((c_res & 0x0100) != 0);

	uint8_t result = cpu->core.a + two_q;

	common_func_reg_zero_status_bit(cpu, result);
	common_func_reg_parity_status_bit(cpu, result);
	common_func_reg_sign_status_bit(cpu, result);

	INCR_PC_X_CNT(cpu, 2);
}

static uint16_t get_direct_address_from_mem(memory_t *mem, cpu_model_t *cpu);

OPCODE_FUNC(lxi_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[cpu->core.pc];

	switch(opcode) {
		case 0x01:
			cpu->core.c = rom->memory[cpu->core.pc + 1];
			cpu->core.b = rom->memory[cpu->core.pc + 2];
			break;
		case 0x11:
			cpu->core.e = rom->memory[cpu->core.pc + 1];
			cpu->core.d = rom->memory[cpu->core.pc + 2];
			break;
		case 0x21:
			cpu->core.l = rom->memory[cpu->core.pc + 1];
			cpu->core.h = rom->memory[cpu->core.pc + 2];
			break;
		case 0x31:
			cpu->core.stack = get_direct_address_from_mem(rom, cpu);
			break;
	}

	INCR_PC_X_CNT(cpu, 3);
}

// ---------------------------------------------------------------
// 		DIRECT ADDRESSING INSTRUCTIONS
// ---------------------------------------------------------------
static uint16_t get_direct_address_from_mem(memory_t *mem, cpu_model_t *cpu)
{
	uint8_t laddr = mem->memory[cpu->core.pc+1];
	uint8_t haddr = mem->memory[cpu->core.pc+2];

	return ((haddr << 8) | laddr);
}

OPCODE_FUNC(sta_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t addr = get_direct_address_from_mem(rom, cpu); 

	ram->memory[addr] = cpu->core.a;

	INCR_PC_X_CNT(cpu, 3);
}

OPCODE_FUNC(lda_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t addr = get_direct_address_from_mem(rom, cpu); 

	cpu->core.a = ram->memory[addr];

	INCR_PC_X_CNT(cpu, 3);
}

OPCODE_FUNC(shld_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t addr = get_direct_address_from_mem(rom, cpu); 

	ram->memory[addr] = cpu->core.l;
	ram->memory[addr +1] = cpu->core.h;

	INCR_PC_X_CNT(cpu, 3);
}

OPCODE_FUNC(lhld_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t addr = get_direct_address_from_mem(rom, cpu); 

	cpu->core.l = ram->memory[addr];
	cpu->core.h = ram->memory[addr+1];

	INCR_PC_X_CNT(cpu, 3);
}

// ---------------------------------------------------------------
// 		JUMP INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(pchl_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t pc_counter = (cpu->core.h << 8) | cpu->core.l;

	SET_PC_CNT(cpu, pc_counter);
}

OPCODE_FUNC(jmp_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t addr = get_direct_address_from_mem(rom, cpu);

	SET_PC_CNT(cpu, addr);
}

OPCODE_FUNC(jc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c == 1) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jnc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c == 0) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z == 1) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jnz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z == 0) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jm_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s == 1) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jp_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s == 0) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(jpe_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p == 1) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}

}

OPCODE_FUNC(jpo_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p == 0) {
		uint16_t addr = get_direct_address_from_mem(rom, cpu);
		SET_PC_CNT(cpu, addr);
	}

	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

// ---------------------------------------------------------------
// 		CALL SUBROUTINE INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(call_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint16_t pc_to_store = cpu->core.pc + 3;
	uint8_t hbit = (pc_to_store >> 8) & 0xFF;
	uint8_t lbit = pc_to_store & 0xFF;
	uint8_t l_addr_bit = rom->memory[cpu->core.pc+1];
	uint8_t h_addr_bit = rom->memory[cpu->core.pc+2];

	push_on_the_stack(hbit, lbit, ram, cpu);

	cpu->core.pc = (h_addr_bit << 8) | l_addr_bit;
}

OPCODE_FUNC(cc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c == 1) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cnc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c == 0) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z == 1) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cnz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z == 0) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cm_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s == 0) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cp_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s == 1) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cpe_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p == 1) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

OPCODE_FUNC(cpo_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p == 0) {
		call_instr(ram, rom, cpu);
	}
	else {
		INCR_PC_X_CNT(cpu, 3);
	}
}

// ---------------------------------------------------------------
// 		RETURN FROM SUBROUTINE INSTRUCTIONS
// ---------------------------------------------------------------
static void return_instr(memory_t *ram, cpu_model_t *cpu)
{
	uint8_t hbit = 0;
	uint8_t lbit = 0;

	pop_from_the_stack(&hbit, &lbit, ram, cpu);

	cpu->core.pc = (hbit << 8) | lbit;
}

OPCODE_FUNC(ret_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	return_instr(ram, cpu);
}

OPCODE_FUNC(rz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z = 1) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rnz_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.z = 1) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c = 1) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rnc_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.c = 0) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rm_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s = 1) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rp_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.s = 0) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rpe_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p = 1) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rpo_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	if (cpu->core.status.bits.p = 0) {
		return_instr(ram, cpu);
	}

	else {
		INCR_PC_CNT(cpu);
	}
}

OPCODE_FUNC(rst_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	uint8_t opcode = rom->memory[0];
	uint8_t hbit   = (uint8_t) (((cpu->core.pc + 1) >> 8) & 0xFF);
	uint8_t lbit   = (uint8_t) ((cpu->core.pc + 1) & 0xFF);
	uint16_t cnt   = 0x00;

	push_on_the_stack(hbit, lbit, ram, cpu);

	switch (opcode) {
		case 0xC7:
			cnt = 8 * 0;
			break;
		case 0xD7:
			cnt = 8 * 2;
			break;
		case 0xE7:
			cnt = 8 * 4;
			break;
		case 0xF7:
			cnt = 8 * 6;
			break;
		case 0xCF:
			cnt = 8 * 1;
			break;
		case 0xDF:
			cnt = 8 * 3;
			break;
		case 0xEF:
			cnt = 8 * 5;
			break;
		case 0xFF:
			cnt = 8 * 7;
			break;
	}

	cpu->core.pc = cnt;

}

// ---------------------------------------------------------------
// 		INTERRUPT FLIP-FLOP INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(di_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->interrupt_enable = false;	
	INCR_PC_CNT(cpu);
}

OPCODE_FUNC(ei_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	cpu->interrupt_enable = true;
	INCR_PC_CNT(cpu);

}

// ---------------------------------------------------------------
// 		INPUT/OUTPUT INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(out_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	INCR_PC_X_CNT(cpu, 2);
}

OPCODE_FUNC(in_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	INCR_PC_X_CNT(cpu, 2);
}

// ---------------------------------------------------------------
// 		HALT INSTRUCTIONS
// ---------------------------------------------------------------
OPCODE_FUNC(hlt_instr)(memory_t* ram, memory_t* rom, cpu_model_t* cpu)
{
	INCR_PC_CNT(cpu);

	cpu->is_running = 0;
}

// -----------------------------------------------------------------------------------
void reset_cpu(cpu_model_t *cpu_8080)
{
	cpu_8080->is_running = 0;
	memset(&cpu_8080->core, 0, sizeof(cpu_core_t));
}

uint8_t cpu_get_reg_value(cpu_model_t *cpu_8080, char reg)
{
	switch (reg) {
		case 'A':
			reg = cpu_8080->core.a;
			break;
		case 'B':
			reg = cpu_8080->core.b;
			break;
		case 'C':
			reg = cpu_8080->core.c;
			break;
		case 'D':
			reg = cpu_8080->core.d;
			break;
		case 'E':
			reg = cpu_8080->core.e;
			break;
		case 'H':
			reg = cpu_8080->core.h;
			break;
		case 'L':
			reg = cpu_8080->core.l;
			break;
		case 'M':
			reg = cpu_8080->core.m;
			break;
	}

	return reg;
}

// -----------------------------------------------------------------------------------
void cpu_set_reg_value(cpu_model_t *cpu_8080, char reg, uint8_t val)
{
	switch (reg) {
		case 'A':
			cpu_8080->core.a = val;
			break;
		case 'B':
			cpu_8080->core.b = val;
			break;
		case 'C':
			cpu_8080->core.c = val;
			break;
		case 'D':
			cpu_8080->core.d = val;
			break;
		case 'E':
			cpu_8080->core.e = val;
			break;
		case 'H':
			cpu_8080->core.h = val;
			break;
		case 'L':
			cpu_8080->core.l = val;
			break;
		case 'M':
			cpu_8080->core.m = val;
			break;
	}
}


// -----------------------------------------------------------------------------------
void execute_interrupt_opcode_cmd(memory_t *ram, memory_t* rom, cpu_model_t* cpu_8080)
{
	uint16_t opcode = rom->memory[0];

	(*assembly_instr[opcode])(ram, rom, cpu_8080);
}

bool execute_single_cpu_cycle(memory_t* ram, memory_t* rom, cpu_model_t* cpu_8080)
{
	uint16_t opcode = rom->memory[cpu_8080->core.pc];

	(*assembly_instr[opcode])(ram, rom, cpu_8080);

	return (cpu_8080->core.pc < rom->memory_size);
}

// -----------------------------------------------------------------------------------
void execute_cpu(memory_t* ram, memory_t* rom, cpu_model_t* cpu_8080)   {

	while(rom->memory_size > cpu_8080->core.pc)   {

		if(cpu_8080->is_running == 1)
		{
			dissamble_curr_instr(rom, cpu_8080->core.pc);

			uint16_t opcode  = rom->memory[cpu_8080->core.pc];

			(*assembly_instr[opcode])(ram, rom, cpu_8080);
		}

		display_curr_cpu_status(&cpu_8080->core);	
	}
}
