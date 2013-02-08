/****************************************************************************/
/*                                                                          */
/*   PC-DCL -  DCL command line interpreter for Windows.                    */
/*   Copyright (C) 1991-2007 by Michel Valentin                             */
/*                                                                          */
/*   This program is free software; you can redistribute it and/or modify   */
/*   it under the terms of the enclosed   license.                          */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                          */
/****************************************************************************/
/*lint -e801 use of goto is deprecated*/
/*lint -e818 * could be declared as constant*/
/*lint -e661 off by one */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "platform.h"
#include "dcl.h"
#include "dcltime.h"
#include "termio.h"
#ifdef _WIN32
#include <tlhelp32.h>
#include "psapi.h"
#else
#include <sys/utsname.h>
#endif

extern char **environ;

// Show -----------------------------------------------------
RPARAM_T SHOW_PARAM[] = {
    {1,"What",MANDATORY},
    {2,"",VALUE},
    {0,"",0}
    };
RPARAM_T SHOW_QUAL[] = {
    {1,"/ALL",0},
    {2,"/LOCAL",0},
    {3,"/GLOBAL",0},
    {4,"/BRIEF",0},
    {5,"/FULL",0},
    {6,"/SIZE",VALUE},
    {7,"/DRIVERS",0},
    {8,"/PROGRAMS",0},
    {9,"/OUTPUT",VALUE},
    {10,"/CONTINUOUS",0},
    {0,"",0}
    };

int show_cpu(PARAM_T *p, PARAM_T *q);
int show_default(PARAM_T *p, PARAM_T *q);
int show_devices(PARAM_T *p, PARAM_T *q);
int show_devices_sub(char *name,char size_type, BOOL bFull);
int show_dosvar(PARAM_T *p, PARAM_T *q);
int show_license(PARAM_T *p, PARAM_T *q);
int show_logical(PARAM_T *p, PARAM_T *q);
int show_memory(PARAM_T *p, PARAM_T *q);
int show_process(PARAM_T *p, PARAM_T *q);
int show_status(PARAM_T *p, PARAM_T *q);
int show_symbol(PARAM_T *p, PARAM_T *q);
int show_system(PARAM_T *p, PARAM_T *q);
int show_term(PARAM_T *p, PARAM_T *q);
int show_time(PARAM_T *p, PARAM_T *q);
int show_translation(PARAM_T *p, PARAM_T *q);
int show_user(PARAM_T *p, PARAM_T *q);
int show_key(PARAM_T *p, PARAM_T *q);
int show_version(PARAM_T *p, PARAM_T *q);
int show_process_brief(DWORD dwPid);
int show_process_all(DWORD dwPid);
int show_process_cont(DWORD dwPid);
#ifdef _WIN32
int GetProcessUser(HANDLE hProcess, char *pchUsername, DWORD dwLength);
#endif
#ifdef __linux
void GetLogicalDriveStrings(size_t maxlen, char *buffer);
#endif
int get_process_info(DWORD dwPid, PROCESSINFO *pi);

FILE 		*outfile;
PROCESSINFO	pi;

int dcl_show(PARAM_T *p, PARAM_T *q)
{
    char    *option[]  =  {"CPU", "DEFAULT","DEVICES","DOSVAR","KEY","LICENSE","LOGICAL",
                           "MEMORY", "PROCESS", "STATUS","SYMBOL","SYSTEM","TERMINAL",
                           "TIME", "TRANSLATION","USER","VERSION",NULL};

    enum    option_tag {OPT_CPU, OPT_DEFAULT,OPT_DEVICES,OPT_DOSVAR,OPT_KEY,OPT_LICENSE,
                        OPT_LOGICAL, OPT_MEMORY,OPT_PROCESS,OPT_STATUS,OPT_SYMBOL,OPT_SYSTEM,
                        OPT_TERM, OPT_TIME,OPT_TRANSLATION,OPT_USER,OPT_VERSION};


    char    token[MAX_TOKEN];
    char    vms[MAX_TOKEN];
    char    dos[MAX_TOKEN];
    char    drive[_MAX_DRIVE];
    char    dir[_MAX_DIR];
    char    file[_MAX_FNAME];
    char    ext[_MAX_EXT];
    int     recurse;
    int     i;

    NEXT_LINE();
    if (cmd[C].subr) return(0);
    if (!dcl[D].cc[dcl[D].ccl]) return(0);
    if (p == NULL || q == NULL) return(DCL_ERROR);

    _STATUS = 0;
    outfile = dcl[D].SYS_OUTPUT;
    (void) cmd_parse_line(dcl_line,SHOW_PARAM,SHOW_QUAL,p,q);

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT && q[i].tag == 9) {        /* /OUTPUT */
            dcl_string(q[i].value,vms,MAX_TOKEN);
            cvfs_vms_to_dos(vms,dos,&recurse);
            if (!cvfs_check_device(dos)){
                _splitpath(dos,drive,dir,file,ext);
                if (strlen(file) == 0){
                    strcat(dos,"SHOW");
                }
                if (strlen(ext) == 0){
                    strcat(dos,".LIS");
                }
            }
            outfile = fopen(dos,"wt");
            if (outfile == NULL) {
                (void) dcl_printf(dcl[D].SYS_OUTPUT,"%s: %s\n",vms,strerror(errno));
                _SEVERITY = 2;
                _STATUS = 19;
                return(DCL_ERROR);
            }
        }
    }
/////////////////////////////////////

    dcl_string(p[0].value,token,MAX_TOKEN);
    switch (dcl_search(option,token)){
        case OPT_CPU        :   (void) show_cpu(p,q);
                                break;
        case OPT_DEFAULT    :   (void) show_default(p,q);
                                break;
        case OPT_DEVICES    :   (void) show_devices(p,q);
                                break;
        case OPT_DOSVAR     :   (void) show_dosvar(p,q);
                                break;
        case OPT_KEY        :   (void) show_key(p,q);
                                break;
        case OPT_LICENSE    :   (void) show_license(p,q);
                                break;
        case OPT_LOGICAL    :   (void) show_logical(p,q);
                                break;
        case OPT_MEMORY     :   (void) show_memory(p,q);
                                break;
        case OPT_PROCESS    :   (void) show_process(p,q);
                                break;
        case OPT_STATUS     :   (void) show_status(p,q);
                                break;
        case OPT_SYMBOL     :   (void) show_symbol(p,q);
                                break;
        case OPT_SYSTEM     :   (void) show_system(p,q);
                                break;
        case OPT_TERM       :   (void) show_term(p,q);
                                break;
        case OPT_TIME       :   (void) show_time(p,q);
                                break;
        case OPT_TRANSLATION:   (void) show_translation(p,q);
                                break;
        case OPT_USER       :   (void) show_user(p,q);
                                break;
        case OPT_VERSION    :   (void) show_version(p,q);
                                break;
        case NOT_FOUND      :   (void) dcl_printf(outfile,"Invalid SHOW option.\n");
                                _STATUS = 19;
                                _SEVERITY = 2;
                                break;
        case AMBIGUOUS      :   (void) dcl_printf(outfile,"Ambiguous SHOW option.\n");
                                _STATUS = 19;
                                _SEVERITY = 2;
                                break;
        default              :
                                break;
        }
    if (outfile != dcl[D].SYS_OUTPUT) {
       fflush(outfile);
       fclose(outfile);
    }
    return(0);
}


//---------------------------------------------------------------------------
int show_cpu(PARAM_T *p, PARAM_T *q)
{
    char    token[MAX_TOKEN];
    int     f_full  = 0;
    int     i       = 0;

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *token = 0;
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT) {
            switch (q[i].tag) {
                case 4:                                 /* /BRIEF    */
                    f_full = 0;
                    break;
                case 5:                                 /* /FULL    */
                    f_full = 1;
                    break;
                case 9:
                    break;
                default:
                    (void) dcl_printf(outfile,"Invalid command qualifier (%s).\n",token);
                    _SEVERITY = 2;
                    _STATUS = 19;
                    return(-1);
            } /* end switch */
        }
    }

#ifdef _WIN32
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define MAX_DATA_LENGTH 256

    long    lRet   = ERROR_SUCCESS;
    HKEY    hKey   = NULL;
//    CHAR    achKey[MAX_KEY_LENGTH];
//    DWORD   cbName;
    CHAR    achClass[MAX_PATH] = "";
    DWORD   cchClassName = MAX_PATH;
    DWORD   cSubKeys=0;
    DWORD   cbMaxSubKey;
    DWORD   cchMaxClass;
    DWORD   cValues;
    DWORD   cchMaxValue;
    DWORD   cbMaxValueData;
    DWORD   cbSecurityDescriptor;
    FILETIME ftLastWriteTime;
    CHAR    achKeyName[MAX_KEY_LENGTH];
    int     nCpu = 0;
    CHAR    achValue[MAX_VALUE_NAME];
    DWORD   cchValue = MAX_VALUE_NAME;
    BYTE    achData[MAX_DATA_LENGTH];
    DWORD   cbData = MAX_DATA_LENGTH;
    DWORD   dwType = 0;

    for (nCpu = 0; lRet == ERROR_SUCCESS; nCpu++) {
        sprintf(achKeyName, "%s%d",
                            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\",
                            nCpu);
        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, achKeyName,
                            0, KEY_QUERY_VALUE + KEY_ENUMERATE_SUB_KEYS , &hKey );
        if(lRet == ERROR_SUCCESS) {
            (void) dcl_printf(outfile,"\nCPU %d\n", nCpu);
            lRet = RegQueryInfoKey(hKey,                    // key handle
                                   achClass,                // buffer for class name
                                   &cchClassName,           // size of class string
                                   NULL,                    // reserved
                                   &cSubKeys,               // number of subkeys
                                   &cbMaxSubKey,            // longest subkey size
                                   &cchMaxClass,            // longest class string
                                   &cValues,                // number of values for this key
                                   &cchMaxValue,            // longest value name
                                   &cbMaxValueData,         // longest value data
                                   &cbSecurityDescriptor,   // security descriptor
                                   &ftLastWriteTime);       // last write time
            // Enumerate the key values.
            if (cValues) {
                DWORD j = 0;
                DWORD w = 0;
                DWORD dwValue = 0;
                BYTE  *pch;

                for (j = 0; lRet == ERROR_SUCCESS && j < cValues; j++) {
                    cchValue = MAX_VALUE_NAME;
                    achValue[0] = '\0';
                    cbData = MAX_DATA_LENGTH;
                    achData[0] = '\0';
                    lRet = RegEnumValue(hKey, j,
                                        achValue,
                                        &cchValue,
                                        NULL,
                                        &dwType,
                                        achData,
                                        &cbData);
                    if (lRet == ERROR_SUCCESS)  {
                        if (strnicmp(achValue, "Identifier", 10) == 0 ||
                            strnicmp(achValue, "ProcessorN", 10) == 0 ||
                            strnicmp(achValue, "VendorIden", 10) == 0 ||
                            strnicmp(achValue, "~MHz", 4) == 0        ||
                            strnicmp(achValue, "FeatureSet", 10) == 0) {
                            switch (dwType) {
                            case REG_SZ:
                                pch = achData;
                                while(*pch == ' ') pch++;
                                (void) dcl_printf(outfile,"%s = %s\n", achValue, pch);
                                break;
                            case REG_DWORD:
                                memcpy(&dwValue, achData, sizeof(dwValue));
                                (void) dcl_printf(outfile,"%s = %u  (0x%X)\n", achValue, dwValue, dwValue);
                                break;
                            default:
                                (void) dcl_printf(outfile,"%s = ", achValue);
                                for( w = 0; w < cchValue; w++) {
                                    (void) dcl_printf(outfile,"%2.2X ", (int)achData[w]);
                                }
                                (void) dcl_printf(outfile,"\n");
                                break;
                            }
                        }
                    }
                }
            }
            RegCloseKey(hKey);
        }
    }
#else
    FILE 	*fp = NULL;
    char  	work[MAX_TOKEN];
    char  	cmd[MAX_TOKEN];
    int		line =0;
    strcpy(cmd, "cat /proc/cpuinfo");
    if ((fp = popen(cmd, "r")) != NULL) {
        while(fgets(work, sizeof(work), fp))  {
        	if (strncmp(work, "processor", 9) == 0) {
        		line = 0;
        	}
        	line++;
        	if (f_full || line <= 5) {
            	(void)dcl_printf(outfile,"%s", work);
        	}
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
    (void) dcl_printf(outfile,"\n");
#endif

    return(0);
}

//---------------------------------------------------------------------------
int show_default(PARAM_T *p, PARAM_T *q)
{
    char    dos[MAX_TOKEN];
    char    vms[MAX_TOKEN];

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    *dos = 0; *vms = 0;
    if (_getcwd(dos,MAX_TOKEN) != NULL) {
        if (dos[strlen(dos)-1] != SLASH_CHR) strcat(dos,SLASH_STR);
        cvfs_dos_to_vms(dos,vms);
    }
    (void) dcl_printf(outfile,"%s\n",vms);
    return(0);
}

//---------------------------------------------------------------------------
#define BYTES       1
#define BLOCKS      2
#define KILOBYTES   3
#define MEGABYTES   4
#define GIGABYTES   5

int show_devices(PARAM_T *p, PARAM_T *q)
{
    char    *size_q[7] = {"NONE","BYTES","BLOCKS","KILOBYTES","MEGABYTES","GIGABYTES",NULL};
    char    name[MAX_TOKEN];
    int     i;
    int     j;
    int     retcod = DCL_OK;
    char    size_type = 1;
    char    buffer[1024];
    BOOL    bFull = FALSE;

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *name = 0;

    for (i = 0; q[i].tag && retcod == DCL_OK; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 5:                            /*  /FULL   */
                    bFull = TRUE;
                    break;
                case 6:                            /*  /SIZE   */
                    if (!*q[i].value) strcpy(q[i].value,"BYTES");
                    j = dcl_search(size_q,q[i].value);
                    switch (j) {
                        case AMBIGUOUS:
                            (void) dcl_printf(outfile,"Ambiguous /SIZE qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        case NOT_FOUND:
                            (void) dcl_printf(outfile,"Invalid /SIZE qualifier\n");
                            _SEVERITY = 2;
                            _STATUS = 19;
                            retcod = DCL_ERROR;
                            break;
                        case BYTES:
                        case BLOCKS:
                        case KILOBYTES:
                        case MEGABYTES:
                        case GIGABYTES:
                            size_type = (char) j;
                            break;
                        default:
                            ;
                        }
                    break;
                case 9:   // /out
                    break;
                default:
                    ;
                } /* end switch */
        }   /* end for */

    if (retcod == DCL_OK) {
        dcl_string(p[1].value,name,MAX_TOKEN);
        if (*name) {
            (void) cvfs_translate_logical(name);
#ifdef __linux
            memset(buffer, 0, sizeof(buffer));
            (void) GetLogicalDriveStrings(sizeof(buffer), buffer);
            for(i = 0; buffer[i]; i = i + 1 + (int)strlen(&buffer[i])) {
           		DISKINFO di;
           		if (diskinfo(&buffer[i],&di)==DCL_OK) {
           			char str[256];
           			strcpy(str, &buffer[i]);
                   	if (strcasecmp(str, di.szMountPoint)==0) {
                   		strcpy(name, di.szMountPoint);
                   		break;
            		}
            	}
            }
#endif
            i = show_devices_sub(name,size_type, bFull);
            if (i){
                (void) dcl_printf(outfile,"Device not ready.\n");
                _SEVERITY = 2;
                _STATUS = 15;
                }
            return(i);
            }
        else {                              /* all */
            memset(buffer, 0, sizeof(buffer));
            (void) GetLogicalDriveStrings(sizeof(buffer), buffer);

            for(i = 0; buffer[i]; i = i + 1 + (int)strlen(&buffer[i])) {
                (void) show_devices_sub(&buffer[i],size_type, bFull);
            }
        }/*lint !e661 */
    }
    return(retcod);
}

int show_devices_sub(char *name,char size_type, BOOL bFull)
{
    DISKINFO    di;
#ifdef _WIN32
    int         drive;
#endif
    unsigned long long total_size;
    unsigned long long avail_space;
    char        total_size_str[14];
    char        avail_space_str[14];
    char        unit[6];
    char        disk[MAX_TOKEN];

    if (name == NULL) return(DCL_ERROR);
#ifdef _WIN32
    drive = *name - '@';
    if (diskinfo(drive,&di)) return(DCL_ERROR);
    total_size = di.TotalNumberOfBytes;
    avail_space = di.TotalNumberOfFreeBytes;
    disk[0] = name[0];
    disk[1] = name[1];
    disk[2] = 0;
#else
    if (diskinfo(name,&di)) return(DCL_ERROR);
    total_size = di.Total1kBlocks * 1024L;
    avail_space = di.Available1kBlocks * 1024L;
    strcpy(disk, di.szName);
#endif

    switch (size_type) {
        case KILOBYTES:
            total_size   /= 1024L;
            avail_space  /= 1024L;
            strcpy(unit, "Kb");
            break;
        case MEGABYTES:
            total_size  /= 1024000L;
            avail_space /= 1024000L;
            strcpy(unit, "Mb");
            break;
        case GIGABYTES:
            total_size  /= 1024000000L;
            avail_space /= 1024000000L;
            strcpy(unit, "Gb");
            break;
        default:
            strcpy(unit, "bytes");
    }
    if (bFull) {
        dcl_tostring_left(total_size, total_size_str, sizeof(total_size_str));
        dcl_tostring_left(avail_space, avail_space_str, sizeof(avail_space_str));
        (void) dcl_printf(outfile,"%s", disk);
#ifdef _WIN32
        (void) dcl_printf(outfile,"  Volume Id  : %-20.20s      Serial Nr  : %4.4X-%4.4X\n",
               di.szName,
               di.dwVolumeSerialNumber >> 16,
               di.dwVolumeSerialNumber & 0x0000FFFF);
        (void) dcl_printf(outfile,"     Drive type : %-20.20s      File system: %s\n",
               di.szType,
               di.szFileSystemName);
        (void) dcl_printf(outfile,"     Free space : %-14.14s %-5.5s      Total space: %-14.14s %-5.5s\n",
                   avail_space_str, unit, total_size_str, unit);
#else
        (void) dcl_printf(outfile,"\n\tDrive type : %-20.20s\n\tMount Point: %s\n",
               di.szType,
               di.szMountPoint);
        (void) dcl_printf(outfile,"\tFree space : %-14.14s %-5.5s\n\tTotal space: %-14.14s %-5.5s\n",
                   avail_space_str, unit, total_size_str, unit);
#endif
    }
    else {
        dcl_tostring(total_size, total_size_str, sizeof(total_size_str));
        dcl_tostring(avail_space, avail_space_str, sizeof(avail_space_str));
        (void) dcl_printf(outfile,
                          "%s    Total disk space : %s %s, available : %s %s.\n",
                          disk, total_size_str, unit, avail_space_str, unit);
    }
    return(DCL_OK);
}
//---------------------------------------------------------------------------
int show_dosvar(PARAM_T *p, PARAM_T *q)
{
    char    name[MAX_TOKEN];
    char    *value;
    int     i;

    UNREFERENCED_PARAM(q);

    if (p == NULL) return(DCL_ERROR);

    *name = 0;

    dcl_string(p[1].value,name,MAX_TOKEN);
    if (*name) {
        (void) strupr(name);
        if ((value = getenv(name)) != NULL)
           (void) dcl_printf(outfile,"%s=%s\n",name,value);
        else
           (void) dcl_printf(outfile,"%s=\n",name);
        }
    else {                              /* all */
        for (i = 0; environ[i]; i++)
            (void) dcl_printf(outfile,"%d %s\n",i+1 ,environ[i]);
        }

    return(DCL_OK);
}
//---------------------------------------------------------------------------
int show_license(PARAM_T *p, PARAM_T *q)
{
    char *license = "PC-DCL -  DCL command line interpreter for Windows / Linux.\n"
                    "Copyright (C) 1991-2009 by Michel Valentin\n"
                    "\n"
                    "This program is free software; you can redistribute it and/or modify\n"
                    "it under the terms of the enclosed license.\n"
                    "\n"
                    "This program is distributed in the hope that it will be useful,\n"
                    "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
                    "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
                    "\n";

    (void) dcl_printf(outfile,license);

    UNREFERENCED_PARAM(q);
    UNREFERENCED_PARAM(p);

    return(DCL_OK);
}
//---------------------------------------------------------------------------
int show_logical(PARAM_T *p, PARAM_T *q)
{
    char    name[LOGICAL_MAX_NAME];
    char    value[LOGICAL_MAX_VALUE];
    int     i;

    UNREFERENCED_PARAM(q);

    if (p == NULL) return(DCL_ERROR);

    *name = 0; *value = 0;

    dcl_string(p[1].value,name,MAX_TOKEN);
    if (*name) {
        if (name[strlen(name) - 1] != ':')
            strcat(name,":");
        strcpy(value,name);
        (void) cvfs_translate_logical(value);
        if (strcasecmp(name,value)) {
            (void) dcl_printf(outfile,"\"%s\" = \"%s\"\n",name,value);
            }
        }
    else {                              /* all */
        for (i = 0; i < LOGICAL_MAX; i++) {
            if (LOGICAL_TABLE[i].flag != 0) {
                (void) dcl_printf(outfile,"\"%s\" = \"%s\"\n",
                        LOGICAL_TABLE[i].name,LOGICAL_TABLE[i].value);
            }
        }
    }
    return(DCL_OK);
}
//---------------------------------------------------------------------------
int show_memory(PARAM_T *p, PARAM_T *q)
{
#ifdef _WIN32
    MEMORYSTATUS    mi;

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    mi.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&mi);
    (void) dcl_printf(outfile,"Total physical memory : %d K.\n",mi.dwTotalPhys / 1024);
    (void) dcl_printf(outfile,"Free  physical memory : %d K.\n",mi.dwAvailPhys / 1024);
    (void) dcl_printf(outfile,"Total virtual  memory : %d K.\n",mi.dwTotalVirtual / 1024);
    (void) dcl_printf(outfile,"Free  virtual  memory : %d K.\n",mi.dwAvailVirtual / 1024);
#else
    FILE 	*fp = NULL;
    char  	work[MAX_TOKEN];
    char  	cmd[MAX_TOKEN];
    int		line =0;
    strcpy(cmd, "cat /proc/meminfo");
    if ((fp = popen(cmd, "r")) != NULL) {
        while(fgets(work, sizeof(work), fp))  {
        	line++;
            (void)dcl_printf(outfile,"%s", work);
            if (line > 3) {
            	break;
            }
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
    (void) dcl_printf(outfile,"\n");

#endif
    return(0);
}

//---------------------------------------------------------------------------
int show_process(PARAM_T *p, PARAM_T *q)
{
    DWORD   dwPid;
    char    name[SYMBOL_MAX_NAME];
    int     f_all   = 0;
    int     f_cont  = 0;
    int     i       = 0;

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *name = 0;
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /* /ALL      */
                    f_all = 1;
                    break;
                case 4:                                 /* /BRIEF    */
                    f_all = 0;
                    break;
                case 10:                                /* /CONT     */
                    f_cont = 1;
                    f_all = 0;
                    break;
                case 9:                                 /* /OUT      */
                    if (f_cont) {
                        (void) dcl_printf(outfile,"/CONTINUOUS and /OUTPUT are mutually exclusive.\n");
                        _SEVERITY = 2;
                        _STATUS = 19;
                        return(-1);
                    }
                    break;
                default:
                    (void) dcl_printf(outfile,"Invalid command qualifier.\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    return(-1);
                } /* end switch */
        }   /* end for */

    dcl_string(p[1].value,name,SYMBOL_MAX_NAME);

    if (*name) {
        dwPid = atol(name);
    }
    else {
        dwPid = getpid();
    }

    if (f_all) {
        (void) show_process_all(dwPid);
    }
    else if (f_cont) {
        (void) show_process_cont(dwPid);
    }
    else {
       (void) show_process_brief(dwPid);
    }

    return(0);
}

int show_process_brief(DWORD dwPid)
{
    time_t          tl;
    struct tm       tm;
    char            szComputerName[MAX_TOKEN];
    char            szUserName[MAX_TOKEN];
    char            szProcessName[MAX_TOKEN];
    DWORD           size = MAX_TOKEN;
    DWORD           dwPriorityClass = 0;
    DWORD           cntThreads = 0;

    tm_str_to_long("", &tl);
    tm_long_to_tm(&tl, &tm);


    *szComputerName = 0;
    *szUserName = 0;
    *szProcessName = 0;
#ifdef _WIN32
    (void)GetComputerName((LPSTR)szComputerName,(PDWORD)&size);
#else
    gethostname(szComputerName, size);
#endif

#ifdef _WIN32
    {
        HANDLE hProcessSnap;
        PROCESSENTRY32  pe32;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
        if (hProcess == INVALID_HANDLE_VALUE || hProcess == 0) {
            (void) dcl_printf(outfile,"Cannot open process %d. (error %d)\n",dwPid, (int)GetLastError());
             _SEVERITY = 2;
             _STATUS = 19;
             return(-1);
        }
        dwPriorityClass = GetPriorityClass(hProcess);
        (void)GetProcessUser(hProcess, szUserName, sizeof(szUserName));
        (void)CloseHandle(hProcess);
        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap != INVALID_HANDLE_VALUE) {
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hProcessSnap, &pe32)) {
                do {
                    if (dwPid == pe32.th32ProcessID) {
                        cntThreads =  pe32.cntThreads;
                        strcpy(szProcessName, pe32.szExeFile);
                        break;
                    }
                } while (Process32Next(hProcessSnap, &pe32));
            }
            CloseHandle(hProcessSnap);
        }
    }
#endif
#ifdef __linux
    {
        PROCESSINFO pi;
        int			rc = 0;

        rc = get_process_info(dwPid, &pi);
        if (rc == -1) {
             _SEVERITY = 2;
             _STATUS = 19;
             return(-1);
        }
        strcpy(szComputerName, pi.szComputerName);
        strcpy(szUserName, pi.szUserName);
        strcpy(szProcessName, pi.szProcessName);
        dwPriorityClass = pi.dwNice;
        cntThreads = pi.dwThreads;

    }
#endif

    (void) dcl_printf(outfile,
            "%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.%2.2d   User: %-16.16s Process ID:   %ld\n\r",
            tm.tm_mday,
            MONTH[tm.tm_mon + 1],
            tm.tm_year + 1900,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            0,
            szUserName,
            dwPid);
    (void) dcl_printf(outfile,
            "                          Node: %-16.16s Process name: %s\n\n",
            szComputerName, szProcessName);
    (void)dcl_printf(outfile, "Base priority: %d\n", dwPriorityClass);
    (void)dcl_printf(outfile, "Thread count : %d\n", cntThreads);
    if (dwPid == getpid()) {
        (void)dcl_printf(outfile, "Default file spec: ");
        (void)show_default(NULL,NULL);
    }

    return(0);
}

int show_process_modules(DWORD dwPid)
{
    (void)dcl_printf(outfile, "\nProcess Modules info.\n");
#ifdef _WIN32
    {
        HANDLE          hModuleSnap = INVALID_HANDLE_VALUE;
        MODULEENTRY32   me32;

        hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
        if(hModuleSnap != INVALID_HANDLE_VALUE) {
            me32.dwSize = sizeof(MODULEENTRY32);
            if(Module32First(hModuleSnap, &me32)) {
                do {
                    if (me32.th32ProcessID == dwPid) {
                        (void)dcl_printf(outfile, "%s\n", me32.szModule );
                        (void)dcl_printf(outfile, "  executable    : %s\n",   me32.szExePath );
                        (void)dcl_printf(outfile, "  base address  : 0x%08X\n", (DWORD) me32.modBaseAddr );
                        (void)dcl_printf(outfile, "  base size     : %d\n", me32.modBaseSize );
                    }
                } while( Module32Next( hModuleSnap, &me32 ) );
            }
            CloseHandle( hModuleSnap );
        }
    }
#elif defined __linux
    /*
     * UID        PID  PPID  C    SZ   RSS PSR STIME TTY      STAT   TIME CMD
     * valentin 20934 20747  0  1699  4104   0 16:31 pts/2    Ss     0:00 bash
     *
     */
    FILE 	*fp = NULL;
    //int		rc = 0;
    char  	work[MAX_TOKEN];
    char  	cmd[MAX_TOKEN];

    sprintf(cmd, "cat /proc/%ld/maps", dwPid);
    if ((fp = popen(cmd, "r")) != NULL) {
    	dcl_printf(outfile, "address           perms offset  dev   inode      pathname\n");

        while (fgets(work, sizeof(work), fp))  {
        	dcl_printf(outfile, "%s", work);
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}

int show_process_threads(DWORD dwPid)
{
    (void)dcl_printf(outfile, "\nProcess threads info.\n");
#ifdef _WIN32
    {
        HANDLE          hThreadSnap = INVALID_HANDLE_VALUE;
        HANDLE          hThread     = INVALID_HANDLE_VALUE;
        THREADENTRY32   te32;
        DWORD           dwExitCode  = 0;
//        BOOL            bIOPending  = 0;
        char            szState[32];
        FILETIME        CreationTime;
        FILETIME        ExitTime;
        FILETIME        KernelTime;
        FILETIME        UserTime;
        SYSTEMTIME      SystemTime;

        hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwPid);
        if(hThreadSnap != INVALID_HANDLE_VALUE) {
            te32.dwSize = sizeof(THREADENTRY32);
            if(Thread32First(hThreadSnap, &te32)) {
                do {
                    if (te32.th32OwnerProcessID == dwPid) {
                        (void)dcl_printf(outfile, "Thread ID       : 0x%08X\n", (DWORD) te32.th32ThreadID);
                        hThread = OpenThread(THREAD_QUERY_INFORMATION , 0, te32.th32ThreadID);
                        (void)GetExitCodeThread(hThread, &dwExitCode);
//                        GetThreadIOPendingFlag(hThread,&bIOPending);
                        if (dwExitCode == STILL_ACTIVE) {
//                            if (bIOPending)
//                                strcpy(szState, "IO Pending");
//                            else
                                strcpy(szState, "Active");
                        }
                        else {
                            sprintf(szState, "Terminated (rc %d)", (int)dwExitCode);
                        }
                        (void)dcl_printf(outfile, "  state         : %s\n", szState);
                        (void)dcl_printf(outfile, "  base priority : %d\n", te32.tpBasePri);
                        GetThreadTimes(hThread, &CreationTime, &ExitTime, &KernelTime, &UserTime);
                        (void)FileTimeToSystemTime(&CreationTime, &SystemTime);
                        (void) dcl_printf(outfile,"  Creation time : %2.2d:%2.2d:%2.2d.%3.3d\n",
                                                  SystemTime.wHour,
                                                  SystemTime.wMinute,
                                                  SystemTime.wSecond,
                                                  SystemTime.wMilliseconds);
                        (void)FileTimeToSystemTime(&ExitTime, &SystemTime);
                        (void) dcl_printf(outfile,"  Exit time     : %2.2d:%2.2d:%2.2d.%3.3d\n",
                                                  SystemTime.wHour,
                                                  SystemTime.wMinute,
                                                  SystemTime.wSecond,
                                                  SystemTime.wMilliseconds);
                        (void)FileTimeToSystemTime(&KernelTime, &SystemTime);
                        (void) dcl_printf(outfile,"  Kernel time   : %2.2d:%2.2d:%2.2d.%3.3d\n",
                                                  SystemTime.wHour,
                                                  SystemTime.wMinute,
                                                  SystemTime.wSecond,
                                                  SystemTime.wMilliseconds);
                        (void)FileTimeToSystemTime(&UserTime, &SystemTime);
                        (void) dcl_printf(outfile,"  User time     : %2.2d:%2.2d:%2.2d.%3.3d\n",
                                                  SystemTime.wHour,
                                                  SystemTime.wMinute,
                                                  SystemTime.wSecond,
                                                  SystemTime.wMilliseconds);
                        CloseHandle(hThread);
                    }
                } while( Thread32Next( hThreadSnap, &te32 ) );
            }
            CloseHandle( hThreadSnap );
        }
    }
#elif defined __linux
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}

int show_process_times(DWORD dwPid)
{
    (void)dcl_printf(outfile, "\nProcess times info.\n");
#ifdef _WIN32
    {
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        FILETIME        CreationTime;
        FILETIME        ExitTime;
        FILETIME        KernelTime;
        FILETIME        UserTime;
        SYSTEMTIME      SystemTime;

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
        (void)GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
        (void)FileTimeToSystemTime(&CreationTime, &SystemTime);
        (void) dcl_printf(outfile,"Creation time : %2d:%2.2d:%2.2d.%3.3d\n",
                                      SystemTime.wHour,
                                      SystemTime.wMinute,
                                      SystemTime.wSecond,
                                      SystemTime.wMilliseconds);
        (void)FileTimeToSystemTime(&ExitTime, &SystemTime);
        (void) dcl_printf(outfile,"Exit time     : %2d:%2.2d:%2.2d.%3.3d\n",
                                      SystemTime.wHour,
                                      SystemTime.wMinute,
                                      SystemTime.wSecond,
                                      SystemTime.wMilliseconds);
        (void)FileTimeToSystemTime(&KernelTime, &SystemTime);
        (void) dcl_printf(outfile,"Kernel time   : %2d:%2.2d:%2.2d.%3.3d\n",
                                      SystemTime.wHour,
                                      SystemTime.wMinute,
                                      SystemTime.wSecond,
                                      SystemTime.wMilliseconds);
        (void)FileTimeToSystemTime(&UserTime, &SystemTime);
        (void) dcl_printf(outfile,"User time     : %2d:%2.2d:%2.2d.%3.3d\n",
                                      SystemTime.wHour,
                                      SystemTime.wMinute,
                                      SystemTime.wSecond,
                                      SystemTime.wMilliseconds);
        (void)CloseHandle(hProcess);
    }
#elif defined(__linux)
	(void) dcl_printf(outfile,"Creation time : %s\n", pi.szStartTime);
	(void) dcl_printf(outfile,"Elapsed time  : %s\n", pi.szElapsedTime);
	(void) dcl_printf(outfile,"CPU time      : %s\n", pi.szCPUTime);
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}

int show_process_groups(DWORD dwPid)
{
        (void)dcl_printf(outfile, "\nProcess groups.\n");
#ifdef _WIN32
    {
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        HANDLE              hToken;
        TOKEN_GROUPS        *pToken;
        char                buffer[4096];
        DWORD               dwBufLen    = sizeof(buffer);
        DWORD               dwInfoLen   = 0;
        char                pchName[1024];
        DWORD               cchName     = sizeof(pchName);
        int                 i   = 0;
        char                pchReferencedDomainName[1024];
        DWORD               cchReferencedDomainName = sizeof(pchReferencedDomainName);
        SID_NAME_USE        snu;

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            if (GetTokenInformation(hToken, TokenGroups, &buffer, dwBufLen, &dwInfoLen)) {
                pToken = (TOKEN_GROUPS*) buffer;
                for(i = 0; i < pToken->GroupCount; i++) {
                    if (LookupAccountSid(NULL, pToken->Groups[i].Sid, pchName, &cchName,
                                     pchReferencedDomainName, &cchReferencedDomainName, &snu)) {
                        (void)dcl_printf(outfile, "    %s\n", pchName);
                    }
                }
            }
            CloseHandle(hToken);
        }
        (void)CloseHandle(hProcess);
    }
#elif defined(__linux)
    /*
     * ps -o gid,group,fgid,fgroup,rgid,rgroup
     */
    FILE 	*fp = NULL;
    char  	work[MAX_TOKEN];
    char  	cmd[MAX_TOKEN];

    sprintf(cmd, "ps -o gid,group,fgid,fgroup,rgid,rgroup %ld", dwPid);
    if ((fp = popen(cmd, "r")) != NULL) {
        while (fgets(work, sizeof(work), fp))  {
			(void) dcl_printf(outfile,"%s\n", work);
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}

int show_process_privileges(DWORD dwPid)
{
#ifdef _WIN32
    (void)dcl_printf(outfile, "\nProcess privileges.\n");
    {
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        HANDLE              hToken;
        TOKEN_PRIVILEGES    *pToken;
        char                buffer[4096];
        DWORD               dwBufLen    = sizeof(buffer);
        DWORD               dwInfoLen   = 0;
        char                pchName[1024];
        DWORD               cchName     = sizeof(pchName);
        int                 i   = 0;

        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            if (GetTokenInformation(hToken, TokenPrivileges, &buffer, dwBufLen, &dwInfoLen)) {
                pToken = (TOKEN_PRIVILEGES*) buffer;
                for(i = 0; i < pToken->PrivilegeCount; i++) {
                    if (LookupPrivilegeName(NULL, &pToken->Privileges[i].Luid,pchName, &cchName)) {
                        (void)dcl_printf(outfile, "    %s\n", pchName);
                    }
                }
            }
            CloseHandle(hToken);
        }
        (void)CloseHandle(hProcess);
    }
#endif
    return(0);
}

int show_process_memory(DWORD dwPid)
{
        (void)dcl_printf(outfile, "\nProcess memory info.\n");
#ifdef _WIN32
    {
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        HMODULE         hPsapi         = NULL;
        FARPROC         pfnGetProcessMemoryInfo     = NULL;
        PROCESS_MEMORY_COUNTERS   pmc;

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

        hPsapi = LoadLibrary("PSAPI.DLL");
        if (hPsapi != NULL) {
            pfnGetProcessMemoryInfo = GetProcAddress(hPsapi, "GetProcessMemoryInfo");
        }
        if (pfnGetProcessMemoryInfo != NULL) {
            (void)pfnGetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc));
            (void)dcl_printf(outfile, "page fault count            : %d.\n", pmc.PageFaultCount);
            (void)dcl_printf(outfile, "peak working set size       : %d.\n", pmc.PeakWorkingSetSize);
            (void)dcl_printf(outfile, "current workingset size     : %d.\n", pmc.WorkingSetSize);
            (void)dcl_printf(outfile, "peak paged pool usage       : %d.\n", pmc.QuotaPeakPagedPoolUsage);
            (void)dcl_printf(outfile, "current paged pool usage    : %d.\n", pmc.QuotaPagedPoolUsage);
            (void)dcl_printf(outfile, "peak nonpaged pool usage    : %d.\n", pmc.QuotaPeakNonPagedPoolUsage);
            (void)dcl_printf(outfile, "current nonpaged pool usage : %d.\n", pmc.QuotaNonPagedPoolUsage);
            (void)dcl_printf(outfile, "current space allocated for the pagefile : %d.\n", pmc.PagefileUsage);
            (void)dcl_printf(outfile, "peak space allocated for the pagefile    : %d.\n", pmc.PeakPagefileUsage);
        }
        if (hPsapi != NULL) {
            (void)FreeLibrary(hPsapi);
        }
        (void)CloseHandle(hProcess);
    }
#elif defined(__linux)
	(void)dcl_printf(outfile, "total program size            : %ldKB.\n", pi.dwMemSize);
	(void)dcl_printf(outfile, "resident set size             : %ldKB.\n", pi.dwMemRSS);
	(void)dcl_printf(outfile, "Number of shared pages        : %ld.\n", pi.dwMemShared);
	(void)dcl_printf(outfile, "Number of code pages          : %ld.\n", pi.dwMemCode);
	(void)dcl_printf(outfile, "Number of pages of data/stack : %ld.\n", pi.dwMemData);
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}

int show_process_IOcount(DWORD dwPid)
{
        (void)dcl_printf(outfile, "\nProcess IO count.\n");
#ifdef _WIN32
    {
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        IO_COUNTERS     ioc;
        FARPROC         pfnGetProcessIoCounters     = NULL;


        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);

        pfnGetProcessIoCounters =  GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),
                                                         "GetProcessIoCounters");
        if (pfnGetProcessIoCounters != NULL) {
            (void)pfnGetProcessIoCounters(hProcess, &ioc);
            (void)dcl_printf(outfile, "Read  operation count : %I64u\t(%I64u bytes).\n", ioc.ReadOperationCount, ioc.ReadTransferCount);
            (void)dcl_printf(outfile, "Write operation count : %I64u\t(%I64u bytes).\n", ioc.WriteOperationCount, ioc.WriteTransferCount);
            (void)dcl_printf(outfile, "Other operation count : %I64u\t(%I64u bytes).\n", ioc.OtherOperationCount, ioc.OtherTransferCount);
        }
        else {
            (void)dcl_printf(outfile, "IO counters not available for this OS version.\n");
        }
        (void)CloseHandle(hProcess);
    }
#elif defined __linux
    /*
     * UID        PID  PPID  C    SZ   RSS PSR STIME TTY      STAT   TIME CMD
     * valentin 20934 20747  0  1699  4104   0 16:31 pts/2    Ss     0:00 bash
     *
     */
    FILE 	*fp = NULL;
    //int		rc = 0;
    char  	work[MAX_TOKEN];
    char  	cmd[MAX_TOKEN];

    sprintf(cmd, "cat /proc/%ld/io", dwPid);
    if ((fp = popen(cmd, "r")) != NULL) {
        while (fgets(work, sizeof(work), fp))  {
        	dcl_printf(outfile, "%s", work);
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
#else
    (void)dcl_printf(outfile, "in construction...\n");
#endif
    return(0);
}


int show_process_all(DWORD dwPid)
{
    int rc = get_process_info(dwPid, &pi);
    if (rc != -1) {
    	(void) show_process_brief(dwPid);
        (void)show_process_memory(dwPid);
        (void)show_process_times(dwPid);
        (void)show_process_groups(dwPid);
        (void)show_process_privileges(dwPid);
        (void)show_process_IOcount(dwPid);
        (void)show_process_modules(dwPid);
        (void)show_process_threads(dwPid);
    }
    return(0);
}

int get_process_info(DWORD dwPid, PROCESSINFO *pi)
{
    memset(pi, 0, sizeof(PROCESSINFO));
#ifdef _WIN32
    {
        HANDLE          hProcessSnap = INVALID_HANDLE_VALUE;;
        HANDLE          hProcess = INVALID_HANDLE_VALUE;
        HMODULE         hPsapi = NULL;
        FARPROC         pfnGetProcessMemoryInfo  = NULL;
        FARPROC         pfnGetProcessIoCounters  = NULL;
//        FARPROC         pfnGetProcessHandleCount = NULL;
        HANDLE          hModuleSnap = INVALID_HANDLE_VALUE;
        PROCESSENTRY32  pe32;
        MODULEENTRY32   me32;
        DWORD           size = MAX_TOKEN;

        (void)GetComputerName((LPSTR)pi->szComputerName,(PDWORD)&size);

        hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPid);
        if (hProcess == INVALID_HANDLE_VALUE || hProcess == 0) {
            return(-1);
        }
        pi->dwPriorityClass = GetPriorityClass(hProcess);
        (void)GetProcessUser(hProcess, pi->szUserName, sizeof(pi->szUserName));
//        pfnGetProcessHandleCount =  GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetProcessHandleCount");
//        if (pfnGetProcessHandleCount != NULL) {
//            (void)pfnGetProcessHandleCount(hProcess, &pi->dwHandles);
//        }
        pfnGetProcessIoCounters =  GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")),"GetProcessIoCounters");
        if (pfnGetProcessIoCounters != NULL) {
            (void)pfnGetProcessIoCounters(hProcess, &pi->ioc);
        }
        (void)GetProcessTimes(hProcess, &pi->CreationTime, &pi->ExitTime, &pi->KernelTime, &pi->UserTime);

        hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hProcessSnap != INVALID_HANDLE_VALUE) {
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hProcessSnap, &pe32)) {
                do {
                    if (dwPid == pe32.th32ProcessID) {
                        pi->dwThreads =  pe32.cntThreads;
                        strcpy(pi->szProcessName, pe32.szExeFile);
                        break;
                    }
                } while (Process32Next(hProcessSnap, &pe32));
            }
            CloseHandle(hProcessSnap);
        }

        hModuleSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
        if(hModuleSnap != INVALID_HANDLE_VALUE) {
            me32.dwSize = sizeof(MODULEENTRY32);
            if(Module32First(hModuleSnap, &me32)) {
                do {
                    if (me32.th32ProcessID == dwPid) {
                        strcpy(pi->szExename, me32.szExePath);
                        break;
                    }
                } while( Module32Next( hModuleSnap, &me32 ) );
            }
            CloseHandle( hModuleSnap );
        }

        hPsapi = LoadLibrary("PSAPI.DLL");
        if (hPsapi != NULL) {
            pfnGetProcessMemoryInfo = GetProcAddress(hPsapi, "GetProcessMemoryInfo");
        }
        if (pfnGetProcessMemoryInfo != NULL) {
            (void)pfnGetProcessMemoryInfo(hProcess, &pi->pmc, sizeof(pi->pmc));
        }
        if (hPsapi != NULL) {
            (void)FreeLibrary(hPsapi);
        }
        CloseHandle(hProcess);
    }
#elif defined(__linux)
    {
        FILE 	*fp = NULL;
        int		rc = 0;
        char  	work[MAX_TOKEN];
        char  	cmd[MAX_TOKEN];
        long    m1,m2,m3,m4,m5,m6,m7;
        size_t	size = MAX_TOKEN;

        // 	brief
        gethostname(pi->szComputerName, size);
        sprintf(cmd, "ps h -o comm,uid,user,gid,group,nice,nlwp,stat,cmd %ld", dwPid);
        if ((fp = popen(cmd, "r")) != NULL) {
            if (fgets(work, sizeof(work), fp))  {
            	rc = sscanf(work, "%s %lu %s %lu %s %lu %lu %s %s",
            			pi->szProcessName, &pi->dwUID, pi->szUserName, &pi->dwGID, pi->szGroupName,
            			&pi->dwNice, &pi->dwThreads, pi->szStat, pi->szExename);
            }
            pclose(fp);
        }
        else {
            _STATUS = errno;
            _SEVERITY = 2;
            return(-1);
        }
        // MEM info
        sprintf(cmd, "cat /proc/%ld/statm", dwPid);
        if ((fp = popen(cmd, "r")) != NULL) {
            if (fgets(work, sizeof(work), fp))  {
            	rc = sscanf(work, "%ld %ld %ld %ld %ld %ld %ld", &m1, &m2, &m3, &m4, &m5, &m6, &m7);
            	if (rc > 0) {
            		pi->dwMemSize = m1;
            		pi->dwMemRSS = m2;
            		pi->dwMemShared = m3;
            		pi->dwMemCode = m4;
            		pi->dwMemData = m6;
            	}
            }
            pclose(fp);
        }

        // times
        sprintf(cmd, "ps h -o start,etime,time %ld", dwPid);
        if ((fp = popen(cmd, "r")) != NULL) {
            if (fgets(work, sizeof(work), fp))  {
            	rc = sscanf(work, "%s %s %s", pi->szStartTime, pi->szElapsedTime, pi->szCPUTime);
            }
            pclose(fp);
        }

        // IO counters
        sprintf(cmd, "cat /proc/%ld/io", dwPid);
        if ((fp = popen(cmd, "r")) != NULL) {
            while (fgets(work, sizeof(work), fp))  {
            	char name[MAX_TOKEN];
            	DWORD value = 0;
            	rc = sscanf(work, "%s %lu", name, &value);
            	if (rc > 0) {
            		if (strcmp(name, "syscr:") == 0) pi->dwSysCR = value;
            		if (strcmp(name, "syscw:") == 0) pi->dwSysCW = value;
            		if (strcmp(name, "read_bytes:") == 0) pi->dwReadBytes = value;
            		if (strcmp(name, "write_bytes:") == 0) pi->dwWriteBytes = value;
            		if (strcmp(name, "write_bytes:") == 0) pi->dwCancelledWriteBytes = value;
            	}
            }
            pclose(fp);
        }
    }
#else
#endif
    return(0);
}

int show_process_cont(DWORD dwPid)
{
	time_t          tl;
    struct tm       tm;
    PROCESSINFO     pi;
    int             rc;
#ifdef _WIN32
    SYSTEMTIME      SystemTime;
#endif

    while(1) {
        (void) tio_clrscr();
#ifdef _WIN32
        rc = get_process_info(dwPid, &pi);
        if (rc == -1) {
            (void) dcl_printf(outfile,"Cannot open process %d. (error %d)\n",dwPid, (int)GetLastError());
             _SEVERITY = 2;
             _STATUS = 19;
             return(-1);
        }
        tm_str_to_long("", &tl);
        tm_long_to_tm(&tl, &tm);
        (void) dcl_printf(outfile,
            "    Process %-56.56s%2.2d:%2.2d:%2.2d\n\n\n",
            pi.szProcessName,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec
            );
        (void) dcl_printf(outfile,"    PID          %-06.06d               Working Set      %d\n\n",
                                  dwPid, pi.pmc.WorkingSetSize);
        (void) dcl_printf(outfile,"    UIC          %-12.12s         Virtual pages    %d\n\n",
                                  pi.szUserName, pi.pmc.PagefileUsage);
        (void)FileTimeToSystemTime(&pi.KernelTime, &SystemTime);
        (void) dcl_printf(outfile,"    Kernel time  %2.2d:%2.2d:%2.2d.%3.3d         Read  IO         %I64d (%I64d bytes)\n\n",
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond,
                                  SystemTime.wMilliseconds,
                                  pi.ioc.ReadOperationCount, pi.ioc.ReadTransferCount);
        (void)FileTimeToSystemTime(&pi.UserTime, &SystemTime);
        (void) dcl_printf(outfile,"    User time    %2.2d:%2.2d:%2.2d.%3.3d         Write IO         %I64d (%I64d bytes)\n\n",
                                  SystemTime.wHour,
                                  SystemTime.wMinute,
                                  SystemTime.wSecond,
                                  SystemTime.wMilliseconds,
                                  pi.ioc.WriteOperationCount, pi.ioc.WriteTransferCount);
        (void) dcl_printf(outfile,"    Threads      %-06.06d               Other IO         %I64d (%I64d bytes)\n\n",
                                  pi.dwThreads, pi.ioc.OtherOperationCount, pi.ioc.OtherTransferCount);
        (void) dcl_printf(outfile,"                                      Page faults      %d\n\n",
                                  pi.pmc.PageFaultCount);
        (void) dcl_printf(outfile,"\n%s\n", pi.szExename);
#elif defined (__linux)
        rc = get_process_info(dwPid, &pi);
        if (rc == -1) {
            (void) dcl_printf(outfile,"Cannot open process %d.\n",dwPid);
             _SEVERITY = 2;
             _STATUS = 19;
             return(-1);
        }
        tm_str_to_long("", &tl);
        tm_long_to_tm(&tl, &tm);
        (void) dcl_printf(outfile,
            "    Process %-56.56s%2.2d:%2.2d:%2.2d\n\n\n",
            pi.szProcessName,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec
            );
        (void) dcl_printf(outfile,"    PID          %-06.06d               Working Set      %d\n\n",
                                  dwPid, pi.dwMemRSS);
        (void) dcl_printf(outfile,"    UIC          %-12.12s         Memory size      %d\n\n",
                                  pi.szUserName, pi.dwMemSize);
        (void) dcl_printf(outfile,"    Kernel time  %-12.12s         Read  IO         %lu (%lu bytes)\n\n",
                                  pi.szCPUTime, pi.dwSysCR, pi.dwWriteBytes);
        (void) dcl_printf(outfile,"    User time    %-12.12s         Write IO         %lu (%lu bytes)\n\n",
                                  pi.szElapsedTime, pi.dwSysCW, pi.dwWriteBytes);
        (void) dcl_printf(outfile,"    Threads      %-06.06d               Cached IO        %lu bytes\n\n",
                                  pi.dwThreads, pi.dwCancelledWriteBytes);
        (void) dcl_printf(outfile,"\n%s\n", pi.szExename);
#else
        (void)dcl_printf(outfile, "in construction...");
#endif
        if (tio_wait_for_char(5)) {
            int ch = tio_getch();
            if (ch == 'e' || ch == 'E' || CTRL_Z || CTRL_Y){
                (void)dcl_printf(outfile, "\n");
                 break;
            }
        }
    }
    return(0);
}

//---------------------------------------------------------------------------
int show_status(PARAM_T *p, PARAM_T *q)
{
    time_t         tl;
    struct tm      tm;
    time_t         time_l;
    double         elapsed,hour,min,sec;

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    tm_str_to_long("", &tl);
    tm_long_to_tm(&tl, &tm);
    (void) dcl_printf(outfile,
            "Status on %2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.%2.2d\n",
            tm.tm_mday,
            MONTH[tm.tm_mon + 1],
            tm.tm_year + 1900,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            0);



    (void) time(&time_l);
    elapsed = difftime(time_l,start_time);
    hour = elapsed / 3600; elapsed = fmod(elapsed,3600.0);
    min  = elapsed / 60;
    sec  = fmod(elapsed,60.0);
    (void) dcl_printf(outfile,"Elapsed time : ");
    if (floor(hour) == 1)
        (void) dcl_printf(outfile,"%.f hour, ",hour);
    if (floor(hour) > 1)
        (void) dcl_printf(outfile,"%.f hours, ",hour);
    if (floor(min) > 0)
        (void) dcl_printf(outfile,"%.f min, ",min);
    (void) dcl_printf(outfile,"%.f sec.\n",sec);
    return(0);
}

//---------------------------------------------------------------------------
int show_symbol(PARAM_T *p, PARAM_T *q)
{
    char    name[SYMBOL_MAX_NAME];
    SYMSCAN ss;
    int     f_all   = 0;
    int     f_global= 0;
    int     f_local = 0;
    int     len     = 0;
    int     i = 0;
    long    l;

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *name = 0;
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /* /ALL      */
                    f_all = 1;
                    break;
                case 2:                                 /* /LOCAL    */
                    f_global = 0;
                    f_local  = 1;
                    break;
                case 3:                                 /*  /GLOBAL  */
                    f_global = 1;
                    f_local  = 0;
                    break;
                case 9:
                    break;
                default:
                    (void) dcl_printf(outfile,"Invalid command qualifier (%s).\n",q[i].name);
                    _SEVERITY = 2;
                    _STATUS = 19;
                    return(-1);
                } /* end switch */
        }   /* end for */

    dcl_string(p[1].value,name,SYMBOL_MAX_NAME);

    if (f_all && *name) {
        (void) dcl_printf(outfile,"/ALL and symbol-name are mutually exclusive.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(-1);
        }

    while(name[len] && name[len] != '*') len++;
    symbol_first(&ss);
    while (ss.name[0]) {
        if (ss.value[0]) {
           if (f_all || strncasecmp(name,ss.name,(size_t)len)==0) {
                if (!(f_global || f_local) ||
                    (f_global && ss.level == 0) ||
                    (f_local && ss.level == D)) {
                    (void) dcl_printf(outfile,"%s =",ss.name);
                    if (ss.level == 0)
                        (void) dcl_printf(outfile,"=");
                    (void) dcl_printf(outfile," %s",ss.value);
                    if (EXP_isnumber(ss.value)) {
                        l = atol(ss.value);
                        (void) dcl_printf(outfile,"    Hex = %08.8lX    Octal = %06.6o",l,(unsigned int) l);
                    }
                    (void) dcl_printf(outfile,"\n");
                }
            }
        }
        symbol_next(&ss);
    }
    if (f_all) {
        (void) dcl_printf(outfile,"$STATUS = %d\n",_MSTATUS);
        (void) dcl_printf(outfile,"$SEVERITY = %d\n",_MSEVERITY);
    }
    return(0);
}

//---------------------------------------------------------------------------
int show_system(PARAM_T *p, PARAM_T *q)
{
    time_t         tl;
    struct tm      tm;
    time_t          time_l;
    long            elapsed,hour,min,sec;
    char            work[MAX_TOKEN];
    DWORD           size = MAX_TOKEN;
    int             f_programs     = 1;
    int             f_drivers     = 0;
    int                f_all        = 0;
    int             i             = 0;
#ifdef _WIN32
    int             nCount         = 0;
    HMODULE         hPsapi         = NULL;
#else
    FILE            *fp         = NULL;
    char            cmd[32];
#endif

    UNREFERENCED_PARAM(p);

    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
            case 1:                                 /* /ALL      */
                f_programs = 1;
                f_drivers = 1;
                f_all = 1;
                break;
#ifdef _WIN32
            case 7:                                 /* /DRIVERS   */
                f_drivers = 1;
                f_programs = 0;
                break;
            case 8:                                 /*  /PROGRAMS */
                f_drivers = 0;
                f_programs = 1;
                break;
#endif
            case 9:
                break;
            default:
                (void) dcl_printf(outfile,"Invalid command qualifier (%s).\n",q[i].name);
                _SEVERITY = 2;
                _STATUS = 19;
                return(-1);
            } /* end switch */
    }   /* end for */

#ifdef _WIN32
    hPsapi = LoadLibrary("PSAPI.DLL");
    if (hPsapi == NULL) {
        (void) dcl_printf(outfile,"Not implemented for this OS version.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(-1);
    }
#endif
    tm_str_to_long("", &tl);
    tm_long_to_tm(&tl, &tm);

    *work = 0;
#ifdef _WIN32
    (void)GetComputerName((LPSTR)work,(PDWORD)&size);
#else
    gethostname(work, size);
#endif

    (void) dcl_printf(outfile,
            "PC-DCL %s on node %s    %2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.%2.2d  ",
            VERSION,
            work,
            tm.tm_mday,
            MONTH[tm.tm_mon + 1],
            tm.tm_year + 1900,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            0);

    (void) time(&time_l);
    elapsed = (long)floor(difftime(time_l,start_time));
    hour = elapsed / 3600; elapsed = elapsed % 3600;
    min  = elapsed / 60;
    sec  = elapsed % 60;
    (void) dcl_printf(outfile,"Uptime %d:%2.2d:%2.2d.\n", hour,min,sec);
#ifdef _WIN32
    if (f_programs) {
        FARPROC pfnEnumProcesses            = GetProcAddress(hPsapi, "EnumProcesses");
        FARPROC pfnEnumProcessModules       = GetProcAddress(hPsapi, "EnumProcessModules");
        FARPROC pfnGetModuleBaseName        = GetProcAddress(hPsapi, "GetModuleBaseNameA");
        //***FARPROC pfnGetModuleFileNameEx      = GetProcAddress(hPsapi, "GetModuleFileNameExA");
        FARPROC pfnGetProcessMemoryInfo     = GetProcAddress(hPsapi, "GetProcessMemoryInfo");
        DWORD   aProcesses[1024], cbNeeded, cProcesses;
        unsigned int u;

        if (pfnEnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) { /*lint !e746 */
            cProcesses = cbNeeded / sizeof(DWORD);
            (void) dcl_printf(outfile,"    PID  %-20.20s%-12.12s%12.12s\t%9.9s %9.9s\n",
                                      "Process name",
                                      "User name",
                                      "CPU    ",
                                      "Page flts",
                                      "Pages");
            for (u = 0; u < cProcesses; u++) {
                char szProcessName[MAX_PATH] = "unknown";
                char szUserName[MAX_PATH]      = "unknown";
                //*****char szFileName[MAX_PATH] = "";
                // Get a handle to the process.
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                              PROCESS_VM_READ,
                                              FALSE, aProcesses[u]);
                if (hProcess) {
                    HMODULE                 hMod;
                    PROCESS_MEMORY_COUNTERS psmemCounters;
                    FILETIME                CreationTime;
                    FILETIME                ExitTime;
                    FILETIME                KernelTime;
                    FILETIME                UserTime;
                    SYSTEMTIME              SystemTime;

                    if (pfnEnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {                      /*lint !e746 */
                        (void)pfnGetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName));             /*lint !e746 */
                        (void)pfnGetProcessMemoryInfo(hProcess, &psmemCounters, sizeof(PROCESS_MEMORY_COUNTERS));     /*lint !e746 */
                        (void)GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime);
                        (void)FileTimeToSystemTime(&UserTime, &SystemTime);
                        (void)GetProcessUser(hProcess, szUserName, sizeof(szUserName));
                        (void) dcl_printf(outfile,"%3.3d %4.4u %-20.20s%-12.12s%2d:%2.2d:%2.2d.%3.3d\t%9d %9d\n",
                                          ++nCount, aProcesses[u], szProcessName, szUserName,
                                          SystemTime.wHour,
                                          SystemTime.wMinute,
                                          SystemTime.wSecond,
                                          SystemTime.wMilliseconds,
                                          psmemCounters.PageFaultCount,
                                          psmemCounters.PagefileUsage);
                    }
                    (void)CloseHandle(hProcess);
                }
            }
        }
        (void) dcl_printf(outfile,"\n");
    }
    if (f_drivers) {
        FARPROC pfnEnumDeviceDrivers        = GetProcAddress(hPsapi, "EnumDeviceDrivers");
        FARPROC pfnGetDeviceDriverBaseName  = GetProcAddress(hPsapi, "GetDeviceDriverBaseNameA");
        FARPROC pfnGetDeviceDriverFileName  = GetProcAddress(hPsapi, "GetDeviceDriverFileNameA");
        DWORD   aProcesses[1024], cbNeeded, cProcesses;

        if (pfnEnumDeviceDrivers(aProcesses, sizeof(aProcesses), &cbNeeded)) { /*lint !e746 */
            cProcesses = cbNeeded / sizeof(DWORD);
            (void) dcl_printf(outfile,"      Addr.  Name              File name\n");
            for(i = 0; i < (int)cProcesses; i++) {
                char szBaseName[MAX_PATH] = "unknown";
                char szFileName[MAX_PATH] = "";
                (void)pfnGetDeviceDriverBaseName(aProcesses[i], szBaseName, sizeof(szBaseName)); /*lint !e746 */
                (void)pfnGetDeviceDriverFileName(aProcesses[i], szFileName, sizeof(szFileName)); /*lint !e746 */
                (void) dcl_printf(outfile,"%3.3d %p %-16.16s  %s\n", i+1, aProcesses[i], szBaseName, szFileName);
            }
        }

        (void) dcl_printf(outfile,"\n");
    }
    (void)FreeLibrary(hPsapi);
#else
    if (f_all) {
        strcpy(cmd, "ps -ef");
    }
    else {
        strcpy(cmd, "ps -f");
    }
    if ((fp = popen(cmd, "r")) != NULL) {
        while(fgets(work, sizeof(work), fp))  {
            (void)dcl_printf(outfile,"%s", work);
        }
        pclose(fp);
    }
    else {
        _STATUS = errno;
        _SEVERITY = 2;
        return(-1);
    }
    (void) dcl_printf(outfile,"\n");
#endif
    return(0);
}

//---------------------------------------------------------------------------
int show_term(PARAM_T *p, PARAM_T *q)
{
    char    *color_q[] = {"BLACK","BLUE","GREEN","CYAN","RED","MAGENTA","BROWN","LIGHTGRAY",
                         "DARKGRAY","LIGHTBLUE","LIGHTGREEN","LIGHTCYAN","LIGHTRED",
                         "LIGHTMAGENTA","YELLOW","WHITE",NULL};

    enum    color  {BLACK,BLUE,GREEN,CYAN,RED,MAGENTA,BROWN,LIGHTGRAY,
                    DARKGRAY,LIGHTBLUE,LIGHTGREEN,LIGHTCYAN,LIGHTRED,
                    LIGHTMAGENTA,YELLOW,WHITE};

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    (void) dcl_printf(outfile, "Terminal Info\n");
    (void) dcl_printf(outfile, "Type         : %s\n", terminfo.szInfo);
    (void) dcl_printf(outfile, "Size         : %d * %d\n", terminfo.Size.X, terminfo.Size.Y);
    if (terminfo.bg_color <= WHITE && terminfo.fg_color <= WHITE)
        (void) dcl_printf(outfile, "For. colour  : %s\nBck. colour  : %s\n",
                          color_q[terminfo.fg_color], color_q[terminfo.bg_color]);
    else
        (void) dcl_printf(outfile, "For. colour  : %d\nBck. colour  : %d\n",
                          terminfo.fg_color, terminfo.bg_color);
    return(0);
}

//---------------------------------------------------------------------------
int show_time(PARAM_T *p, PARAM_T *q)
{
    time_t         tl;
    struct tm      tm;

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    tm_str_to_long("", &tl);
    tm_long_to_tm(&tl, &tm);

    (void) dcl_printf(outfile,
            "%2.2d-%s-%4.4d %2.2d:%2.2d:%2.2d.%2.2d\n",
            tm.tm_mday,
            MONTH[tm.tm_mon + 1],
            tm.tm_year + 1900,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec,
            0);
    return(0);
}

//---------------------------------------------------------------------------
int show_translation(PARAM_T *p, PARAM_T *q)
{
    char    token[MAX_TOKEN];
    char    value[MAX_TOKEN];

    UNREFERENCED_PARAM(q);

    if (p == NULL) return(DCL_ERROR);

    *value = 0;

    dcl_string(p[1].value,token,MAX_TOKEN);
    if (*token) {
        if (token[strlen(token) - 1] == ':')
            token[strlen(token) - 1] = 0;
        (void) logical_get(token,value);
          (void) dcl_printf(outfile,"\"%s\" = \"%s\"\n",token,value);
        }
    return(0);
}
//---------------------------------------------------------------------------
int show_user(PARAM_T *p, PARAM_T *q)
{
    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    strcpy(INI_NAME,"INI$USERNAME");
    (void) logical_get(INI_NAME,INI_VALUE);
    (void) dcl_printf(outfile,"%s\n",INI_VALUE);
    return(0);
}


//---------------------------------------------------------------------------
int show_key(PARAM_T *p, PARAM_T *q)
{
    char    *keys[25] = {"F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
                         "SF1","SF2","SF3","SF4","SF5","SF6","SF7","SF8","SF9","SF10",
                         "SF11","SF12",NULL};
    char    token[MAX_TOKEN];
    char    name[SYMBOL_MAX_NAME];
    int     f_all   = 0;
    int     f_full  = 0;
    int     first   = 0;
    int     last    = 0;
    int     i       = 0;

    if (p == NULL || q == NULL) return(DCL_ERROR);

    *token = 0; *name = 0;
    for (i = 0; q[i].tag; i++) {
        if (q[i].flag & PRESENT)
            switch (q[i].tag) {
                case 1:                                 /* /ALL      */
                    f_all = 1;
                    break;
                case 4:                                 /* /BRIEF    */
                    f_full = 0;
                    break;
                case 5:                                 /* /FULL    */
                    f_full = 1;
                    break;
                case 9:
                    break;
                default:
                    (void) dcl_printf(outfile,"Invalid command qualifier.\n");
                    _SEVERITY = 2;
                    _STATUS = 19;
                    return(-1);
                } /* end switch */
        }   /* end for */

    dcl_string(p[1].value,name,SYMBOL_MAX_NAME);

    if (f_all && *name) {
        (void) dcl_printf(outfile,"/ALL and key-name are mutually exclusive.\n");
        _SEVERITY = 2;
        _STATUS = 19;
        return(-1);
        }

    if (!*name)
        f_all = 1;

    if (f_all){
        first = 0;
        last  = 23;
        }
    else {
        i = dcl_search_key(keys,name);
        if (i < 0) {
            (void) dcl_printf(outfile,"Invalid KEY parameter\n");
            _SEVERITY = 2;
            _STATUS = 19;
            return(-1);
            }
        first = i;
        last = i;
        }

    for (i = first; i <= last; i++){
        if (keydef[i].value) {
            if (f_full) {
                (void) dcl_printf(outfile,"%4.4s -  %s\n",keys[i],keydef[i].value);
                if (keydef[i].echo)
                    (void) dcl_printf(outfile,"        /ECHO,");
                else
                    (void) dcl_printf(outfile,"        /NOECHO,");
                if (keydef[i].erase)
                    (void) dcl_printf(outfile,"/ERASE,");
                else
                    (void) dcl_printf(outfile,"/NOERASE,");
                if (keydef[i].terminate)
                    (void) dcl_printf(outfile,"/TERMINATE\n");
                else
                    (void) dcl_printf(outfile,"/NOTERMINATE\n");

                }
            else {
                (void) dcl_printf(outfile,"%4.4s -  %s\n",keys[i],keydef[i].value);
                }
            }
        }
    return(0);
}

//---------------------------------------------------------------------------
#define BUFSIZE 80
#define SM_SERVERR2 89
#ifdef _WIN32
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
#endif

int show_version(PARAM_T *p, PARAM_T *q)
{
    char            szWinVer[MAX_TOKEN];
    char            szWork[128];

    UNREFERENCED_PARAM(p);
    UNREFERENCED_PARAM(q);

    *szWinVer = 0;
    *szWork   = 0;

#ifdef _WIN32
   {
       OSVERSIONINFOEX osvi;
       SYSTEM_INFO si;
       PGNSI pGNSI;
       BOOL bOsVersionInfoEx;

       ZeroMemory(&si, sizeof(SYSTEM_INFO));
       ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

       // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
       // If that fails, try using the OSVERSIONINFO structure.

       osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

       if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
       {
          osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
          if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) )
             return FALSE;
       }

       // Call GetNativeSystemInfo if supported
       // or GetSystemInfo otherwise.

       pGNSI = (PGNSI) GetProcAddress(
          GetModuleHandle(TEXT("kernel32.dll")),
          "GetNativeSystemInfo");
       if(NULL != pGNSI)
          pGNSI(&si);
       else GetSystemInfo(&si);

       switch (osvi.dwPlatformId)
       {
          // Test for the Windows NT product family.

          case VER_PLATFORM_WIN32_NT:

          // Test for the specific product.

          if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1 )
          {
             if( osvi.wProductType == VER_NT_WORKSTATION )
                 strcat (szWinVer,"Windows 7 ");
             else strcat (szWinVer,"Windows Server 2008 R2 " );
          }

          if ( osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0 )
          {
             if( osvi.wProductType == VER_NT_WORKSTATION )
                 strcat (szWinVer,"Windows Vista ");
             else strcat (szWinVer,"Windows Server 2008 " );
          }

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2 )
          {
             if( GetSystemMetrics(SM_SERVERR2) )
                strcat (szWinVer,"Microsoft Windows Server 2003 \"R2\" ");
             else if( osvi.wProductType == VER_NT_WORKSTATION &&
                si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64)
             {
                strcat (szWinVer,"Microsoft Windows XP Professional x64 Edition ");
             }
             else strcat (szWinVer,"Microsoft Windows Server 2003, ");
          }

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
             strcat (szWinVer,"Microsoft Windows XP ");

          if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
             strcat (szWinVer,"Microsoft Windows 2000 ");

          if ( osvi.dwMajorVersion <= 4 )
             strcat (szWinVer,"Microsoft Windows NT ");

          // Test for specific product on Windows NT 4.0 SP6 and later.
          if( bOsVersionInfoEx )
          {
             // Test for the workstation type.
             if ( osvi.wProductType == VER_NT_WORKSTATION &&
                  si.wProcessorArchitecture!=PROCESSOR_ARCHITECTURE_IA64)
             {
                if( osvi.dwMajorVersion == 4 )
                   strcat (szWinVer,"Workstation 4.0 " );
                else if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                   strcat (szWinVer,"Home Edition " );
                else strcat (szWinVer,"Professional " );
             }

             // Test for the server type.
             else if ( osvi.wProductType == VER_NT_SERVER ||
                       osvi.wProductType == VER_NT_DOMAIN_CONTROLLER )
             {
                if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==2)
                {
                   if ( si.wProcessorArchitecture ==
                        PROCESSOR_ARCHITECTURE_IA64 )
                   {
                       if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                          strcat (szWinVer,"Datacenter Edition "
                                   "for Itanium-based Systems" );
                       else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                          strcat (szWinVer,"Enterprise Edition "
                                   "for Itanium-based Systems" );
                   }

                   else if ( si.wProcessorArchitecture ==
                       PROCESSOR_ARCHITECTURE_IA64 )
                   {
                       if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                          strcat (szWinVer,"Datacenter x64 Edition " );
                       else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                          strcat (szWinVer,"Enterprise x64 Edition " );
                       else strcat (szWinVer,"Standard x64 Edition " );
                   }

                   else
                   {
                       if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                          strcat (szWinVer,"Datacenter Edition " );
                       else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                          strcat (szWinVer,"Enterprise Edition " );
                       else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                          strcat (szWinVer,"Web Edition " );
                       else strcat (szWinVer,"Standard Edition " );
                   }
                }
                else if(osvi.dwMajorVersion==5 && osvi.dwMinorVersion==0)
                {
                   if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                      strcat (szWinVer,"Datacenter Server " );
                   else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                      strcat (szWinVer,"Advanced Server " );
                   else strcat (szWinVer,"Server " );
                }
                else  // Windows NT 4.0
                {
                   if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                      strcat (szWinVer,"Server 4.0, Enterprise Edition " );
                   else strcat (szWinVer,"Server 4.0 " );
                }
             }
          }
          // Test for specific product on Windows NT 4.0 SP5 and earlier
          else
          {
             HKEY hKey;
             TCHAR szProductType[BUFSIZE];
             DWORD dwBufLen=BUFSIZE*sizeof(TCHAR);
             LONG lRet;

             lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                TEXT("SYSTEM\\CurrentControlSet\\Control\\"
                     "ProductOptions"), 0, KEY_QUERY_VALUE, &hKey );
             if( lRet != ERROR_SUCCESS )
                return FALSE;

             lRet = RegQueryValueEx( hKey, TEXT("ProductType"),
                NULL, NULL, (LPBYTE) szProductType, &dwBufLen);
             RegCloseKey( hKey );

             if( (lRet != ERROR_SUCCESS) ||
                 (dwBufLen > BUFSIZE*sizeof(TCHAR)) )
                return FALSE;

             if ( lstrcmpi( TEXT("WINNT"), szProductType) == 0 )
                strcat (szWinVer,"Workstation " );
             if ( lstrcmpi( TEXT("LANMANNT"), szProductType) == 0 )
                strcat (szWinVer,"Server " );
             if ( lstrcmpi( TEXT("SERVERNT"), szProductType) == 0 )
                strcat (szWinVer,"Advanced Server " );
             sprintf(szWork, "%d.%d ", (int)osvi.dwMajorVersion, (int)osvi.dwMinorVersion );
             strcat(szWinVer, szWork);
          }

          // Display service pack (if any) and build number.

          if( osvi.dwMajorVersion == 4 &&
              lstrcmpi( osvi.szCSDVersion, TEXT("Service Pack 6") ) == 0 )
          {
             HKEY hKey;
             LONG lRet;

             // Test for SP6 versus SP6a.
             lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\"
                     "Hotfix\\Q246009"), 0, KEY_QUERY_VALUE, &hKey );
             if( lRet == ERROR_SUCCESS ){
                sprintf(szWork, "Service Pack 6a (Build %d)\n",
                (int)osvi.dwBuildNumber & 0xFFFF );
                strcat(szWinVer, szWork);
             }
             else // Windows NT 4.0 prior to SP6a
             {
                sprintf(szWork, "%s (Build %d)\n",
                   osvi.szCSDVersion,
                   (int)osvi.dwBuildNumber & 0xFFFF);
                strcat(szWinVer, szWork);
             }

             RegCloseKey( hKey );
          }
          else // not Windows NT 4.0
          {
             sprintf(szWork, "%s (Build %d)\n",
                osvi.szCSDVersion,
                (int)osvi.dwBuildNumber & 0xFFFF);
                strcat(szWinVer, szWork);
          }

          break;

          // Test for the Windows Me/98/95.
          case VER_PLATFORM_WIN32_WINDOWS:

          if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
          {
              strcat(szWinVer,"Microsoft Windows 95 ");
              if (osvi.szCSDVersion[1]=='C' || osvi.szCSDVersion[1]=='B')
                 strcat(szWinVer,"OSR2 " );
          }

          if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
          {
              strcat(szWinVer,"Microsoft Windows 98 ");
              if ( osvi.szCSDVersion[1]=='A' || osvi.szCSDVersion[1]=='B')
                 strcat(szWinVer,"SE " );
          }

          if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
          {
              strcat(szWinVer,"Microsoft Windows Millennium Edition\n");
          }
          break;

          case VER_PLATFORM_WIN32s:

          strcat(szWinVer,"Microsoft Win32s\n");
          break;
       }
   }
#else
    struct utsname uts;
    (void)uname(&uts);
//    sprintf(szWinVer,
//            "%s %s %s %s %s",
//            uts.sysname, uts.nodename, uts.release, uts.version, uts.machine);
    sprintf(szWinVer,
            "%s %s",
            uts.sysname, uts.release);
#endif
    (void) dcl_printf(outfile,"PC-DCL Version %s %2.2s-%3.3s-%s\n",
                               VERSION, &__DATE__[4], __DATE__, &__DATE__[7]);
    (void) dcl_printf(outfile,"running on %s\n",szWinVer);

    return(0);
}

#ifdef _WIN32
int GetProcessUser(HANDLE hProcess, char *pchUsername, DWORD dwLength)
{
    HANDLE      hToken;
    TOKEN_USER  *pToken;
    char        buffer[1024];
    DWORD       dwBufLen    = sizeof(buffer);
    DWORD       dwInfoLen   = 0;
    char        pchName[1024];
    DWORD       cchName     = sizeof(pchName);
    char        pchReferencedDomainName[1024];
    DWORD       cchReferencedDomainName = sizeof(pchReferencedDomainName);
    SID_NAME_USE snu;
    int         nRetCod = 0;

    if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
        if (GetTokenInformation(hToken, TokenUser, &buffer, dwBufLen, &dwInfoLen)) {
            pToken = (TOKEN_USER*) buffer;
            if (LookupAccountSid(NULL, pToken->User.Sid, pchName, &cchName,
                                 pchReferencedDomainName, &cchReferencedDomainName, &snu)) {
                strncpy(pchUsername, pchName, dwLength - 1);
            }
            else {
                nRetCod = GetLastError();
            }
        }
        else {
            nRetCod = GetLastError();
        }
        CloseHandle(hToken);
    }
    return(nRetCod);
}
#endif

#ifdef __linux
void GetLogicalDriveStrings(size_t maxlen, char *buffer)
{
	int		rc	= 0;
	FILE	*fp = NULL;
	char	line[512];
	char	name[256];
	char	type[256];
	char	blocks[256];
	char	used[256];
	char	available[256];
	char  	pct[256];
	char	mount[256];
	size_t  len = 0;
	char	*ptr = buffer;

	memset(buffer, 0, maxlen);
	fp = popen("df -T 2>/dev/null", "r");
	if (fp != NULL) {
    	while(fgets(line, sizeof(line), fp))  {
    		if (*line == '/') {
            	rc = sscanf(line, "%s %s %s %s %s %s %s ",
            			name, type, blocks, used, available, pct, mount);
            	if (rc > 0) {
            		if (len + strlen(mount) < maxlen) {
            			strcat(ptr, mount);
            			ptr = ptr + strlen(mount) + 1;
            			len = len + strlen(mount) + 1;
            		}
            	}

    		}
        }
		pclose(fp);
	}
}
#endif
