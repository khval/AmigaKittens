#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>
#include <iostream>
#include <vector>

#ifdef __amigaos4__
#include <proto/exec.h>
#include <proto/retroMode.h>
#include <amosKittens.h>
#endif

#include "debug.h"
#include "stack.h"
#include "amosKittens.h"
#include "commands.h"
#include "commandsData.h"
#include "kittyErrors.h"

extern std::vector<struct defFn> defFns;

extern unsigned short last_token;
extern int tokenMode;
extern int tokenlength;

double to_rad_factor=1.0f;
double to_degree_factor=1.0f ;
int decimals = 2;		// FIX sets text formating 
extern char *_file_pos_ ;

extern int parenthesis[MAX_PARENTHESIS_COUNT];

//extern char *read_kitty_args(char *tokenBuffer, struct glueCommands *sdata);
extern char *read_kitty_args(char *tokenBuffer, int read_stack, unsigned short end_token );

#define args (instance.stack - data->stack +1)

char *_mathInc( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	struct kittyData *var = NULL;
	char *ptr = data -> tokenBuffer ;

	if (NEXT_TOKEN( ptr ) == 0x0006)
	{
		struct reference *ref = (struct reference *) (ptr + 2);
		var = getVar(ref->ref);
	}

	if (var)
	{
		switch (var->type)
		{
			case type_int:
				var->integer.value++;
				break;
	
			case type_int | type_array:
				(&var->int_array -> ptr) [var -> index].value++;
				break;
	
			default:
				setError(ERROR_Type_mismatch,data->tokenBuffer);
		}
	}

	popStack(__stack - data->stack);
	return NULL;
}

char *_mathDec( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	struct kittyData *var = NULL;
	char *ptr = data -> tokenBuffer ;

	if (NEXT_TOKEN( ptr ) == 0x0006)
	{
		struct reference *ref = (struct reference *) (ptr + 2);
		var = getVar(ref->ref);
	}

	if (var)
	{
		switch (var->type)
		{
			case type_int:
				var->integer.value--;
				break;
	
			case type_int | type_array:
				(&var->int_array->ptr) [var -> index].value--;
				break;
	
			default:
				setError(ERROR_Type_mismatch,data->tokenBuffer);
		}
	}

	popStack(__stack - data->stack);
	return NULL;
}


char *_mathAdd( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	struct kittyData *var = NULL;

	char *ptr = data -> tokenBuffer ;
	bool success = true;

	if (NEXT_TOKEN( ptr ) == 0x0006)
	{
		struct reference *ref = (struct reference *) (ptr + 2);
		var = getVar(ref->ref);
	}

	if (var)
	{
		int _value = 0;

		switch (args)
		{
			case 2:
				{
					int _inc = getStackNum(__stack );
					_value = getStackNum(__stack - 1 ) + _inc;
				}
				break;

			case 4:
				{
					int _inc = getStackNum(__stack - 2 );
					int _from = getStackNum(__stack - 1 );
					int _to = getStackNum(__stack );

					 _value = getStackNum(__stack - 3 );

					if (_inc>0)
					{
						_value += _inc;	
						if (_value > _to ) _value = _from;	// if more then max reset to min
						if (_value < _from) _value =_from;	// limit to min
					}
					else
					{
						_value += _inc;	
						if (_value < _from) _value = _to;	// if less then min reset to max
						if (_value > _to) _value = _to;		// limit to max
					}
				}
				break;

			default:
				success = false;
		}

		if (success)
		{
			switch (var->type)
			{
				case type_int:
					var->integer.value= _value;
					break;
	
				case type_int | type_array:
					(&var->int_array -> ptr) [var -> index].value = _value;
					break;
	
				default:
					setError(ERROR_Type_mismatch,data->tokenBuffer);
			}
		}
	}

	if (success == false) setError(22,data->tokenBuffer);

	popStack(__stack - data->stack);
	return NULL;
}

char *mathInc(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (instance.cmdStack) if (__stack) if (cmdTmp[instance.cmdStack-1].flag == cmd_index ) cmdTmp[--instance.cmdStack].cmd(&cmdTmp[instance.cmdStack],0);

	stackCmdNormal( _mathInc, tokenBuffer );
	return tokenBuffer;
}

char *mathDec(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (instance.cmdStack) if (__stack) if (cmdTmp[instance.cmdStack-1].flag == cmd_index ) cmdTmp[--instance.cmdStack].cmd(&cmdTmp[instance.cmdStack],0);

	stackCmdNormal( _mathDec, tokenBuffer );
	return tokenBuffer;
}

char *mathAdd(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (instance.cmdStack) if (__stack) if (cmdTmp[instance.cmdStack-1].flag == cmd_index ) cmdTmp[--instance.cmdStack].cmd(&cmdTmp[instance.cmdStack],0);

	stackCmdNormal( _mathAdd, tokenBuffer );
	return tokenBuffer;
}


//------------------------------------------------------------


char *_mathSin( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( sin( r * to_rad_factor ) );
	return NULL;
}

char *_mathCos( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( cos(  r * to_rad_factor ) );
	return NULL;
}

char *_mathTan( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( tan(  r * to_rad_factor ) );
	return NULL;
}

char *_mathAcos( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( acos( r ) * to_degree_factor );
	return NULL;
}

char *_mathAsin( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( asin( r ) * to_degree_factor );
	return NULL;
}

char *_mathAtan( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( atan( r ) * to_degree_factor );
	return NULL;
}

char *_mathHsin( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( sinh( r * to_rad_factor ) );
	return NULL;
}

char *_mathHcos( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( cosh( r * to_rad_factor ) );
	return NULL;
}

char *_mathHtan( struct glueCommands *data, int nextToken )
{
	double r =0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	r = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( tanh( r * to_rad_factor ) );
	return NULL;
}

char *_mathLog( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( log10( d ) );
	return NULL;
}

char *_mathExp( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( exp( d ) );
	return NULL;
}

char *_mathLn( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( log( d ) );
	return NULL;
}

char *_mathSqr( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackDecimal( sqrt( d ) );
	return NULL;
}

char *_mathAbs( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	
	// don't need to pop stack, if type is not changed or if args are only 1.	

	if (args == 1)
	{
		switch (kittyStack[__stack].type)
		{
			case type_int:	 
					kittyStack[__stack].integer.value =  abs(kittyStack[__stack].integer.value);
					break;

			case type_float:  
					kittyStack[__stack].decimal.value = fabs( kittyStack[__stack].decimal.value );
					break;
		}
	}
	else
	{
		popStack(__stack - data->stack);
		setError( 22, data->tokenBuffer );
	}

	return NULL;
}

char *_mathInt( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = floor(getStackDecimal(__stack));
	popStack(__stack - data->stack);
	setStackNum( (int) d ) ;
	return NULL;
}

char *_mathSgn( struct glueCommands *data, int nextToken )
{
	double d = 0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	d = getStackDecimal(__stack);
	popStack(__stack - data->stack);
	setStackNum( (d<0) ? -1 : ((d>0) ? 1 : 0) ) ;
	return NULL;
}

char *_mathRnd( struct glueCommands *data, int nextToken )
{
	int n = 0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	n = getStackNum(__stack );
	popStack(__stack - data->stack);
	setStackNum( rand() % (n+1) );
	return NULL;
}

char *_mathRandomize( struct glueCommands *data, int nextToken )
{
	int n = 0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	n = getStackNum(__stack );
	popStack(__stack - data->stack);
	srand( n );
	return NULL;
}

char *_mathMax( struct glueCommands *data, int nextToken )
{
	double a = 0.0, b=0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args == 2)
	{
		a = getStackDecimal(__stack-1);
		b = getStackDecimal(__stack);
	}

	popStack(__stack - data->stack);
	setStackDecimal( a>b ? a: b );
	return NULL;
}

char *_mathMin( struct glueCommands *data, int nextToken )
{
	double a = 0.0, b=0.0;
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 2)
	{
		a = getStackDecimal(__stack-1);
		b = getStackDecimal(__stack);
	}
	popStack(__stack - data->stack);
	setStackDecimal( a<b ? a: b );

	return NULL;
}

char *_mathSwap( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (args==2)
	{
		struct kittyData tmp;

		// swap stack

		tmp = kittyStack[__stack -1];
		kittyStack[__stack-1] =kittyStack[__stack];
		kittyStack[__stack]=tmp;

		// read the stack back in.
		// read kitty_args pop's stack.
		read_kitty_args( data -> tokenBuffer, data -> stack, 0x0000 );
	}
	else
	{
		popStack(__stack - data->stack);
	}

	return NULL;
}

char *_mathFix( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	if (args == 1)	decimals = getStackNum(__stack );
	popStack(__stack - data->stack);
	return NULL;
}

char *_mathDefFn( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	popStack(__stack - data->stack);
	return NULL;
}

//------------------------------------------------------------

char *mathDegree(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	to_rad_factor=M_PI/180.0f;
	to_degree_factor=180.0f/M_PI ;

	return tokenBuffer;
}

char *mathRadian(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	to_rad_factor=1.0f;
	to_degree_factor=1.0f ;
	return tokenBuffer;
}

// is const number so,

char *mathPi(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	setStackDecimal(M_PI);
	flushCmdParaStack( NEXT_TOKEN(tokenBuffer) );		// PI is on stack, we are ready.
	return tokenBuffer;
}

char *mathSin(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathSin, tokenBuffer );
	return tokenBuffer;
}

char *mathCos(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathCos, tokenBuffer );
	return tokenBuffer;
}

char *mathTan(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathTan, tokenBuffer );
	return tokenBuffer;
}

char *mathAcos(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathAcos, tokenBuffer );
	return tokenBuffer;
}

char *mathAsin(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathAsin, tokenBuffer );
	return tokenBuffer;
}

char *mathAtan(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathAtan, tokenBuffer );
	return tokenBuffer;
}

char *mathHsin(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathHsin, tokenBuffer );
	return tokenBuffer;
}

char *mathHcos(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathHcos, tokenBuffer );
	return tokenBuffer;
}

char *mathHtan(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathHtan, tokenBuffer );
	return tokenBuffer;
}

char *mathLog(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathLog, tokenBuffer );
	return tokenBuffer;
}

char *mathExp(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdNormal( _mathExp, tokenBuffer );
	return tokenBuffer;
}

char *mathLn(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdNormal( _mathLn, tokenBuffer );
	return tokenBuffer;
}

char *mathSqr(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	stackCmdParm( _mathSqr, tokenBuffer );
	return tokenBuffer;
}

char *mathAbs(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathAbs, tokenBuffer );
	return tokenBuffer;
}

char *mathInt(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathInt, tokenBuffer );
	return tokenBuffer;
}

char *mathSgn(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathSgn, tokenBuffer );
	return tokenBuffer;
}

char *mathRnd(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathRnd, tokenBuffer );
	return tokenBuffer;
}

char *mathRandomize(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdNormal( _mathRandomize, tokenBuffer );
	return tokenBuffer;
}

char *mathMax(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathMax, tokenBuffer );
	return tokenBuffer;
}

char *mathMin(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathMin, tokenBuffer );
	return tokenBuffer;
}

char *mathSwap(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdNormal( _mathSwap, tokenBuffer );
	return tokenBuffer;
}

char *mathFix(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _mathFix, tokenBuffer );
	return tokenBuffer;
}

char *mathDefFn(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	if (NEXT_TOKEN(tokenBuffer) == 0x0006 )
	{
		struct reference *ref = (struct reference *) (tokenBuffer + 2);

		if (ref -> ref)
		{
			tokenBuffer = defFns[ ref -> ref -1 ].skipAddr;
		}
	}

	return tokenBuffer;
}


char *_mathFnArgsEnd( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);
	flushCmdParaStack( 0 );

	return NULL;
}


char *_mathFnReturn( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	getchar();

	return data -> tokenBuffer;
}


char *_mathFn( struct glueCommands *data, int nextToken )
{
	int ref = data -> lastVar;

	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__ ,__LINE__);

	if (ref)
	{
		char *ptr = defFns[  ref -1 ].fnAddr;

		if (NEXT_TOKEN(ptr)==0x0074)
		{
			struct kittyData backup;

			// execute at end of args 

				data -> cmd_type = cmd_onEol;	// force flush to stop.
				data -> cmd = _mathFnArgsEnd;

			// Start reading args.

				ptr+=2;
				ptr = read_kitty_args( ptr, data -> stack, token_parenthesis_end );

				if ( (*(unsigned short *) ptr) == token_parenthesis_end) ptr+=2;
				if ( (*(unsigned short *) ptr) == token_equal) ptr+=2;

			data -> tokenBuffer = _file_pos_;

			backup = kittyStack[__stack];
			popStack(__stack - data->stack);
			setStackNone();
			kittyStack[__stack] = backup;

				data -> cmd_type = cmd_onEol;	// force flush to stop.
				data -> cmd = _mathFnReturn;
				instance.cmdStack++;		// stop stack from being deleted

			return ptr;
		}
	}
	else
	{
		printf("pass1 failed\n");
	}

	popStack(__stack - data->stack);
	return NULL;
}


char *mathFn(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	if (NEXT_TOKEN(tokenBuffer) == 0x0006 )
	{
		struct reference *ref = (struct reference *) (tokenBuffer + 2);
		int l = ref -> length;
			
		stackCmdParm( _mathFn, tokenBuffer );
		cmdTmp[instance.cmdStack-1].lastVar = ref -> ref;

		l += ref -> length & 1;		
		tokenBuffer += 2 + sizeof(struct reference) + l;
	}
	return tokenBuffer;
}

