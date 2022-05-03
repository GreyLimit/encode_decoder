/*
 *	encode_decoder
 *	==============
 *
 *	Program to process an input stream of Assembly Language
 *	definitions and output C/C++ source code providing a
 *	mechanism to decode those instructions.
 *
 *	Usage:
 *		encode_decoder < input_stream > output_stream
 *	or
 *		encode_decoder input_file[.suffix]
 */

#include <stdio.h>

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
 *	Record Commands:
 *
 *	Z	Provide the number of bits which define the size of an
 *		instruction (typically 8 or 16).
 *
 * 			{Z 8}
 *
 * 		Only one per instruction set; must be set before first
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
 *		should be marked with a period or letter.
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
 * 		When no F record has been defined then each record has only
 *		a single result value (being the instruction named in the I record).
 *
 *		If multiple F records are provided then a corresponding number
 *		of result values are created with the decoding data structure.
 *
 * 	T	Provide the name of the array type, defaults "decoder_t".
 *
 *	S	Define the scope of the table, defaults to 'static'.
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
 * 		Given the ability of the pre-processor to output text to
 *		the target files ahead of the decoding data, it is required
 *		to make this the first record at head of the input data.
 *
 * 	E	provide the name of a routine to be placed into the decoding
 * 		tree in the event that decoding does not reach a formal
 *		instruction.
 *
 *		If not defined then decoding stops at the nearest valid opcode
 *		and the output table includes details of where the ambiguity is.
 *
 * 			{E illegal_inst}
 *
 * 	W	Define the maximum number of words required to determine a
 * 		target instruction.
 *
 * 		If this is specified as 1 then the output table will NOT
 * 		include a word index (assuming it always be 0).
 *
 *			{W 1}			No index output
 * 		or
 * 			{W 2}			Index output
 *
 * 	[space]
 * 	[tab]	Content of the record is passed through to the output "as is"
 * 		before the content of the table is generated.
 *
 * 	[Underscore]
 *	_	Content of the record is passed through to the output "as is"
 * 		AFTER the content of the table is generated.
 *
 *	H	Content is passed into the header file with the file name
 *		either based on the input file (with '.h' applied) or simply
 *		also sent to stdout with the other output.
 *
 * 	Block Commands:
 *
 * 		These are Record Commands which are used to bracket a number
 * 		of lines for pass through to one of the header files, the start
 * 		of the source file or the tail of the source file.
 *
 * 	BS	Used to indicate following line belong at the Start of the
 *		source file.
 *
 * 	BE	Used to indicate following line belong at the End of the
 *		source file.
 *
 * 	BH	Used to indicate following lines belong in the header file.
 *
 * 	BC	Used to indicate following lines are only comments and are
 * 		to be skipped.
 *
 * 	B	Used to indicate the end of a block of lines.
 *
 * 	The initial implementation of the block commands had a simple "you're
 * 	either in a block, or you're not" mechanism.  Starting a new block
 * 	with one of BS, BE, BH or BC ended the previous block and started the
 * 	next.
 *
 * 	This has been modified (as well as BF becoming simply B) and now blocks
 * 	can be nested.  Consequently for *every* block starting command there
 * 	must now be a corresponding block ending command.
 *
 * 	This has been done to simplify the mechanism through which header files,
 * 	source files and block comments are handled.  I believe.
 */

/*
 *	Provide constant names for the characters above that have specific
 *	meanings.
 */
#define BEGIN_RECORD		'{'
#define END_RECORD		'}'
#define SIZE_RECORD		'Z'
#define INSTRUCTION_RECORD	'I'
#define FORMAT_RECORD		'F'
#define TYPE_RECORD		'T'
#define SCOPE_RECORD		'S'
#define NAME_RECORD		'N'
#define LANGUAGE_RECORD		'L'
#define ERROR_RECORD		'E'
#define WORDS_RECORD		'W'
#define HEADER_RECORD		'H'

#define BLOCK_RECORD		'B'
#define BLOCK_START		'S'
#define BLOCK_END		'E'
#define BLOCK_HEADER		'H'
#define BLOCK_COMMENT		'C'

#define INSERT_HERE		'%'
#define ONE_BIT			'1'
#define ZERO_BIT		'0'
#define ARGUMENT_BIT		'.'
#define PERIOD			'.'
#define SPACE			' '
#define TAB			'\t'
#define UNDERSCORE		'_'
#define NL			'\n'
#define EOS			'\0'
#define ESCAPE_SYMBOL		'\\'
#define QUESTION		'?'

/*
 *	defines that shape the output of unmatched bits.
 */
#define PLACE_PATTERN		0x88888888
#define PLACE_MARK		'+'
#define PLACE_GAP		'-'
#define PLACE_VARIABLE		'?'


/*
 *	Define simplistic memory allocation routines
 */
#include <malloc.h>
#define NEW(t)	((t *)malloc(sizeof(t)))
#define FREE(p)	free(( void *)(p))

/*
 *	String ops.
 */
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#define DUP(s) strdup(s)

/*
 *	Define an internal type used to handle the bits bits.
 */
#include <stdint.h>
typedef uint32_t word;

/*
 *	Define a logical value.
 */
typedef int bool;
#define TRUE (0==0)
#define FALSE (0==1)

/*
 *	Define the maximum number of words an instruction can
 *	be composed from.
 */
#define MAX_CODES	16

/*
 *	Define the largest input line we are happy to handle.
 */
#define MAX_BUFFER	256

/*
 *	Define the maximum number of formats which the program
 *	will handle.
 */
#define MAX_FORMATS	8

/*
 *	Define the data structure used to capture a single instruction
 */
#define INSTRUCTION struct instruction
INSTRUCTION {
	/*
	 *	Source line number.
	 */
	int		line;
	/*
	 *	Capture the input data describing the instruction.
	 */
	char		*name,				/* Given instruction name			*/
			*comment;			/* Additional commentary provided		*/ 
	int		elements;			/* Number of words forming the instruction	*/
	word		opcode[ MAX_CODES ],		/* The instruction construction.		*/
			mask[ MAX_CODES ];		/* Indicate those bits which are instruction	*/
	char		*description[ MAX_CODES ];	/* Copy of the original bit description		*/
	int		matches;			/* How many versions of this instruction?	*/
	word		unmatched[ MAX_CODES ];		/* The mask giving the bits that are undefined. */
	/*
	 *	We will daisy chain the records together
	 *	as a flexible mechanism for keeping them
	 *	together.
	 */
	INSTRUCTION	*next;
};

/*
 *	This is the data structure used to capture the decoding tree.
 */
#define NODE struct node
NODE {
	/*
	 *	These are the output table index numbers once
	 *	initial processing has been completed.
	 */
	int		index;
	/*
	 *	Provide mechanism to link multiple
	 *	instructions into a data structure
	 *	that can decode instructions.
	 */
	bool		leaf;			/* True if this is a leaf node (and		*/
						/* therefore specifies the instruction		*/
						/* below).					*/
	/*
	 *	If leaf TRUE...
	 */
	INSTRUCTION	*decoded;
	
	/*
	 *	If leaf FALSE...
	 *
	 *	then the follow data is used to distinguish between the
	 *	two branches from this node.
	 */
	int		op_word,		/* Which instruction word are we comparing?	*/
			op_bit;			/* Which bit in that word?			*/
	NODE		*zero,
			*one;
};

/*
 *	Define a record to hold those lines of data which need
 *	to be output AFTER the table has been created.
 */
#define FINISH struct finish
FINISH {
	int	line;
	char	*data;
	FINISH	*next;
};

/************************************************
 *						*
 *	GLOBAL VARIABLES DEFINED HERE		*
 *						*
 ************************************************/

/*
 *	Define the output streams for the program result.
 */
static char		*input_source_file = NULL,
			*output_base_name = NULL,
			*output_source_name = NULL,
			*output_header_name = NULL;
static FILE		*output_source = NULL,
			*output_header = NULL;

/*
 *	Define word size and enabled flag.
 */
static bool		word_size_set = FALSE;
static int		word_size = 0;

/*
 *	Define the maximum number of words required
 * 	to identify an instruction uniquwly.
 */
static bool		maximum_words_set = FALSE;
static int		maximum_words = MAX_CODES;

/*
 *	Define the output formatting; a in front
 *	b afterwards.
 */
static int		output_formats = 0;
static char		*output_format_a[ MAX_FORMATS ],
			*output_format_b[ MAX_FORMATS ];

/*
 *	Define the comment output formatting.
 */
static char		*output_comment_a = NULL,
			*output_comment_b = NULL;

/*
 *	Define the name of the error routine
 */
static char		*error_handler = NULL;

/*
 *	Output data type record and decode name.
 */
static char		*data_type = NULL;
static char		*data_scope = NULL;
static char		*data_name = NULL;

/*
 *	The tail/finish data to be output after
 * 	the table.
 */
static FINISH		*finish_data = NULL,
			**finish_data_tail = &( finish_data );

/*
 *	All the instructions can be found here.
 */
static INSTRUCTION	*instructions = NULL,
			**instruction_tail = &( instructions );

/*
 *	The decode tree is found here
 */
static NODE		*tree = NULL;

/*
 *	Instruction DROP count.
 */
static int		dropped = 0;

/*
 *	Enumeration with variable for tracking where the last line of output went
 */
static enum {
	UNSPECIFIED_TARGET,
	SOURCE_TARGET,
	HEADER_TARGET
} output_target = UNSPECIFIED_TARGET;

/*
 *	Enumeration with variable for tracking the block mode in force.
 */
enum block_modes {
	LINE_MODE,
	START_MODE,
	END_MODE,
	HEADER_MODE,
	COMMENT_MODE
};

#define BLOCK_STACK struct block_stack
BLOCK_STACK  {
	enum block_modes	mode;
	int			line;
	BLOCK_STACK		*prev;
};

static BLOCK_STACK	*root_block_stack = NULL,
			*spare_block_stack = NULL;

void push_mode( enum block_modes mode, int line ) {
	BLOCK_STACK *ptr;

	if(( ptr = spare_block_stack )) {
		spare_block_stack = ptr->prev;
	}
	else {
		ptr = NEW( BLOCK_STACK );
	}
	ptr->mode = mode;
	ptr->line = line;
	ptr->prev = root_block_stack;
	root_block_stack = ptr;
}

int pop_mode( void ) {
	BLOCK_STACK *ptr;
	
	if(( ptr = root_block_stack )) {
		root_block_stack = ptr->prev;
		ptr->prev = spare_block_stack;
		spare_block_stack =  ptr;
		return( TRUE );
	}
	return( FALSE );
}


/************************************************
 *						*
 *	IMPLEMENTATION ROUTINES			*
 *						*
 ************************************************/

/*
 *	Is a character a visible one?
 */
static bool isvisible( int c ) {
	return(( c > SPACE )&&( c < 127 ));
}

/*
 *	Is a character a valid part of an opcode?
 */
static bool isopcode( int c ) {
	return(( c == ZERO_BIT )||( c == ONE_BIT )||( c == ARGUMENT_BIT )||( isalpha( c )));
}

/*
 *	Output filename construction
 */
static char *strcatdup( char *a, const char *b ) {
	int l = strlen( a ) + strlen( b ) + 1;

	char *r = (char *)malloc( l );

	strcpy( r, a );

	return( strcat( r, b ));
}

/*
 *	Process a line of input.
 */
static bool process( int line, char *input, char *comment ) {
	char	record;
	
	switch(( record = *input++ )) {
		case SIZE_RECORD: {
			int	i;

			/*
			 *	S nnn	Provide number of bits per word of instruction
			 */
			output_target = UNSPECIFIED_TARGET;
			i = atoi( input );
			if(( i <= 0 )||( i > ( sizeof( word ) << 3 ))) {
				fprintf( stderr, "Line %d: Invalid word size %d.\n", line, i );
				return( FALSE );
			}
			if( word_size_set ) {
				fprintf( stderr, "Line %d: Cannot reset word size.\n", line );
				return( FALSE );
			}	
			word_size = i;
			word_size_set = TRUE;
			break;
		}
		case WORDS_RECORD: {
			int	i;

			/*
			 *	W nnn	Provide maximum number of words required to
			 *		identify an instruction.
			 */
			output_target = UNSPECIFIED_TARGET;
			i = atoi( input );
			if(( i <= 0 )||( i > MAX_CODES )) {
				fprintf( stderr, "Line %d: Invalid number of words %d.\n", line, i );
				return( FALSE );
			}
			if( maximum_words_set ) {
				fprintf( stderr, "Line %d: Cannot reset number of words.\n", line );
				return( FALSE );
			}	
			maximum_words = i;
			maximum_words_set = TRUE;
			break;
		}
		case FORMAT_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	F ffff[%ffff]
			 */
			output_target = UNSPECIFIED_TARGET;
			if( output_formats >= MAX_FORMATS ) {
				fprintf( stderr, "Line %d: Too many output formats specified (maximum is %d).\n", line, MAX_FORMATS );
				return( FALSE );
			}
			/*
			 *	Strip spaces...
			 */
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No output format found.\n" );
				return( FALSE );
			}
			if(( p = strchr( input, INSERT_HERE ))) {
				*p++ = EOS;
				output_format_a[ output_formats ] = DUP( input );
				output_format_b[ output_formats ] = DUP( p );
			}
			else {
				output_format_a[ output_formats ] = DUP( input );
				output_format_b[ output_formats ] = "";
			}
			output_formats++;
			break;
		}
		case LANGUAGE_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	L language
			 */
			output_target = UNSPECIFIED_TARGET;
			if(( output_comment_a != NULL )||( output_comment_b != NULL )) {
				fprintf( stderr, "Output comment already specified.\n" );
				return( FALSE );
			}
			/*
			 *	Strip spaces...
			 */
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No language found.\n" );
				return( FALSE );
			}
			/*
			 *	We have to remember to set up the output files, but only
			 *	if the base name is non-NULL.
			 */
			if( strcasecmp( input, "c" ) == 0 ) {
				/*
				 *	C Language
				 */
				output_comment_a = "/*";
				output_comment_b = "*/";
				if( output_base_name ) {
					output_header_name = strcatdup( output_base_name, ".h" );
					output_source_name = strcatdup( output_base_name, ".c" );
					if(( output_header = fopen( output_header_name, "w" )) == NULL ) {
						fprintf( stderr, "Unable to open header file '%s'.\n", output_header_name );
						return( FALSE );
					}
					if(( output_source = fopen( output_source_name, "w" )) == NULL ) {
						fprintf( stderr, "Unable to open header file '%s'.\n", output_source_name );
						return( FALSE );
					}
				}
			}
			else {
				if(( strcasecmp( input, "c++" ) == 0 )||( strcasecmp( input, "cpp" ) == 0 )) {
					/*
					 *	C++ Language
					 */
					output_comment_a = "//";
					output_comment_b = "";
					if( output_base_name ) {
						output_header_name = strcatdup( output_base_name, ".h" );
						output_source_name = strcatdup( output_base_name, ".cpp" );
						if(( output_header = fopen( output_header_name, "w" )) == NULL ) {
							fprintf( stderr, "Unable to open header file '%s' for write.\n", output_header_name );
							return( FALSE );
						}
						if(( output_source = fopen( output_source_name, "w" )) == NULL ) {
							fprintf( stderr, "Unable to open source file '%s' for write.\n", output_source_name );
							return( FALSE );
						}
					}
				}
				else {
					fprintf( stderr, "Unrecognised language '%s'.\n", input );
					return( FALSE );
				}	
			}
			break;
		}
		case TYPE_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	Strip spaces...
			 */
			output_target = UNSPECIFIED_TARGET;
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No data type found.\n" );
				return( FALSE );
			}
			if( data_type ) {
				fprintf( stderr, "Data type already set.\n" );
			}
			data_type = DUP( input );
			break;
		}
		case SCOPE_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	Strip spaces...
			 */
			output_target = UNSPECIFIED_TARGET;
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No scope found.\n" );
				return( FALSE );
			}
			if( data_scope ) {
				fprintf( stderr, "Scope already set.\n" );
			}
			data_scope = DUP( input );
			break;
		}
		case NAME_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	Strip spaces...
			 */
			output_target = UNSPECIFIED_TARGET;
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No data name found.\n" );
				return( FALSE );
			}
			if( data_name ) {
				fprintf( stderr, "Data name already set.\n" );
			}
			data_name = DUP( input );
			break;
		}
		case ERROR_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	Strip spaces...
			 */
			output_target = UNSPECIFIED_TARGET;
			p = input;
			while( *p != EOS ) {
				if( isvisible( *p )) {
					p++;
				}
				else {
					/*
					 *	Roll out the white space.
					 */
					q = p;
					r = p+1;
					while(( *q++ = *r++ ));
				}
			}
			if( *input == EOS ) {
				fprintf( stderr, "No error handler found.\n" );
				return( FALSE );
			}
			if( error_handler ) {
				fprintf( stderr, "Error handler already set.\n" );
			}
			error_handler = DUP( input );
			break;
		}
		case SPACE:
		case TAB: {
			/*
			 *	Pass through "as is".
			 */
			if( output_target != SOURCE_TARGET ) {
				output_target = SOURCE_TARGET;
				fprintf( output_source, "#line %d \"%s\"\n", line, input_source_file );
			}
			fprintf( output_source, "%s\n", input );
			break;
		}
		case EOS: {
			/*
			 *	Special case of "pass through": a
			 *	record start symbol as the last
			 *	character in a line.
			 */
			if( output_target != SOURCE_TARGET ) {
				output_target = SOURCE_TARGET;
				fprintf( output_source, "#line %d \"%s\"\n", line, input_source_file );
			}
			fprintf( output_source, "\n" );
			break;
		}
		case UNDERSCORE: {
			/*
			 *	Add more data to the end of the file.
			 */
			output_target = UNSPECIFIED_TARGET;
			FINISH *ptr = NEW( FINISH );
			ptr->line = line;
			ptr->data = DUP( input );
			ptr->next = NULL;
			*finish_data_tail = ptr;
			finish_data_tail = &( ptr->next );
			break;
		}
		case HEADER_RECORD: {
			/*
			 *	Pass through "as is".
			 */
			if( output_target != HEADER_TARGET ) {
				output_target = HEADER_TARGET;
				fprintf( output_header, "#line %d \"%s\"\n", line, input_source_file );
			}
			fprintf( output_header, "%s\n", input );
			break;
		}
		case INSTRUCTION_RECORD: {
			INSTRUCTION	*p;

			/*
			 *	Start with new empty record, and link it in.
			 */
			output_target = UNSPECIFIED_TARGET;
			p = NEW( INSTRUCTION );
			p->line = line;
			p->name = NULL;
			p->comment = DUP( comment );
			p->elements = 0;
			for( int i = 0; i < MAX_CODES; p->opcode[ i++ ] = 0 );
			for( int i = 0; i < MAX_CODES; p->mask[ i++ ] = 0 );
			for( int i = 0; i < MAX_CODES; p->description[ i++ ] = NULL );
			for( int i = 0; i < MAX_CODES; p->unmatched[ i++ ] = 0 );
			p->matches = 0;
			p->next = NULL;
			*instruction_tail = p;
			instruction_tail = &( p->next );
			/*
			 *	Fill in the record; start by breaking the input up into
			 *	space separated units (but still ignoring initial spaces).
			 */
			while(( *input )&&( !isvisible( *input ))) input += 1;
			while( *input ) {
				char	*e;
				int	count;

				/*
				 *	Find end of this word..
				 */
				for( e = input; isvisible( *e ); e++ );
				/*
				 *	Mark end of word as long as it is not the
				 * 	end of the input.
				 */
				if( *e ) {
					*e++ = EOS;
					while(( *e )&&( !isvisible( *e ))) e++;
				}
				/*
				 *	Instruction data or instruction name?  Count
				 *	instruction characters first...
				 */
				count = 0;
				for( char *l = input; *l; l++ ) if( isopcode( *l )) count++;
				/*
				 *	If count is same as string length and this is
				 *	not the last word in the record, then this should
				 *	be an opcode word.
				 */
				if(( count == strlen( input ))&&( *e != EOS )) {
					int	i;
					
					if( count != word_size ) {
						fprintf( stderr, "Found instruction size %d (word size is %d).\n", count, word_size );
						return( FALSE );
					}
					/*
					 *	Can we add this to the instruction record?
					 */
					if( p->elements >= MAX_CODES ) {
						fprintf( stderr, "Found instruction too big (maximum %d words).\n", MAX_CODES );
						return( FALSE );
					}
					/*
					 *	Fill in instruction data.
					 */
					i = p->elements++;
					for( char *l = input; *l; l++ ) {
						switch( *l ) {
							case ONE_BIT: {
								p->opcode[ i ] = ( p->opcode[ i ] << 1 ) | 1;
								p->mask[ i ] = ( p->mask[ i ] << 1 ) | 1;
								break;
							}
							case ZERO_BIT: {
								p->opcode[ i ] = ( p->opcode[ i ] << 1 ) | 0;
								p->mask[ i ] = ( p->mask[ i ] << 1 ) | 1;
								break;
							}
							default: {
								p->opcode[ i ] = ( p->opcode[ i ] << 1 ) | 0;
								p->mask[ i ] = ( p->mask[ i ] << 1 ) | 0;
								break;
							}
						}
					}
					p->description[ i ] = DUP( input );
				}
				else {
					/*
					 *	This should be the last word on the line (ie *e should point
					 *	to the EOS character.
					 */
					if( *e ) {
						fprintf( stderr, "Instruction name '%s' not last word on line\n", input );
						return( FALSE );
					}
					/*
					 *	There SHOULD be some identifier verification at this point.
					 *
					 *	.. eventually.
					 */
					p->name = DUP( input );
				}
				/*
				 *	Move input to, the start of the next text of interest.
				 */
				input = e;
			}
			break;
		}
		default: {
			output_target = UNSPECIFIED_TARGET;
			if( isvisible( record )) {
				fprintf( stderr, "Invalid record identifier '%c'.\n", record );
			}
			else {
				fprintf( stderr, "Invalid record identifier ascii code %d.\n", (int)record );
			}
			return( FALSE );
		}
	}
	return( TRUE );
}

/*
 *	Recursive tree building routine.
 *
 *	Create tree using a binary division mechanism
 */
static NODE *insert( word *mask, INSTRUCTION *list, int count ) {
	int		w,
			b,
			d;
	bool		v;
	word		t;
	NODE		*here;
	INSTRUCTION	*ptr,
			*ones,
			*zeros;
	int		c0,
			c1;

	/*
	 *	Deal with up front cases first.
	 */
	if( count == 0 ) {
		/* Error Leaf node time! */
		here = NEW( NODE );
		here->index = 0;
		here->leaf = TRUE;
		here->decoded = NULL;
		here->op_word = 0;
		here->op_bit = 0;
		here->zero = NULL;
		here->one = NULL;
		return( here );
	}
	if( count == 1 ) {
		/*
		 *	Brief duplication check:  Getting here should mean that
		 *	the AND result of the mask value passed in and the mask
		 * 	value of the instruction results in 0.  If there are n
		 * 	bits still set then that instruction is accessible through
		 * 	2^n alternative opcodes.
		 */
		count = 0;
		for( int i = 0; i < MAX_CODES; i++ ) {
			list->unmatched[ i ] = t = mask[ i ] & list->mask[ i ];
			for( int j = 0; j < word_size; j++ ) {
				if( t & 1 ) count++;
				t >>= 1;
			}
		}
		/*
		 *	How are we handling error/illegal instructions?
		 */
		if( count && error_handler ) {
			/*
			 *	Error handler defined, so we have to (carefully)
			 *	force the system to carry on building the
			 *	decision tree.
			 *
			 *	All we need to do here is find the first ambiguous
			 *	opcode bit, decide which side this instruction goes,
			 *	for a decision node and repeat recursive call.
			 *
			 *	We know that at least one of the unmatched[] elements
			 *	is non-zero.  That is our target opcode word.
			 */
			for( int i = 0; i < MAX_CODES; i++ ) {
				if( list->unmatched[ i ]) {
					/*
					 *	Word i is where we make the decision.
					 *
					 *	step j through bits from msb to lsb.
					 */
					int j = word_size;
					while( j-- ) {
						word t = 1 << j;
						if( list->unmatched[ i ] & t ) {
							/*
							 *	Ambiguous bit found, create new node.
							 */
							here = NEW( NODE );
							here->index = 0;
							here->leaf = FALSE;
							here->decoded = NULL;
							here->op_word = i;
							here->op_bit = j;
							mask[ i ] &= ~t;
							if( list->opcode[ i ] & t ) {
								/* Pick the ONE side. */
								here->zero = insert( mask, NULL, 0 );
								here->one = insert( mask, list, 1 );
							}
							else {
								 /* Pick the ZERO size. */
								here->zero = insert( mask, list, 1 );
								here->one = insert( mask, NULL, 0 );
							}
							mask[ i ] |= t;
							return( here );
						}
					}
				}
			}
			/*
			 *	If we fall out here then this IS a programming error.
			 */
			fprintf( stderr, "Coding error '%s' line %d.\n", __FILE__, __LINE__ );
			exit( -1 );
		}
		/*
		 *	No Error handler has been defined, so output this node
		 *	with the duplication information attached to it.
		 */
		list->matches = 1 << count;
		/* Leaf node time! */
		here = NEW( NODE );
		here->index = 0;
		here->leaf = TRUE;
		here->decoded = list;
		here->op_word = 0;
		here->op_bit = 0;
		here->zero = NULL;
		here->one = NULL;
		return( here );
	}
	/*
	 *	The list is multiple options, so...
	 * 
	 *	Find the word and bit which we will be using to
	 *	split the list into two parts.
	 *
	 * 	w	The opcode number (0 .. MAX_CODES-1)
	 *
	 * 	b	The bit number (word_size-1 .. 0)
	 *
	 * 	d	the difference between the number of 0 bit
	 * 		records and 1 bit records.
	 *
	 * 	v	true if a valid differentiation has been
	 * 		located.
	 */
	v = FALSE;
	for( int i = 0; i < MAX_CODES; i++ ) {
		int j = word_size;
		while( j-- ) {
			word k = 1 << j;
			c1 = 0;
			c0 = 0;
			if( mask[ i ] & k ) {
				for( INSTRUCTION *l = list; l; l = l->next ) {
					if( l->mask[ i ] & k ) {
						if( l->opcode[ i ] & k ) {
							c1++;
						}
						else {
							c0++;
						}
					}
				}
			}
			if(( c1 + c0 ) == count ) {
				int x = (( c1 < c0 )?( c0 - c1 ):( c1 - c0 ));
				if( v ) {
					if( x < d ) {
						d = x;
						w = i;
						b = j;
					}
				}
				else {
					v = TRUE;
					d = x;
					w = i;
					b = j;
				}
			}
		}
	}
	/*
	 *	How did we do?
	 */
	if( !v ) {
		/*
		 *	There was no means to differentiate the instructions
		 *	in the list..
		 */
		fprintf( stderr, "----------------------\n" );
		for( ptr = list; ptr; ptr = ptr->next ) {
			fprintf( stderr, "Duplicate Instruction '%s' line %d.\n", ptr->name, ptr->line );
			dropped++;
		}
		fprintf( stderr, "----------------------\n" );
		/* Error Leaf node time! */
		here = NEW( NODE );
		here->index = 0;
		here->leaf = TRUE;
		here->decoded = NULL;
		here->op_word = 0;
		here->op_bit = 0;
		here->zero = NULL;
		here->one = NULL;
		return( here );
	}
	/*
	 *	Set the testing bit.
	 */
	t = 1 << b;
	/*
	 *	Run through the list and divide up.
	 */
	ones = NULL;	c1 = 0;
	zeros = NULL;	c0 = 0;
	while(( ptr = list )) {
		list = list->next;
		if( ptr->opcode[ w ] & t ) {
			ptr->next = ones;
			ones = ptr;
			c1++;
		}
		else {
			ptr->next = zeros;
			zeros = ptr;
			c0++;
		}
	}
	/*
	 *	Make decision node and process the limbs.
	 */
	here = NEW( NODE );
	here->index = 0;
	here->leaf = FALSE;
	here->decoded = NULL;
	here->op_word = w;
	here->op_bit = b;
	mask[ w ] &= ~t;
	here->zero = insert( mask, zeros, c0 );
	here->one = insert( mask, ones, c1 );
	mask[ w ] |= t;
	return( here );
}

/*
 *	Sequence the table into a conceptual array.
 */
static int sequence( NODE *node, int index ) {
	if( node ) {
		node->index = index++;
		index = sequence( node->zero, index );
		index = sequence( node->one, index );
	}
	return( index );
}

/*
 *	Output the decoder tree.
 */
static int emit_decoder( NODE *node, int left ) {
	char		sep;
	word		look;

	sep = ( left > 1 )?',':' ';
	
	if( node->leaf ) {
		INSTRUCTION	*ptr;
		
		if(( ptr = node->decoded )) {
			if( maximum_words > 1 ) {
				fprintf( output_source, "\t{ 0, " );
			}
			else {
				fprintf( output_source, "\t{ " );
			}
			fprintf( output_source, "0, 0" );
			if( output_formats ) {
				for( int i = 0; i < output_formats; i++ ) {
					fprintf( output_source, ", %s%s%s",
							output_format_a[ i ],
							ptr->name,			/* Leaf node function name */
							output_format_b[ i ]);
				}
			}
			else {
				fprintf( output_source, ", %s", ptr->name );		/* Leaf node function name */
			}
			fprintf( output_source, " }%c\t%s [%3d]%3d",
					sep,
					output_comment_a,
					node->index,		/* The index number of this node */
					ptr->line );		/* The line number of the configuration file */
			for( int i = 0; i < MAX_CODES; i++ ) {
				if( ptr->description[ i ]) {
					fprintf( output_source, "\t%s", ptr->description[ i ]);
				}
			}
			if( ptr->matches > 1 ) {
				fprintf( output_source, " [%d", ptr->matches );
				for( int i = 0; i < MAX_CODES; i++ ) {
					if( ptr->description[ i ]) {
						look = 1 << ( strlen( ptr->description[ i ])-1 );
						fprintf( output_source, " " );
						while( look ) {
							if( ptr->unmatched[ i ] & look ) {
								fprintf( output_source, "%c", PLACE_VARIABLE );
							}
							else {
								fprintf( output_source, "%c", (( look & PLACE_PATTERN )? PLACE_MARK: PLACE_GAP ));
							}
							look >>= 1;
						}
						fprintf( output_source, "]" );
					}
				}
			}
			fprintf( output_source, " %s %s\n",
				ptr->comment,		/* The commentary text associated with this line */
				output_comment_b );
		}
		else {
			/*
			 *	Not a decoded instruction, an illegal one.
			 */
			if( maximum_words > 1 ) {
				fprintf( output_source, "\t{ 0, " );
			}
			else {
				fprintf( output_source, "\t{ " );
			}
			fprintf( output_source, "0, 0" );
			if( output_formats ) {
				for( int i = 0; i < output_formats; i++ ) {
					fprintf( output_source, ", %s%s%s",
						output_format_a[ i ],
						error_handler,		/* Leaf node function name */
						output_format_b[ i ]);
				}
			}
			else {
				fprintf( output_source, ", %s", error_handler );
			}
			fprintf( output_source, " }%c\t%s [%3d]\tInvalid Instruction %s\n",
				sep,
				output_comment_a,
				node->index,		/* The index number of this node */
				output_comment_b );
		}
		left--;
	}
	else {
		char	*fmt;

		if( node->op_word >= maximum_words ) {
			fprintf( stderr, "Maximum words value exceeded at array index %d.\n", node->index );
			dropped++;
		}
		if( maximum_words > 1 ) {
			fprintf( output_source, "\t{ %d, ", node->op_word );
		}
		else {
			fprintf( output_source, "\t{ " );
		}
		switch( word_size ) {
			case 8: {
				fmt = "0x%02X, %d";
				break;
			}
			case 16: {
				fmt = "0x%04X, %d";
				break;
			}
			case 32: {
				fmt = "0x%08X, %d";
				break;
			}
			default: {
				fmt = "0x%X, %d";
				break;
			}
		}
		fprintf( output_source, fmt,
			(((word)1) << node->op_bit ),
			node->one->index - node->index );	/* Relative distance down array */
		if( output_formats ) {
			for( int i = 0; i < output_formats; i++ ) fprintf( output_source, ", NULL" );
		}
		else {
			fprintf( output_source, ", NULL" );
		}
		fprintf( output_source, " }%c\t%s [%3d]\t%s\n",
			sep,
			output_comment_a,
			node->index,			/* The index number of this node */
			output_comment_b );

		left--;
		left = emit_decoder( node->zero, left );
		left = emit_decoder( node->one, left );
	}
	return( left );
}

int main( int argc, char *argv[]) {
	char	buffer[ MAX_BUFFER ],
		*record;
	int	line,
		len,
		count;
	FILE	*input;
	word	mask[ MAX_CODES ];
	int	table_size;

	switch( argc ) {
		case 1: {
			/*
			 *	Reading from STDIN
			 */
			input_source_file = "stdin";
			input = stdin;
			output_base_name = NULL;
			break;
		}
		case 2: {
			char	*p;
			
			/*
			 *	File name supplied
			 */
			input_source_file = argv[ 1 ];
			if(( input = fopen( input_source_file, "r" )) == NULL ) {
				fprintf( stderr, "Unable to open file '%s'\n", input_source_file );
				return( 1 );
			}
			/*
			 *	Set up for finding out if we are C or C++
			 */
			if(( p = strchr(( output_base_name = strdup( input_source_file )), PERIOD ))) {
				/*
				 *	Blast the dot away as there is one.
				 */
				*p = EOS;
			}
			break;
		}
		default: {
			fprintf( stderr, "Usage: %s [{filename}]\n", argv[ 0 ]);
			return( 1 );
		}
	}

	/*
	 *	Default output targets to the console.
	 */
	output_source = stdout;
	output_header = stdout;

	/*
	 *	Here we go...
	 */
	line = 0;
	while( fgets( buffer, MAX_BUFFER, input )) {
		line += 1;
		if(( len = strlen( buffer ))) {
			/*
			 *	Check for new line and remove if found.
			 */
			len -= 1;
			if( buffer[ len ] != NL ) {
				fprintf( stderr, "Line %d too long.\n", line );
				return( 1 );
			}
			buffer[ len ] = EOS;
			/*
			 *	Have we got Block Record?  This requires specific
			 *	handling.
			 */
			if(( record = strchr( buffer, BEGIN_RECORD ))) {
				if( record[ 1 ] == BLOCK_RECORD ) {
					switch( record[ 2 ] ) {
						case BLOCK_START: {
							push_mode( START_MODE, line );
							break;
						}
						case BLOCK_END: {
							push_mode( END_MODE, line );
							break;
						}
						case BLOCK_HEADER: {
							push_mode( HEADER_MODE, line );
							break;
						}
						case BLOCK_COMMENT: {
							push_mode( COMMENT_MODE, line );
							break;
						}
						case SPACE:
						case TAB:
						case END_RECORD: {
							if( !pop_mode()) {
								fprintf( stderr, "Block ends without corresponding start, line %d.\n", line );
								return( 1 );
							}
							break;
						}
						default: {
							fprintf( stderr, "Invalid Block record in line %d.\n", line );
							return( 1 );
						}
					}
					/*
					 *	getting here means we have had a valid
					 *	block command.  as the actual line itself
					 *	has no other meaning or purpose we skip
					 *	all the line processing code.
					 */
					output_target = UNSPECIFIED_TARGET;
					/*
					 *	Force loop to start next input line processing.
					 */
					continue;
				}
			}
			/*
			 *	How we operate is dependent on the block mode..
			 */
			if( root_block_stack ) {
				/*
				 *	We are in a block mode..
				 */
				switch( root_block_stack->mode ) {
					case START_MODE: {
						/*
						 *	Output data to the start of the source file.
						 */
						if( output_target != SOURCE_TARGET ) {
							output_target = SOURCE_TARGET;
							fprintf( output_source, "#line %d \"%s\"\n", line, input_source_file );
						}
						fprintf( output_source, "%s\n", buffer );
						break;
					}
					case END_MODE: {
						/*
						 *	Output data to the end of the source file.
						 */
						output_target = UNSPECIFIED_TARGET;
						FINISH *ptr = NEW( FINISH );
						ptr->line = line;
						ptr->data = DUP( buffer );
						ptr->next = NULL;
						*finish_data_tail = ptr;
						finish_data_tail = &( ptr->next );
						break;
					}
					case HEADER_MODE: {
						/*
						 *	Output data to the header file.
						 */
						if( output_target != HEADER_TARGET ) {
							output_target = HEADER_TARGET;
							fprintf( output_header, "#line %d \"%s\"\n", line, input_source_file );
						}
						fprintf( output_header, "%s\n", buffer );
						break;
					}
					default: {
						/*
						 *	Following lines are just comments, free form
						 * 	text to be ignored.
						 */
						 output_target = UNSPECIFIED_TARGET;
						 break;
					}
				}
			}
			else {
				/*
				 * 	No mode stack means we are in line mode.
				 * 
				 *	Contains a record?
				 */
				if( record ) {
					char	*q;

					/*
					 *	Skip record start.
					 */
					record++;
					/*
					 *	Is there an optional record end (remember we have
					 *	to watch out for escaped end symbols).
					 */
					q = record;
					while(( q = strchr( q, END_RECORD ))) {
						if( *(q-1) != ESCAPE_SYMBOL ) {
							*q++ = EOS;
							break;
						}
						/*
						 *	Strip escape and look again.
						 */
						*(q-1) = END_RECORD;
						strcpy( q, q+1 );
					}
					if( q == NULL ) {
						q = "";
					}
					/*
					 *	Process this line from after the
					 *	begin symbol.
					 */
					if( !process( line, record, q )) {
						fprintf( stderr, "Error in line %d.\n", line );
						return( 1 );
					}		
				}
				else {
					output_target = UNSPECIFIED_TARGET;
				}
			}
		}
	}
	
	/*
	 * 	If the block mode stack is not empty then there is an error.
	 */
	if( root_block_stack ) {
		fprintf( stderr, "Unterminated block(s) in file:\n" );
		while( root_block_stack ) {
			fprintf( stderr, "line %d.\n", root_block_stack->line );
			root_block_stack = root_block_stack->prev;
		}
		return( 1 );
	}

	/*
	 *	Fill in missing output information if not supplied.
	 */
	if( output_comment_a == NULL ) output_comment_a = "/*";
	if( output_comment_b == NULL ) output_comment_b = "*/";
	if( data_type == NULL ) data_type = "decoder_t";
	if( data_scope == NULL ) data_scope = "static";
	if( data_name == NULL ) data_name = "decoder";
		
	/*
	 *	Calculate how many instructions have been captured.
	 */
	count = 0;
	for( INSTRUCTION *ptr = instructions; ptr; ptr = ptr->next ) count++;
	
	/*
	 *	Get on and insert instructions into the tree.
	 */
	for( int i = 0; i < MAX_CODES; mask[ i++ ] = ~((word)0));
	tree = insert( mask, instructions, count );

	/*
	 *	Assign sequenced index numbers to the nodes
	 */
	table_size = sequence( tree, 0 );

	/*
	 *	Display the decode tree as an organised array
	 */
	if( strlen( output_comment_b )) {
		/*
		 *	C style start to end comments
		 */
		fprintf( output_source, "%s\n", output_comment_a );
		fprintf( output_source, "\tStart Of Table\n" );
		fprintf( output_source, "\t==============\n" );
		fprintf( output_source, "%s\n", output_comment_b );
	}
	else {
		/*
		 *	C++ style start to end comments
		 */
		fprintf( output_source, "%s\n", output_comment_a );
		fprintf( output_source, "%s\tStart Of Table\n", output_comment_a );
		fprintf( output_source, "%s\t==============\n", output_comment_a );
		fprintf( output_source, "%s\n", output_comment_a );
	}
	fprintf( output_source, "%s %s %s[ %d ] = {\n", data_scope, data_type, data_name, table_size );
	(void)emit_decoder( tree, table_size );
	fprintf( output_source, "};\n" );
	fprintf( output_source, "\n" );
	if( strlen( output_comment_b )) {
		/*
		 *	C style start to end comments
		 */
		fprintf( output_source, "%s\n", output_comment_a );
		fprintf( output_source, "\tEnd Of Table\n" );
		fprintf( output_source, "\t============\n" );
		fprintf( output_source, "%s\n", output_comment_b );
	}
	else {
		/*
		 *	C++ style start to end comments
		 */
		fprintf( output_source, "%s\n", output_comment_a );
		fprintf( output_source, "%s\tEnd Of Table\n", output_comment_a );
		fprintf( output_source, "%s\t============\n", output_comment_a );
		fprintf( output_source, "%s\n", output_comment_a );
	}

	/*
	 * 	Output all of the finish data.. We will re-use the line variable
	 *	for a similar purpose.
	 */
	line = 0;
	while( finish_data ) {
		if( finish_data->line > line ) {
			fprintf( output_source, "#line %d \"%s\"\n", finish_data->line, input_source_file );
			line = finish_data->line + 1;
		}
		fprintf( output_source, "%s\n", finish_data->data );
		finish_data = finish_data->next;
	}

	/*
	 *	Output a status line.
	 */
	if( dropped ) {
		fprintf( stderr, "\nERROR!\n\n\t%d errors detected in configuration data.\n\n", dropped );
	}

	if( output_base_name ) {
		fclose( output_header );
		fclose( output_source );
	}
	
	/*
	 *	Done
	 */
	return(( dropped > 0 )? 1 : 0 );
}

/*
 *	EOF
 */
