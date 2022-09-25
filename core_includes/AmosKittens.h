
#ifndef __amoskittens_h__

#if defined(_MSC_VER) || defined(_linux_)
typedef bool BOOL;
typedef void* APTR;
#endif

#ifdef _MSC_VER
#define PACKED 
#else
#define PACKED __attribute__((packed))
#endif
#define __amoskittens_h__

#ifdef __linux__
#define allocStruct(name,items) (struct type *) malloc( sizeof(struct type) * items )
#define allocArrayData(type,size) (struct type ## ArrayData *) malloc( sizeof(struct type ## ArrayData) + size )
#define freeStruct(adr) free( adr )
#define sys_free FreeVec
#define freeString(adr) sys_free(adr)
#endif

#if defined(__amigaos4__)
#define allocStruct(name,items) (struct name*) AllocVecTags( sizeof(struct name) * items , AVT_Type, MEMF_SHARED, TAG_END )
#define allocType(name,items) (name *) AllocVecTags( sizeof(name) * items , AVT_Type, MEMF_SHARED, TAG_END )
#define allocArrayData(type,size) (struct type ## ArrayData *) AllocVecTags( sizeof(struct type ## ArrayData) + size , AVT_Type, MEMF_SHARED, TAG_END )
#define freeStruct(adr) FreeVec( adr )
#define sys_public_alloc(size) AllocVecTags( size, AVT_Type, MEMF_SHARED, TAG_END )
#define sys_public_alloc_clear(size) AllocVecTags( size, AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END )
#define sys_priv_alloc(size) AllocVecTags( size, AVT_Type, MEMF_PRIVATE, TAG_END )
#define sys_priv_alloc_clear(size) AllocVecTags( size, AVT_Type, MEMF_PRIVATE, AVT_ClearWithValue, 0, TAG_END )
#define sys_memavail_gfxmem()
#define sys_memavail_sysmem() AvailMem(MEMF_ANY)
#define sys_free FreeVec
#define freeString(ptr) FreeVec(ptr)
#endif

#define PROC_STACK_SIZE 1000
#define VAR_BUFFERS 1000
#define MAX_PARENTHESIS_COUNT 1000

#define token_newLine	0x0000
#define token_index		0x0074
#define token_comma	0x005C
#define token_nextCmd	0x0054

#define token_semi		0xFF04
#define token_xor		0xFF3E
#define token_or		0xFF4C
#define token_and		0xFF58
#define token_mod		0xFFD4
#define token_add		0xFFC0
#define token_sub		0xFFCA
#define token_mul		0xFFE2
#define token_div		0xFFEC
#define token_power	0xFFF6

#define token_parenthesis_start	0x0074
#define token_parenthesis_end	0x007C

#define token_more_or_equal		0xFF8E
#define token_less_or_equal		0xFF7A
#define token_less_or_equal2		0xFF84
#define token_more_or_equal2	0xFF98
#define token_not_equal			0xFF66
#define token_equal			0xFFA2
#define token_more				0xFFB6
#define token_less				0xFFAC

#define token_trap				0x259A

#define token_goto				0x02A8
#define token_gosub			0x02B2
#define token_proc				0x0386

extern unsigned int amiga_joystick_dir[4];
extern unsigned int amiga_joystick_button[4];

#define NEXT_TOKEN(ptr) *((unsigned short *) ptr)
#define NEXT_INT(ptr) *((int *) (ptr+2))

enum
{
	mode_standard,		// 0
	mode_alloc,			// 1
	mode_input,			// 2
	mode_goto,			// 3
	mode_logical,			// 4
	mode_store,			// 5
	mode_for				// 6
};

#define cmd_normal			0x0001
#define cmd_index			0x0002 
#define cmd_para			0x0004
#define cmd_loop			0x0008
#define cmd_proc			0x0010
#define cmd_onEol			0x0020
#define cmd_onNextCmd		0x0040
#define cmd_onComma		0x0080
#define cmd_onBreak		0x0100
#define cmd_never			0x0200
#define cmd_exit			0x0400
#define cmd_true			0x0800
#define cmd_false			0x1000

enum
{
	type_undefined = 0,
	type_int = 0,
	type_float,		// 1
	type_string,		// 2
	type_file,			// 3
	type_proc = 4,	
	type_array = 8	,	// I'm sure AMOS don't use this, but we do.
	type_none =16,
	type_blocked = 32,			// used in the stack
	type_hidden_blocked = 64		// used in the stack.
};

struct KittyInstance;

#ifdef __amoskittens__
#define KITTENS_CMD_ARGS (struct nativeCommand *cmd, char *tokenBuffer)
#define EXT_CMD_ARGS (struct KittyInstance *instance , struct nativeCommand *cmd, char *tokenBuffer)
#else
#define KITTENS_CMD_ARGS (struct KittyInstance *instance , struct nativeCommand *cmd, char *tokenBuffer)
#endif


struct nativeCommand
{
	int id;
	const char *name;
	int size;
	char *(*fn) KITTENS_CMD_ARGS;
};

enum 
{
	glue_option_for_int = 1,
	glue_option_for_float
};

struct fileContext
{
	char *path;
	char *name;
	unsigned int file;
	unsigned int length;
	unsigned int tokenLength;
	unsigned char *lineStart;
	unsigned char *start;
	unsigned char *ptr;
	unsigned char *end;
	unsigned int lineNumber;
	unsigned int bankSize;
	unsigned char *bank;
};

struct kittyForInt
{
	int step;	
	int have_to;
};

struct kittyForDouble 
{
	double step;	
	double have_to;
};

struct KittyInstance;

struct glueCommands
{
	char *(*cmd) ( struct glueCommands *data, int nextToken );	// can return token location
	char *tokenBuffer;
	
	union
	{
		char *tokenBuffer2;		// a place to store a 2en token buffer pos.
		char *FOR_NUM_TOKENBUFFER;
	};

	union
	{
		int flag;				// should remove flag and use cmd_type
		int cmd_type;
	};

	int lastVar;
	int token;

	int optionsType;
	struct kittyForInt optionsInt;
	struct kittyForDouble optionsFloat;

	int stack;
	int parenthesis_count;
	struct KittyInstance *instance;
};

struct proc 
{
	char *name;
	int ref;
};

struct extension
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	unsigned char ext;
	unsigned char	__align__;
	unsigned short token;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct extension_lib
{
	struct Library *base;
#ifdef amigaos4
	struct Interface *interface;
#endif
	char	*lookup;
	uint32_t crc;
};

struct stringData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	uint16_t size;
	char ptr;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct desimalData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	double value;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct valueData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	int value;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct stringArrayData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	uint16_t size;
	struct stringData *ptr;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct desimalArrayData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	uint16_t size;
	struct desimalData ptr;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct valueArrayData 
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t type;
	uint16_t size;
	struct valueData ptr;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED;

struct kittyData
{
	int count;
	
	union
	{
		struct stringData *str;
		struct valueArrayData *int_array;
		struct desimalArrayData *float_array;
		struct stringArrayData *str_array;
		char *tokenBufferPos;
	};

	union		// we don't need to wast space.
	{
		int index;
		int prev_last_token;
	};

	int cells;

	union
	{
		int *sizeTab;
	};

	struct valueData integer;
	struct desimalData decimal;
	int type;
};

struct kittyVideoInfo
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	uint16_t videoWidth;		// 2
	uint16_t videoHeight;	// 4
	uint16_t display_x;		// 6
	uint16_t display_y; 		// 8
	char _dummy_[20];		// 28
	uint16_t rgb[32];
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED ; 

struct kittyInfo		// where amos programs look for info about the editor.
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	struct kittyVideoInfo *video;
	uint32_t dummy[6];
	uint16_t rgb[8];
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED ;

struct label
{
	int proc;
	char *loopLocation;
	char *tokenLocation;
	char *name;
};

struct globalVar
{
	struct kittyData var;
	char *varName;
	union
	{
		int localIndex;
		int localIndexSize;
	};
	int proc;	// so vars can be attached to proc.
	int pass1_shared_to;	// pass1 should only use this, as it will change.
	bool isGlobal;
	char *procDataPointer;
};

struct stackFrame
{
	int id;
	char *dataPointer;
	struct kittyData *localVarData;
	struct kittyData *localVarDataNext;
};

struct defFn
{
	char *name;
	char *fnAddr;
	char *skipAddr;
};

struct kittyBank 
{
	int id;
	int type;
	char *start;
	char *object_ptr;
	int length;
};

struct kittyField
{
	int size;
	int ref;
};

struct lineAddr
{
	unsigned int file;
	unsigned int lineNumber;
	unsigned int srcStart;
	unsigned int srcEnd;
	unsigned int start;
	unsigned int end;
};

struct kittyFile
{
	FILE *fd;
	int fieldsCount;
	int fieldsSize;
	struct kittyField *fields;
};

struct zone
{
	int screen;
	int x0;
	int y0;
	int x1;
	int y1;
};

struct sampleHeader
{
#ifdef _MSC_VER
#pragma pack(push, 1)
#endif
	char		name[8];
	uint16_t	frequency;
	uint32_t	bytes;
	uint8_t	ptr;
#ifdef _MSC_VER
#pragma pack(pop)
#endif
} PACKED ;

struct envel
{
	int startDuration;
	int duration;
	int volume;
};

struct wave
{
	int id;
	int	bytesPerSecond;	// bytesPerSecond
	struct envel envels[7];
	struct sampleHeader sample;	// this one most be last
};

struct kittyDevice
{
	int id;
	struct MsgPort *port;
	struct IORequest *io;
	bool sendt;
	int error;
};

struct kittyLib
{
	int id;
	struct Library *base;
};

#if defined(__amoskittens__) || defined(__amoskittens_interface_test__)
#define __cmdStack instance.cmdStack
#define __stack instance.stack
#define instance_stack instance.stack
#define instance_cmdStack instance.cmdStack
#define instance_parenthesis_count instance.parenthesis_count
#define instance_token_is_fresh instance.token_is_fresh
#define this_instance_one
#define this_instance_first
#define opt_instance_one
#define opt_instance_first
#define instance_
#define instance_cmdstack_opts
#else
#define __cmdStack instance->cmdStack
#define __stack instance->stack
#define instance_stack instance->stack
#define instance_cmdStack instance->cmdStack
#define instance_parenthesis_count instance->parenthesis_count
#define instance_token_is_fresh instance->token_is_fresh
#define this_instance_one struct KittyInstance *instance
#define this_instance_first struct KittyInstance *instance,
#define opt_instance_one instance
#define opt_instance_first instance,
#define instance_cmdstack_opts \
		cmdTmp[__cmdStack].instance = instance;
#endif

#define stackIfSuccess()					\
	cmdTmp[__cmdStack].cmd = _ifSuccess;		\
	cmdTmp[__cmdStack].tokenBuffer = NULL;	\
	cmdTmp[__cmdStack].flag = cmd_never | cmd_true;	\
	__cmdStack++; \

#define stackIfNotSuccess()					\
	cmdTmp[__cmdStack].cmd = _ifNotSuccess;		\
	cmdTmp[__cmdStack].tokenBuffer = NULL;	\
	cmdTmp[__cmdStack].flag = cmd_never | cmd_false;	\
	__cmdStack++; \

#define stackCmdNormal( fn, buf )				\
	instance_cmdstack_opts ; \
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_normal | cmd_onNextCmd | cmd_onEol;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	cmdTmp[__cmdStack].token = 0; \
	cmdTmp[__cmdStack].parenthesis_count =instance_parenthesis_count; \
	__cmdStack++; \
	instance_token_is_fresh = false; 

#define stackCmdLoop( fn, buf )				\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_loop;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	__cmdStack++; \

#define stackCmdProc( fn, buf )				\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_proc;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	__cmdStack++; \

#define stackCmdFlags( fn, buf, flags )				\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = flags;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	__cmdStack++; \

#define stackCmdIndex( fn, buf )	{			\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_index ;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	cmdTmp[__cmdStack].token = token_index ; \
	cmdTmp[__cmdStack].parenthesis_count =instance_parenthesis_count; \
	__cmdStack++; } \

#define stackCmdParm( fn, buf )				\
	instance_cmdstack_opts ; \
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_para | cmd_onNextCmd | cmd_onEol;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	cmdTmp[__cmdStack].token = 0; \
	cmdTmp[__cmdStack].parenthesis_count =instance_parenthesis_count; \
	__cmdStack++; \
	instance_token_is_fresh = false; 

#define stackCmdMathOperator(fn,_buffer,_token)				\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = _buffer;	\
	cmdTmp[__cmdStack].flag = cmd_para | cmd_onComma | cmd_onNextCmd | cmd_onEol; \
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	cmdTmp[__cmdStack].token = _token; \
	cmdTmp[__cmdStack].parenthesis_count =instance_parenthesis_count; \
	__cmdStack++; \

#define stackCmdOnBreakOrNewCmd(fn,buf,_token)				\
	cmdTmp[__cmdStack].cmd = fn;		\
	cmdTmp[__cmdStack].tokenBuffer = buf;	\
	cmdTmp[__cmdStack].flag = cmd_para | cmd_onBreak | cmd_onComma | cmd_onNextCmd | cmd_onEol;	\
	cmdTmp[__cmdStack].lastVar = last_var;	\
	cmdTmp[__cmdStack].stack = __stack; \
	cmdTmp[__cmdStack].token = _token; \
	cmdTmp[__cmdStack].parenthesis_count =instance_parenthesis_count; \
	__cmdStack++; \

extern int currentLine;

extern bool equal_symbol;
extern struct nativeCommand NativeCommand[];
extern int findNativeCommand(unsigned short lastToken,unsigned short token);
extern bool findSymbol(unsigned short token);
extern int commandCnt;

extern struct proc procStack[];

extern struct extension_lib	kitty_extensions[32];
extern int procStackCount;
extern struct stringData *var_param_str;
extern int var_param_num;
extern double var_param_decimal;
extern int proc_stack_frame;

extern struct stackFrame procStcakFrame[PROC_STACK_SIZE];

extern char *_file_start_;
extern char *_file_end_;

extern APTR contextDir;

extern void (*do_breakdata) ( struct nativeCommand *cmd, char *tokenBuffer );

extern struct glueCommands input_cmd_context;

struct errorAt
{
	int code;
	int trapCode;
	char *posResume;
	char *pos;
	bool newError;
};

enum
{
	e_cmdTo_default = 1
};

// --------------------------------------------------------------------------------------------
//          Api provides a programmable interface into amos kittens 
//       the API allows extensions to access routines from the outside
// --------------------------------------------------------------------------------------------

#define VBL_FUNC_ARGS ( void *custom )

struct kittyApi
{
//	-- runtime --

	void (*setCmdTo) (int option);

//	-- error --

	void (*setError) (int,char *);

//	-- engine --

	void (*engineLock) (void);
	void (*engineUnlock) (void);
	void (*engineAddVblInterrupt) ( void (*fn) VBL_FUNC_ARGS, void *custom );
	void (*engineRemoveVblInterrupt) ( void (*fn) VBL_FUNC_ARGS );

//	-- debug --

	void (*dumpStack) (void);

//	-- screen --

	void (*freeScreenBobs) (int);

//	-- banks --

	struct kittyBank *(*findBankById) (int);
	struct kittyBank *(*findBankByIndex) (int);
	struct kittyBank *(*firstBank)();
	int (*getBankListSize)();
	struct kittyBank *(*reserveAs) ( int, int ,int, const char *, char * );
	void (*freeBank) (int);
	void *(*getBankObject) (int id);

//	--  text --

	void (*kittyText) (struct retroScreen *screen, int x, int y,struct stringData *txt);
	void *(*newTextWindow) ( struct retroScreen *, int );
	void (*freeAllTextWindows) ( struct retroScreen * );

//	-- audio --

	void (*audioLock) ();
	void (*audioUnlock) ();
	bool (*audioPlayWave) (struct wave *wave, int len, int channels);
	bool (*audioPlay) (uint8_t * data,int len, int channel, int frequency);
	void (*audioDeviceFlush) (int voices);
	void (*audioSetSampleLoop) ( ULONG voices, bool value );

//	-- system --

	void (*waitvbl) ();

//	-- blocks --

	struct retroBlock *(*findBlock_in_blocks) ( int id );
	struct retroBlock *(*findBlock_in_cblocks) ( int id );

//	-- bobs --

	struct retroSpriteObject *(*getBob) (unsigned int id);

//	-- sprite --

	int (*XSprite_formula) (int x);
	int (*YSprite_formula) (int y);
	int (*from_XSprite_formula) (int x);
	int (*from_YSprite_formula) (int y);

//	-- zones --

	int (*find_zone_in_any_screen_hard) ( int hx, int hy );
	int (*find_zone_in_any_screen_pixel) ( int hx, int hy );
	int (*find_zone_in_only_screen_hard) ( int screen, int hx, int hy );
	int (*find_zone_in_only_screen_pixel) ( int screen, int hx, int hy );

};

// --------------------------------------------------------------------------------------------
//               Instance provides a shared data into amos kittens 
//     the Instance allows extensions to access data from the outside
// --------------------------------------------------------------------------------------------

struct KittyInstance
{
	int last_var;
	struct globalVar *globalVars;
	unsigned short last_token;
	int tokenMode;
	int tokenlength;
	void *extensions_context[32];
	struct retroScreen *screens[8] ;
	struct retroVideo *video;
	struct retroRGB DefaultPalette[256];
	struct retroSprite *sprites ;
	struct retroSprite *icons ;
	struct kittyData *kittyStack;
	struct kittyFile files[10];
	struct glueCommands *cmdTmp;
	struct errorAt kittyError;
	struct zone *zones ;
	int zones_allocated ;
	int current_screen;
	int current_extension;
	int current_pattern;
	int current_resource_bank;
	int stack ;
	int cmdStack ;
	char *tokenBufferResume;
	bool token_is_fresh;
	int parenthesis_count;
	int engine_mouse_key ;
	int engine_mouse_x ;
	int engine_mouse_y ;
	bool engine_wait_key;
	bool engine_key_repeat;
	bool engine_key_down;
	bool engine_stopped;
	bool engine_mouse_hidden;
	bool engine_pal_mode;
	uint32_t engine_back_color;
	int engine_key_state[256];
	int xgr;
	int ygr;
	int GrWritingMode;
	int paintMode;
	bool audio_3k3_lowpass;
	LONG volume;
	struct kittyApi api;
};

#if defined(__amoskittens__)
extern char *(*jump_mode) (struct reference *ref, char *ptr);
extern char *jump_mode_goto (struct reference *ref, char *ptr);
extern char *jump_mode_gosub (struct reference *ref, char *ptr);
extern void (**do_input) ( struct nativeCommand *cmd, char *tokenBuffer );
extern char *(**do_to) ( struct nativeCommand *cmd, char *tokenBuffer );
extern char *do_to_default( struct nativeCommand *cmd, char *tokenbuffer );
extern void do_std_next_arg(nativeCommand *cmd, char *ptr);
extern struct kittyData kittyStack[];
extern struct glueCommands cmdTmp[];
#define last_var instance.last_var
#endif

#if defined(__amoskittens__) || defined(__amoskittens_interface_test__) || defined(__amoskittens_amal_test__)
extern struct KittyInstance instance;
#endif

#endif

