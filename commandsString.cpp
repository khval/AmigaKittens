
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include "amosKittens.h"
#include "commands.h"
#include "debug.h"
#include <string>
#include <iostream>
#include "errors.h"

extern int last_var;
extern struct globalVar globalVars[];
extern unsigned short last_token;
extern int tokenMode;

extern void _str(const char *str);

using namespace std;

#define NEXT_TOKEN(ptr) *((short *) ptr)
#define NEXT_INT(ptr) *((int *) (ptr+2))

char *_stackString( int n )
{
	if (kittyStack[n].type == type_string)
	{
		return (kittyStack[n].str);
	}
	return NULL;
}

int _stackInt( int n )
{
	if (kittyStack[n].type == type_int)
	{
		return (kittyStack[n].value);
	}
	return 0;
}

char *_print( struct glueCommands *data )
{
	int n;
	printf("PRINT: ");

	for (n=data->stack;n<=stack;n++)
	{
//		printf("stack %d, type: %d value %d\n",n, kittyStack[n].type, kittyStack[n].value);

		switch (kittyStack[n].type)
		{
			case type_int:
				printf("%d", kittyStack[n].value);
				break;
			case type_float:
				printf("%f", kittyStack[n].decimal);
				break;
			case type_string:
				if (kittyStack[n].str) printf("%s", kittyStack[n].str);
				break;

		}

		if (n<=stack) printf("    ");
	}
	printf("\n");
	return NULL;
}

char *cmdPrint(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _print, ptr );
	return ptr;
}

char *_left( struct glueCommands *data )
{
	int args = stack - data->stack ;
	char *str;
	char *tmp = NULL;
	int _len;

	if (args == 2)
	{
		str = _stackString( data->stack + 1 );
		_len = _stackInt( data->stack + 2 );
		tmp = strndup(str, _len );

		printf("len %d\n", _len);
	}	

	stack -=args;

	if (tmp) _str(tmp);

	return NULL;
}


char *_mid( struct glueCommands *data )
{
	int args = stack - data->stack ;
	char *str;
	char *tmp = NULL;
	int _start, _len;

	printf("%s: args %d\n",__FUNCTION__,args);

	if (args == 3)
	{
		str = _stackString( data->stack + 1 );
		_start = _stackInt( data->stack + 2 ) -1;
		_len = _stackInt( data->stack + 3 );

		if (_start<0) _start = 0;
		if (_start>strlen(str)-1) _start = strlen(str)-1;

		tmp = strndup(str + _start, _len );

		printf("len %d\n", _len);
	}	

	stack -=args;

	if (tmp) _str(tmp);

	return NULL;
}

char *_right( struct glueCommands *data )
{
	int args = stack - data->stack ;
	char *str;
	char *tmp = NULL;
	int _start, _len;

	printf("%s: args %d\n",__FUNCTION__,args);

	if (args == 2)
	{
		str = _stackString( data->stack + 1 );
		_len = _stackInt( data->stack + 2 );
		if (_len>strlen(str)) _start = strlen(str);

		tmp = strdup(str + strlen(str) - _len );
	}	

	stack -=args;

	if (tmp) _str(tmp);

	return NULL;
}

char *_hex( struct glueCommands *data )
{
	int args = stack - data->stack ;
	printf("%s: args %d\n",__FUNCTION__,args);

	stack -=args;

	return NULL;
}

char *_bin( struct glueCommands *data )
{
	int args = stack - data->stack ;
	printf("%s: args %d\n",__FUNCTION__,args);

	stack -=args;

	return NULL;
}


char *cmdLeft(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _left, ptr );
	return ptr;
}

char *cmdMid(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _mid, ptr );
	return ptr;
}

char *cmdRight(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _right, ptr );
	return ptr;
}

char *cmdHex(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _mid, ptr );
	return ptr;
}

char *cmdBin(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _right, ptr );
	return ptr;
}

