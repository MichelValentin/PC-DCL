#ifdef _WIN32
//#pragma message "Compiling for WIN32"
#elif defined __linux
//#pragma message "Compiling for Linux"
#else
#pragma message "Compiling on an invalid platform"
ERROR
#endif

#include "platform.h"
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include "flexlist.h"
#include "findfile.h"

#ifdef _WIN32
#include <tlhelp32.h>
#include "psapi.h"
#endif

#ifndef NDEBUG
#define VERSION     "4.07D"
#else
#define VERSION     "4.07"
#endif
#define DCL_EXIT        0
#define DCL_INIT        1
#define DCL_INIT_HOME   2
#define DCL_LOGIN       3
#define DCL_LOGIN_HOME  4
#define DCL_ARG         5
#define DCL_RUN         6
#define DCL_LOGOUT      7
#define DCL_LOGOUT_HOME 8
#define MAX_TOKEN           260
#define MAX_LABEL           32
#define LOGICAL_MAX_NAME    32
#define LOGICAL_MAX_VALUE   MAX_TOKEN
#define SYMBOL_MAX_NAME     32
#define SYMBOL_MAX_VALUE    MAX_TOKEN
#define SYMBOL_INTEGER      1
#define SYMBOL_STRING       0
#define SYMSCO_LOCAL    1
#define SYMSCO_GLOBAL   2
#define VERIFY_PROC     1
#define VERIFY_IMAGE    2
#define MODE_BATCH       1
#define MODE_INTERACTIVE 2
#define LOG_USER    0
#define LOG_ALLOC   1
#define LOG_SYSTEM  2
#define DCL_IGNORE  0
#define DCL_RETRY   1
#define DCL_ABORT   2
#define OVERSTRIKE  0
#define INSERT      1
#define MAX_KBD_STACK  64
#define CONFIRM_YES     0
#define CONFIRM_NO      1
#define CONFIRM_ALL     2
#define CONFIRM_QUIT    3
#define _SUCCESS         0
#define _WARNING         1
#define _ERROR           2
#define _SEVERE_ERROR    3
#define NEXT_LINE()       if (EXCPT == 0) cmd[C].ln++
#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#define NOT_FOUND -1
#define AMBIGUOUS -2
#define DCL_OK    0
#define DCL_ERROR -1
#define MANDATORY 0x08
#define VALUE     0x04
#define EXPR      0x02
#define PRESENT   0x01
#define INFINITE_TIMEOUT 0L
#define OPEN_MODE_NONE    0
#define OPEN_MODE_READ    1
#define OPEN_MODE_WRITE   2
#define OPEN_MODE_APPEND  4

#ifndef _MAX_PATH
#define _MAX_PATH   260 /* max. length of full pathname */
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE  3   /* max. length of drive component */
#endif
#ifndef _MAX_DIR
#define _MAX_DIR    256 /* max. length of path component */
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME  256 /* max. length of file name component */
#endif
#ifndef _MAX_EXT
#define _MAX_EXT    256 /* max. length of extension component */
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define UNREFERENCED_PARAM(a) ((a) = (a));

typedef struct _DCL_FIND_DATA {
    DWORD           dwFileAttributes;
    time_t          CreationTime;
    time_t          AccessTime;
    time_t          WriteTime;
    DWORD           nFileSize;
    char            cFileName[_MAX_PATH];
    char            cAlternateFileName[14];
} DCL_FIND_DATA, *LPDCL_FIND_DATA;

typedef struct {
    int     flag;
    char    name[LOGICAL_MAX_NAME];
    char    value[LOGICAL_MAX_VALUE];
    char    system_flag;
    int     filenum;
    char    mode;
    int     recl;
    long    rptr;
    long    wptr;
    } LOGICAL;
#define LOGICAL_MAX 256
extern LOGICAL LOGICAL_TABLE[LOGICAL_MAX];

typedef struct {
    char    name[32];
    int     line;
    char    subr;
    } CMD_LABEL;

typedef struct {
    char    filename[_MAX_PATH];
    FILE    *SYS_INPUT;
    FILE    *SYS_OUTPUT;
    char    outname[_MAX_PATH];
    Flist   symbol_list;
    int     symbol_scope;
    int     cc[9];
    int     ccl;
    int     ON;
    int     ON_severity;
    char    ON_error_command[MAX_TOKEN];
    char    ON_break_command[MAX_TOKEN];
    }  DCL;

typedef struct {
    char    filespec[MAX_TOKEN];
    struct  _finddata_t ffblk;
    long    handle;
    } SEARCH;

typedef struct {
    char    mask[9];
    char    idx;
    char    names[512][16];
    } PROCESS;

typedef struct {
    Flist   cmd;
    Flist   label;
    Flist   stack;
    Flist   l_stack;
    int     ln;
    char    stop;
    char    subr;
    char    cond;
    }  CMD;

typedef struct {
    char    *value;
    char    echo;
    char    erase;
    char    terminate;
    }  KEYDEF;

#ifdef _WIN32
    typedef struct {
        char            szName[MAX_TOKEN];
        char            szFileSystemName[MAX_TOKEN];
        char            szType[MAX_TOKEN];
        DWORD           dwVolumeSerialNumber;
        unsigned long long       FreeBytesAvailable;
        unsigned long long       TotalNumberOfBytes;
        unsigned long long       TotalNumberOfFreeBytes;
        } DISKINFO;
#else
	typedef struct {
   		char            szName[MAX_TOKEN];
   		char            szType[MAX_TOKEN];
		char            szMountPoint[MAX_TOKEN];
		unsigned long long  Total1kBlocks;
		unsigned long long	Used1kBlocks;
		unsigned long long	Available1kBlocks;
		unsigned long 		pctUsage;
		} DISKINFO;
#endif
#ifdef __linux
		typedef struct {
			DWORD	dwMemTotal;
			DWORD	dwMemFree;
			DWORD	dwBuffers;
			DWORD	dwCached;
		} MEMORYSTATUS;
#endif
typedef struct {
    short   X;
    short   Y;
} XYCOORD, *PXYCOORD;

typedef struct {
//    CONSOLE_SCREEN_BUFFER_INFO  csbi;
    XYCOORD                     Size;
    unsigned short              fg_color;
    unsigned short              bg_color;
    char						szInfo[MAX_TOKEN];
    } TERMINFO;

typedef struct _Process_info_ {
    char            szUserName[MAX_TOKEN];
    char            szProcessName[MAX_TOKEN];
    char            szComputerName[MAX_TOKEN];
    char            szExename[_MAX_PATH];
#ifdef _WIN32
    IO_COUNTERS     ioc;
    PROCESS_MEMORY_COUNTERS   pmc;
    SYSTEMTIME      tCpuTime;
    DWORD           dwThreads;
    //DWORD           dwHandles;
    DWORD           dwPriorityClass;
    FILETIME        CreationTime;
    FILETIME        ExitTime;
    FILETIME        KernelTime;
    FILETIME        UserTime;
#endif
#ifdef __linux
    DWORD           dwThreads;
    DWORD			dwNice;
    DWORD			dwUID;
    DWORD			dwGID;
    char			szGroupName[MAX_TOKEN];
    char			szStat[MAX_TOKEN];
    DWORD			dwSysCR;
    DWORD			dwSysCW;
    DWORD			dwReadBytes;
    DWORD			dwWriteBytes;
    DWORD			dwCancelledWriteBytes;
    DWORD			dwMemSize;
    DWORD			dwMemRSS;
    DWORD			dwMemShared;
    DWORD			dwMemCode;
    DWORD			dwMemData;
    char			szStartTime[32];
    char			szElapsedTime[32];
    char			szCPUTime[32];
#endif
} PROCESSINFO;


// *********************************
// *  Common functions prototypes  *
// *********************************�

int  dclini(int argc,char **argv);
char *EXP_compute(char *str,char *result);
int  EXP_isnumber(char *s);
void init_term(void);
int  kbdread(char *buffer,int maxlen,Flist stack,long timeout);
int  f_lexical(char *name,char *value);
int  logical_put(char *name,char *value,int system_flag);
int  logical_put_file(char *name,char *value,int system_flag,int filenum,char mode,
                     int recl, long rptr, long wptr);
int  logical_get(char *name,char *value);
void logical_get_file(char *name,char *value,int *filenum,char *mode,
                          int *recl, long *rptr, long *wptr);
int  logical_del(char *name);
int  dcl_string_assign(char *sym,char *oper);
int  dcl_sym_assign(char *sym,char *oper);

int  dcl_process_command_line(int exception);
int  dcl_search(char **table,char *key);
void dcl_string(char *input, char *output, int len);
void dcl_get_param(char *str,char *param);
int  dcl_searchdirlst(char *cmd,char *path_list,int subdir,
                     int fn(char *,DCL_FIND_DATA *,void *, char),
                     void *fn_param);
int  dcl_searchdir(char *cmd,char *path_name,int subdir,
                   int fn(char *,DCL_FIND_DATA *,void *, char),
                   void *fn_param);
int  dcl_confirm(char *msg);
char *dcl_get_token(char *s,char *t);
char *dcl_get_token_raw(char *s,char *t);
int  dcl_enter_proc_level(char *outfile,char **p);
int  dcl_search_key(char **table,char *key);
int  dcl_exit_subroutine(void);
int  dcl_spawn(char *command, int nowait);
int  wildcard(char *path);
int  xstricmp(const char *pat, const char *str);
#ifdef _WIN32
int  diskinfo(int drive,DISKINFO *di);
#else
int  diskinfo(char *drive,DISKINFO *di);
#endif
int  isdir(const char *pathname);
void dcl_init_term(void);
void dcl_reset_term(void);
int  dcl_printf(FILE *file,const char *format,...);
void dcl_interprete_prompt(char *prompt,char *new_prompt);
void dcl_trim(char *instr, char *outstr);
void dcl_tostring(unsigned long long ull, char *str, size_t size);
void dcl_tostring_left(unsigned long long ull, char *str, size_t size);

/***********************/
/*   PC-DCL commands   */
/***********************/

typedef struct {
   char     tag;
   char     *name;
   char     flag;
   } RPARAM_T;

typedef struct {
   char     tag;
   char     name[32];
   char     value[MAX_TOKEN];
   char     flag;
   char     *expr;
   } PARAM_T;

typedef struct {
    char        name[32];
    int         (*fn)(PARAM_T *p,PARAM_T *q);
    RPARAM_T     *p;
    RPARAM_T     *q;
    } COMMAND_T;
extern COMMAND_T command[];
extern RPARAM_T ALLOCATE_PARAM[],ALLOCATE_QUAL[];
extern RPARAM_T APPEND_PARAM[],APPEND_QUAL[];
extern RPARAM_T ASSIGN_PARAM[],ASSIGN_QUAL[];
extern RPARAM_T CALL_PARAM[],CALL_QUAL[];
extern RPARAM_T DUMMY_PARAM[],DUMMY_QUAL[];
extern RPARAM_T CLOSE_PARAM[],CLOSE_QUAL[];
extern RPARAM_T COPY_PARAM[],COPY_QUAL[];
extern RPARAM_T CREATE_PARAM[],CREATE_QUAL[];
extern RPARAM_T DEALLOCATE_PARAM[],DEALLOCATE_QUAL[];
extern RPARAM_T DEASSIGN_PARAM[],DEASSIGN_QUAL[];
extern RPARAM_T DEFINE_PARAM[],DEFINE_QUAL[];
extern RPARAM_T DELETE_PARAM[],DELETE_QUAL[];
extern RPARAM_T DIR_PARAM[],DIR_QUAL[];
extern RPARAM_T GOSUB_PARAM[];
extern RPARAM_T HELP_PARAM[];
extern RPARAM_T IF_PARAM[];
extern RPARAM_T INQUIRE_PARAM[],INQUIRE_QUAL[];
extern RPARAM_T ON_PARAM[];
extern RPARAM_T OPEN_PARAM[],OPEN_QUAL[];
extern RPARAM_T PRINT_PARAM[],PRINT_QUAL[];
extern RPARAM_T READ_PARAM[],READ_QUAL[];
extern RPARAM_T RECALL_PARAM[],RECALL_QUAL[];
extern RPARAM_T RENAME_PARAM[],RENAME_QUAL[];
extern RPARAM_T RETURN_PARAM[];
extern RPARAM_T RUN_PARAM[];
extern RPARAM_T SET_PARAM[],SET_QUAL[];
extern RPARAM_T SHOW_PARAM[],SHOW_QUAL[];
extern RPARAM_T THEN_PARAM[],THEN_QUAL[];
extern RPARAM_T TYPE_PARAM[],TYPE_QUAL[];
extern RPARAM_T WAIT_PARAM[];
extern RPARAM_T WRITE_PARAM[],WRITE_QUAL[];

int cmd_search_table(char *key);
int cmd_prepro(RPARAM_T *rp,RPARAM_T *pq,PARAM_T **p,PARAM_T **q);
int cmd_parse_line(char *cmdline,RPARAM_T *rp,RPARAM_T *rq,PARAM_T *param,PARAM_T *qual);

int dcl_allocate(PARAM_T *p,PARAM_T *q);
int dcl_append(PARAM_T *p,PARAM_T *q);
int dcl_assign(PARAM_T *p,PARAM_T *q);
int dcl_call(PARAM_T *p,PARAM_T *q);
int dcl_cancel(PARAM_T *p,PARAM_T *q);
int dcl_continue(PARAM_T *p,PARAM_T *q);
int dcl_close(PARAM_T *p,PARAM_T *q);
int dcl_cls(PARAM_T *p,PARAM_T *q);
int dcl_copy(PARAM_T *p,PARAM_T *q);
int dcl_create(PARAM_T *p,PARAM_T *q);
int dcl_deallocate(PARAM_T *p,PARAM_T *q);
int dcl_deassign(PARAM_T *p,PARAM_T *q);
int dcl_define(PARAM_T *p,PARAM_T *q);
int dcl_delete(PARAM_T *p,PARAM_T *q);
int dcl_dir(PARAM_T *p,PARAM_T *q);
int dcl_else(PARAM_T *p,PARAM_T *q);
int dcl_endif(PARAM_T *p,PARAM_T *q);
int dcl_endwhile(PARAM_T *p,PARAM_T *q);
int dcl_endsubroutine(PARAM_T *p,PARAM_T *q);
int dcl_exit(PARAM_T *p,PARAM_T *q);
int dcl_gosub(PARAM_T *p,PARAM_T *q);
int dcl_goto(PARAM_T *p,PARAM_T *q);
int dcl_help(PARAM_T *p,PARAM_T *q);
int dcl_if(PARAM_T *p,PARAM_T *q);
int dcl_inquire(PARAM_T *p,PARAM_T *q);
int dcl_logout(PARAM_T *p,PARAM_T *q);
int dcl_on(PARAM_T *p,PARAM_T *q);
int dcl_open(PARAM_T *p,PARAM_T *q);
int dcl_print(PARAM_T *p,PARAM_T *q);
int dcl_read(PARAM_T *p,PARAM_T *q);
int dcl_recall(PARAM_T *p,PARAM_T *q);
int dcl_rename(PARAM_T *p,PARAM_T *q);
int dcl_repeat(PARAM_T *p,PARAM_T *q);
int dcl_return(PARAM_T *p,PARAM_T *q);
int dcl_run(PARAM_T *p,PARAM_T *q);
int dcl_set(PARAM_T *p,PARAM_T *q);
int dcl_set_file(PARAM_T *p,PARAM_T *q);
int dcl_show(PARAM_T *p,PARAM_T *q);
int dcl_subroutine(PARAM_T *p,PARAM_T *q);
int dcl_then(PARAM_T *p,PARAM_T *q);
int dcl_type(PARAM_T *p,PARAM_T *q);
int dcl_until(PARAM_T *p,PARAM_T *q);
int dcl_wait(PARAM_T *p,PARAM_T *q);
int dcl_while(PARAM_T *p,PARAM_T *q);
int dcl_write(PARAM_T *p,PARAM_T *q);


/***************/
/*   Symbols   */
/***************/

typedef struct {
    char    name[SYMBOL_MAX_NAME];
    char    value[SYMBOL_MAX_VALUE];
    int     minlen;
    char    type;   // string or integer
    } SYMBOL;

typedef struct {
    char    name[SYMBOL_MAX_NAME];
    char    value[SYMBOL_MAX_VALUE];
    int     level;
    int     num;
    } SYMSCAN;

int symbol_put(char *name,char *value,int level);
int symbol_get(char *name,char *value);
void symbol_del(char *name);
void symbol_del_lvl(char *name,int lvl);
void symbol_first(SYMSCAN *ss);
void symbol_next(SYMSCAN *ss);

/**************************/
/*  Filespec conversions  */
/**************************/
void cvfs_dos_to_vms(char *internal,char *external);
void cvfs_lst_to_dos(char *lst_external,char *lst_internal,int *lst_recurse);
void cvfs_vms_to_dos(char *external,char *internal,int *recurse);
int  cvfs_translate_logical(char *s);
int  cvfs_check_device(char *name);
int  cvfs_cmp_file_name(char *pat, char *str);
/**********************************/
/*  Common external declarations  */
/**********************************/

extern  int     _SEVERITY;
extern  int     _STATUS;
extern  char    *MONTH[13];
extern  char    INI_NAME[MAX_TOKEN];
extern  char    INI_VALUE[MAX_TOKEN];
extern  char    INI_HOME[_MAX_PATH];
extern  char    INI_DCL[_MAX_PATH];
extern  char    dcl_line[1024];
extern  FILE    *dclfile[64];
extern  char    dcl_prompt[33];
extern  char    dcl_org_prompt[33];
extern  DCL     dcl[8];
extern  CMD     cmd[8];
extern  int     D;
extern  int     C;
extern  int     EXCPT;
extern  int     HARDERR;
extern  Flist   data_stack;
extern  int     _MSTATUS;
extern  int     _MSEVERITY;
extern  int     mode;
extern  int     dcl_subr;
extern  int     DI_err;
//extern  Flist   logical_list;
extern  Flist   cmd_stack;
extern  int     recall;
extern  KEYDEF  keydef[24];
extern  time_t  start_time;
extern  int     verify;
extern  char    CTRL_Y;
extern  char    CTRL_Z;
extern  int     default_insert_mode;
extern  TERMINFO terminfo;
extern  char    CONTROL_Y_ALLOWED;
extern  char    CONTROL_T_ALLOWED;
extern  char    SUBDIR;
extern  char    SWITCHAR;
extern  char    history_filename[_MAX_PATH];

