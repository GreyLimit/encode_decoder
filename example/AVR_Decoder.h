//
//	AVR_Decoder.h
//

#ifndef _AVR_DECODER_H_
#define _AVR_DECODER_H_

//
//	Include the other definitions we need.
//
#include "Base.h"
#include "AVR.h"

//
//	Define a type to be used to handle the
//	pointers to functions we are storing
//	and returning.
//
typedef void (*function_code)( AVR_State *state );

//
//	Declare the look up functions (whose code
//	is included in the instructions file.
//
extern function_code decode_opcode( word opcode );

#endif

//
//	EOF
//
