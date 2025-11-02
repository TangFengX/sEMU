
#include <stdio.h>
#include "main.h"
#include <stdint.h>
#include <string.h>

#define DBG_DEFINE_GLOBALS
#include "dbg.h"

int DBG_GLOBAL_LEVEL = DBG_LEVEL_DEBUG;          
int DBG_ENABLE_COLOR = 1;                        
int DBG_THREAD_SAFE  = 0;                        
FILE *DBG_OUTPUT     = NULL;                     
const char *dbg_log_format = "%F:%l %M\n"; 

typedef enum{
    MODE_ASSEMBLE_FILE,
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
    char *output_file_gpr_img;
    char *log_file;
    char *report_file;
    uint32_t target_cycles;
    int verbose;
    int log;
    int report;
}ProgramArgs;

#define TARGET_CYCLES_DEFAULT 10
#define IMG_FILE_NAME_DEFAULT "img"
#define _str(x) #x


#define DBG_DEFINE_GLOBALS




int main(int argc, char *argv[]) {
    char* current_arg=NULL;
    ProgramArgs args={
        .mode = MODE_ERROR,
        .input_file = NULL,
        .file_name = NULL,
        .output_file_assemble = NULL,
        .output_file_analyze = NULL,
        .output_file_hex = NULL,
        .img_file_name = IMG_FILE_NAME_DEFAULT,
        .output_file_mem_img = NULL,
        .output_file_gpr_img = NULL,
        .log_file = NULL,
        .report_file = NULL,
        .target_cycles = TARGET_CYCLES_DEFAULT,
        .log = 0,
        .report = 0,
        .verbose = 0
    };
    log_info("Get %d arguments",argc);

    for(int i=0;i<argc;i++){
        current_arg=argv[i];
        if(strcmp(current_arg,"-a")==0||strcmp(current_arg, "--assemble") == 0){
            args.mode=MODE_ASSEMBLE_FILE;
        }
        if(strcmp(current_arg,"-l")==0||strcmp(current_arg, "--analyze") == 0){
            args.mode=MODE_ANALZE_FILE;
        }
        if(strcmp(current_arg,"-x")==0||strcmp(current_arg, "--hex") == 0){
            args.mode=MODE_HEX_FILE;
        }
        if(strcmp(current_arg,"-r")==0||strcmp(current_arg, "--run") == 0){
            args.mode=MODE_RUN;
        }
        if(strcmp(current_arg,"-i")==0||strcmp(current_arg, "--input") == 0){
            if(i+1<argc){
                args.input_file=argv[i+1];

                char*name_buffer=(char*)malloc(strlen(args.input_file)+1);
                check_mem(name_buffer);

                get_file_names(args.input_file,name_buffer,strlen(args.input_file)+1);
                args.file_name=(char*)malloc(strlen(name_buffer)+1);
                check_mem(args.file_name);

                strncpy(args.file_name,name_buffer,strlen(name_buffer)+1);
                free(name_buffer);
                log_info("Input file:%s",args.input_file);
                log_info("File name:%s",args.file_name);
            }else{
                log_err("No input file specified after %s",current_arg);
                return -1;
            }
        }
        if(strcmp(current_arg,"-v")==0||strcmp(current_arg, "--verbose") == 0){
            args.verbose=1;
            log_info("Verbose mode enabled");
        }
        if(strcmp(current_arg,"--log") == 0){
            args.log=1;
            log_info("Log enabled");
        }
        if(strcmp(current_arg,"--report") == 0){
            args.report=1;
            log_info("Report enabled");
        }
        if(strcmp(current_arg,"-c")==0||strcmp(current_arg, "--cycles") == 0){
            if(i+1<argc){
                args.target_cycles= (uint32_t) strtoul(argv[i+1], NULL, 10);
                log_info("Target cycles:%u",args.target_cycles);
            }else{
                log_err("No target cycles specified after %s",current_arg);
                return -1;
            }
        }
        if(strcmp(current_arg,"--img") == 0){
            if(i+1<argc){
                args.img_file_name=argv[i+1];
                log_info("Image file name:%s",args.img_file_name);
            }else{
                log_err("No image file name specified after %s",current_arg);
                return -1;
            }
        }

        log_info("Current mode:%s",_str(args.mode));
    
    }

    DBG_GLOBAL_LEVEL=(args.verbose)?DBG_LEVEL_DEBUG:DBG_LEVEL_NONE;
    const char dbg_log_format = "%F:%l %M\n";
    


    
    error:
        return 0;


    return 0;
}
