//
//	AVR_Decoder.cpp
//	---------------
//
//	Containing only the decoder table generated
//	by 'encode_decoder' from the data file
//	'AVR_Decoder.txt'.
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
//	Error handling interface.
//
#include "Exception.h"

//
//	Define the decoder node type.
//
typedef struct {
	word		mask,		// Which bit do we test (no word index required).
			jump;		// How many to jump if bit is 1.
	function_code	code;		// The routine to return when we complete decoding.
} decoder_entry;


//
//	Some useful tools.
//
//	4 bit oriented.
//
#define NIBBLE(v)	((v)&0x0F)
#define HALF(a,b,c)	((NIBBLE(a)+NIBBLE(b)+(c))>9)
//
//	8 bit oriented
//
#define BYTE(v)		((v)&0xFF)
#define LSBIT(v)	((v)&0x01)
#define MSBIT(v)	((v)&0x80)
#define SIGN(v)		(MSBIT(v)!=0)
//
//	16 bit oriented.
//
#define SIGNW(v)	(((v)&0x8000)!=0)
#define LSBYTE(v)	((v)&0xFF)
#define MSBYTE(v)	(((v)>>8)&0xFF)
#define COMBINE(h,l)	(((h)<<8)|(l))


word op_illegal( word opcode, AVR_State *state ) {
	//
	//	Called when any decoded opcode does not uniquely identify a
	//	single specific instruction.
	//
	state->_error->raise( Illegal_Instruction, state->_pc, opcode );
	//
	//	Action here will depend on the CPU being
	//	simulated.
	//
	//	At present nothing is being done on any CPU.
	//
	switch( state->_cpu ) {
		case AVR_CPU: {
			break;
		}
		case AVRe_CPU: {
			break;
		}
		case AVRet_CPU: {
			break;
		}
		case AVRxm_CPU: {
			break;
		}
		case AVRxt_CPU: {
			break;
		}
		case AVRrc_CPU: {
			break;
		}
		default: {
			//
			//	Default is to ignore the illegal instruction
			//	and continue program execution.
			//
			break;
		}
	}
	//
	//	All illegal instructions, if ignored, take one cycle.
	//
	return( 1 );
}
word op_nop( word opcode, AVR_State *state ) {
	return( 0 );
}
//
//	Start Of Table
//	==============
//
static decoder_entry decode_table[ 341 ] = {
	{ 0x4000,286,	NULL },	//   0 
	{ 0x8000,54,	NULL },	//   1 
	{ 0x2000,44,	NULL },	//   2 
	{ 0x1000,36,	NULL },	//   3 
	{ 0x0800,32,	NULL },	//   4 
	{ 0x0400,30,	NULL },	//   5 
	{ 0x0200,20,	NULL },	//   6 
	{ 0x0100,18,	NULL },	//   7 
	{ 0x0080,16,	NULL },	//   8 
	{ 0x0040,14,	NULL },	//   9 
	{ 0x0020,12,	NULL },	//  10 
	{ 0x0010,10,	NULL },	//  11 
	{ 0x0008,8,	NULL },	//  12 
	{ 0x0004,6,	NULL },	//  13 
	{ 0x0002,4,	NULL },	//  14 
	{ 0x0001,2,	NULL },	//  15 
	{ 0,0,	op_nop },	//  16/250 0000000000000000  NOP						(No Operation) 
	{ 0,0,	op_illegal },	//  17 Invalid Instruction 
	{ 0,0,	op_illegal },	//  18 Invalid Instruction 
	{ 0,0,	op_illegal },	//  19 Invalid Instruction 
	{ 0,0,	op_illegal },	//  20 Invalid Instruction 
	{ 0,0,	op_illegal },	//  21 Invalid Instruction 
	{ 0,0,	op_illegal },	//  22 Invalid Instruction 
	{ 0,0,	op_illegal },	//  23 Invalid Instruction 
	{ 0,0,	op_illegal },	//  24 Invalid Instruction 
	{ 0,0,	op_movw },	//  25/255 00000001DDDDRRRR  MOVW Rd+1:Rd,Rr+1:Rr	Rd+1:Rd = Rr+1:Rr	(Move register pair) 
	{ 0x0100,2,	NULL },	//  26 
	{ 0,0,	op_muls },	//  27/256 00000010ddddrrrr  MULS Rd,Rr		R1:R0 = Rd x Rr		(signed = signed x signed) 
	{ 0x0080,4,	NULL },	//  28 
	{ 0x0008,2,	NULL },	//  29 
	{ 0,0,	op_mulsu },	//  30/258 000000110ddd0rrr  MULSU Rd,Rr		R1:R0 = Rd x Rr		(signed = signed x unsigned) 
	{ 0,0,	op_fmul },	//  31/259 000000110ddd1rrr  FMUL Rd,Rr		R1:R0 = Rd x Rr		(unsigned (1.15) = unsigned (1.7) x unsigned (1.7)) 
	{ 0x0008,2,	NULL },	//  32 
	{ 0,0,	op_fmuls },	//  33/260 000000111ddd0rrr  FMULS Rd,Rr		R1:R0 = Rd x Rr		(signed (1.15) = signed (1.7) x signed (1.7)) 
	{ 0,0,	op_fmulsu },	//  34/261 000000111ddd1rrr  FMULSU Rd,Rr		R1:R0 = Rd x Rr		(signed (1.15) = signed (1.7) x unsigned (1.7)) 
	{ 0,0,	op_cpc },	//  35/263 000001rdddddrrrr  CPC Rd,Rr		Rd - Rr - C		(Compare with Carry) 
	{ 0x0400,2,	NULL },	//  36 
	{ 0,0,	op_sbc },	//  37/264 000010rdddddrrrr  SBC Rd,Rr		Rd = Rd - Rr - C 
	{ 0,0,	op_add },	//  38/265 000011rdddddrrrr  ADD Rd,Rr		Rd = Rd + Rr		(LSL Rd when Rd==Rr) 
	{ 0x0800,4,	NULL },	//  39 
	{ 0x0400,2,	NULL },	//  40 
	{ 0,0,	op_cpse },	//  41/266 000100rdddddrrrr  CPSE Rd,Rr		Skip next instruction if Rd == Rr 
	{ 0,0,	op_cp },	//  42/267 000101rdddddrrrr  CP Rd,Rr		Rd - Rr			(Compare) 
	{ 0x0400,2,	NULL },	//  43 
	{ 0,0,	op_sub },	//  44/268 000110rdddddrrrr  SUB Rd,Rr		Rd = Rd - Rr 
	{ 0,0,	op_adc },	//  45/269 000111rdddddrrrr  ADC Rd,Rr		Rd = Rd + Rr + C	(ROL Rd when Rd==Rr) 
	{ 0x1000,8,	NULL },	//  46 
	{ 0x0800,4,	NULL },	//  47 
	{ 0x0400,2,	NULL },	//  48 
	{ 0,0,	op_and },	//  49/270 001000rdddddrrrr  AND Rd,Rr		Rd = Rd & Rr 
	{ 0,0,	op_eor },	//  50/271 001001rdddddrrrr  EOR Rd,Rr		Rd = Rd ^ Rr 
	{ 0x0400,2,	NULL },	//  51 
	{ 0,0,	op_or },	//  52/272 001010rdddddrrrr  OR Rd,Rr		Rd = Rd | Rr 
	{ 0,0,	op_mov },	//  53/273 001011rdddddrrrr  MOV Rd,Rr		Rd = Rr 
	{ 0,0,	op_cpi },	//  54/275 0011KKKKddddKKKK  CPI Rd,K		Rd - K			(Compare with 8-bit immediate) 
	{ 0x1000,8,	NULL },	//  55 
	{ 0x0200,4,	NULL },	//  56 
	{ 0x0008,2,	NULL },	//  57 
	{ 0,0,	op_ld_z_q },	//  58/307 10q0qq0ddddd0qqq  LD Rd,Z+q		Rd = DS(Z+q)			(See Note) 
	{ 0,0,	op_ld_y_q },	//  59/303 10q0qq0ddddd1qqq  LD RD,Y+q		Rd = DS(Y+q)			(See Note) 
	{ 0x0008,2,	NULL },	//  60 
	{ 0,0,	op_std_z_q },	//  61/334 10q0qq1rrrrr0qqq  STD Z+q,Rd		DS(Z+q) = Rd			(See Note) 
	{ 0,0,	op_std_y_q },	//  62/330 10q0qq1rrrrr1qqq  STD Y+q,Rd		DS(Y+q) = Rd			(See Note) 
	{ 0x0800,212,	NULL },	//  63 
	{ 0x2000,210,	NULL },	//  64 
	{ 0x0400,64,	NULL },	//  65 
	{ 0x0200,32,	NULL },	//  66 
	{ 0x0008,16,	NULL },	//  67 
	{ 0x0004,8,	NULL },	//  68 
	{ 0x0002,4,	NULL },	//  69 
	{ 0x0001,2,	NULL },	//  70 
	{ 0,0,	op_lds },	//  71/281 1001000ddddd0000 kkkkkkkkkkkkkkkk  LDS rd,k		Rd = DS(RAMPD:k)	(Data Space Immediate address) 
	{ 0,0,	op_ld_zp },	//  72/308 1001000ddddd0001  LD Rd,Z+		Rd = DS(Z+) 
	{ 0x0001,2,	NULL },	//  73 
	{ 0,0,	op_ld_nz },	//  74/309 1001000ddddd0010  LD Rd,−Z		Rd = DS(-Z) 
	{ 0,0,	op_illegal },	//  75 Invalid Instruction 
	{ 0x0002,4,	NULL },	//  76 
	{ 0x0001,2,	NULL },	//  77 
	{ 0,0,	op_lpm_z },	//  78/284 1001000ddddd0100  LPM Rd,Z		Rd = PS(Z) 
	{ 0,0,	op_lpm_zp },	//  79/285 1001000ddddd0101  LPM Rd,Z+		Rd = PS(Z+) 
	{ 0x0001,2,	NULL },	//  80 
	{ 0,0,	op_elpm_z },	//  81/288 1001000ddddd0110  ELPM Rd,Z		Rd = PS(RAMPZ:Z) 
	{ 0,0,	op_elpm_zp },	//  82/289 1001000ddddd0111  ELPM Rd,Z+		Rd = PS(RAMPZ:Z+) 
	{ 0x0002,8,	NULL },	//  83 
	{ 0x0004,4,	NULL },	//  84 
	{ 0x0001,2,	NULL },	//  85 
	{ 0,0,	op_illegal },	//  86 Invalid Instruction 
	{ 0,0,	op_ld_yp },	//  87/304 1001000ddddd1001  LD Rd,Y+		Rd = DS(Y+) 
	{ 0x0001,2,	NULL },	//  88 
	{ 0,0,	op_ld_x },	//  89/291 1001000ddddd1100  LD Rd,X		Rd = DS(X) 
	{ 0,0,	op_ld_xp },	//  90/292 1001000ddddd1101  LD Rd,X+		Rd = DS(X+) 
	{ 0x0004,4,	NULL },	//  91 
	{ 0x0001,2,	NULL },	//  92 
	{ 0,0,	op_ld_ny },	//  93/305 1001000ddddd1010  LD Rd,−Y		Rd = DS(-Y) 
	{ 0,0,	op_illegal },	//  94 Invalid Instruction 
	{ 0x0001,2,	NULL },	//  95 
	{ 0,0,	op_ld_nx },	//  96/293 1001000ddddd1110  LD Rd,-X		Rd = DS(-X) 
	{ 0,0,	op_pop },	//  97/294 1001000ddddd1111  POP Rd		Rd = DS(+STACK) 
	{ 0x0008,16,	NULL },	//  98 
	{ 0x0004,8,	NULL },	//  99 
	{ 0x0002,4,	NULL },	// 100 
	{ 0x0001,2,	NULL },	// 101 
	{ 0,0,	op_sts },	// 102/311 1001001ddddd0000 kkkkkkkkkkkkkkkk  STS k,rd		DS(RAMPD:k) = Rd 
	{ 0,0,	op_st_zp },	// 103/335 1001001ddddd0001  ST Z+,Rd		DS(Z+) = Rd 
	{ 0x0001,2,	NULL },	// 104 
	{ 0,0,	op_st_nz },	// 105/336 1001001ddddd0010  ST −Z,Rd		DS(-Z) = Rd 
	{ 0,0,	op_illegal },	// 106 Invalid Instruction 
	{ 0x0002,4,	NULL },	// 107 
	{ 0x0001,2,	NULL },	// 108 
	{ 0,0,	op_xch },	// 109/313 1001001ddddd0100  XCH Z,Rd 		DS(Z) <-> Rd 
	{ 0,0,	op_las },	// 110/314 1001001ddddd0101  LAS Z,Rd		DS(Z) = DS(Z) | Rd, Rd = DS(Z)	(Atomic read and set) 
	{ 0x0001,2,	NULL },	// 111 
	{ 0,0,	op_lac },	// 112/315 1001001ddddd0110  LAC Z,Rd		DS(Z) = DS(Z) & ~Rd, Rd = DS(Z)	(Atomic read and clear) 
	{ 0,0,	op_lat },	// 113/316 1001001ddddd0111  LAT Z,Rd		DS(Z) = DS(Z) ^ Rd, Rd = DS(Z)	(Atomic read and toggle) 
	{ 0x0002,8,	NULL },	// 114 
	{ 0x0004,4,	NULL },	// 115 
	{ 0x0001,2,	NULL },	// 116 
	{ 0,0,	op_illegal },	// 117 Invalid Instruction 
	{ 0,0,	op_st_yp },	// 118/331 1001001ddddd1001  ST Y+,Rd		DS(Y+) = Rd 
	{ 0x0001,2,	NULL },	// 119 
	{ 0,0,	op_st },	// 120/318 1001001ddddd1100  ST X,Rd		DS(X) = Rd 
	{ 0,0,	op_st },	// 121/319 1001001ddddd1101  ST X+,Rd		DS(X+) = Rd 
	{ 0x0004,4,	NULL },	// 122 
	{ 0x0001,2,	NULL },	// 123 
	{ 0,0,	op_st_ny },	// 124/332 1001001ddddd1010  ST −Y,Rd		DS(-Y) = Rd 
	{ 0,0,	op_illegal },	// 125 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 126 
	{ 0,0,	op_st },	// 127/320 1001001ddddd1110  ST -X,Rd		DS(-X) = Rd 
	{ 0,0,	op_push },	// 128/321 1001001ddddd1111  PUSH Rd		DS(STACK-) = Rd 
	{ 0x0200,142,	NULL },	// 129 
	{ 0x0008,16,	NULL },	// 130 
	{ 0x0004,8,	NULL },	// 131 
	{ 0x0002,4,	NULL },	// 132 
	{ 0x0001,2,	NULL },	// 133 
	{ 0,0,	op_com },	// 134/388 1001010ddddd0000  COM Rd		Rd = ~Rd		(Ones complement) 
	{ 0,0,	op_neg },	// 135/389 1001010ddddd0001  NEG Rd		Rd = -Rd		(Twos complement) 
	{ 0x0001,2,	NULL },	// 136 
	{ 0,0,	op_swap },	// 137/390 1001010ddddd0010  SWAP Rd		Rd[0:3] <-> Rd[7:4]	(Swap nibbles in Rd) 
	{ 0,0,	op_inc },	// 138/391 1001010ddddd0011  INC Rd		Rd = Rd + 1		(Increment Rd) 
	{ 0x0002,4,	NULL },	// 139 
	{ 0x0001,2,	NULL },	// 140 
	{ 0,0,	op_reserved },	// 141/392 1001010ddddd0100  			Reserved 
	{ 0,0,	op_asr },	// 142/393 1001010ddddd0101  ASR Rd		Rd = Rd / 2		(Arithmetic right shift) 
	{ 0x0001,2,	NULL },	// 143 
	{ 0,0,	op_lsr },	// 144/394 1001010ddddd0110  LSR Rd		Rd = Rd >> 1		(Logical right shift) 
	{ 0,0,	op_ror },	// 145/395 1001010ddddd0111  ROR Rd		C -> Rd -> C		(Rotate right Rd through Carry) 
	{ 0x0002,118,	NULL },	// 146 
	{ 0x0004,116,	NULL },	// 147 
	{ 0x0080,52,	NULL },	// 148 
	{ 0x0100,32,	NULL },	// 149 
	{ 0x0010,16,	NULL },	// 150 
	{ 0x0040,8,	NULL },	// 151 
	{ 0x0020,4,	NULL },	// 152 
	{ 0x0001,2,	NULL },	// 153 
	{ 0,0,	op_sec },	// 154/362 1001010000001000  SEC			C = 1 
	{ 0,0,	op_ijmp },	// 155/338 1001010000001001  IJMP			PC = Z 
	{ 0x0001,2,	NULL },	// 156 
	{ 0,0,	op_sen },	// 157/364 1001010000101000  SEN			N = 1 
	{ 0,0,	op_illegal },	// 158 Invalid Instruction 
	{ 0x0020,4,	NULL },	// 159 
	{ 0x0001,2,	NULL },	// 160 
	{ 0,0,	op_ses },	// 161/366 1001010001001000  SES			S = 1 
	{ 0,0,	op_illegal },	// 162 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 163 
	{ 0,0,	op_set },	// 164/368 1001010001101000  SET			T = 1 
	{ 0,0,	op_illegal },	// 165 Invalid Instruction 
	{ 0x0040,8,	NULL },	// 166 
	{ 0x0020,4,	NULL },	// 167 
	{ 0x0001,2,	NULL },	// 168 
	{ 0,0,	op_sez },	// 169/363 1001010000011000  SEZ			Z = 1 
	{ 0,0,	op_eijmp },	// 170/339 1001010000011001  EIJMP			PC = EIND:Z 
	{ 0x0001,2,	NULL },	// 171 
	{ 0,0,	op_sev },	// 172/365 1001010000111000  SEV			V = 1 
	{ 0,0,	op_illegal },	// 173 Invalid Instruction 
	{ 0x0020,4,	NULL },	// 174 
	{ 0x0001,2,	NULL },	// 175 
	{ 0,0,	op_seh },	// 176/367 1001010001011000  SEH			H = 1 
	{ 0,0,	op_illegal },	// 177 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 178 
	{ 0,0,	op_sei },	// 179/369 1001010001111000  SEI			I = 1 
	{ 0,0,	op_illegal },	// 180 Invalid Instruction 
	{ 0x0001,8,	NULL },	// 181 
	{ 0x0040,6,	NULL },	// 182 
	{ 0x0020,4,	NULL },	// 183 
	{ 0x0010,2,	NULL },	// 184 
	{ 0,0,	op_ret },	// 185/373 1001010100001000  RET			PC = DS(+STACK)		(Pops 2 or 3 bytes as appropriate to AVR architecture) 
	{ 0,0,	op_reti },	// 186/375 1001010100011000  RETI			PC = DS(+STACK),I = 1 
	{ 0,0,	op_reserved },	// 187/378 10010101001x1000  			Reserved 
	{ 0,0,	op_reserved },	// 188/379 1001010101xx1000  			Reserved 
	{ 0x0010,6,	NULL },	// 189 
	{ 0x0040,4,	NULL },	// 190 
	{ 0x0020,2,	NULL },	// 191 
	{ 0,0,	op_icall },	// 192/374 1001010100001001  ICALL Z		DS(STACK-) = PC, PC = Z	(Pushes 2 or 3 bytes) 
	{ 0,0,	op_illegal },	// 193 Invalid Instruction 
	{ 0,0,	op_illegal },	// 194 Invalid Instruction 
	{ 0x0040,4,	NULL },	// 195 
	{ 0x0020,2,	NULL },	// 196 
	{ 0,0,	op_eicall },	// 197/376 1001010100011001  EICALL Z		DS(STACK-) = PC, PC = EIND:Z 
	{ 0,0,	op_illegal },	// 198 Invalid Instruction 
	{ 0,0,	op_illegal },	// 199 Invalid Instruction 
	{ 0x0100,32,	NULL },	// 200 
	{ 0x0040,16,	NULL },	// 201 
	{ 0x0020,8,	NULL },	// 202 
	{ 0x0010,4,	NULL },	// 203 
	{ 0x0001,2,	NULL },	// 204 
	{ 0,0,	op_clc },	// 205/347 1001010010001000  CLC			C = 0 
	{ 0,0,	op_illegal },	// 206 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 207 
	{ 0,0,	op_clz },	// 208/348 1001010010011000  CLZ			Z = 0 
	{ 0,0,	op_illegal },	// 209 Invalid Instruction 
	{ 0x0010,4,	NULL },	// 210 
	{ 0x0001,2,	NULL },	// 211 
	{ 0,0,	op_cln },	// 212/349 1001010010101000  CLN			N = 0 
	{ 0,0,	op_illegal },	// 213 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 214 
	{ 0,0,	op_clv },	// 215/350 1001010010111000  CLV			V = 0 
	{ 0,0,	op_illegal },	// 216 Invalid Instruction 
	{ 0x0020,8,	NULL },	// 217 
	{ 0x0010,4,	NULL },	// 218 
	{ 0x0001,2,	NULL },	// 219 
	{ 0,0,	op_cls },	// 220/351 1001010011001000  CLS			S = 0 
	{ 0,0,	op_illegal },	// 221 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 222 
	{ 0,0,	op_clh },	// 223/352 1001010011011000  CLH			H = 0 
	{ 0,0,	op_illegal },	// 224 Invalid Instruction 
	{ 0x0010,4,	NULL },	// 225 
	{ 0x0001,2,	NULL },	// 226 
	{ 0,0,	op_clt },	// 227/353 1001010011101000  CLT			T = 0 
	{ 0,0,	op_illegal },	// 228 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 229 
	{ 0,0,	op_cli },	// 230/354 1001010011111000  CLI			I = 0 
	{ 0,0,	op_illegal },	// 231 Invalid Instruction 
	{ 0x0040,16,	NULL },	// 232 
	{ 0x0020,8,	NULL },	// 233 
	{ 0x0010,4,	NULL },	// 234 
	{ 0x0001,2,	NULL },	// 235 
	{ 0,0,	op_sleep },	// 236/381 1001010110001000  SLEEP						(Put CPU into sleep mode) 
	{ 0,0,	op_illegal },	// 237 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 238 
	{ 0,0,	op_break },	// 239/382 1001010110011000  BREAK						(Stop CPU, enter debug mode) 
	{ 0,0,	op_illegal },	// 240 Invalid Instruction 
	{ 0x0010,4,	NULL },	// 241 
	{ 0x0001,2,	NULL },	// 242 
	{ 0,0,	op_wdr },	// 243/383 1001010110101000  WDR						(Restart Watch Dog Timer) 
	{ 0,0,	op_illegal },	// 244 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 245 
	{ 0,0,	op_reserved },	// 246/384 1001010110111000  			Reserved  
	{ 0,0,	op_illegal },	// 247 Invalid Instruction 
	{ 0x0020,8,	NULL },	// 248 
	{ 0x0010,4,	NULL },	// 249 
	{ 0x0001,2,	NULL },	// 250 
	{ 0,0,	op_lpm_r0 },	// 251/283 1001010111001000  LPM			R0 = PS(Z) 
	{ 0,0,	op_illegal },	// 252 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 253 
	{ 0,0,	op_elpm_r0 },	// 254/287 1001010111011000  ELPM			R0 = PS(RAMPZ:Z) 
	{ 0,0,	op_illegal },	// 255 Invalid Instruction 
	{ 0x0010,4,	NULL },	// 256 
	{ 0x0001,2,	NULL },	// 257 
	{ 0,0,	op_spm },	// 258/385 1001010111101000  SPM						(See section 116 in "AVR Instruction Set Manual") 
	{ 0,0,	op_illegal },	// 259 Invalid Instruction 
	{ 0x0001,2,	NULL },	// 260 
	{ 0,0,	op_spm },	// 261/386 1001010111111000  SPM Z+ 					(See section 117 in "AVR Instruction Set Manual") 
	{ 0,0,	op_illegal },	// 262 Invalid Instruction 
	{ 0,0,	op_jmp },	// 263/398 1001010kkkkk110k kkkkkkkkkkkkkkkk  JMP k		PC = k 
	{ 0x0004,6,	NULL },	// 264 
	{ 0x0001,2,	NULL },	// 265 
	{ 0,0,	op_dec },	// 266/396 1001010ddddd1010  DEC Rd		Rd = Rd - 1 
	{ 0x0100,2,	NULL },	// 267 
	{ 0,0,	op_des },	// 268/371 10010100kkkk1011  DES k			Data Encryption Action 'k' 
	{ 0,0,	op_illegal },	// 269 Invalid Instruction 
	{ 0,0,	op_call },	// 270/399 1001010kkkkk111k kkkkkkkkkkkkkkkk  CALL k		DS(STACK-) = PC, PC = k 
	{ 0x0100,2,	NULL },	// 271 
	{ 0,0,	op_adiw },	// 272/410 10010110kkddkkkk  ADIW Rd,k		Rd+1:Rd += k		(where "d=24+(dd<<1)" and k is unsigned 6 bit value)  
	{ 0,0,	op_sbiw },	// 273/411 10010111kkddkkkk  SBIW Rp,k		Rd+1:Rd -= k		(where "d=24+(dd<<1)" and k is unsigned 6 bit value) 
	{ 0,0,	op_in },	// 274/418 10110aadddddaaaa  IN Rd,a		Rd = IO(a)		(Output register to IO address) 
	{ 0x2000,10,	NULL },	// 275 
	{ 0x0400,8,	NULL },	// 276 
	{ 0x0200,4,	NULL },	// 277 
	{ 0x0100,2,	NULL },	// 278 
	{ 0,0,	op_cbi },	// 279/413 10011000aaaaabbb  CBI a,b		IO(a)[b] = 0		(Clear bit b in IO register a) 
	{ 0,0,	op_sbic },	// 280/414 10011001aaaaabbb  SBIC a,b		If IO(a)[b] == 0 skip	(Skip next instruction if bit b in IO register a is clear) 
	{ 0x0100,2,	NULL },	// 281 
	{ 0,0,	op_sbi },	// 282/415 10011010aaaaabbb  SBI a,b		IO(a)[b] = 1		(Set bit b in IO register a) 
	{ 0,0,	op_sbis },	// 283/416 10011011aaaaabbb  SBIS a,b		If IO(a)[b] == 1 skip	(Skip next instruction if bit b in IO register a is set) 
	{ 0,0,	op_mul },	// 284/417 100111rdddddrrrr  MUL Rd,Rr		R1:R0 = Rr x Rd		(unsigned = unsigned x unsigned) 
	{ 0,0,	op_out },	// 285/419 10111aadddddaaaa  OUT a,Rd		IO(a) = Rd		(Read register from IO address) 
	{ 0x8000,8,	NULL },	// 286 
	{ 0x2000,4,	NULL },	// 287 
	{ 0x1000,2,	NULL },	// 288 
	{ 0,0,	op_sbci },	// 289/276 0100KKKKddddKKKK  SBCI Rd,K		Rd = Rd - K - C		(Subtract 8-bit immediate and Carry) 
	{ 0,0,	op_subi },	// 290/277 0101KKKKddddKKKK  SUBI Rd,K		Rd = Rd - K		(Subtract 8-bit immediate) 
	{ 0x1000,2,	NULL },	// 291 
	{ 0,0,	op_ori },	// 292/278 0110KKKKddddKKKK  ORI Rd,K		Rd = Rd | K		(Or with 8-bit immediate. Also: SBR Rd,K) 
	{ 0,0,	op_andi },	// 293/279 0111KKKKddddKKKK  ANDI Rd,K		Rd = Rd & K		(And with 8-bit immediate. Also: CBR Rd,~K) 
	{ 0x2000,4,	NULL },	// 294 
	{ 0x1000,2,	NULL },	// 295 
	{ 0,0,	op_rjmp },	// 296/421 1100jjjjjjjjjjjj  RJMP j		PC = PC + j		(Relative jump, j is signed 12 bit immediate) 
	{ 0,0,	op_rcall },	// 297/422 1101jjjjjjjjjjjj  RCALL j		DS(STACK-) = PC, PC = PC + j (Relative call, j is signed 12 bit immediate) 
	{ 0x1000,2,	NULL },	// 298 
	{ 0,0,	op_ldi },	// 299/423 1110KKKKddddKKKK  LDI Rd,K		Rd = K			(Load register with 8 bit immediate) 
	{ 0x0800,32,	NULL },	// 300 
	{ 0x0400,16,	NULL },	// 301 
	{ 0x0004,8,	NULL },	// 302 
	{ 0x0002,4,	NULL },	// 303 
	{ 0x0001,2,	NULL },	// 304 
	{ 0,0,	op_brcs },	// 305/440 111100jjjjjjj000  BRCS/BRLO j		if C == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brzs },	// 306/441 111100jjjjjjj001  BRZS/BREQ j		if Z == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 307 
	{ 0,0,	op_brns },	// 308/442 111100jjjjjjj010  BRNS/BRMI j		if N == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brvs },	// 309/443 111100jjjjjjj011  BRVS j		if V == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0002,4,	NULL },	// 310 
	{ 0x0001,2,	NULL },	// 311 
	{ 0,0,	op_brss },	// 312/444 111100jjjjjjj100  BRSS/BRLT j		if S == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brhs },	// 313/445 111100jjjjjjj101  BRHS j		if H == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 314 
	{ 0,0,	op_brts },	// 315/446 111100jjjjjjj110  BRTS j		if T == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_bris },	// 316/447 111100jjjjjjj111  BRIS/BRIE j		if I == 1 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0004,8,	NULL },	// 317 
	{ 0x0002,4,	NULL },	// 318 
	{ 0x0001,2,	NULL },	// 319 
	{ 0,0,	op_brcc },	// 320/464 111101jjjjjjj000  BRCC/BRSH j		if C == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brzc },	// 321/465 111101jjjjjjj001  BRZC/BRNE j		if Z == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 322 
	{ 0,0,	op_brnc },	// 323/466 111101jjjjjjj010  BRNC/BRPL j		if N == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brvc },	// 324/467 111101jjjjjjj011  BRVC j		if V == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0002,4,	NULL },	// 325 
	{ 0x0001,2,	NULL },	// 326 
	{ 0,0,	op_brsc },	// 327/468 111101jjjjjjj100  BRSC/BRGE j		if S == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_brhc },	// 328/469 111101jjjjjjj101  BRHC j		if H == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0001,2,	NULL },	// 329 
	{ 0,0,	op_brtc },	// 330/470 111101jjjjjjj110  BRTC j		if T == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0,0,	op_bric },	// 331/471 111101jjjjjjj111  BRIC/BRID j		if I == 0 then PC = PC + j	(j is 7-bit signed offset) 
	{ 0x0008,8,	NULL },	// 332 
	{ 0x0400,4,	NULL },	// 333 
	{ 0x0200,2,	NULL },	// 334 
	{ 0,0,	op_bld },	// 335/473 1111100ddddd0bbb  BLD Rd,b		Rd[b] = T 
	{ 0,0,	op_bst },	// 336/474 1111101ddddd0bbb  BST Rd,b		T = Rd[b] 
	{ 0x0200,2,	NULL },	// 337 
	{ 0,0,	op_sbrc },	// 338/475 1111110ddddd0bbb  SBRC Rd,b		Skip next if Rd[b] == 0 
	{ 0,0,	op_sbrs },	// 339/476 1111111ddddd0bbb  SBRS Rd,b		Skip next if Rd[b] == 1 
	{ 0,0,	op_reserved } 	// 340/477 11111xxddddd1bbb  			Reserved 
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
