//
//	AVR_Decoder.cpp
//	===============
//
//	Containing only the decoder table generated
//	by 'encode_decoder' from the data file
//	'AVR_instructions.txt'.
//

//
//	Include standard stuff.
//
#include <stdlib.h>

//
//	Include the definitions required for the
//	table to compile.
//
#include "AVR_Decoder.h"

//
//	Define the decoder node type.
//
typedef struct {
	word		mask,		// Which bit do we test (no word index required).
			jump;		// How many to jump if bit is 1.
	function_code	code;		// The routine to return when we complete decoding.
} decoder_entry;

//
//	Start Of Table
//	==============
//
static decoder_entry decode_table[ 259 ] = {
	{ 0x4000,204,	NULL },	//   0 
	{ 0x8000,38,	NULL },	//   1 
	{ 0x2000,28,	NULL },	//   2 
	{ 0x1000,20,	NULL },	//   3 
	{ 0x0800,16,	NULL },	//   4 
	{ 0x0400,14,	NULL },	//   5 
	{ 0x0200,4,	NULL },	//   6 
	{ 0x0100,2,	NULL },	//   7 
	{ 0,0,	op_nop },	//   8[100] 00000000........  NOP						(No Operation) 
	{ 0,0,	op_movw },	//   9[101] 00000001DDDDRRRR  MOVW Rd+1:Rd,Rr+1:Rr	Rd+1:Rd = Rr+1:Rr	(Move register pair) 
	{ 0x0100,2,	NULL },	//  10 
	{ 0,0,	op_muls },	//  11[102] 00000010ddddrrrr  MULS Rd,Rr		R1:R0 = Rd x Rr		(signed = signed x signed) 
	{ 0x0080,4,	NULL },	//  12 
	{ 0x0008,2,	NULL },	//  13 
	{ 0,0,	op_mulsu },	//  14[104] 000000110ddd0rrr  MULSU Rd,Rr		R1:R0 = Rd x Rr		(signed = signed x unsigned) 
	{ 0,0,	op_fmul },	//  15[105] 000000110ddd1rrr  FMUL Rd,Rr		R1:R0 = Rd x Rr		(unsigned (1.15) = unsigned (1.7) x unsigned (1.7)) 
	{ 0x0008,2,	NULL },	//  16 
	{ 0,0,	op_fmuls },	//  17[106] 000000111ddd0rrr  FMULS Rd,Rr		R1:R0 = Rd x Rr		(signed (1.15) = signed (1.7) x signed (1.7)) 
	{ 0,0,	op_fmulsu },	//  18[107] 000000111ddd1rrr  FMULSU Rd,Rr		R1:R0 = Rd x Rr		(signed (1.15) = signed (1.7) x unsigned (1.7)) 
	{ 0,0,	op_cpc },	//  19[109] 000001rdddddrrrr  CPC Rd,Rr		Rd - Rr - C		(Compare with Carry) 
	{ 0x0400,2,	NULL },	//  20 
	{ 0,0,	op_sbc },	//  21[110] 000010rdddddrrrr  SBC Rd,Rr		Rd = Rd - Rr - C 
	{ 0,0,	op_add },	//  22[111] 000011rdddddrrrr  ADD Rd,Rr		Rd = Rd + Rr		(LSL Rd when Rd==Rr) 
	{ 0x0800,4,	NULL },	//  23 
	{ 0x0400,2,	NULL },	//  24 
	{ 0,0,	op_cpse },	//  25[112] 000100rdddddrrrr  CPSE Rd,Rr		Skip next instruction if Rd == Rr 
	{ 0,0,	op_cp },	//  26[113] 000101rdddddrrrr  CP Rd,Rr		Rd - Rr			(Compare) 
	{ 0x0400,2,	NULL },	//  27 
	{ 0,0,	op_sub },	//  28[114] 000110rdddddrrrr  SUB Rd,Rr		Rd = Rd - Rr 
	{ 0,0,	op_adc },	//  29[115] 000111rdddddrrrr  ADC Rd,Rr		Rd = Rd + Rr + C	(ROL Rd when Rd==Rr) 
	{ 0x1000,8,	NULL },	//  30 
	{ 0x0800,4,	NULL },	//  31 
	{ 0x0400,2,	NULL },	//  32 
	{ 0,0,	op_and },	//  33[116] 001000rdddddrrrr  AND Rd,Rr		Rd = Rd & Rr 
	{ 0,0,	op_eor },	//  34[117] 001001rdddddrrrr  EOR Rd,Rr		Rd = Rd ^ Rr 
	{ 0x0400,2,	NULL },	//  35 
	{ 0,0,	op_or },	//  36[118] 001010rdddddrrrr  OR Rd,Rr		Rd = Rd | Rr 
	{ 0,0,	op_mov },	//  37[119] 001011rdddddrrrr  MOV Rd,Rr		Rd = Rr 
	{ 0,0,	op_cpi },	//  38[121] 0011KKKKddddKKKK  CPI Rd,K		Rd - K			(Compare with 8-bit immediate) 
	{ 0x1000,8,	NULL },	//  39 
	{ 0x0200,4,	NULL },	//  40 
	{ 0x0008,2,	NULL },	//  41 
	{ 0,0,	op_ld_z_q },	//  42[153] 10q0qq0ddddd0qqq  LD Rd,Z+q		Rd = DS(Z+q)			(See Note) 
	{ 0,0,	op_ld_y_q },	//  43[149] 10q0qq0ddddd1qqq  LD RD,Y+q		Rd = DS(Y+q)			(See Note) 
	{ 0x0008,2,	NULL },	//  44 
	{ 0,0,	op_std_z_q },	//  45[180] 10q0qq1rrrrr0qqq  STD Z+q,Rd		DS(Z+q) = Rd			(See Note) 
	{ 0,0,	op_std_y_q },	//  46[176] 10q0qq1rrrrr1qqq  STD Y+q,Rd		DS(Y+q) = Rd			(See Note) 
	{ 0x0800,146,	NULL },	//  47 
	{ 0x2000,144,	NULL },	//  48 
	{ 0x0400,52,	NULL },	//  49 
	{ 0x0200,26,	NULL },	//  50 
	{ 0x0008,14,	NULL },	//  51 
	{ 0x0004,6,	NULL },	//  52 
	{ 0x0002,4,	NULL },	//  53 
	{ 0x0001,2,	NULL },	//  54 
	{ 0,0,	op_lds },	//  55[127] 1001000ddddd0000 kkkkkkkkkkkkkkkk  LDS rd,k		Rd = DS(RAMPD:k)	(Data Space Immediate address) 
	{ 0,0,	op_ld_zp },	//  56[154] 1001000ddddd0001  LD Rd,Z+		Rd = DS(Z+) 
	{ 0,0,	op_ld_nz },	//  57[155] 1001000ddddd0010 [2]  LD Rd,−Z		Rd = DS(-Z) 
	{ 0x0002,4,	NULL },	//  58 
	{ 0x0001,2,	NULL },	//  59 
	{ 0,0,	op_lpm_z },	//  60[130] 1001000ddddd0100  LPM Rd,Z		Rd = PS(Z) 
	{ 0,0,	op_lpm_zp },	//  61[131] 1001000ddddd0101  LPM Rd,Z+		Rd = PS(Z+) 
	{ 0x0001,2,	NULL },	//  62 
	{ 0,0,	op_elpm_z },	//  63[134] 1001000ddddd0110  ELPM Rd,Z		Rd = PS(RAMPZ:Z) 
	{ 0,0,	op_elpm_zp },	//  64[135] 1001000ddddd0111  ELPM Rd,Z+		Rd = PS(RAMPZ:Z+) 
	{ 0x0002,6,	NULL },	//  65 
	{ 0x0004,2,	NULL },	//  66 
	{ 0,0,	op_ld_yp },	//  67[150] 1001000ddddd1001 [2]  LD Rd,Y+		Rd = DS(Y+) 
	{ 0x0001,2,	NULL },	//  68 
	{ 0,0,	op_ld_x },	//  69[137] 1001000ddddd1100  LD Rd,X		Rd = DS(X) 
	{ 0,0,	op_ld_xp },	//  70[138] 1001000ddddd1101  LD Rd,X+		Rd = DS(X+) 
	{ 0x0004,2,	NULL },	//  71 
	{ 0,0,	op_ld_ny },	//  72[151] 1001000ddddd1010 [2]  LD Rd,−Y		Rd = DS(-Y) 
	{ 0x0001,2,	NULL },	//  73 
	{ 0,0,	op_ld_nx },	//  74[139] 1001000ddddd1110  LD Rd,-X		Rd = DS(-X) 
	{ 0,0,	op_pop },	//  75[140] 1001000ddddd1111  POP Rd		Rd = DS(+STACK) 
	{ 0x0008,14,	NULL },	//  76 
	{ 0x0004,6,	NULL },	//  77 
	{ 0x0002,4,	NULL },	//  78 
	{ 0x0001,2,	NULL },	//  79 
	{ 0,0,	op_sts },	//  80[157] 1001001ddddd0000 kkkkkkkkkkkkkkkk  STS k,rd		DS(RAMPD:k) = Rd 
	{ 0,0,	op_st_zp },	//  81[181] 1001001ddddd0001  ST Z+,Rd		DS(Z+) = Rd 
	{ 0,0,	op_st_nz },	//  82[182] 1001001ddddd0010 [2]  ST −Z,Rd		DS(-Z) = Rd 
	{ 0x0002,4,	NULL },	//  83 
	{ 0x0001,2,	NULL },	//  84 
	{ 0,0,	op_xch },	//  85[159] 1001001ddddd0100  XCH Z,Rd 		DS(Z) <-> Rd 
	{ 0,0,	op_las },	//  86[160] 1001001ddddd0101  LAS Z,Rd		DS(Z) = DS(Z) | Rd, Rd = DS(Z)	(Atomic read and set) 
	{ 0x0001,2,	NULL },	//  87 
	{ 0,0,	op_lac },	//  88[161] 1001001ddddd0110  LAC Z,Rd		DS(Z) = DS(Z) & ~Rd, Rd = DS(Z)	(Atomic read and clear) 
	{ 0,0,	op_lat },	//  89[162] 1001001ddddd0111  LAT Z,Rd		DS(Z) = DS(Z) ^ Rd, Rd = DS(Z)	(Atomic read and toggle) 
	{ 0x0002,6,	NULL },	//  90 
	{ 0x0004,2,	NULL },	//  91 
	{ 0,0,	op_st_yp },	//  92[177] 1001001ddddd1001 [2]  ST Y+,Rd		DS(Y+) = Rd 
	{ 0x0001,2,	NULL },	//  93 
	{ 0,0,	op_st },	//  94[164] 1001001ddddd1100  ST X,Rd		DS(X) = Rd 
	{ 0,0,	op_st },	//  95[165] 1001001ddddd1101  ST X+,Rd		DS(X+) = Rd 
	{ 0x0004,2,	NULL },	//  96 
	{ 0,0,	op_st_ny },	//  97[178] 1001001ddddd1010 [2]  ST −Y,Rd		DS(-Y) = Rd 
	{ 0x0001,2,	NULL },	//  98 
	{ 0,0,	op_st },	//  99[166] 1001001ddddd1110  ST -X,Rd		DS(-X) = Rd 
	{ 0,0,	op_push },	// 100[167] 1001001ddddd1111  PUSH Rd		DS(STACK-) = Rd 
	{ 0x0200,88,	NULL },	// 101 
	{ 0x0008,16,	NULL },	// 102 
	{ 0x0004,8,	NULL },	// 103 
	{ 0x0002,4,	NULL },	// 104 
	{ 0x0001,2,	NULL },	// 105 
	{ 0,0,	op_com },	// 106[222] 1001010ddddd0000  COM Rd		Rd = ~Rd		(Ones complement) 
	{ 0,0,	op_neg },	// 107[223] 1001010ddddd0001  NEG Rd		Rd = -Rd		(Twos complement) 
	{ 0x0001,2,	NULL },	// 108 
	{ 0,0,	op_swap },	// 109[224] 1001010ddddd0010  SWAP Rd		Rd[0:3] <-> Rd[7:4]	(Swap nibbles in Rd) 
	{ 0,0,	op_inc },	// 110[225] 1001010ddddd0011  INC Rd		Rd = Rd + 1		(Increment Rd) 
	{ 0x0002,4,	NULL },	// 111 
	{ 0x0001,2,	NULL },	// 112 
	{ 0,0,	op_reserved },	// 113[226] 1001010ddddd0100  			Reserved 
	{ 0,0,	op_asr },	// 114[227] 1001010ddddd0101  ASR Rd		Rd = Rd / 2		(Arithmetic right shift) 
	{ 0x0001,2,	NULL },	// 115 
	{ 0,0,	op_lsr },	// 116[228] 1001010ddddd0110  LSR Rd		Rd = Rd >> 1		(Logical right shift) 
	{ 0,0,	op_ror },	// 117[229] 1001010ddddd0111  ROR Rd		C -> Rd -> C		(Rotate right Rd through Carry) 
	{ 0x0002,66,	NULL },	// 118 
	{ 0x0004,64,	NULL },	// 119 
	{ 0x0080,32,	NULL },	// 120 
	{ 0x0100,20,	NULL },	// 121 
	{ 0x0010,10,	NULL },	// 122 
	{ 0x0040,6,	NULL },	// 123 
	{ 0x0020,4,	NULL },	// 124 
	{ 0x0001,2,	NULL },	// 125 
	{ 0,0,	op_sec },	// 126[196] 1001010000001000  SEC			C = 1 
	{ 0,0,	op_ijmp },	// 127[184] 1001010000001001  IJMP			PC = Z 
	{ 0,0,	op_sen },	// 128[198] 1001010000101000 [2]  SEN			N = 1 
	{ 0x0020,2,	NULL },	// 129 
	{ 0,0,	op_ses },	// 130[200] 1001010001001000 [2]  SES			S = 1 
	{ 0,0,	op_set },	// 131[202] 1001010001101000 [2]  SET			T = 1 
	{ 0x0040,6,	NULL },	// 132 
	{ 0x0020,4,	NULL },	// 133 
	{ 0x0001,2,	NULL },	// 134 
	{ 0,0,	op_sez },	// 135[197] 1001010000011000  SEZ			Z = 1 
	{ 0,0,	op_eijmp },	// 136[185] 1001010000011001  EIJMP			PC = EIND:Z 
	{ 0,0,	op_sev },	// 137[199] 1001010000111000 [2]  SEV			V = 1 
	{ 0x0020,2,	NULL },	// 138 
	{ 0,0,	op_seh },	// 139[201] 1001010001011000 [2]  SEH			H = 1 
	{ 0,0,	op_sei },	// 140[203] 1001010001111000 [2]  SEI			I = 1 
	{ 0x0001,8,	NULL },	// 141 
	{ 0x0040,6,	NULL },	// 142 
	{ 0x0020,4,	NULL },	// 143 
	{ 0x0010,2,	NULL },	// 144 
	{ 0,0,	op_ret },	// 145[207] 1001010100001000  RET			PC = DS(+STACK)		(Pops 2 or 3 bytes as appropriate to AVR architecture) 
	{ 0,0,	op_reti },	// 146[209] 1001010100011000  RETI			PC = DS(+STACK),I = 1 
	{ 0,0,	op_reserved },	// 147[212] 10010101001x1000  			Reserved 
	{ 0,0,	op_reserved },	// 148[213] 1001010101xx1000  			Reserved 
	{ 0x0010,2,	NULL },	// 149 
	{ 0,0,	op_icall },	// 150[208] 1001010100001001 [4]  ICALL Z		DS(STACK-) = PC, PC = Z	(Pushes 2 or 3 bytes) 
	{ 0,0,	op_eicall },	// 151[210] 1001010100011001 [4]  EICALL Z		DS(STACK-) = PC, PC = EIND:Z 
	{ 0x0100,16,	NULL },	// 152 
	{ 0x0040,8,	NULL },	// 153 
	{ 0x0020,4,	NULL },	// 154 
	{ 0x0010,2,	NULL },	// 155 
	{ 0,0,	op_clc },	// 156[187] 1001010010001000 [2]  CLC			C = 0 
	{ 0,0,	op_clz },	// 157[188] 1001010010011000 [2]  CLZ			Z = 0 
	{ 0x0010,2,	NULL },	// 158 
	{ 0,0,	op_cln },	// 159[189] 1001010010101000 [2]  CLN			N = 0 
	{ 0,0,	op_clv },	// 160[190] 1001010010111000 [2]  CLV			V = 0 
	{ 0x0020,4,	NULL },	// 161 
	{ 0x0010,2,	NULL },	// 162 
	{ 0,0,	op_cls },	// 163[191] 1001010011001000 [2]  CLS			S = 0 
	{ 0,0,	op_clh },	// 164[192] 1001010011011000 [2]  CLH			H = 0 
	{ 0x0010,2,	NULL },	// 165 
	{ 0,0,	op_clt },	// 166[193] 1001010011101000 [2]  CLT			T = 0 
	{ 0,0,	op_cli },	// 167[194] 1001010011111000 [2]  CLI			I = 0 
	{ 0x0040,8,	NULL },	// 168 
	{ 0x0020,4,	NULL },	// 169 
	{ 0x0010,2,	NULL },	// 170 
	{ 0,0,	op_sleep },	// 171[215] 1001010110001000 [2]  SLEEP						(Put CPU into sleep mode) 
	{ 0,0,	op_break },	// 172[216] 1001010110011000 [2]  BREAK						(Stop CPU, enter debug mode) 
	{ 0x0010,2,	NULL },	// 173 
	{ 0,0,	op_wdr },	// 174[217] 1001010110101000 [2]  WDR						(Restart Watch Dog Timer) 
	{ 0,0,	op_reserved },	// 175[218] 1001010110111000 [2]  			Reserved  
	{ 0x0020,4,	NULL },	// 176 
	{ 0x0010,2,	NULL },	// 177 
	{ 0,0,	op_lpm_r0 },	// 178[129] 1001010111001000 [2]  LPM			R0 = PS(Z) 
	{ 0,0,	op_elpm_r0 },	// 179[133] 1001010111011000 [2]  ELPM			R0 = PS(RAMPZ:Z) 
	{ 0x0010,2,	NULL },	// 180 
	{ 0,0,	op_spm },	// 181[219] 1001010111101000 [2]  SPM						(See section 116 in "AVR Instruction Set Manual") 
	{ 0,0,	op_spm },	// 182[220] 1001010111111000 [2]  SPM Z+ 					(See section 117 in "AVR Instruction Set Manual") 
	{ 0,0,	op_jmp },	// 183[232] 1001010kkkkk110k kkkkkkkkkkkkkkkk  JMP k		PC = k 
	{ 0x0004,4,	NULL },	// 184 
	{ 0x0001,2,	NULL },	// 185 
	{ 0,0,	op_dec },	// 186[230] 1001010ddddd1010  DEC Rd		Rd = Rd - 1 
	{ 0,0,	op_des },	// 187[205] 10010100kkkk1011 [2]  DES k			Data Encryption Action 'k' 
	{ 0,0,	op_call },	// 188[233] 1001010kkkkk111k kkkkkkkkkkkkkkkk  CALL k		DS(STACK-) = PC, PC = k 
	{ 0x0100,2,	NULL },	// 189 
	{ 0,0,	op_adiw },	// 190[244] 10010110kkppkkkk  ADIW Rd,k		Rd+1:Rd += k		(where "d=24+(p<<1)" and k is unsigned 6 bit value)  
	{ 0,0,	op_sbiw },	// 191[245] 10010111kkppkkkk  SBIW Rp,k		Rd+1:Rd -= k		(where "d=24+(p<<1)" and k is unsigned 6 bit value) 
	{ 0,0,	op_in },	// 192[252] 10110aadddddaaaa  IN Rd,a		Rd = IO(a)		(Output register to IO address) 
	{ 0x2000,10,	NULL },	// 193 
	{ 0x0400,8,	NULL },	// 194 
	{ 0x0200,4,	NULL },	// 195 
	{ 0x0100,2,	NULL },	// 196 
	{ 0,0,	op_cbi },	// 197[247] 10011000aaaaabbb  CBI a,b		IO(a)[b] = 0		(Clear bit b in IO register a) 
	{ 0,0,	op_sbic },	// 198[248] 10011001aaaaabbb  SBIC a,b		If IO(a)[b] == 0 skip	(Skip next instruction if bit b in IO register a is clear) 
	{ 0x0100,2,	NULL },	// 199 
	{ 0,0,	op_sbi },	// 200[249] 10011010aaaaabbb  SBI a,b		IO(a)[b] = 1		(Set bit b in IO register a) 
	{ 0,0,	op_sbis },	// 201[250] 10011011aaaaabbb  SBIS a,b		If IO(a)[b] == 1 skip	(Skip next instruction if bit b in IO register a is set) 
	{ 0,0,	op_mul },	// 202[251] 100111rdddddrrrr  MUL Rd,Rr		R1:R0 = Rr x Rd		(unsigned = unsigned x unsigned) 
	{ 0,0,	op_out },	// 203[253] 10111aadddddaaaa  OUT a,Rd		IO(a) = Rd		(Read register from IO address) 
	{ 0x8000,8,	NULL },	// 204 
	{ 0x2000,4,	NULL },	// 205 
	{ 0x1000,2,	NULL },	// 206 
	{ 0,0,	op_sbci },	// 207[122] 0100KKKKddddKKKK  SBCI Rd,K		Rd = Rd - K - C		(Subtract 8-bit immediate and Carry) 
	{ 0,0,	op_subi },	// 208[123] 0101KKKKddddKKKK  SUBI Rd,K		Rd = Rd - K		(Subtract 8-bit immediate) 
	{ 0x1000,2,	NULL },	// 209 
	{ 0,0,	op_ori },	// 210[124] 0110KKKKddddKKKK  ORI Rd,K		Rd = Rd | K		(Or with 8-bit immediate. Also: SBR Rd,K) 
	{ 0,0,	op_andi },	// 211[125] 0111KKKKddddKKKK  ANDI Rd,K		Rd = Rd & K		(And with 8-bit immediate. Also: CBR Rd,~K) 
	{ 0x2000,4,	NULL },	// 212 
	{ 0x1000,2,	NULL },	// 213 
	{ 0,0,	op_rjmp },	// 214[255] 1100jjjjjjjjjjjj  RJMP j		PC = PC + j		(Relative jump, j is signed 12 bit immediate) 
	{ 0,0,	op_rcall },	// 215[256] 1101jjjjjjjjjjjj  RCALL j		DS(STACK-) = PC, PC = PC + j (Relative call, j is signed 12 bit immediate) 
	{ 0x1000,2,	NULL },	// 216 
	{ 0,0,	op_ldi },	// 217[257] 1110KKKKddddKKKK  LDI Rd,K		Rd = K			(Load register with 8 bit immediate) 
	{ 0x0800,32,	NULL },	// 218 
	{ 0x0400,16,	NULL },	// 219 
	{ 0x0004,8,	NULL },	// 220 
	{ 0x0002,4,	NULL },	// 221 
	{ 0x0001,2,	NULL },	// 222 
	{ 0,0,	op_brcs },	// 223[259] 111100jjjjjjj000  BRCS j		if C == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brzs },	// 224[260] 111100jjjjjjj001  BRZS j		if Z == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 225 
	{ 0,0,	op_brns },	// 226[261] 111100jjjjjjj010  BRNS j		if N == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brvs },	// 227[262] 111100jjjjjjj011  BRVS j		if V == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0002,4,	NULL },	// 228 
	{ 0x0001,2,	NULL },	// 229 
	{ 0,0,	op_brss },	// 230[263] 111100jjjjjjj100  BRSS j		if S == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brhs },	// 231[264] 111100jjjjjjj101  BRHS j		if H == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 232 
	{ 0,0,	op_brts },	// 233[265] 111100jjjjjjj110  BRTS j		if T == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_bris },	// 234[266] 111100jjjjjjj111  BRIS j		if I == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0004,8,	NULL },	// 235 
	{ 0x0002,4,	NULL },	// 236 
	{ 0x0001,2,	NULL },	// 237 
	{ 0,0,	op_brcc },	// 238[268] 111101jjjjjjj000  BRCC j		if C == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brzc },	// 239[269] 111101jjjjjjj001  BRZC j		if Z == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 240 
	{ 0,0,	op_brnc },	// 241[270] 111101jjjjjjj010  BRNC j		if N == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brvc },	// 242[271] 111101jjjjjjj011  BRVC j		if V == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0002,4,	NULL },	// 243 
	{ 0x0001,2,	NULL },	// 244 
	{ 0,0,	op_brsc },	// 245[272] 111101jjjjjjj100  BRSC j		if S == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brhc },	// 246[273] 111101jjjjjjj101  BRHC j		if H == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 247 
	{ 0,0,	op_brtc },	// 248[274] 111101jjjjjjj110  BRTC j		if T == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_bric },	// 249[275] 111101jjjjjjj111  BRIC j		if I == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0008,8,	NULL },	// 250 
	{ 0x0400,4,	NULL },	// 251 
	{ 0x0200,2,	NULL },	// 252 
	{ 0,0,	op_bld },	// 253[277] 1111100ddddd0bbb  BLD Rd,b		Rd[b] = T 
	{ 0,0,	op_bst },	// 254[278] 1111101ddddd0bbb  BST Rd,b		T = Rd[b] 
	{ 0x0200,2,	NULL },	// 255 
	{ 0,0,	op_sbrc },	// 256[279] 1111110ddddd0bbb  SBRC Rd,b		Skip next if Rd[b] == 0 
	{ 0,0,	op_sbrs },	// 257[280] 1111111ddddd0bbb  SBRS Rd,b		Skip next if Rd[b] == 1 
	{ 0,0,	op_reserved } 	// 258[281] 11111xxddddd1bbb  			Reserved 
};

//
//	End Of Table
//	============
//

//
//	The lookup function.
//
function_code decode_opcode( word opcode ) {
	decoder_entry	*ptr;
	word		test;

	ptr = decode_table;
	while(( test = ptr->mask )) ptr += ( opcode & test )? ptr->jump: 1;
	return( ptr->code );
}

//
//	EOF
//
