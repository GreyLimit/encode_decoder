# encode_decoder
A rough-n-ready program to generate a table to enable software to decode CPU instructions

I'm not putting a great deal of effort into publishing this as I wrote it to support me in writinng another program.  But here goes:-

This program was throwwn together to facilitate generating a 'decision' table that would enable, with minimal(ish) space overhead, software to decode a CPU binary opcode back to a known instruction.  This being a core part of any CPU/MCU simulation.

For example when supplied the following input data:
```
{S8
{F op_
{I 00000000 nop
{I 0001.... add
{I 0010.... sub
{I 01...... save
{I 10...... load
{I 11...... x
```
it generates the following otput:
```
decoder_t decoder[ 11 ] = {
  { 0, 0x80,8,	NULL },	/*   0 */
  { 0, 0x40,6,	NULL },	/*   1 */
  { 0, 0x20,4,	NULL },	/*   2 */
  { 0, 0x10,2,	NULL },	/*   3 */
  { 0, 0,0,	op_nop },	/*   4[  3] 00000000 [16]  */
  { 0, 0,0,	op_add },	/*   5[  4] 0001....  */
  { 0, 0,0,	op_sub },	/*   6[  5] 0010.... [2]  */
  { 0, 0,0,	op_save },	/*   7[  6] 01......  */
  { 0, 0x40,2,	NULL },	/*   8 */
  { 0, 0,0,	op_load },	/*   9[  7] 10......  */
  { 0, 0,0,	op_x } 	/*  10[  8] 11......  */
};
```
So what does this mean? Quick summary (before longer one follows).

Data interpreted by the program starts after a brace (`{`) and runs to the end of the line or a corresponding close brace (`}`) unless the close brace is escaped (`\}`).  This forms a single record.

The first character after the open brace indicates the content of the record, and determines the format for the content of the record.

`{S n}` tells the program how many bits (n) form a unit of opcode data.  This will typically be either 8 or 16, but can be any modest positive number.

`{F f}` tells the program how to decorate/format the name when outputting it.

`{I c c c ... n}` provides a binary description of a single instruction where 'c' (repeated as required) is the binary description of the opcode, each element with a number of bits as specified by the S record.  Where bits are arguments to the opcode then letters or periods MUST be used instead of '0' or '1'.

So what does it output?  It outputs a binary decision tree enabling a reasonably efficient lookup of the actual instruction.  The fields in each record represent the index number of the opcode element to be checked, the bit mask giving the bit to be checked in that word, the number of records to 'move down' if the bit tested is 1, and finally an instruction name.  If the bit tested was 0 then simply move to the next record.  A leaf node (ie a successful instruction decode) is indicated by the bit mask being zero.

This system means a pointer can be placed at the head of the table and a simple loop, moving the pointer forward by a know amount at each step, will eventually find what it is looking for.

Code something like this pseudo code would do the task:
```
instruction *find( decoder_t *table, word *opcode ) {
  decoder_t *ptr;
  word test;
  ptr = table;
  while(( test = ptr->mask )) ptr += ( opcode[ ptr->index ] & test )? ptr->is_one: 1;
  return ptr->inst;
}
```

The following is taken for the source code as a more complete description of the record formats supported (note W record affects the number of fields in the table output):

```
/*
 *	The input stream has the following format definition.
 *
 *	o	Only data following an open brace (to the end of the line
 *		or a close brace) is considered 'data'.  Everything
 *		else is considered a comment.
 *
 *	o	The character immediately following the open brace
 *		provides the type of the data being supplied in
 *		the rest of the data item.
 *
 *	Input data types:
 *
 *	S	Provide the number of bits which form an instruction
 *		unit (typically 8 or 16).
 *
 * 			{S 8}
 *
 * 		Only ONE per instruction set, must be set before first
 *		instruction definition.
 *
 *	I	Provide details of an instruction definition.  This is
 *		a series of binary numbers, each of S bits long separated
 *		by white space.  The final word, not a binary number and
 *		not needing to be S bits long, is the name of the instruction.
 *		This is not case sensitive, and is effectively passed through
 *		to the output without interpretation.
 *
 * 		Where bits of the instruction are arguments to the instruction
 *		(and are therefore not actually part of the instruction) they
 *		should be marked with a period.
 *
 *			{I 00000000 NOP}
 * 		or
 *			{I 0101.... ADD}
 *
 * 	F	Provide formatting details for outputting instruction names.
 *		In the absence of a percent symbol this is taken as a prefix,
 * 		where a percent symbol is provided then this is where the
 * 		instruction name will be inserted in the output.
 *
 * 			{F opcode_%_func}
 *
 * 		Optional, but only definition per instruction set.  Any white
 *		spaces are automatically removed.
 *
 * 	T	Provide the name of the array type, defaults "decoder_t".
 *
 * 	N	Provide the name of the array (of the above type), defaults
 * 		"decoder".
 *
 * 	L	Language selection:
 *
 * 			{L C}			Select C
 * 		or
 * 			{L C++}			Select C++
 * 			{L CPP}
 *
 * 	E	provide the name of a routine to be placed into the decoding
 * 		tree in the event that decoding does not reach a formal
 *		instruction (defaults "illegal")
 *
 * 			{E illegal_inst}
 *
 * 	W	Define the maximum number of words required to determine the
 * 		target instruction.
 *
 * 		IF this is specified as 1 then the output table will NOT
 * 		include a word index (assuming it to be 0 always).
 *
 *			{W 1}			Index not output
 * 		or
 * 			{W 2}			Index output
 *
 * 	[space]
 * 	[tab]	Content of the record is passed through to the output "as is"
 * 		before the content of the table is generated.
 */
```
