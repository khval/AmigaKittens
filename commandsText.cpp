#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include "debug.h"
#include <string>
#include <iostream>
#include <proto/retroMode.h>

#include "stack.h"
#include "amosKittens.h"
#include "commandsData.h"
#include "commandsText.h"
#include "commandsScreens.h"
#include "errors.h"
#include "engine.h"
#include "bitmap_font.h"

extern int pen0;
extern int pen1;
extern int pen2;
extern int last_var;
extern struct retroScreen *screens[8] ;
extern struct retroVideo *video;
extern int current_screen;
extern int cursor_color;

bool curs_on = true;
int pen = 2;
int paper = 1;
int _tab_size = 3;

bool next_print_line_feed = false;
void _print_break( struct nativeCommand *cmd, char *tokenBuffer );

char *_textLocate( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	struct retroScreen *screen; 
	bool success = false;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==2)
	{
		if (screen=screens[current_screen])
		{
			clear_cursor(screen);
			screen -> locateX = getStackNum( stack-1 );
			screen -> locateY = getStackNum( stack );
			draw_cursor(screen);
		}
		next_print_line_feed = false;
		success = true;
	}

	if (success == false) setError(22);

	popStack( stack - data->stack );
	return NULL;
}

char *_textHome( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	struct retroScreen *screen; 
	bool success = false;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
		if (screen=screens[current_screen])
		{
			clear_cursor(screen);
			screen -> locateX = 0;
			screen -> locateY = 0;
			draw_cursor(screen);
		}
		next_print_line_feed = false;
		success = true;
	}

	if (success == false) setError(22);

	popStack( stack - data->stack );
	return NULL;
}


char *textLocate(struct nativeCommand *cmd, char *tokenBuffer)
{
	stackCmdNormal( _textLocate, tokenBuffer );
	setStackNone();
	return tokenBuffer;
}

char *_textPen( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;

	bool success = false;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
		pen  = getStackNum( stack );
		success = true;
	}

	if (success == false) setError(22);

	popStack( stack - data->stack );
	return NULL;
}

char *textPen(struct nativeCommand *cmd, char *tokenBuffer)
{
	stackCmdNormal( _textPen, tokenBuffer );
	return tokenBuffer;
}

char *_textPaper( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;

	bool success = false;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
		paper = getStackNum( stack );
		success = true;
	}

	if (success == false) setError(22);

	popStack( stack - data->stack );
	return NULL;
}

void __print_text(const char *txt, int maxchars)
{
	if (engine_started)
	{
		if (screens[current_screen])
		{
			_my_print_text(  screens[current_screen], (char *) txt, maxchars);
		}
	}
	else
	{
		printf("%s", txt);
	}
}

void __print_num( int num )
{
	char tmp[50];

	if (num>-1)
	{
		sprintf(tmp," %d",num);
	}
	else
	{
		sprintf(tmp,"%d",num);
	}
	__print_text(tmp,0);
}

void __print_double( double d )
{
	char tmp[40];

	if (d>=0.0)
	{
		sprintf(tmp," %0.3lf",d);
	}
	else
	{
		sprintf(tmp,"%0.3lf",d);
	}
	__print_text(tmp,0);
}

char *_print( struct glueCommands *data, int nextToken )
{
	int n;

	for (n=data->stack;n<=stack;n++)
	{
		switch (kittyStack[n].type)
		{
			case type_int:
				__print_num( kittyStack[n].value);
				break;
			case type_float:
				__print_double( kittyStack[n].decimal);
				break;
			case type_string:
				if (kittyStack[n].str) __print_text(kittyStack[n].str,0);
				break;
			case type_none:
				if (n>data->stack) next_print_line_feed = false;
				break;
		}

		if ((n<stack)&&( kittyStack[n+1].type != type_none ))  __print_text("    ",0);
	}

	if (screens[current_screen]) draw_cursor(screens[current_screen]);

	popStack( stack - data->stack );
	do_breakdata = NULL;	// done doing that.

	return NULL;
}

char *_textCentre( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	struct retroScreen *screen; 
	int n;
	int charsPerLine = 100;
	const char *txt = NULL;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args!=1) setError(22);

	if (engine_started)
	{
		if (screen = screens[current_screen])
		{
			txt = getStackString(stack);
			charsPerLine = screen -> realWidth / 8;

			clear_cursor(screen);

			if (txt)
			{
				screen -> locateX = (charsPerLine/2) - (strlen( txt ) / 2);

				if (screen -> locateX<0)
				{
					txt -= screen -> locateX;	// its read only.
					screen -> locateX = 0;
				}
			}
		}
	}

	if (txt)
	{
		__print_text(txt,0);
	}

	if (screens[current_screen]) draw_cursor(screens[current_screen]);

	popStack( stack - data->stack );
	do_breakdata = NULL;	// done doing that.

	return NULL;
}

char *textPaper(struct nativeCommand *cmd, char *tokenBuffer)
{
	stackCmdNormal( _textPaper, tokenBuffer );
	return tokenBuffer;
}

char *textCentre(struct nativeCommand *cmd, char *tokenBuffer)
{
	stackCmdNormal( _textCentre, tokenBuffer );
	return tokenBuffer;
}

char *_addDataToText( struct glueCommands *data, int nextToken );

void _print_break( struct nativeCommand *cmd, char *tokenBuffer )
{
	stackCmdParm( _addDataToText, tokenBuffer );
	stack++;
 	kittyStack[stack].type = type_none;
}

char *textPrint(nativeCommand *cmd, char *ptr)
{
	stackCmdNormal( _print, ptr );
	do_breakdata = _print_break;

	if (screens[current_screen]) clear_cursor(screens[current_screen]);
	if (next_print_line_feed == true) __print_text("\n",0);
	next_print_line_feed = true;

	setStackNone();

	return ptr;
}

char *textCursOff(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	curs_on = false;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	return tokenBuffer;
}

char *textCursOn(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	curs_on = true;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	return tokenBuffer;
}

char *textMemorizeX(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}

char *textMemorizeY(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}

char *textRememberX(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}

char *textRememberY(struct nativeCommand *cmd, char *tokenBuffer)
{
	clear_cursor(screens[current_screen]);
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}


char *textHome(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textHome, tokenBuffer );
	return tokenBuffer;
}

char *textInverseOn(struct nativeCommand *cmd, char *tokenBuffer)
{
	int t;

	t = pen0 ;
	pen0 = pen1;
	pen1 = t;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}

char *textInverseOff(struct nativeCommand *cmd, char *tokenBuffer)
{
	int t;

	t = pen0 ;
	pen0 = pen1;
	pen1 = t;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	return tokenBuffer;
}

char *_textBorderStr( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	char *newstr = NULL;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 2)
	{
		char *txt = getStackString( stack-1 );
		int border = getStackNum( stack );

		if ((txt)&&(border>=0)&&(border<16))
		{
			newstr = (char *) malloc( strlen(txt) + 6 + 1 ); 
			if (newstr)
			{
				sprintf(newstr,"%cE0%s%cR%c",27,txt,27,48+ border );
			}
		}

		if (newstr == NULL) setError(60);
	}
	else setError(22);

	popStack( stack - data->stack );
	if (newstr) setStackStr( newstr );

	return NULL;
}

char *textBorderStr(struct nativeCommand *cmd, char *tokenBuffer)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textBorderStr, tokenBuffer );
	return tokenBuffer;
}

char *_textAt( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	int x =-1,y= -1;
	char str[] = {27,'X','0',27,'Y','0',0};

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 2)
	{
		if (screens[current_screen])
		{
			x = screens[current_screen] -> locateX ;
			y = screens[current_screen] -> locateY ;
		}
		
		 stack_get_if_int( stack -1,&x);
		 stack_get_if_int( stack,&y);

		if (x>-1) str[2]='0'+x;
		if (y>-1) str[5]='0'+y;
	}
	else setError(22);

	popStack( stack - data->stack );
	setStackStrDup( str );

	return NULL;
}

extern char *textAt(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textAt, ptr );
	kittyStack[stack].type = type_none; 
	return ptr;
}

char *_textPenStr( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	char str[] = {27,'P','0',0};

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 1)
	{
		int n = getStackNum( stack );
		if (n>-1) str[2]='0'+n;
	}
	else setError(22);

	popStack( stack - data->stack );
	setStackStrDup( str );

	return NULL;
}

extern char *textPenStr(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textPenStr, ptr );
	return ptr;
}

char *_textPaperStr( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	char str[] = {27,'B','0',0};

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 1)
	{
		int n = getStackNum( stack );
		if (n>-1) str[2]='0'+n;
	}
	else setError(22);

	popStack( stack - data->stack );
	setStackStrDup( str );

	return NULL;
}

extern char *textPaperStr(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textPaperStr, ptr );
	return ptr;
}

char *_textWriting( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	popStack( stack - data->stack );
	return NULL;
}

char *textWriting(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textWriting, ptr );
	return ptr;
}

char *_textShadeOff( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	popStack( stack - data->stack );
	return NULL;
}

char *textShadeOff(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textShadeOff, ptr );
	return ptr;
}

char *_textShadeOn( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	popStack( stack - data->stack );
	return NULL;
}

char *textShadeOn(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textShadeOn, ptr );
	return ptr;
}

char *_textUnderOff( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	popStack( stack - data->stack );
	return NULL;
}

char *textUnderOff(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textShadeOn, ptr );
	return ptr;
}

char *_textUnderOn( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	popStack( stack - data->stack );
	return NULL;
}

char *textUnderOn(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textShadeOn, ptr );
	return ptr;
}

char *_textXText( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	int n = 0;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 1)
	{
		n = getStackNum( stack ) / 8;
	}
	else setError(22);

	popStack( stack - data->stack );
	setStackNum( n  );

	return NULL;
}

char *textXText(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textXText, ptr );
	return ptr;
}

char *_textYText( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	int n = 0;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 1)
	{
		n = getStackNum( stack ) / 8;
	}
	else setError(22);

	popStack( stack - data->stack );
	setStackNum( n  );

	return NULL;
}

char *textYText(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdParm( _textYText, ptr );
	return ptr;
}

char *_textCMove( struct glueCommands *data, int nextToken )
{
	struct retroScreen *screen; 
	int args = stack - data->stack +1 ;
	int x = 0, y = 0;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 2)
	{
		if (screen = screens[current_screen])
		{
			clear_cursor(screen);

			x = screen -> locateX ;
			y = screen -> locateY ;

			x += getStackNum( stack - 1 ) ;
			y += getStackNum( stack ) ;

			screen -> locateX = x;
			screen -> locateY = y;

			draw_cursor(screen);
		}
	}
	else setError(22);

	popStack( stack - data->stack );

	return NULL;
}

char *textCMove(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textCMove, ptr );
	setStackDecimal(0);
	return ptr;
}

char *textCLeft(nativeCommand *cmd, char *ptr)
{
	if (screens[current_screen])
	{
		clear_cursor(screens[current_screen]);
		screens[current_screen] -> locateX-- ;
		if (screens[current_screen] -> locateX<0) screens[current_screen] -> locateX = screens[current_screen] -> realWidth / 8;
		draw_cursor(screens[current_screen]);
	}
	return ptr;
}

char *textCRight(nativeCommand *cmd, char *ptr)
{
	if (screens[current_screen])
	{
		clear_cursor(screens[current_screen]);
		screens[current_screen] -> locateX++ ;
		draw_cursor(screens[current_screen]);
	}
	return ptr;
}

char *textCUp(nativeCommand *cmd, char *ptr)
{
	if (screens[current_screen])
	{
		clear_cursor(screens[current_screen]);
		screens[current_screen] -> locateY--;
		if (screens[current_screen] -> locateY<0) screens[current_screen] -> locateY = 0;
		draw_cursor(screens[current_screen]);
	}
	return ptr;
}


char *textCDown(nativeCommand *cmd, char *ptr)
{
	if (screens[current_screen])
	{
		clear_cursor(screens[current_screen]);
		screens[current_screen] -> locateY++ ;
		draw_cursor(screens[current_screen]);
	}
	return ptr;
}


char *_textSetTab( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	int x = 0, y = 0;

	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args == 1)
	{
		_tab_size = getStackNum(stack);
	}
	else setError(22);

	popStack( stack - data->stack );

	return NULL;
}

char *textSetTab(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textSetTab, ptr );
	setStackDecimal(0);
	return ptr;
}

char *_textSetCurs( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	popStack( stack - data->stack );
	return NULL;
}

char *textSetCurs(nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textSetCurs, ptr );

	return ptr;
}

char *_textCursPen( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
		cursor_color = getStackNum( stack ) ;
		if (screens[current_screen]) draw_cursor(screens[current_screen]);
	}

	popStack( stack - data->stack );
	return NULL;
}

char *textCursPen(struct nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textCursPen, ptr );

	return ptr;
}

char *_textVscroll( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
	}

	popStack( stack - data->stack );
	return NULL;
}

char *textVscroll(struct nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textVscroll, ptr );
	return ptr;
}

char *_textHscroll( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);

	if (args==1)
	{
	}

	popStack( stack - data->stack );
	return NULL;
}

char *textHscroll(struct nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textVscroll, ptr );
	return ptr;
}


char *_textCline( struct glueCommands *data, int nextToken )
{
	int args = stack - data->stack +1 ;
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	int chars = 100000;
	int x0=0,y0,x1=0,y1;

	struct retroScreen *screen = screens[current_screen];

	if ((args==1) && (screen))
	{
		y0 = screen -> locateY *8;
		y1 = y0+7;

		switch(kittyStack[stack].type)
		{
			case type_none:
					x0 = 0;
					x1 = screen -> realWidth;
					break;
			case type_int:
					chars = getStackNum( stack );
					x0 = screen -> locateY *8;
					x1 = x0 + 7;
					if (chars<0) x0 += chars * 8;
					if (chars>0) x1 += chars * 8;
					break;
		}
		
		retroBAR(screen,x0,y0,x1,y1,paper);
	}

	popStack( stack - data->stack );
	return NULL;
}

char *textCline(struct nativeCommand *cmd, char *ptr)
{
	proc_names_printf("%s:%d\n",__FUNCTION__,__LINE__);
	stackCmdNormal( _textCline, ptr );
	setStackNone();
	return ptr;
}
