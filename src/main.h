#ifndef __MAIN_H__
#define __MAIN_H__
#include <stdint.h>
#define DBG_DEFINE_GLOBALS
extern int DBG_GLOBAL_LEVEL;
extern int DBG_ENABLE_COLOR;
extern int DBG_THREAD_SAFE;
extern FILE *DBG_OUTPUT;
extern const char *dbg_log_format;


#include "dbg.h"

#define TARGET_CYCLES_DEFAULT 7000
#define IMG_FILE_NAME_DEFAULT "img"
#define _str(x) #x


#define DBG_DEFINE_GLOBALS






typedef enum{
    MODE_ASSEMBLY_FILE,
    MODE_ANALZE_FILE,
    MODE_HEX_FILE,
    MODE_RUN,
    MODE_ERROR
} ProgramMode;

typedef struct{
    ProgramMode mode;
    char *input_file;
    char *file_name;
    char *output_file_assemble;
    char *output_file_analyze;
    char *output_file_hex;
    char *img_file_name;
    char *output_file_mem_img;
    char *log_file;
    char *report_file;
    int target_cycles;
    int verbose;
    int log;
    int report;
}ProgramArgs;

void get_file_names(const char* input_file,char* base_name_buffer,size_t buffer_size);
void create_file_if_not_exists(const char* file_name);



#endif // !__MAIN_H__