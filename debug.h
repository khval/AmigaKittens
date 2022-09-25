
#ifdef __amigaos4__
#ifdef __linux__
#undef __linux__
#endif
#endif

// --------- debug options --------------- (most common debug options)

#define show_proc_names_no
#define show_token_numbers_no
#define show_debug_printf_no
#define show_cleanup_debug_no
#define show_stacktrace_for_errors_no

//--------- other options ------------

#define enable_interface_debug_no
#define show_error_at_file_no
#define show_debug_amal_no
#define show_array_no
#define show_dump_vars_undefined_no
#define enable_limit_mouse_no
#define enable_engine_debug_output_no
#define enable_audio_debug_output_no
#define enable_end_of_program_debug_no
#define enable_config_debug_no
#define enable_extension_debug_no

// --------- debug options include ---------

#define show_include_debug_no

// --------- debug options pass1 ----------- (debug options for pretest)

#define show_pass1_tokens_no
#define show_pass1_procedure_fixes_no
#define show_pass1_end_of_file_no
#define show_pass1_modified_code_no

// ------------- CRC options ------------------ (keep this to no, unless you need to find a memory corruption bug)

#define enable_ext_crc_no
#define enable_vars_crc_no
#define enable_bank_crc_no			// can find memory corruption in pass1

// ------------ optimizer ----------------------

#define run_program_yes
#define enable_fast_execution_yes		// Some debug option do not work when this is enabled.

//------------- end of options -----------------

void dump_global();
void dump_local( int n );
void dump_prog_stack();
void dump_stack();
void dump_labels();
void dump_banks();
void dump_end_of_program();
void dump_lines();
void dump_680x0_regs();
void dump_screens();
void dump_zones();
void dump_sprite();
void dump_all_bobs();
void dump_bobs_on_screen(int screen_id);
void dump_channels();
void dump_anim();
bool var_has_name( struct kittyData *var, const char *name );

#ifdef __amigaos__
extern struct Window *debug_Window;
#endif

void open_debug_window();
void close_debug_window();

struct lineFromPtr
{
	unsigned int line;
	unsigned int file;
};
 
extern struct lineFromPtr lineFromPtr;

void getLineFromPointer( char *address );
uint32_t mem_crc( char *mem, uint32_t size );

#ifdef show_debug_printf_yes
#define dprintf printf
#define dgetLineFromPointer getLineFromPointer
#else
#define dprintf(fmt,...)
#define dgetLineFromPointer(...)
#endif

#ifdef show_cleanup_debug_yes
#define cleanup_printf printf
#else
#define cleanup_printf(fmt,...)
#endif

#ifdef show_proc_names_yes
#define proc_names_printf printf
#else
#define proc_names_printf(fmt,...)
#endif

#ifdef enable_interface_debug_yes
#define interface_printf printf
#else
#define interface_printf(fmt,...)
#endif

#ifdef show_pass1_tokens_yes
#define pass1_printf printf
#else
#define pass1_printf(fmt,...)
#endif

#ifdef enable_config_debug_yes
#define config_printf printf
#else
#define config_printf(fmt,...)
#endif

#ifdef enable_extension_debug_yes
#define extension_printf printf
#else
#define extension_printf(fmt,...)
#endif


#ifdef show_debug_amal_yes
	#ifdef __amigaos4__
		#ifdef test_app
			#define AmalPrintf printf
		#else
			#define AmalPrintf Printf_iso
		#endif
	#endif

	#ifdef __linux__
		#define AmalPrintf printf
	#endif
#else
	#define AmalPrintf(fmt,...)
#endif

#ifdef __amigaos4__
void Printf_iso(const char *fmt,...);
#endif

#ifdef __linux__
#define Printf_iso(fmt,...) fprintf(engine_fd,fmt,__VA_ARGS__)
#endif

void debug_draw_wave(struct wave *wave);
const char *getTypeName( int type );

#define NYI(name) printf("%s not yet implemented\n",name)

