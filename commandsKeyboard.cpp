#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#ifdef __amigaos4__
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/locale.h>
#include <proto/keymap.h>
#include <proto/diskfont.h>
#include <diskfont/diskfonttag.h>
#include <proto/retroMode.h>
#endif

#ifdef __linux__
#include <stdint.h>
#include <unistd.h>
#include "os/linux/stuff.h"
#include <retromode.h>
#include <retromode_lib.h>
#endif

#include <amosKittens.h>
#include <stack.h>
#include "debug.h"
#include "commands.h"
#include "commandsData.h"
#include "kittyErrors.h"
#include "engine.h"
#include "amosstring.h"

extern std::vector<struct keyboard_buffer> keyboardBuffer;

extern ULONG *codeset_page;
extern struct globalVar globalVars[];

int _scancode;
int _keyshift;

extern int bobDoUpdate ;
extern int bobUpdateNextWait ;

extern bool next_print_line_feed;
extern char *(*_do_set) ( struct glueCommands *data, int nextToken ) ;

char *_setVar( struct glueCommands *data,int nextToken );

void _input_arg( struct nativeCommand *cmd, char *tokenBuffer );
void _inputLine_arg( struct nativeCommand *cmd, char *tokenBuffer );
void __print_text( struct retroScreen *screen, struct stringData *txt, int maxchars);
void __print_char_array( struct retroScreen *screen, const char *txt, int maxchars);

int input_count = 0;
std::string input_str;

using namespace std;

int keyboardLag = 20*1000/50;	// ms
int keyboardSpeed = 10*1000/50;	// ms


#ifdef __linux__

void atomic_get_char( char *buf)
{

}

#endif

#ifdef __amigaos4__

struct keyboard_buffer current_key;

char *scancodeToTxt( unsigned int scancode, int qualifier)
{
	ULONG actual;
	char buffer[20];
	struct InputEvent event;
	bzero(&event,sizeof(struct InputEvent));

	event.ie_NextEvent	=	0;
	event.ie_Class		=	IECLASS_RAWKEY;
	event.ie_SubClass	=	0;

	if (scancode)
	{
		event.ie_Code = scancode;
		event.ie_Qualifier = qualifier;
		actual = MapRawKey(&event, buffer, 20, 0);

		if (actual)
		{
			char buf[2];
			buf[0] = buffer[0];
			buf[1]=0;
			return strdup(buf);
		}
	}
	return strdup("");
}

// only for keyboard handling

void handel_key( const struct keyboard_buffer *current_key, char *buf_out )
{
	ULONG actual;
	char buffer[20];
	struct InputEvent event;
	bzero(&event,sizeof(struct InputEvent));

	event.ie_NextEvent	=	0;
	event.ie_Class		=	IECLASS_RAWKEY;
	event.ie_SubClass	=	0;

	if (current_key -> Code)
	{
		_scancode = current_key -> Code;

		event.ie_Code = current_key -> Code;
		event.ie_Qualifier = current_key -> Qualifier;
		actual = MapRawKey(&event, buffer, 20, 0);

		if (actual)
		{
			buf_out[0] = buffer[0];
			buf_out[1]=0;
		}
	}
	else
	{
		buf_out[0] = keyboardBuffer[0].Char;
		buf_out[1]=0;
	}
}


struct timeval key_press_time;
struct timeval repeat_time;

#define delta_time_ms(t1,t2) (((t2.tv_sec - t1.tv_sec) *1000)+((t2.tv_usec - t1.tv_usec) /1000))

void atomic_get_char( struct stringData *str)
{
	struct timeval ctime;
	char *buf = &str->ptr;
	buf[0]=0;
	buf[1]=0;
	str -> size = 0;

	if (engine_ready())
	{
		engine_lock();
		if ( ! keyboardBuffer.empty() )
		{
			current_key = keyboardBuffer[0];

			if (current_key.event == kitty_key_down )
			{
				handel_key( &current_key, buf );
				str -> size = buf[0] ? 1: 0;

				gettimeofday(&key_press_time, NULL);
				gettimeofday(&repeat_time, NULL);
			}

			keyboardBuffer.erase(keyboardBuffer.begin());
		}
		else
		{
			if (current_key.event == kitty_key_down )
			{
				gettimeofday(&ctime, NULL);
	
				if (delta_time_ms(key_press_time,ctime) > (keyboardLag+keyboardSpeed))
				{
#if 0
					printf("delta_time_ms(key_press_time,ctime): %d > keyboardLag: %d + keyboardSpeed: %d\n", 
							delta_time_ms(key_press_time,ctime),
							keyboardLag,keyboardSpeed); 
#endif

					if (delta_time_ms(repeat_time,ctime) > keyboardSpeed  )
					{
						handel_key( &current_key, buf );
						str -> size = buf[0] ? 1: 0;

						gettimeofday(&repeat_time, NULL);
					}
				}
			}
		}

		engine_unlock();
	}
}

#endif

void kitty_getline(string &input)
{
	bool done = false;

	struct stringData * str = alloc_amos_string( 1 );

	int rightx;
	int charsPerRow;
	int charspace;

	int cursx = 0;
	int scrollx = 0;

	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	input = "";
	
	retroScreen *screen = instance.screens[instance.current_screen];
	retroTextWindow *textWindow = NULL;

	if (screen)
	{
		textWindow = screen -> currentTextWindow;
	}

	if ((engine_ready())&&(textWindow))
	{
		draw_cursor( screen );
		rightx = textWindow -> locateX;
		charsPerRow = textWindow -> charsPerRow;
		charspace = charsPerRow-rightx - 1;

		done = false;

		do
		{
			atomic_get_char(str);

#ifdef __amigaos4__			
			WaitTOF();
#endif
#ifdef __linux__
			sleep(1);
#endif

			if (str -> size != 0) printf("char %d\n",str -> ptr);

			switch (str->ptr)
			{
				case 0:

					break;

				case 8:

					if (input.length()>0)
					{
						if (cursx == (int) input.length())
						{
							cursx --;
							input.erase(cursx,1);
						}

						clear_cursor( instance.screens[instance.current_screen] );
						textWindow -> locateX = rightx;

						if (cursx - scrollx < 0) scrollx--;

						__print_char_array( instance.screens[instance.current_screen], input.c_str() + scrollx,charspace);
						clear_cursor( instance.screens[instance.current_screen] );
						draw_cursor( instance.screens[instance.current_screen] );
					}
					break;

				case 13:

					done=true;
					break;

				default:
					input += str->ptr;
					cursx ++;

					clear_cursor( instance.screens[instance.current_screen] );
					textWindow -> locateX = rightx;

					if (cursx - scrollx > charspace) scrollx++;
					__print_char_array( screen, input.c_str() + scrollx,charspace);

					draw_cursor( instance.screens[instance.current_screen] );
					break;	
			}

		} while ( (done == false) && (instance.engine_stopped==false) );
	}
	else
	{
		getline(cin, input);
	}

	if (str) sys_free(str);
}

char *cmdWaitKey(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	struct stringData * str = alloc_amos_string( 1 );

	do
	{
		engine_lock();
		if (bobUpdateNextWait)
		{
			bobDoUpdate = 1;
			bobUpdateNextWait = 0;
		}
		engine_unlock();

		atomic_get_char( str );
		Delay(1);

		if (engine_ready() == false) break;

	} while ( str->size == 0 );

	freeString(str);

	return tokenBuffer;
}


char *cmdInkey(struct nativeCommand *cmd, char *tokenBuffer )
{
	struct stringData * str = alloc_amos_string( 1 );

	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	_scancode = 0;
	atomic_get_char( str );
	setStackStr(str);

	return tokenBuffer;
}

char *_InputStrN( struct glueCommands *data, int nextToken )
{
	int args =__stack - data->stack +1 ;
	struct stringData * str = alloc_amos_string( 1 );
	string tmp;

	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	_scancode = 0;

	if (args==1)
	{
		int n = getStackNum(__stack );
		while ((int) tmp.length()<n)
		{
			atomic_get_char( str);
			tmp += str -> ptr;

#ifdef __amigaos4__
			WaitTOF();
#else
			sleep(1);
#endif
		}
	}
	else
	{
		setError(22,data->tokenBuffer);
	}

	{
		struct stringData *ret = alloc_amos_string( tmp.length() );
		sprintf( &ret -> ptr, "%s", tmp.c_str()  );
		setStackStr( ret ); 
	}

	if (str) sys_free(str);

	return NULL;
}


char *cmdScancode(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	setStackNum(_scancode);
	return tokenBuffer;
}

char *cmdScanshift(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	// 0 no shift
	// 1 Left shift
	// 2 right shift
	// 3 both shifts 

	setStackNum(_keyshift & 3);
	return tokenBuffer;
}

char *cmdKeyShift(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	setStackNum(_keyshift);
	return tokenBuffer;
}


char *cmdClearKey(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	engine_lock();
	keyboardBuffer.erase(keyboardBuffer.begin(),keyboardBuffer.begin()+keyboardBuffer.size());
	engine_unlock();

	_scancode = 0;

	setStackNum(0);
	return tokenBuffer;
}


char *_cmdKeyState( struct glueCommands *data,int nextToken )
{
	int args =__stack - data->stack +1 ;
	bool success = false;
	int ret = 0;

	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	if (args==1)
	{
		int key = getStackNum(__stack );

		if ((key>-1)&&(key<256))
		{
			ret = instance.engine_key_state[key];
			success = true;
		}
	}

	if (success == false) setError(22,data->tokenBuffer);

	popStack(__stack - data->stack );
	setStackNum(ret);
	return NULL;
}

char *cmdKeyState(struct nativeCommand *cmd, char *tokenBuffer )
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdParm( _cmdKeyState, tokenBuffer );
	return tokenBuffer;
}

char *_Input( struct glueCommands *data,int nextToken )
{
	int args =__stack - data -> stack +1;
	_input_arg( NULL, NULL );
	popStack(__stack - data -> stack  );
	do_input[instance.parenthesis_count] = do_std_next_arg;
	do_breakdata = NULL;
	return NULL;
}

char *_LineInput( struct glueCommands *data,int nextToken )
{
	int args =__stack - data -> stack +1;
	_inputLine_arg( NULL, NULL );
	popStack(__stack - data -> stack  );
	do_input[instance.parenthesis_count] = do_std_next_arg;
	do_breakdata = NULL;
	return NULL;
}


extern int _last_var_index;		// we need to know what index was to keep it.
extern int _set_var_index;		// we need to resore index 

void _input_arg( struct nativeCommand *cmd, char *tokenBuffer )
{
	int args = 0;
	size_t i;
	std::string arg = "";
	struct glueCommands data;
	bool success = false;
	int num;
	double des;
	struct retroScreen *screen = instance.screens[instance.current_screen];

	if (cmd == NULL)
	{
		args =__stack - cmdTmp[instance.cmdStack].stack + 1;
	}
	else
	{
		if (instance.cmdStack) if (cmdTmp[instance.cmdStack-1].cmd == _Input)
		{
			args =__stack - cmdTmp[instance.cmdStack-1].stack + 1;
		}
	}
	
	if ((input_count == 0)&&(__stack))		// should be one arg.
	{
		struct stringData *str = getStackString(__stack-args+1 );
		if (str)  __print_text( screen, str ,0 );
	}
	else if (input_str.empty())
	{
		__print_char_array( screen, "??? ",0);
	}

	do
	{
		do
		{
			while (input_str.empty() && engine_ready() ) kitty_getline( input_str);

			i = input_str.find(",");	
			if (i != std::string::npos)
			{
				arg = input_str.substr(0,i); input_str.erase(0,i+1);
			}
			else	
			{
				arg = input_str; input_str = "";
			}
		}
		while ( arg.empty() && engine_ready() );

		if (last_var)
		{
			switch (globalVars[last_var -1].var.type & 7)
			{	
				case type_string:
					success = true; break;
				case type_int:
					success = arg.find_first_not_of( "-0123456789" ) == std::string::npos; break;
				case type_float:
					success = arg.find_first_not_of( "-0123456789." ) == std::string::npos; break;
			}
		}
	}
	while (!success && engine_ready());

	clear_cursor( instance.screens[instance.current_screen] );

	__print_char_array( screen, "\n" ,0 );

	if (last_var)
	{
		switch (globalVars[last_var -1].var.type & 7)
		{	
			case type_string:
				{
					struct stringData *ret = alloc_amos_string( arg.length() );
					sprintf( &ret -> ptr, "%s", arg.c_str()  );
					setStackStr( ret ); 
				}
				break;

			case type_int:
				sscanf(arg.c_str(),"%d",&num); setStackNum(num); 
				break;

			case type_float:
				sscanf(arg.c_str(),"%lf",&des); setStackDecimal(des); 
				break;
		}
	}

	data.lastVar = last_var;
	_set_var_index = _last_var_index;

	_setVar( &data,0 );
	input_count ++;
}

void _inputLine_arg( struct nativeCommand *cmd, char *tokenBuffer )
{
	int args = 0;
	std::string arg = "";
	struct glueCommands data;
	bool success = false;
	int num;
	double des;
	struct retroScreen *screen = instance.screens[instance.current_screen];

	if (cmd == NULL)
	{
		args =__stack - cmdTmp[instance.cmdStack].stack + 1;
	}
	else
	{
		if (instance.cmdStack) if (cmdTmp[instance.cmdStack-1].cmd == _Input)
		{
			args =__stack - cmdTmp[instance.cmdStack-1].stack + 1;
		}
	}
	
	if (input_count == 0)		// should be one arg.
	{
		struct stringData *str = getStackString(__stack );
		bool have_question = false;

		if (str) if (str -> size)
		{
			__print_text( screen, str ,0 );
			have_question = true;
		}

		if (have_question == false)
		{
			__print_char_array( screen, "? ",0);
		}
	}
	else if (input_str.empty())
	{
		__print_char_array( screen, "?? ",0);
	}

	engine_lock();
	draw_cursor( instance.screens[instance.current_screen] );
	engine_unlock();

	do
	{
		do
		{
			while (input_str.empty() && engine_ready() ) kitty_getline(input_str);

			clear_cursor( instance.screens[instance.current_screen] );

			arg = input_str; input_str = "";
		}
		while ( arg.empty() && engine_ready() );

		if (last_var)
		{
			switch (globalVars[last_var -1].var.type & 7)
			{	
				case type_string:
					success = true; break;
				case type_int:
					success = arg.find_first_not_of( "-0123456789" ) == std::string::npos; break;
				case type_float:
					success = arg.find_first_not_of( "-0123456789." ) == std::string::npos; break;
			}
		}

		printf("%s:%d -- success = %s\n",__FUNCTION__,__LINE__, success ? "True" : "False");

	}
	while ( (!success) && engine_ready() );

	__print_char_array( screen, "\n" ,0 );

	switch (globalVars[last_var -1].var.type & 7)
	{	
		case type_string:
			{
				struct stringData *ret = alloc_amos_string( arg.length() );
				sprintf( &ret -> ptr, "%s", arg.c_str()  );
				setStackStr( ret ); 
			}
			break;

		case type_int:
			sscanf(arg.c_str(),"%d",&num); setStackNum(num); break;

		case type_float:
			sscanf(arg.c_str(),"%lf",&des); setStackDecimal(des); break;
	}

	data.lastVar = last_var;
	_setVar( &data,0 );
	input_count ++;
}

void breakdata_inc_stack( struct nativeCommand *cmd, char *tokenBuffer )
{
	__stack++;
}

char *cmdInput(nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	struct retroScreen *screen = instance.screens[instance.current_screen];
	input_count = 0;
	input_str = "";

	if (screen) clear_cursor(screen);
	if (next_print_line_feed == true) __print_char_array( screen, "\n",0);
	next_print_line_feed = true;

	do_input[instance.parenthesis_count] = _input_arg;
	do_breakdata = breakdata_inc_stack;
	stackCmdNormal( _Input, tokenBuffer );

	return tokenBuffer;
}

char *cmdInputStrN(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);

	input_count = 0;
	input_str = "";

	do_input[instance.parenthesis_count] = _input_arg;
	do_breakdata = breakdata_inc_stack;
	stackCmdNormal( _InputStrN, tokenBuffer );

	return tokenBuffer;
}

char *cmdLineInput(nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	struct retroScreen *screen = instance.screens[instance.current_screen];

	if (screen) clear_cursor(screen);
	if (next_print_line_feed == true) __print_char_array(screen,"\n",0);
	next_print_line_feed = true;

	input_count = 0;
	input_str = "";

	do_input[instance.parenthesis_count] = _inputLine_arg;
	do_breakdata = breakdata_inc_stack;
	stackCmdNormal( _LineInput, tokenBuffer );

	return tokenBuffer;
}

char *_cmdPutKey( struct glueCommands *data,int nextToken )
{
	int args =__stack - data -> stack +1;

	if (args==1)
	{
		struct stringData *str = getStackString(__stack);
		if (str)
		{
			char *c;
			char *se = &(str -> ptr) + str -> size;
			for (c = &str -> ptr; c<se; c++)
			{
				atomic_add_key( kitty_key_down, 0,0, *c );
				atomic_add_key( kitty_key_up, 0,0, *c );
			}
		}
	}
	else
	{
		setError(22,data->tokenBuffer);
	}

	popStack(__stack - data -> stack  );
	return NULL;
}


char *cmdPutKey(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%s:%d\n",__FILE__,__FUNCTION__,__LINE__);
	stackCmdNormal( _cmdPutKey, tokenBuffer );
	return tokenBuffer;
}

char *_cmdKeySpeed( struct glueCommands *data,int nextToken )
{
	int args =__stack - data -> stack +1;

	if (args==2)
	{
		keyboardLag = getStackNum(__stack-1) * 1000 / 50;
		keyboardSpeed = getStackNum(__stack) * 1000 / 50;
	}
	else setError(22,data->tokenBuffer);

	popStack(__stack - data -> stack  );
	return NULL;
}

char *cmdKeySpeed(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _cmdKeySpeed, tokenBuffer );
	return tokenBuffer;
}

struct stringData *F1_keys[20];

int keyStr_index =0;

char *_set_keyStr( struct glueCommands *data, int nextToken )
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);


	if ((keyStr_index>=1)&&(keyStr_index<=20))
	{
		if (F1_keys[keyStr_index-1])
		{
			sys_free(F1_keys[keyStr_index-1]);
			F1_keys[keyStr_index-1] = NULL;
		}

		F1_keys[keyStr_index-1] = amos_strdup(getStackString(__stack));
	}

	_do_set = _setVar;
	return NULL;
}

char *_cmdKeyStr( struct glueCommands *data,int nextToken )
{
	int args =__stack - data -> stack +1;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	keyStr_index = getStackNum(__stack );
	_do_set = _set_keyStr;

	popStack(__stack - data -> stack  );
	return NULL;
}

char *cmdKeyStr(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdIndex( _cmdKeyStr, tokenBuffer );
	return tokenBuffer;
}

