#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include "debug.h"
#include <string>
#include <iostream>
#include <vector>

#include "stack.h"
#include "amosKittens.h"
#include "commands.h"
#include "commandsErrors.h"
#include "errors.h"

extern int last_var;
extern struct globalVar globalVars[];
extern unsigned short last_token;
extern int tokenMode;
extern int tokenlength;

extern char *findLabel( char *name );
extern int findVarPublic( char *name, int type );
extern std::vector<struct label> labels;

char *(*onErrorTemp)(char *ptr) = NULL;
char *(*onError)(char *ptr) = NULL;
char *on_error_goto_location = NULL;
char *on_error_proc_location = NULL;
char *resume_location = NULL;


char *errOnError(nativeCommand *cmd, char *tokenBuffer)
{
	unsigned short next_token; 

	onError = onErrorBreak;	// default.

	switch ( NEXT_TOKEN(tokenBuffer ))
	{
		case 0x02A8:	// Goto

				printf("On Error ... Goto ...\n");

				tokenBuffer += 2;
				
				next_token = NEXT_TOKEN(tokenBuffer );

				printf("Next token %04x\n",next_token);

				if ((next_token == 0x006) || (next_token == 0x0018))
				{
					char *name;
					struct reference *ref = (struct reference *) (tokenBuffer + 2);
					name = strndup( tokenBuffer + 2 + sizeof(struct reference), ref->length );

					if (name)
					{
						printf("name %s\n",name);

						on_error_goto_location = findLabel(name);
						onError = onErrorGoto;
						free(name);
					}

					tokenBuffer += (2 + sizeof(struct reference) + ref -> length) ;		
				}
				break;

		case 0x0386:	// Proc

				printf("On Error ... Gosub ...\n");

				tokenBuffer += 2;

				if (NEXT_TOKEN(tokenBuffer ) == 0x0012)	// proc
				{
					char *name;
					struct reference *ref = (struct reference *) (tokenBuffer + 2);
					name = strndup( tokenBuffer + 2 + sizeof(struct reference), ref->length );

					if (name)
					{
						int found = findVarPublic(name, ref -> flags);
						if (found)
						{
							on_error_proc_location = globalVars[found -1].var.tokenBufferPos;
							onError = onErrorProc;
						}

						free(name);
					}

					tokenBuffer += (2 + sizeof(struct reference) + ref -> length) ;	
				}
				break;
	}
	return tokenBuffer;
}

char *errEndProc(struct nativeCommand *cmd, char *tokenBuffer );

char *errResumeLabel(nativeCommand *cmd, char *tokenBuffer)
{
	struct reference *ref;
	printf("Next token %04x\n",NEXT_TOKEN(tokenBuffer));

	switch (NEXT_TOKEN(tokenBuffer))
	{
		case 0x0018:

			ref = (struct reference *) (tokenBuffer + 2);
			tokenBuffer += 2;

			if (ref->ref)
			{
				resume_location = labels[ref->ref-1].tokenLocation+2;
			}

			tokenBuffer += sizeof(struct reference) + ref -> length;
			break;

		case 0x0000:
		case 0x0054:

			if (dropProgStackToProc( _procedure ))
			{
				if (cmdTmp[cmdStack-1].cmd == _procedure ) 
				{
					printf(" maybe need flush some stack here? %d - %d --\n", cmdTmp[cmdStack-1].stack, stack );
					tokenBuffer=cmdTmp[--cmdStack].cmd(&cmdTmp[cmdStack],0);
				}
			}
			if ( resume_location ) tokenBuffer = resume_location;
			break;
	}

	return tokenBuffer;
}

char *errResumeNext(nativeCommand *cmd, char *tokenBuffer)
{
	struct reference *ref;
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	printf("this command is not yet working!!!\n");
	getchar();
	return tokenBuffer;
}


char *onErrorBreak(char *ptr)
{
	return NULL;
}

char *onErrorIgnore(char *ptr)
{
	return ptr;
}

char *onErrorGoto(char *ptr)
{
	kittyError.newError = false;
	return on_error_goto_location -2;
}

char *onErrorProc(char *ptr)
{
	kittyError.newError = false;
	stackCmdLoop( _procedure, ptr);
	return on_error_proc_location -2;
}

char *_errError( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;

	if (args == 1)
	{
		setError( getStackNum(stack), data -> tokenBuffer );
	}

	popStack( stack - data->stack );
	return NULL;
}

char *errError(struct nativeCommand *cmd, char *tokenBuffer)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _errError, tokenBuffer );
	return tokenBuffer;
}

char *_errResume( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;

	if (args == 1)
	{

	}

	popStack( stack - data->stack );
	return NULL;
}

char *errResume(struct nativeCommand *cmd, char *tokenBuffer)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _errResume, tokenBuffer );
	setStackNone();
	return tokenBuffer;
}


char *_errTrap( struct glueCommands *data, int nextToken )
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (onErrorTemp)
	{
		onError = onErrorTemp;
		onErrorTemp = NULL;
	}

	return NULL;
}

char *errTrap(nativeCommand *err, char *tokenBuffer)
{
	printf("%s:%d\n",__FUNCTION__,__LINE__);

	onErrorTemp = onError;
	onError = onErrorIgnore;
	stackCmdFlags( _errTrap, tokenBuffer, cmd_onNextCmd | cmd_onEol );

	return tokenBuffer;
}

