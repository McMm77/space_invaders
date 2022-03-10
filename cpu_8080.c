#include "cpu.h"

FILE* pLogFile = NULL;

typedef enum opcode_index {
        eNOP = 0,  eLXIB,   eSTAXB,  eINXB,   eINRB,  eDCRB,    eMVIB,  eRLC,   eNOP1,  eDADB,  eLDAXB,  eDCXB,  eINRC,  eDCRC,  eMVIC,  eRRC,
        eNOP2,     eLXID,   eSTAXD,  eINXD,   eINRD,  eDCRD,    eMVID,  eRAL,   eNOP3,  eDADD,  eLDAXD,  eDCXD,  eINRE,  eDCRE,  eMVIE,  eRAR,
	eNOP4,     eLXIH,   eSHLD,   eINXH,   eINRH,  eDCRH,    eMVIH,  eDAA,   eNOP5,  eDADH,  eLHLD,   eDCXH,  eINRL,  eDCRL,  eMVIL,  eCMA,
	eNOP6,     eLXISP,  eSTA,    eINXSP,  eINRM,  eDCRM,    eMVIM,  eSTC,   eNOP7,  eDADSP, eLDA,    eDCX,   eINRA,  eDCRA,  eMVIA,  eCMC,
	eMOVBB,    eMOVBC,  eMOVBD,  eMOVBE,  eMOVBH, eMOVBL,   eMOVBM, eMOVBA, eMOVCB, eMOVCC, eMOVCD,  eMOVCE, eMOVCH, eMOVCL, eMOVCM, eMOVCA,
	eMOVDB,    eMOVDC,  eMOVDD,  eMOVDE,  eMOVDH, eMOVDL,   eMOVDM, eMOVDA, eMOVEB, eMOVEC, eMOVED,  eMOVEE, eMOVEH, eMOVEL, eMOVEM, eMOVEA,
	eMOVHB,    eMOVHC,  eMOVHD,  eMOVHE,  eMOVHH, eMOVHL,   eMOVHM, eMOVHA, eMOVLB, eMOVLC, eMOVLD,  eMOVLE, eMOVLH, eMOVLL, eMOVLM, eMOVLA,
	eMOVMB,    eMOVMC,  eMOVMD,  eMOVME,  eMOVMH, eMOVML,   eHLT,   eMOVMA, eMOVAB, eMOVAC, eMOVAD,  eMOVAE, eMOVAH, eMOVAL, eMOVAM, eMOVAA,
        eADDB,     eADDC,   eADDD,   eADDE,   eADDH,  eADDL,    eADDM,  eADDA,  eADCB,  eADCC,  eADCD,   eADCE,  eADCH,  eADCL,  eADCM,  eADCA,
	eSUBB,     eSUBC,   eSUBD,   eSUBE,   eSUBH,  eSUBL,    eSUBM,  eSUBA,  eSBBB,  eSBBC,  eSBBD,   eSBBE,  eSBBH,  eSBBL,  eSBBM,  eSBBA,
	eANAB,     eANAC,   eANAD,   eANAE,   eANAH,  eANAL,    eANAM,  eANAA,  eXRAB,  eXRAC,  eXRAD,   eXRAE,  eXRAH,  eXRAL,  eXRAM,  eXRAA,
	eORAB,     eORAC,   eORAD,   eORAE,   eORAH,  eORAL,    eORAM,  eORAA,  eCMPB,  eCMPC,  eCMPD,   eCMPE,  eCMPH,  eCMPL,  eCMPM,  eCMPA,
	eRNZ,      ePOPB,   eJNZ,    eJMP,    eCNZ,   ePUSHB,   eADI,   eRST_0, eRZ,    eRET,   eJZ,     eJMP1,  eCZ,    eCALL,  eACI,   eRST_1,
	eRNC,      ePOPD,   eJNC,    eOUT,    eCNC,   ePUSHD,   eSUI,   eRST_2, eRC,    eRET1,  eJC,     eIN,    eCC,    eCALL1, eSBI,   eRST_3,
	eRPO,      ePOPH,   eJPO,    eXTHL,   eCPO,   ePUSHH,   eANI,   eRST_4, eRPE,   ePCHL,  eJPE,    eXCHG,  eCPE,   eCALL2, eXRI,   eRST_5,
	eRP,       ePOPPSW, eJP,     eDI,     eCP,    ePUSHPSW, eORI,   eRST_6, eRM,    eSPHL,  eJM,     eEI,    eCM,    eCALL3, eCPI,   eRST_7
} opcode_index_t;

typedef struct opcode_instr opcode_instr_t;

typedef uint8_t (*opcode_handler) (opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);

struct opcode_instr {
        opcode_index_t index;           /* opcode index                           */
        const char *ass_uniq_cmd;             /* unique readable command                */
        const char *ass_cmd;                  /* readable command                       */
        uint8_t min_cycle_time;         /* minimal cycle time (in case of branch) */
        uint8_t max_cycle_time;         /* maximal cycle time (in case of branch) */
        uint8_t opcode_size;            /* opcode size (1,2 or 3)                 */
        opcode_handler func;            /* function pointer to handler            */
};

static inline void increase_program_counter(cpu_model_t*, opcode_instr_t*);
static inline void set_program_counter(cpu_model_t*, uint16_t);

static uint8_t opcode_handler_not_supported(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_nop(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lxi_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_stax_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inx_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rlc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dad_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ldax_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcx_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rrc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lxi_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_stax_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inx_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ral(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dad_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ldax_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcx_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rar(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lxi_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_shld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inx_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_daa(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dad_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lhld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcx_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cma(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lxi_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sta(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inx_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_stc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dad_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_lda(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcx_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_inr_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_dcr_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mvi_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_be(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_bm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ba(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_cb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_cc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_cd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ce(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ch(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_cl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_cm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ca(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_db(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_dc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_dd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_de(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_dh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_dl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_dm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_da(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_eb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ec(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ed(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ee(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_eh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_el(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_em(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ea(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_he(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_hm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ha(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_lb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_lc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_le(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_lh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ll(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_lm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_la(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_mb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_mc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_md(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_me(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_mh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ml(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_hlt(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ma(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ab(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ac(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ad(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ae(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_ah(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_al(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_am(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_mov_aa(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_add_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adc_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sub_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbb_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ana_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xra_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ora_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cmp_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_pop_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jmp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_push_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_adi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst0(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ret(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_call(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_aci(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst1(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_pop_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_out(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_push_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sui(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst2(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_in(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sbi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst3(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_pop_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xthl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_push_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ani(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst4(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_pchl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xchg(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_xri(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst5(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_pop_psw(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_di(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_push_psw(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ori(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst6(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_sphl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_jm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_ei(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_cpi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);
static uint8_t opcode_handler_rst7(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu);

static opcode_instr_t opcode_table[] = {{eNOP1, "NOP", "NOP", 4, 4, 1, opcode_handler_nop},
                                        {eLXIB, "LXIB", "LXI B", 10, 10, 3, opcode_handler_lxi_b},
                                        {eSTAXB, "STAXB", "STAX B", 7, 7, 1, opcode_handler_stax_b},
                                        {eINXB, "INXB", "INX B", 5, 5, 1, opcode_handler_inx_b},
                                        {eINRB, "INRB", "INR B", 5, 5, 1, opcode_handler_inr_b},
                                        {eDCRB, "DCRB", "DCR B", 5, 5, 1, opcode_handler_dcr_b},
                                        {eMVIB, "MVIB", "MVI B", 7, 7, 2, opcode_handler_mvi_b},
                                        {eRLC, "RLC", "RLC", 4, 4, 1, opcode_handler_rlc},
                                        {eNOP1, "NOP1", "NOP", 4, 4, 1, opcode_handler_not_supported},
                                        {eDADB, "DADB", "DAD B", 10, 10, 1, opcode_handler_dad_b},
                                        {eLDAXB, "LDAXB", "LDAX B", 7, 7, 1, opcode_handler_ldax_b},
                                        {eDCXB, "DCXB", "DCX B", 5, 5, 1, opcode_handler_dcx_b},
                                        {eINRC, "INRC", "INR C", 5, 5, 1, opcode_handler_inr_c},
                                        {eDCRC, "DCRC", "DCR C", 5, 5, 1, opcode_handler_dcr_c},
                                        {eMVIC, "MVIC", "MVI C", 7, 7, 2, opcode_handler_mvi_c},
                                        {eRRC, "RRC", "RRC", 4, 4, 1, opcode_handler_rrc},

                                        {eNOP2, "NOP2", "NOP", 4, 4, 1, opcode_handler_not_supported},
                                        {eLXID, "LXID", "LXI D", 10, 10, 3, opcode_handler_lxi_d},
                                        {eSTAXD, "STAXD", "STAX D", 7, 7, 1, opcode_handler_stax_d},
                                        {eINXD, "INXD", "INX D", 5, 5, 1, opcode_handler_inx_d},
                                        {eINRD, "INRD", "INR D", 5, 5, 1, opcode_handler_inr_d},
                                        {eDCRD, "DCRD", "DCR D", 5, 5, 1, opcode_handler_dcr_d},
                                        {eMVID, "MVID", "MVI D", 7, 7, 2, opcode_handler_mvi_d},
                                        {eRAL, "RAL", "RAL", 4, 4, 1, opcode_handler_ral},
                                        {eNOP3, "NOP3", "NOP", 4, 4, 1, opcode_handler_not_supported},
                                        {eDADD, "DADD", "DAD D", 10, 10, 1, opcode_handler_dad_d},
                                        {eLDAXD, "LDAXD", "LDAX D", 7, 7, 1, opcode_handler_ldax_d},
                                        {eDCXD, "DCXD", "DCX D", 5, 5, 1, opcode_handler_dcx_d},
                                        {eINRE, "INRE", "INR E", 5, 5, 1, opcode_handler_inr_e},
                                        {eDCRE, "DCRE", "DCR E", 5, 5, 1, opcode_handler_dcr_e},
                                        {eMVIE, "MVIE", "MVI E", 7, 7, 2, opcode_handler_mvi_e},
                                        {eRAR, "RAR", "RAR", 4, 4, 1, opcode_handler_rar},

					{eNOP4, "NOP4", "NOP", 4, 4, 1, opcode_handler_not_supported},
					{eLXISP, "LXIH", "LXI H", 10, 10, 3, opcode_handler_lxi_h},
					{eSHLD, "SHLD", "SHLD", 16, 16, 3, opcode_handler_shld},
					{eINXH, "INXH", "INX H", 5, 5, 1, opcode_handler_inx_h},
					{eINRH, "INRH", "INR H", 5, 5, 1, opcode_handler_inr_h},
					{eDCRH, "DCRH", "DCR H", 5, 5, 1, opcode_handler_dcr_h},
					{eMVIH, "MVIH", "MVI H", 7, 7, 2, opcode_handler_mvi_h},
					{eDAA, "DAA", "DAA", 4, 4, 1, opcode_handler_daa},
					{eNOP5, "NOP5", "NOP", 4, 4, 1, opcode_handler_not_supported},
					{eDADH, "DADH", "DAD H", 10, 10, 1, opcode_handler_dad_h},
					{eLHLD, "LHLD", "LHLD", 16, 16, 3, opcode_handler_lhld},
					{eDCXH, "DCXH", "DCX H", 5, 5, 1, opcode_handler_dcx_h},
					{eINRL, "INRL", "INR L", 5, 5, 1, opcode_handler_inr_l},
					{eDCRL, "DCRL", "DCR L", 5, 5, 1, opcode_handler_dcr_l},
					{eMVIL, "MVIL", "MVI L", 7, 7, 2, opcode_handler_mvi_l},
					{eCMA, "CMA", "CMA", 4, 4, 1, opcode_handler_cma},

					{eNOP6, "NOP6", "NOP", 4, 4, 1, opcode_handler_not_supported},
					{eLXISP, "LXISP", "LXI SP", 10, 10, 3, opcode_handler_lxi_sp},
					{eSTA, "STA", "STA", 13, 13, 3, opcode_handler_sta},
					{eINXSP, "INXSP", "INX SP", 5, 5, 1, opcode_handler_inx_sp},
					{eINRM, "INRM", "INR M", 10, 10, 1, opcode_handler_inr_m},
					{eDCRM, "DCRM", "DCR M", 10, 10, 1, opcode_handler_dcr_m},
					{eMVIM, "MVIM", "MVI M", 10, 10, 2, opcode_handler_mvi_m},
					{eSTC, "STC", "STC", 4, 4, 1, opcode_handler_stc},
					{eNOP7, "NOP7", "NOP", 4, 4, 1, opcode_handler_not_supported},
					{eDADSP, "DADSP", "DAD SP", 10, 10, 1, opcode_handler_dad_sp},
					{eLDA, "LDA", "LDA", 13, 13, 3, opcode_handler_lda},
					{eDCX, "DCXSP", "DCX SP", 5, 5, 1, opcode_handler_dcx_sp},
					{eINRA, "INRA", "INR A", 5, 5, 1, opcode_handler_inr_a},
					{eDCRA, "DCRA", "DCR A", 5, 5, 1, opcode_handler_dcr_a},
					{eMVIA, "MVIA", "MVI A", 7, 7, 2, opcode_handler_mvi_a},
					{eCMC, "CMC", "CMC", 4, 4, 1, opcode_handler_cmc},

					{eMOVBB, "MOVBB", "MOV B,B", 4, 4, 1, opcode_handler_mov_bb},
					{eMOVBC, "MOVBC", "MOV B,C", 4, 4, 1, opcode_handler_mov_bc},
					{eMOVBD, "MOVBD", "MOV B,D", 4, 4, 1, opcode_handler_mov_bd},
					{eMOVBE, "MOVBE", "MOV B,E", 4, 4, 1, opcode_handler_mov_be},
					{eMOVBH, "MOVBH", "MOV B,H", 4, 4, 1, opcode_handler_mov_bh},
					{eMOVBL, "MOVBL", "MOV B,L", 4, 4, 1, opcode_handler_mov_bl},
					{eMOVBM, "MOVBM", "MOV B,M", 7, 7, 1, opcode_handler_mov_bm},
					{eMOVBA, "MOVBA", "MOV B,A", 4, 4, 1, opcode_handler_mov_ba},

					{eMOVCB, "MOVCB", "MOV C,B", 4, 4, 1, opcode_handler_mov_cb},
					{eMOVCC, "MOVCC", "MOV C,C", 4, 4, 1, opcode_handler_mov_cc},
					{eMOVCD, "MOVCD", "MOV C,D", 4, 4, 1, opcode_handler_mov_cd},
					{eMOVCE, "MOVCE", "MOV C,E", 4, 4, 1, opcode_handler_mov_ce},
					{eMOVCH, "MOVCH", "MOV C,H", 4, 4, 1, opcode_handler_mov_ch},
					{eMOVCL, "MOVCL", "MOV C,L", 4, 4, 1, opcode_handler_mov_cl},
					{eMOVCM, "MOVCM", "MOV C,M", 7, 7, 1, opcode_handler_mov_cm},
					{eMOVCA, "MOVCA", "MOV C,A", 4, 4, 1, opcode_handler_mov_ca},

					{eMOVDB, "MOVDB", "MOV D,B", 4, 4, 1, opcode_handler_mov_db},
					{eMOVDC, "MOVDC", "MOV D,C", 4, 4, 1, opcode_handler_mov_dc},
					{eMOVDD, "MOVDD", "MOV D,D", 4, 4, 1, opcode_handler_mov_dd},
					{eMOVDE, "MOVDE", "MOV D,E", 4, 4, 1, opcode_handler_mov_de},
					{eMOVDH, "MOVDH", "MOV D,H", 4, 4, 1, opcode_handler_mov_dh},
					{eMOVDL, "MOVDL", "MOV D,L", 4, 4, 1, opcode_handler_mov_dl},
					{eMOVDM, "MOVDM", "MOV D,M", 7, 7, 1, opcode_handler_mov_dm},
					{eMOVDA, "MOVDA", "MOV D,A", 4, 4, 1, opcode_handler_mov_da},

					{eMOVEB, "MOVEB", "MOV E,B", 4, 4, 1, opcode_handler_mov_eb},
					{eMOVEC, "MOVEC", "MOV E,C", 4, 4, 1, opcode_handler_mov_ec},
					{eMOVED, "MOVED", "MOV E,D", 4, 4, 1, opcode_handler_mov_ed},
					{eMOVEE, "MOVEE", "MOV E,E", 4, 4, 1, opcode_handler_mov_ee},
					{eMOVEH, "MOVEH", "MOV E,H", 4, 4, 1, opcode_handler_mov_eh},
					{eMOVEL, "MOVEL", "MOV E,L", 4, 4, 1, opcode_handler_mov_el},
					{eMOVEM, "MOVEM", "MOV E,M", 7, 7, 1, opcode_handler_mov_em},
					{eMOVEA, "MOVEA", "MOV E,A", 4, 4, 1, opcode_handler_mov_ea},

					{eMOVHB, "MOVHB", "MOV H,B", 4, 4, 1, opcode_handler_mov_hb},
					{eMOVHC, "MOVHC", "MOV H,C", 4, 4, 1, opcode_handler_mov_hc},
					{eMOVHD, "MOVHD", "MOV H,D", 4, 4, 1, opcode_handler_mov_hd},
					{eMOVHE, "MOVHE", "MOV H,E", 4, 4, 1, opcode_handler_mov_he},
					{eMOVHH, "MOVHH", "MOV H,H", 4, 4, 1, opcode_handler_mov_hh},
					{eMOVHL, "MOVHL", "MOV H,L", 4, 4, 1, opcode_handler_mov_hl},
					{eMOVHM, "MOVHM", "MOV H,M", 7, 7, 1, opcode_handler_mov_hm},
					{eMOVHA, "MOVHA", "MOV H,A", 4, 4, 1, opcode_handler_mov_ha},

					{eMOVLB, "MOVLB", "MOV L,B", 4, 4, 1, opcode_handler_mov_lb},
					{eMOVLC, "MOVLC", "MOV L,C", 4, 4, 1, opcode_handler_mov_lc},
					{eMOVLD, "MOVLD", "MOV L,D", 4, 4, 1, opcode_handler_mov_ld},
					{eMOVLE, "MOVLE", "MOV L,E", 4, 4, 1, opcode_handler_mov_le},
					{eMOVLH, "MOVLH", "MOV L,H", 4, 4, 1, opcode_handler_mov_lh},
					{eMOVLL, "MOVLL", "MOV L,L", 4, 4, 1, opcode_handler_mov_ll},
					{eMOVLM, "MOVLM", "MOV L,M", 7, 7, 1, opcode_handler_mov_lm},
					{eMOVLA, "MOVLA", "MOV L,A", 4, 4, 1, opcode_handler_mov_la},

					{eMOVMB, "MOVMB", "MOV M,B", 7, 7, 1, opcode_handler_mov_mb},
					{eMOVMC, "MOVMC", "MOV M,C", 7, 7, 1, opcode_handler_mov_mc},
					{eMOVMD, "MOVMD", "MOV M,D", 7, 7, 1, opcode_handler_mov_md},
					{eMOVME, "MOVME", "MOV M,E", 7, 7, 1, opcode_handler_mov_me},
					{eMOVMH, "MOVMH", "MOV M,H", 7, 7, 1, opcode_handler_mov_mh},
					{eMOVML, "MOVML", "MOV M,L", 7, 7, 1, opcode_handler_mov_ml},
					{eHLT,   "HLT",    "HLT",    7, 7, 1, opcode_handler_hlt},
					{eMOVMA, "MOVMA", "MOV M,A", 7, 7, 1, opcode_handler_mov_ma},

					{eMOVAB, "MOVAB", "MOV A,B", 4, 4, 1, opcode_handler_mov_ab},
					{eMOVAC, "MOVAC", "MOV A,C", 4, 4, 1, opcode_handler_mov_ac},
					{eMOVAD, "MOVAD", "MOV A,D", 4, 4, 1, opcode_handler_mov_ad},
					{eMOVAE, "MOVAE", "MOV A,E", 4, 4, 1, opcode_handler_mov_ae},
					{eMOVAH, "MOVAH", "MOV A,H", 4, 4, 1, opcode_handler_mov_ah},
					{eMOVAL, "MOVAL", "MOV A,L", 4, 4, 1, opcode_handler_mov_al},
					{eMOVAM, "MOVAM", "MOV A,M", 7, 7, 1, opcode_handler_mov_am},
					{eMOVAA, "MOVAA", "MOV A,A", 4, 4, 1, opcode_handler_mov_aa},

					{eADDB, "ADDB", "ADD B", 4, 4, 1, opcode_handler_add_b},
					{eADDC, "ADDC", "ADD C", 4, 4, 1, opcode_handler_add_c},
					{eADDD, "ADDD", "ADD D", 4, 4, 1, opcode_handler_add_d},
					{eADDE, "ADDE", "ADD E", 4, 4, 1, opcode_handler_add_e},
					{eADDH, "ADDH", "ADD H", 4, 4, 1, opcode_handler_add_h},
					{eADDL, "ADDL", "ADD L", 4, 4, 1, opcode_handler_add_l},
					{eADDM, "ADDM", "ADD M", 7, 7, 1, opcode_handler_add_m},
					{eADDA, "ADDA", "ADD A", 4, 4, 1, opcode_handler_add_a},

					{eADCB, "ADCB", "ADC B", 4, 4, 1, opcode_handler_adc_b},
					{eADCC, "ADCC", "ADC C", 4, 4, 1, opcode_handler_adc_c},
					{eADCD, "ADCD", "ADC D", 4, 4, 1, opcode_handler_adc_d},
					{eADCE, "ADCE", "ADC E", 4, 4, 1, opcode_handler_adc_e},
					{eADCH, "ADCH", "ADC H", 4, 4, 1, opcode_handler_adc_h},
					{eADCL, "ADCL", "ADC L", 4, 4, 1, opcode_handler_adc_l},
					{eADCM, "ADCM", "ADC M", 7, 7, 1, opcode_handler_adc_m},
					{eADCA, "ADCA", "ADC A", 4, 4, 1, opcode_handler_adc_a},

					{eSUBB, "SUBB", "SUB B", 4, 4, 1, opcode_handler_sub_b},					
					{eSUBC, "SUBC", "SUB C", 4, 4, 1, opcode_handler_sub_c},					
					{eSUBD, "SUBD", "SUB D", 4, 4, 1, opcode_handler_sub_d},					
					{eSUBE, "SUBE", "SUB E", 4, 4, 1, opcode_handler_sub_e},					
					{eSUBH, "SUBH", "SUB H", 4, 4, 1, opcode_handler_sub_h},					
					{eSUBL, "SUBL", "SUB L", 4, 4, 1, opcode_handler_sub_l},					
					{eSUBM, "SUBM", "SUB M", 7, 7, 1, opcode_handler_sub_m},
					{eSUBA, "SUBA", "SUB A", 4, 4, 1, opcode_handler_sub_a},

					{eSBBB, "SBBB", "SBB B", 4, 4, 1, opcode_handler_sbb_b},					
					{eSBBC, "SBBC", "SBB C", 4, 4, 1, opcode_handler_sbb_c},					
					{eSBBD, "SBBD", "SBB D", 4, 4, 1, opcode_handler_sbb_d},					
					{eSBBE, "SBBE", "SBB E", 4, 4, 1, opcode_handler_sbb_e},					
					{eSBBH, "SBBH", "SBB H", 4, 4, 1, opcode_handler_sbb_h},					
					{eSBBL, "SBBL", "SBB L", 4, 4, 1, opcode_handler_sbb_l},					
					{eSBBM, "SBBM", "SBB M", 7, 7, 1, opcode_handler_sbb_m},					
					{eSBBA, "SBBA", "SBB A", 4, 4, 1, opcode_handler_sbb_a},
					
					{eANAB, "ANAB", "ANA B", 4, 4, 1, opcode_handler_ana_b},					
					{eANAC, "ANAC", "ANA C", 4, 4, 1, opcode_handler_ana_c},					
					{eANAD, "ANAD", "ANA D", 4, 4, 1, opcode_handler_ana_d},					
					{eANAE, "ANAE", "ANA E", 4, 4, 1, opcode_handler_ana_e},					
					{eANAH, "ANAH", "ANA H", 4, 4, 1, opcode_handler_ana_h},					
					{eANAL, "ANAL", "ANA L", 4, 4, 1, opcode_handler_ana_l},					
					{eANAM, "ANAM", "ANA M", 7, 7, 1, opcode_handler_ana_m},					
					{eANAA, "ANAA", "ANA A", 4, 4, 1, opcode_handler_ana_a},
					
					{eXRAB, "XRAB", "XRA B", 4, 4, 1, opcode_handler_xra_b},					
					{eXRAC, "XRAC", "XRA C", 4, 4, 1, opcode_handler_xra_c},					
					{eXRAD, "XRAD", "XRA D", 4, 4, 1, opcode_handler_xra_d},					
					{eXRAE, "XRAE", "XRA E", 4, 4, 1, opcode_handler_xra_e},					
					{eXRAH, "XRAH", "XRA H", 4, 4, 1, opcode_handler_xra_h},					
					{eXRAL, "XRAL", "XRA L", 4, 4, 1, opcode_handler_xra_l},					
					{eXRAM, "XRAM", "XRA M", 7, 7, 1, opcode_handler_xra_m},					
					{eXRAA, "XRAA", "XRA A", 4, 4, 1, opcode_handler_xra_a},
					
					{eORAB, "ORAB", "ORA B", 4, 4, 1, opcode_handler_ora_b},					
					{eORAC, "ORAC", "ORA C", 4, 4, 1, opcode_handler_ora_c},					
					{eORAD, "ORAD", "ORA D", 4, 4, 1, opcode_handler_ora_d},					
					{eORAE, "ORAE", "ORA E", 4, 4, 1, opcode_handler_ora_e},					
					{eORAH, "ORAH", "ORA H", 4, 4, 1, opcode_handler_ora_h},					
					{eORAL, "ORAL", "ORA L", 4, 4, 1, opcode_handler_ora_l},					
					{eORAM, "ORAM", "ORA M", 7, 7, 1, opcode_handler_ora_m},					
					{eORAA, "ORAA", "ORA A", 4, 4, 1, opcode_handler_ora_a},

					{eCMPB, "CMPB", "CMP B", 4, 4, 1, opcode_handler_cmp_b},					
					{eCMPC, "CMPC", "CMP C", 4, 4, 1, opcode_handler_cmp_c},					
					{eCMPD, "CMPD", "CMP D", 4, 4, 1, opcode_handler_cmp_d},					
					{eCMPE, "CMPE", "CMP E", 4, 4, 1, opcode_handler_cmp_e},					
					{eCMPH, "CMPH", "CMP H", 4, 4, 1, opcode_handler_cmp_h},					
					{eCMPL, "CMPL", "CMP L", 4, 4, 1, opcode_handler_cmp_l},					
					{eCMPM, "CMPM", "CMP M", 7, 7, 1, opcode_handler_cmp_m},					
					{eCMPA, "CMPA", "CMP A", 4, 4, 1, opcode_handler_cmp_a},

					{eRNZ, "RNZ", "RNZ", 5, 11, 1, opcode_handler_rnz},
					{ePOPB, "POPB", "POP B", 10, 10, 1, opcode_handler_pop_b},
					{eJNZ, "JNZ", "JNZ", 10, 10, 3, opcode_handler_jnz},
					{eJMP, "JMP", "JMP", 10, 10, 3, opcode_handler_jmp},
					{eCNZ, "CNZ", "CNZ", 11, 17, 3, opcode_handler_cnz},
					{ePUSHB, "PUSHB", "PUSH B", 11, 11, 1, opcode_handler_push_b},
					{eADI, "ADI", "ADI", 7, 7, 2, opcode_handler_adi},
					{eRST_0, "RST0", "RST 0", 11, 11, 1, opcode_handler_rst0},
					{eRZ, "RZ", "RZ", 5, 11, 1, opcode_handler_rz},
					{eRET, "RET", "RET", 10, 10, 1, opcode_handler_ret},
					{eJZ, "JZ", "JZ", 10, 10, 3, opcode_handler_jz},
					{eJMP, "JMP", "JMP", 10, 10, 3, opcode_handler_not_supported},
					{eCZ, "CZ", "CZ", 11, 17, 3, opcode_handler_cz},
					{eCALL, "CALL", "CALL", 17, 17, 3, opcode_handler_call},
					{eACI, "ACI", "ACI", 7, 7, 2, opcode_handler_aci},
					{eRST_1, "RST1", "RST1", 11, 11, 1, opcode_handler_rst1},

					{eRNC, "RNC", "RNC", 5, 11, 1, opcode_handler_rnc},
					{ePOPD, "POPD", "POPD", 10, 10, 1, opcode_handler_pop_d},
					{eJNC, "JNC", "JNC", 10, 10, 3, opcode_handler_jnc},
					{eOUT, "OUT", "OUT", 10, 10, 2, opcode_handler_out},
					{eCNC, "CNC", "CNC", 11, 17, 3, opcode_handler_cnc},
					{ePUSHD, "PUSHD", "PUSH D", 11, 11, 1, opcode_handler_push_d},
					{eSUI, "SUI", "SUI", 7, 7, 2, opcode_handler_sui},
					{eRST_2, "RST2", "RST2", 11, 11, 1, opcode_handler_rst2},
					{eRC, "RC", "RC", 5, 11, 1, opcode_handler_rc},
					{eRET1, "RET", "RET", 10, 10, 1, opcode_handler_not_supported},
					{eJC, "JC", "JC", 10, 10, 3, opcode_handler_jc},
					{eIN, "IN", "IN", 10, 10, 2, opcode_handler_in},
					{eCC, "CC", "CC", 11, 17, 3, opcode_handler_cc},
					{eCALL1, "CALL1", "CALL", 17, 17, 3, opcode_handler_not_supported},
					{eSBI, "SBI", "SBI", 7, 7, 2, opcode_handler_sbi},
					{eRST_3, "RST3", "RST3", 11, 11, 1, opcode_handler_rst3},

					{eRPO, "RPO", "RPO", 5, 11, 1, opcode_handler_rpo},
					{ePOPH, "POPH", "POP H", 10, 10, 1, opcode_handler_pop_h},
					{eJPO, "JPO", "JPO", 10, 10, 3, opcode_handler_jpo},
					{eXTHL, "XTHL", "XTHL", 18, 18, 1, opcode_handler_xthl},
					{eCPO, "CPO", "CPO", 11, 17, 3, opcode_handler_cpo},
					{ePUSHH, "PUSHH", "PUSH H", 11, 11, 1, opcode_handler_push_h},
					{eANI, "ANI", "ANI", 7, 7, 2, opcode_handler_ani},
					{eRST_4, "RST4", "RST4", 11, 11, 1, opcode_handler_rst4},
					{eRPE, "RPE", "RPE", 5, 11, 1, opcode_handler_rpe},
					{ePCHL, "PCHL", "PCHL", 5, 5, 1, opcode_handler_pchl},
					{eJPE, "JPE", "JPE", 10, 10, 3, opcode_handler_jpe},
					{eXCHG, "XCHG", "XCHG", 5, 5, 1, opcode_handler_xchg},
					{eCPE, "CPE", "CPE", 11, 17, 3, opcode_handler_cpe},
					{eCALL2, "CALL2", "CALL", 17, 17, 3, opcode_handler_not_supported},
					{eXRI, "XRI", "XRI", 7, 7, 2, opcode_handler_xri},
					{eRST_5, "RST5", "RST5", 11, 11, 1, opcode_handler_rst5},

					{eRP, "RP", "RP", 5, 11, 1, opcode_handler_rp},
					{ePOPPSW, "POPPSW", "POP PSW", 10, 10, 1, opcode_handler_pop_psw},
					{eJP, "JP", "JP", 10, 10, 3, opcode_handler_jp},
					{eDI, "DI", "DI", 4, 4, 1, opcode_handler_di},
					{eCP, "CP", "CP", 11, 17, 3, opcode_handler_cp},
					{ePUSHPSW, "PUSHPSW", "PUSH PSW", 11, 11, 1, opcode_handler_push_psw},
					{eORI, "ORI", "ORI", 7, 7, 2, opcode_handler_ori},
					{eRST_6, "RST6", "RST6", 11, 11, 1, opcode_handler_rst6},
					{eRM, "RM", "RM", 5, 11, 1, opcode_handler_rm},
					{eSPHL, "SPHL", "SPHL", 5, 5, 1, opcode_handler_sphl},
					{eJM, "JM", "JM", 10, 10, 3, opcode_handler_jm},
					{eEI, "EI", "EI", 4, 4, 1, opcode_handler_ei},
					{eCM, "CM", "CM", 11, 17, 3, opcode_handler_cm},
					{eCALL3, "CALL3", "CALL", 17, 17, 3, opcode_handler_not_supported},
					{eCPI, "CPI", "CPI", 7, 7, 2, opcode_handler_cpi},
					{eRST_7, "RST7", "RST7", 11, 11, 1, opcode_handler_rst7}
};

static void set_c_flag(cpu_model_t *cpu, uint8_t val, uint8_t temp_val)
{
	uint16_t prod = val + temp_val;
	cpu->core.status.bits.c = (0x0100 & prod) ? 1 : 0;
}

static void set_ac_flag(cpu_model_t *cpu, uint8_t val, uint8_t temp_val)
{
	uint8_t prod = (val & 0x0F) + (temp_val & 0x0F);
	cpu->core.status.bits.ac = (0x10 & prod) ? 1 : 0;
}

static void set_p_flag(cpu_model_t* cpu, uint8_t val)
{
	uint8_t cnt = 0;
	for(int i = 0 ; i < 8 ; i++) {
		uint8_t bit = ((val >> i) & 0x01);
		cnt += (bit == 1) ? 1 : 0;
	}

	cpu->core.status.bits.p = (cnt & 0x01) ? 0 : 1; 
}

static void set_z_flag(cpu_model_t* cpu, uint8_t val)
{
	cpu->core.status.bits.z = (val == 0) ? 1 : 0;
}

static void set_s_flag(cpu_model_t* cpu, uint8_t val)
{
	cpu->core.status.bits.s = (val & 0x80) ? 1 : 0;
}

static void store_u8_into_mem(uint16_t addr, uint8_t val, memory_t *mem)
{
	mem->memory[addr] = val;
}

static void store_u16_into_mem(uint16_t addr, uint16_t val, memory_t *mem)
{
	uint8_t hval = (uint8_t) ((val >> 8) & 0xFF);
	uint8_t lval = (uint8_t) (val & 0xFF);

	mem->memory[addr];
	mem->memory[addr+1];
}


static uint8_t retrieve_u8_from_mem(uint16_t addr, memory_t *mem)
{
	return mem->memory[addr];
}

static uint16_t retrieve_u16_from_mem(uint16_t addr, memory_t *mem)
{
	uint8_t lbit = mem->memory[addr];
	uint8_t hbit = mem->memory[addr+1];

	return ((hbit << 8) | lbit);
}

static uint8_t pop_u8_from_stack(memory_t *mem, cpu_model_t *cpu)
{
	return retrieve_u8_from_mem((cpu->core.stack)++, mem);
}

static void push_u8_to_stack(uint8_t val, memory_t *mem, cpu_model_t *cpu)
{
	store_u8_into_mem(--(cpu->core.stack), val, mem);
}

static void push_u16_to_stack(uint16_t val, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t hval = (uint8_t) ((val >> 8) & 0xFF);
	uint8_t lval = (uint8_t) (val & 0xFF);

	push_u8_to_stack(hval, mem, cpu);
	push_u8_to_stack(lval, mem, cpu);
}

static uint16_t pop_u16_from_stack(memory_t *mem, cpu_model_t *cpu)
{
	uint8_t lval = pop_u8_from_stack(mem, cpu);
	uint8_t hval = pop_u8_from_stack(mem, cpu);

	return ((uint16_t) ((hval << 8) | lval));
}

static void add_instr(cpu_model_t *cpu, uint8_t val)
{
	set_c_flag(cpu, cpu->core.a, val);
	set_ac_flag(cpu, cpu->core.a, val);

	cpu->core.a += val;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void adc_instr(cpu_model_t *cpu, uint8_t val)
{
	uint8_t cbit = cpu->core.status.bits.c;

	set_c_flag(cpu, cpu->core.a, (val + cbit));
	set_ac_flag(cpu, cpu->core.a, (val + cbit));

	cpu->core.a += (val + cbit);

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void sub_instr(cpu_model_t *cpu, uint8_t val)
{
	uint8_t two_qu = ~val + 1;

	cpu->core.status.bits.c = (cpu->core.a < val) ? 1 : 0;
	set_ac_flag(cpu, cpu->core.a, val);

	cpu->core.a += two_qu;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void sbb_instr(cpu_model_t *cpu, uint8_t val)
{
	uint8_t two_qu = ~(val + cpu->core.status.bits.c) + 1;

	cpu->core.status.bits.c = (cpu->core.a < val) ? 1 : 0;
	set_ac_flag(cpu, cpu->core.a, val);

	cpu->core.a += two_qu;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void ana_instr(cpu_model_t *cpu, uint8_t val)
{
	cpu->core.a &= val;

	cpu->core.status.bits.c = 0;
	cpu->core.status.bits.ac = 0;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void xra_instr(cpu_model_t *cpu, uint8_t val)
{
	cpu->core.a ^= val;

	cpu->core.status.bits.c = 0;
	cpu->core.status.bits.ac = 0;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void ora_instr(cpu_model_t *cpu, uint8_t val)
{
	cpu->core.a |= val;

	cpu->core.status.bits.c = 0;
	cpu->core.status.bits.ac = 0;

	set_p_flag(cpu, cpu->core.a);
	set_z_flag(cpu, cpu->core.a);
	set_s_flag(cpu, cpu->core.a);
}

static void cmp_instr(cpu_model_t *cpu, uint8_t val)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t two_qu = ~val + 1;

	cpu->core.status.bits.c = (reg_a < val) ? 1 : 0;
	set_ac_flag(cpu, reg_a, val);

	reg_a += two_qu;

	set_p_flag(cpu, reg_a);
	set_z_flag(cpu, reg_a);
	set_s_flag(cpu, reg_a);
} 

static uint8_t opcode_handler_not_supported(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	return 0;
}

static uint8_t opcode_handler_mvi_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.b = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.c = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.d = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.e = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.h = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.l = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	store_u8_into_mem(addr, val, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mvi_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t data = retrieve_u8_from_mem(cpu->core.pc + 1, mem);
	cpu->core.a = data; 

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ldax_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.b << 8) | cpu->core.c;

	cpu->core.a = retrieve_u8_from_mem(addr, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ldax_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.d << 8) | cpu->core.e;

	cpu->core.a = retrieve_u8_from_mem(addr, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_stax_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.b << 8) | cpu->core.c;

	store_u8_into_mem(addr, cpu->core.a, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_stax_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.d << 8) | cpu->core.e;

	store_u8_into_mem(addr, cpu->core.a, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_nop(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xthl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t stack_addr = cpu->core.stack;
	uint8_t lbit = retrieve_u8_from_mem(stack_addr, mem);
	uint8_t hbit = retrieve_u8_from_mem(stack_addr+1, mem);

	store_u8_into_mem(stack_addr, cpu->core.l, mem);
	store_u8_into_mem(stack_addr+1, cpu->core.h, mem);

	cpu->core.l = lbit;
	cpu->core.h = hbit;
	
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_push_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u8_to_stack(cpu->core.b, mem, cpu);
	push_u8_to_stack(cpu->core.c, mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_push_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u8_to_stack(cpu->core.d, mem, cpu);
	push_u8_to_stack(cpu->core.e, mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_push_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u8_to_stack(cpu->core.h, mem, cpu);
	push_u8_to_stack(cpu->core.l, mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_push_psw(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t res = 0x00;

	res |= cpu->core.status.bits.c << 0;
	res |= 0x1 << 1;
	res |= cpu->core.status.bits.p << 2;
	res |= cpu->core.status.bits.ac << 4;
	res |= cpu->core.status.bits.z << 6;
	res |= cpu->core.status.bits.s << 7;

	push_u8_to_stack(cpu->core.a, mem, cpu);
	push_u8_to_stack(res, mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_pop_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = pop_u8_from_stack(mem, cpu);
	cpu->core.b = pop_u8_from_stack(mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_pop_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = pop_u8_from_stack(mem, cpu);
	cpu->core.d = pop_u8_from_stack(mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_pop_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = pop_u8_from_stack(mem, cpu);
	cpu->core.h = pop_u8_from_stack(mem, cpu);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_pop_psw(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg = pop_u8_from_stack(mem, cpu);
	cpu->core.a = pop_u8_from_stack(mem, cpu);

	cpu->core.status.bits.c = (reg >> 0) & 0x01;
	cpu->core.status.bits.p = (reg >> 2) & 0x01;
	cpu->core.status.bits.ac = (reg >> 4) & 0x01;
	cpu->core.status.bits.z = (reg >> 6) & 0x01;
	cpu->core.status.bits.s = (reg >> 7) & 0x01;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xchg(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t d_reg_temp = cpu->core.d;
	uint8_t e_reg_temp = cpu->core.e;

	cpu->core.d = cpu->core.h;
	cpu->core.e = cpu->core.l;

	cpu->core.h = d_reg_temp;
	cpu->core.l = e_reg_temp;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sphl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = retrieve_u16_from_mem(cpu->core.pc+1, mem);

	cpu->core.l = retrieve_u8_from_mem(addr, mem);
	cpu->core.h = retrieve_u8_from_mem(addr+1, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dad_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t bc_val = (cpu->core.b << 8) | cpu->core.c;
	uint16_t hl_val = (cpu->core.h << 8) | cpu->core.l;

	uint32_t prod = hl_val + bc_val;

	cpu->core.status.bits.c = ((prod & 0x00010000) != 0);

	cpu->core.h = (uint8_t) ((prod >> 8) & 0xFF);
	cpu->core.l = (uint8_t) (prod & 0xFF);

	cpu->core.pc += opcode->opcode_size;

	return 0;

}

static uint8_t opcode_handler_dad_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t de_val = (cpu->core.d << 8) | cpu->core.e;
	uint16_t hl_val = (cpu->core.h << 8) | cpu->core.l;

	uint32_t prod = hl_val + de_val;

	cpu->core.status.bits.c = ((prod & 0x00010000) != 0);

	cpu->core.h = (uint8_t) ((prod >> 8) & 0xFF);
	cpu->core.l = (uint8_t) (prod & 0xFF);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dad_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t hl_val = (cpu->core.h << 8) | cpu->core.l;
	uint32_t prod = hl_val << 1;

	cpu->core.status.bits.c = ((prod & 0x00010000) != 0);

	cpu->core.h = (uint8_t) ((prod >> 8) & 0xFF);
	cpu->core.l = (uint8_t) (prod & 0xFF);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dad_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t hl_val = (cpu->core.h << 8) | cpu->core.l;

	uint32_t prod = hl_val + cpu->core.stack;

	cpu->core.status.bits.c = ((prod & 0x00010000) != 0);

	cpu->core.h = (uint8_t) ((prod >> 8) & 0xFF);
	cpu->core.l = (uint8_t) (prod & 0xFF);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cma(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = ~(cpu->core.a);

	cpu->core.pc += opcode->opcode_size;

	return 0; 
}

static void dcx_instr(uint8_t *hbit, uint8_t *lbit)
{
	uint16_t dcx_reg = (*hbit << 8) | *lbit;

	dcx_reg--;

	*hbit = (uint8_t) (dcx_reg >> 8) & 0xFF;
	*lbit = (uint8_t) (dcx_reg & 0xFF);
}

static uint8_t opcode_handler_dcx_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcx_instr(&(cpu->core.b), &(cpu->core.c));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcx_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcx_instr(&(cpu->core.d), &(cpu->core.e));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcx_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcx_instr(&(cpu->core.h), &(cpu->core.l));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcx_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.stack--;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static void inx_instr(uint8_t *hbit, uint8_t *lbit)
{
	uint16_t inx_reg = (*hbit << 8) | *lbit;

	inx_reg++;

	*hbit = (uint8_t) (inx_reg >> 8) & 0xFF;
	*lbit = (uint8_t) (inx_reg & 0xFF);
}

static uint8_t opcode_handler_inx_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inx_instr(&(cpu->core.b), &(cpu->core.c));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inx_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inx_instr(&(cpu->core.d), &(cpu->core.e));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inx_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inx_instr(&(cpu->core.h), &(cpu->core.l));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inx_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.stack++;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static void inr_instr(cpu_model_t* cpu, uint8_t *val)
{
	uint8_t temp_val = *val;

	(*val)++;

	set_c_flag(cpu, *val, temp_val);
	set_ac_flag(cpu, *val, temp_val);

	set_p_flag(cpu, *val);
	set_z_flag(cpu, *val);
	set_s_flag(cpu, *val);
}

static uint8_t opcode_handler_inr_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.b);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.c);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.d);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.e);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.h);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.l);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	inr_instr(cpu, &cpu->core.a);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_inr_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t mem_val = retrieve_u8_from_mem(addr, mem);

	inr_instr(cpu, &mem_val);

	store_u8_into_mem(addr, mem_val, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static void dcr_instr(cpu_model_t* cpu, uint8_t *val)
{
	uint8_t temp_val = *val;

	(*val)--;

	set_c_flag(cpu, *val, temp_val);
	set_ac_flag(cpu, *val, temp_val);

	set_p_flag(cpu, *val);
	set_z_flag(cpu, *val);
	set_s_flag(cpu, *val);
}

static uint8_t opcode_handler_dcr_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.b));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.c));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.d));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.e));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.h));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.l));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	dcr_instr(cpu, &val);

	store_u8_into_mem(addr, val, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_dcr_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	dcr_instr(cpu, &(cpu->core.a));

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_rlc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t hbit = ((reg_a & 0x80) != 0);

	reg_a <<= 1;
	reg_a |= hbit;

	cpu->core.status.bits.c = hbit;
	cpu->core.a = reg_a;
	
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ral(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t hbit = ((reg_a & 0x80) != 0);
	uint8_t cbit = cpu->core.status.bits.c;

	reg_a <<= 1;
	reg_a |= cbit;

	cpu->core.status.bits.c = hbit;
	cpu->core.a = reg_a;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_rar(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t lbit = ((reg_a & 0x01) != 0);

	reg_a >>= 1;
	reg_a |= (lbit << 7);

	cpu->core.status.bits.c = lbit;
	cpu->core.a = reg_a;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_rrc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t lbit = ((reg_a & 0x01) != 0);

	reg_a >>= 1;
	reg_a |= (lbit << 7);

	cpu->core.status.bits.c = lbit;
	cpu->core.a = reg_a;
	
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.status.bits.c = (cpu->core.status.bits.c == 0) ? 1 : 0;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_stc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.status.bits.c = 1;

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_bb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_bc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_bd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_be(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_bh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_bl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;

}

static uint8_t opcode_handler_mov_bm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.b = mem->memory[addr];

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ba(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.b = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_cb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_cc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_cd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ce(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ch(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_cl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_cm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.c = mem->memory[addr];
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ca(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_db(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_dc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_dd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_de(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_dh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_dl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_dm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.d = mem->memory[addr];
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_da(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.d = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_eb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ec(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ed(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ee(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_eh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_el(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_em(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.e = mem->memory[addr];
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ea(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hd(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_he(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_hm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.h = mem->memory[addr];
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ha(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.h = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_lb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_lc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_le(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_lh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ll(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_lm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.l = mem->memory[addr];
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_la(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_mb(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_mc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_md(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_me(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_mh(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ml(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_hlt(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->is_running = false;

	return cpu->core.pc += opcode->opcode_size;
}

static uint8_t opcode_handler_mov_ma(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;

	mem->memory[addr] = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ab(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.b;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ac(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.c;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ad(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.d;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ae(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.e;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_ah(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.h;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_al(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.l;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_am(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	cpu->core.a = mem->memory[addr];

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_mov_aa(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.a = cpu->core.a;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	add_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_add_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	add_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	adc_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adc_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	adc_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sub_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	sub_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sub_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbb_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	sbb_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;}

static uint8_t opcode_handler_sbb_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	sbb_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	ana_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ana_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ana_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	xra_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xra_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	xra_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	ora_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ora_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	ora_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.b);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_c(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.c);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.d);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_e(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.e);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.h);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_l(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.l);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_m(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = (cpu->core.h << 8) | cpu->core.l;
	uint8_t val = retrieve_u8_from_mem(addr, mem);

	cmp_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cmp_a(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cmp_instr(cpu, cpu->core.a);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_adi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	add_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_aci(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	adc_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sui(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	sub_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sbi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	sbb_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ani(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	ana_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_cpi(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	cmp_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_xri(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	xra_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ori(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t val = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	
	ora_instr(cpu, val);
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ret(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.pc = pop_u16_from_stack(mem, cpu);

	return 0;
}

static uint8_t opcode_handler_rpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 1)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 0)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 1)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 0)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 1)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 0)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 1)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 0)	{
		cpu->core.pc = pop_u16_from_stack(mem, cpu);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_pchl(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t pc_counter = (cpu->core.h << 8) | cpu->core.l;

	cpu->core.pc = pc_counter;

	return 0;
}

static uint8_t opcode_handler_jmp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);

	return 0;
}

static uint8_t opcode_handler_jpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 0)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_jpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 1)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}


static uint8_t opcode_handler_jnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 0)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_jz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 1)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}
static uint8_t opcode_handler_jnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 0)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_jc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 1)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_jp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 0)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_jm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 1)	{
		cpu->core.pc = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	}
	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_call(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t pc_counter = retrieve_u16_from_mem(cpu->core.pc + 1, mem);
	uint16_t pc_store = cpu->core.pc + opcode->opcode_size;

	push_u16_to_stack(pc_store, mem, cpu);

	cpu->core.pc = pc_counter;
	return 0;
}

static uint8_t opcode_handler_cc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 1)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cnc(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.c == 0)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 1)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cnz(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.z == 0)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 0)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cm(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.s == 1)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}


static uint8_t opcode_handler_cpe(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 1)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_cpo(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	if (cpu->core.status.bits.p == 0)	{
		opcode_handler_call(opcode, mem, cpu);
	}

	else {
		cpu->core.pc += opcode->opcode_size;
	}

	return 0;
}

static uint8_t opcode_handler_rst0(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc, mem, cpu);

	cpu->core.pc = 0 * 8;
	return 0;
}

static uint8_t opcode_handler_rst1(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc, mem, cpu);

	cpu->core.pc = 1 * 8;

	return 0;
}

static uint8_t opcode_handler_rst2(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc, mem, cpu);

	cpu->core.pc = 2 * 8;

	return 0;
}

static uint8_t opcode_handler_rst3(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc + 1, mem, cpu);

	cpu->core.pc = 3 * 8;

	return 0;
}

static uint8_t opcode_handler_rst4(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc + 1, mem, cpu);

	cpu->core.pc = 4 * 8;

	return 0;
}

static uint8_t opcode_handler_rst5(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc + 1, mem, cpu);

	cpu->core.pc = 5 * 8;

	return 0;
}

static uint8_t opcode_handler_rst6(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc + 1, mem, cpu);

	cpu->core.pc = 6 * 8;

	return 0;
}

static uint8_t opcode_handler_rst7(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	push_u16_to_stack(cpu->core.pc + 1, mem, cpu);

	cpu->core.pc = 7 * 8;

	return 0;
}

static uint8_t opcode_handler_di(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->interrupt_enable = false;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_ei(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->interrupt_enable = true;
	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_lda(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = retrieve_u16_from_mem(cpu->core.pc+1, mem);
	cpu->core.a = retrieve_u8_from_mem(addr, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_sta(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = retrieve_u16_from_mem(cpu->core.pc+1, mem);

	store_u8_into_mem(addr, cpu->core.a, mem);

	cpu->core.pc += opcode->opcode_size;
	
	return 0;
}

static uint8_t opcode_handler_lhld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = retrieve_u16_from_mem(cpu->core.pc+1, mem);

	cpu->core.l = retrieve_u8_from_mem(addr, mem);
	cpu->core.h = retrieve_u8_from_mem(addr+1, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_shld(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint16_t addr = retrieve_u16_from_mem(cpu->core.pc+1, mem);

	store_u8_into_mem(addr, cpu->core.l, mem);
	store_u8_into_mem(addr+1, cpu->core.h, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_daa(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t reg_a = cpu->core.a;
	uint8_t lb_acc = reg_a & 0x0F;

	if (lb_acc > 9 || cpu->core.status.bits.ac) {
		set_ac_flag(cpu, cpu->core.a, 6);
		cpu->core.a += 6;
	}

	reg_a = cpu->core.a;
	uint8_t hb_acc = (reg_a >> 4) & 0x0F;

	if(hb_acc > 9 || cpu->core.status.bits.c) {
		hb_acc += 6;
		cpu->core.status.bits.c = (hb_acc & 0x10) ? 1 : 0;
		cpu->core.a = (hb_acc << 4) | (cpu->core.a & 0x0F); 
	}

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_lxi_b(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.c = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	cpu->core.b = retrieve_u8_from_mem(cpu->core.pc+2, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_lxi_d(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.e = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	cpu->core.d = retrieve_u8_from_mem(cpu->core.pc+2, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_lxi_h(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.l = retrieve_u8_from_mem(cpu->core.pc+1, mem);
	cpu->core.h = retrieve_u8_from_mem(cpu->core.pc+2, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_lxi_sp(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	cpu->core.stack = retrieve_u16_from_mem(cpu->core.pc+1, mem);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_out(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t port = retrieve_u8_from_mem(cpu->core.pc+1, mem);

	cpu->cb_out(port, cpu->core.a);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

static uint8_t opcode_handler_in(opcode_instr_t *opcode, memory_t *mem, cpu_model_t *cpu)
{
	uint8_t port = retrieve_u8_from_mem(cpu->core.pc+1, mem);

	cpu->core.a = cpu->cb_in(port);

	cpu->core.pc += opcode->opcode_size;

	return 0;
}

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

void display_curr_cpu_8080_status(memory_t *mem, cpu_core_t* core)
{

	printf("PC: 0x%04x\t", core->pc);
	printf("Stack: 0x%04x\n", core->stack);
	printf("Reg B: 0x%02x\tReg C 0x%02x\n", core->b, core->c);
	printf("Reg D: 0x%02x\tReg E 0x%02x\n", core->d, core->e);
	printf("Reg H: 0x%02x\tReg L 0x%02x\n", core->h, core->l);
	printf("Reg A: 0x%02x\tReg M 0x%02x\n", core->a, core->m);
	printf("Status: C: %d P: %d AC: %d z: %d s: %d\n", core->status.bits.c,
							   core->status.bits.p,
							   core->status.bits.ac,
							   core->status.bits.z,
							   core->status.bits.s);
        printf("----------------------------\n");	
}

void execute_interrupt(FILE* log, int nInterrupt, memory_t *mem, cpu_model_t *cpu_8080) {
	switch(nInterrupt)	{
		case 0:
			opcode_handler_rst0(&opcode_table[eRST_0], mem, cpu_8080);
			break;
		case 1:
			opcode_handler_rst1(&opcode_table[eRST_1], mem, cpu_8080);
			break;
		case 2:
			opcode_handler_rst2(&opcode_table[eRST_2], mem, cpu_8080);
			break;
		case 3:
			opcode_handler_rst3(&opcode_table[eRST_3], mem, cpu_8080);
			break;
		case 4:
			opcode_handler_rst4(&opcode_table[eRST_4], mem, cpu_8080);
			break;
		case 5:
			opcode_handler_rst5(&opcode_table[eRST_5], mem, cpu_8080);
			break;
		case 6:
			opcode_handler_rst6(&opcode_table[eRST_6], mem, cpu_8080);
			break;
		case 7:
			opcode_handler_rst7(&opcode_table[eRST_7], mem, cpu_8080);
			break;
	}
}

bool single_cpu_8080_cycle(FILE* log_file, memory_t* mem, cpu_model_t* cpu)
{
	static int flag = 0;
	uint8_t opcode = mem->memory[cpu->core.pc];

	if(cpu->core.pc == 0x0355)
	       flag = 1;

	if(flag != 0) {

		pLogFile = log_file;

		fprintf(log_file, "--------------------------------------------------------\n");
		fprintf(log_file, "Instruction: 0x%04x\t%s\n", cpu->core.pc, opcode_table[opcode].ass_cmd);
		fprintf(log_file, "PC: 0x%04x\t", cpu->core.pc, cpu->core.pc);
		fprintf(log_file, "Stack: 0x%04x\t", cpu->core.stack);
		fprintf(log_file, "Reg B: 0x%02x\tReg C 0x%02x\t", cpu->core.b, cpu->core.c);
		fprintf(log_file, "Reg D: 0x%02x\tReg E 0x%02x\t", cpu->core.d, cpu->core.e);
		fprintf(log_file, "Reg H: 0x%02x\tReg L 0x%02x\n", cpu->core.h, cpu->core.l);
		fprintf(log_file, "Reg A: 0x%02x\tReg M 0x%02x\n", cpu->core.a, cpu->core.m);
		fprintf(log_file, "Status: C: %d P: %d AC: %d z: %d s: %d\n", cpu->core.status.bits.c,
								   cpu->core.status.bits.p,
								   cpu->core.status.bits.ac,
								   cpu->core.status.bits.z,
								   cpu->core.status.bits.s);
	}
	
	opcode_table[opcode].func(&opcode_table[opcode], mem, cpu);

	return (cpu->core.pc < mem->memory_size);
}


bool execute_single_cpu_8080_cycle(memory_t* mem, cpu_model_t* cpu_8080)
{
	uint8_t opcode = mem->memory[cpu_8080->core.pc];

	opcode_table[opcode].func(&opcode_table[opcode], mem, cpu_8080);

	return (cpu_8080->core.pc < mem->memory_size);
}

void execute_cpu_8080(memory_t *mem, cpu_model_t *cpu_8080)   {

	while(mem->memory_size > cpu_8080->core.pc) {
		if(cpu_8080->is_running == 1) {
			uint8_t opcode = mem->memory[cpu_8080->core.pc];

			opcode_table[opcode].func(&opcode_table[opcode], mem, cpu_8080);
		}
	}
}
