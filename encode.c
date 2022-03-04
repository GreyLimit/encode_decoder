/*
 *	Encode
 *
 *	Program to process an input stream of Assembly Language
 *	definitions and output C/C++ source code providing a
 *	mechanism to decode those instructions.
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

/*
 *	Provide constant names for the characters above that have specific
 *	meanings.
 */
#define BEGIN_RECORD		'{'
#define END_RECORD		'}'
#define SIZE_RECORD		'S'
#define INSTRUCTION_RECORD	'I'
#define FORMAT_RECORD		'F'
#define TYPE_RECORD		'T'
#define NAME_RECORD		'N'
#define LANGUAGE_RECORD		'L'
#define ERROR_RECORD		'E'
#define WORDS_RECORD		'W'
#define INSERT_HERE		'%'
#define ONE_BIT			'1'
#define ZERO_BIT		'0'
#define ARGUMENT_BIT		'.'
#define SPACE			' '
#define NL			'\n'
#define EOS			'\0'
#define ESCAPE_SYMBOL		'\\'


/*
 *	Define simplistic memory allocation routines
 */
#include <malloc.h>
#define NEW(t) ((t *)malloc(sizeof(t)))

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
	int		duplicates;			/* How many versions of this instruction?	*/
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

/************************************************
 *						*
 *	GLOBAL VARIABLES DEFINED HERE		*
 *						*
 ************************************************/

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
static bool		enable_comment = FALSE;
static char		*output_format_a = NULL,
			*output_format_b = NULL;

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
static char		*data_name = NULL;

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
 *	Process a line of input.
 */
static bool process( int line, char *input, char *comment ) {
	char	record;
	
	switch(( record = *input )) {
		case SIZE_RECORD: {
			int	i;

			/*
			 *	S nnn	Provide number of bits per word of instruction
			 */
			i = atoi( input+1 );
			if(( i <= 0 )||( i > ( sizeof( word ) << 3 ))) {
				fprintf( stderr, "Invalid word size %d.\n", i );
				return( FALSE );
			}
			if( word_size_set ) {
				fprintf( stderr, "Cannot reset word size.\n" );
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
			i = atoi( input+1 );
			if(( i <= 0 )||( i > MAX_CODES )) {
				fprintf( stderr, "Invalid number of words %d.\n", i );
				return( FALSE );
			}
			if( maximum_words_set ) {
				fprintf( stderr, "Cannot reset number of words.\n" );
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
			if(( output_format_a != NULL )||( output_format_b != NULL )) {
				fprintf( stderr, "Output format already specified.\n" );
				return( FALSE );
			}
			/*
			 *	Strip spaces...
			 */
			input += 1;
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
				output_format_a = DUP( input );
				output_format_b = DUP( p );
			}
			else {
				output_format_a = DUP( input );
				output_format_b = "";
			}
			break;
		}
		case LANGUAGE_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	L language
			 */
			if(( output_comment_a != NULL )||( output_comment_b != NULL )) {
				fprintf( stderr, "Output comment already specified.\n" );
				return( FALSE );
			}
			/*
			 *	Strip spaces...
			 */
			input += 1;
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
			if( strcasecmp( input, "c" ) == 0 ) {
				/*
				 *	C Language
				 */
				output_comment_a = "/*";
				output_comment_b = "*/";
			}
			else {
				if(( strcasecmp( input, "c++" ) == 0 )||( strcasecmp( input, "cpp" ) == 0 )) {
					/*
					 *	C++ Language
					 */
					output_comment_a = "//";
					output_comment_b = "";
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
			input += 1;
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
		case NAME_RECORD: {
			char	*p, *q, *r;
			
			/*
			 *	Strip spaces...
			 */
			input += 1;
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
		case ' ':
		case '\t': {
			/*
			 *	Pass through "as is".
			 */
			printf( "%s\n", input );
			break;
		}
		case INSTRUCTION_RECORD: {
			INSTRUCTION	*p;

			/*
			 *	Start with new empty record, and link it in.
			 */
			p = NEW( INSTRUCTION );
			p->line = line;
			p->name = NULL;
			p->comment = DUP( comment );
			p->elements = 0;
			for( int i = 0; i < MAX_CODES; p->opcode[ i++ ] = 0 );
			for( int i = 0; i < MAX_CODES; p->mask[ i++ ] = 0 );
			for( int i = 0; i < MAX_CODES; p->description[ i++ ] = NULL );
			p->duplicates = 0;
			p->next = NULL;
			*instruction_tail = p;
			instruction_tail = &( p->next );
			/*
			 *	Fill in the record; start by breaking the input up into
			 *	space separated units (but still ignoring initial spaces).
			 */
			input += 1;
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
		fprintf( stderr, "Decode tree contains empty leaf!\n" );
		dropped += 1;
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
			t = mask[ i ] & list->mask[ i ];
			for( int j = 0; j < word_size; j++ ) {
				if( t & 1 ) count++;
				t >>= 1;
			}
		}
		list->duplicates = ( count > 0 )?( 1 << count ):0;
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
		while( j ) {
			word k = 1 << (--j);
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

	sep = ( left > 1 )?',':' ';
	
	if( node->leaf ) {
		INSTRUCTION	*ptr;
		
		if(( ptr = node->decoded )) {
			if( maximum_words > 1 ) {
				printf( "\t{ 0, " );
			}
			else {
				printf( "\t{ " );
			}
			printf( "0,0,\t%s%s%s }%c\t%s %3d[%3d]",
				output_format_a,
				ptr->name,		/* Leaf node function name */
				output_format_b,
				sep,
				output_comment_a,
				node->index,		/* The index number of this node */
				ptr->line );		/* The line number of the configuration file */
			for( int i = 0; i < MAX_CODES; i++ ) {
				if( ptr->description[ i ]) {
					printf( " %s", ptr->description[ i ]);
				}
			}
			if( ptr->duplicates ) printf( " [%d]", ptr->duplicates );
			printf( " %s %s\n",
				ptr->comment,		/* The commentary text associated with this line */
				output_comment_b );
		}
		else {
			if( maximum_words > 1 ) {
				printf( "\t{ 0, " );
			}
			else {
				printf( "\t{ " );
			}
			printf( "0,0,\t%s }%c\t%s %3d Invalid Instruction %s\n",
				error_handler,		/* Leaf node function name */
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
			printf( "\t{ %d, ", node->op_word );
		}
		else {
			printf( "\t{ " );
		}
		switch( word_size ) {
			case 8: {
				fmt = "0x%02X,%d,\tNULL }%c\t%s %3d %s\n";
				break;
			}
			case 16: {
				fmt = "0x%04X,%d,\tNULL }%c\t%s %3d %s\n";
				break;
			}
			case 32: {
				fmt = "0x%08X,%d,\tNULL }%c\t%s %3d %s\n";
				break;
			}
			default: {
				fmt = "0x%X,%d,\tNULL }%c\t%s %3d %s\n";
				break;
			}
		}
		printf( fmt,
			(((word)1) << node->op_bit ),
			node->one->index - node->index,	/* Relative distance down array */
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

	if( argc != 2 ) {
		fprintf( stderr, "Usage: %s {filename} (or - for stdin)\n", argv[ 0 ]);
		return( 1 );
	}

	if( strcmp( argv[ 1 ], "-" ) == 0 ) {
		input = stdin;
	}
	else {
		if(( input = fopen( argv[ 1 ], "r" )) == NULL ) {
			fprintf( stderr, "Unable to open file '%s'\n", argv[ 1 ]);
			return( 1 );
		}
	}
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
			 *	Contains a record?
			 */
			if(( record = strchr( buffer, BEGIN_RECORD ))) {
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
		}
	}

	/*
	 *	Fill in missing output information if not supplied.
	 */
	if( output_format_a == NULL ) output_format_a = "";
	if( output_format_b == NULL ) output_format_b = "";
	if( output_comment_a == NULL ) output_comment_a = "/*";
	if( output_comment_b == NULL ) output_comment_b = "*/";
	if( data_type == NULL ) data_type = "decoder_t";
	if( data_name == NULL ) data_name = "decoder";
	if( error_handler == NULL ) error_handler = "illegal";
		
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
	printf( "%s %s[ %d ] = {\n", data_type, data_name, table_size );
	(void)emit_decoder( tree, table_size );
	printf( "};\n" );

	/*
	 *	Output a status line.
	 */
	if( dropped) {
		printf( "\nERROR!\n\n\t%d errors detected in configuration data.\n\n", dropped );
	}

	/*
	 *	Done
	 */
	return(( dropped > 0 )? 1 : 0 );
}

/*
 *	EOF
 */