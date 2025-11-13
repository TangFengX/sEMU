
#include <stdio.h>
#include "main.h"
#include <stdint.h>
#include <string.h>
#include "assembly.h"
#include "emu.h"

#include "dbg.h"


int DBG_GLOBAL_LEVEL = DBG_LEVEL_DEBUG;          
int DBG_ENABLE_COLOR = 1;                        
int DBG_THREAD_SAFE  = 0;                        
FILE *DBG_OUTPUT     = NULL;                     
const char *dbg_log_format = "%F:%l %M\n"; 

int main(int argc, char *argv[])
{
    
    char *current_arg = NULL;
    ProgramArgs args = {
        .mode = MODE_ERROR,
        .input_file = NULL,
        .file_name = NULL, // base name of input file
        .output_file_assemble = NULL,
        .output_file_analyze = NULL,
        .output_file_hex = NULL,
        .img_file_name = IMG_FILE_NAME_DEFAULT,
        .output_file_mem_img = NULL,
        .log_file = NULL,
        .report_file = NULL,
        .target_cycles = TARGET_CYCLES_DEFAULT,
        .log = 0,
        .report = 0,
        .verbose = 0};
    log_info("Get %d arguments", argc);

    for (int i = 0; i < argc; i++)
    {
        current_arg = argv[i];
        if (strcmp(current_arg, "-a") == 0 || strcmp(current_arg, "--assemble") == 0)
        {
            args.mode = MODE_ASSEMBLY_FILE;
            log_info("Mode set to Assembly File Processing");
        }
        if (strcmp(current_arg, "-l") == 0 || strcmp(current_arg, "--analyze") == 0)
        {
            args.mode = MODE_ANALZE_FILE;
            log_info("Mode set to Analyze File Processing");
        }
        if (strcmp(current_arg, "-x") == 0 || strcmp(current_arg, "--hex") == 0)
        {
            args.mode = MODE_HEX_FILE;
            log_info("Mode set to Hex File Processing");
        }
        if (strcmp(current_arg, "-r") == 0 || strcmp(current_arg, "--run") == 0)
        {
            args.mode = MODE_RUN;
            log_info("Mode set to Simulator Run");
        }
        if (strcmp(current_arg, "-i") == 0 || strcmp(current_arg, "--input") == 0)
        {
            if (i + 1 < argc)
            {
                args.input_file = argv[i + 1];

                char *name_buffer = (char *)malloc(strlen(args.input_file) + 1);
                check_mem(name_buffer);

                get_file_names(args.input_file, name_buffer, strlen(args.input_file) + 1);
                args.file_name = name_buffer;

                log_info("Input file:%s", args.input_file);
                log_info("File name:%s", args.file_name);
            }
            else
            {
                log_err("No input file specified after %s", current_arg);
                return -1;
            }
        }
        if (strcmp(current_arg, "-v") == 0 || strcmp(current_arg, "--verbose") == 0)
        {
            args.verbose = 1;
            log_info("Verbose mode enabled");
        }
        if (strcmp(current_arg, "--log") == 0)
        {
            args.log = 1;
            log_info("Log enabled");
        }
        if (strcmp(current_arg, "--report") == 0)
        {
            args.report = 1;
            log_info("Report enabled");
        }
        if (strcmp(current_arg, "-c") == 0 || strcmp(current_arg, "--cycles") == 0)
        {
            if (i + 1 < argc)
            {
                args.target_cycles = (uint32_t)strtoul(argv[i + 1], NULL, 10);
                log_info("Target cycles:%u", args.target_cycles);
            }
            else
            {
                log_err("No target cycles specified after %s", current_arg);
                return -1;
            }
        }
        if (strcmp(current_arg, "--img") == 0)
        {
            if (i + 1 < argc)
            {
                args.img_file_name = argv[i + 1];
                log_info("Image file name:%s", args.img_file_name);
            }
            else
            {
                log_err("No image file name specified after %s", current_arg);
                return -1;
            }
        }

    }
    DBG_GLOBAL_LEVEL = (args.verbose) ? DBG_LEVEL_DEBUG : DBG_LEVEL_NONE;

    

    if (args.input_file == NULL)
    {
        log_err("No input file specified. Please use -i to specify the input file.");
        return -1;
    }

    const char* dbg_log_format = "%F:%l %M\n";
    args.output_file_assemble = (char *)malloc(strlen(args.file_name) + 5);
    args.output_file_analyze = (char *)malloc(strlen(args.file_name) + 5);
    args.output_file_hex = (char *)malloc(strlen(args.file_name) + 5);
    check_mem(args.output_file_assemble);
    check_mem(args.output_file_analyze);
    check_mem(args.output_file_hex);
    sprintf(args.output_file_assemble, "%s.asmtxt", args.file_name);
    sprintf(args.output_file_analyze, "%s.txt", args.file_name);
    sprintf(args.output_file_hex, "%s.hextxt", args.file_name);

    switch (args.mode)
    {
    case MODE_ASSEMBLY_FILE:
        create_file_if_not_exists(args.output_file_analyze);
        create_file_if_not_exists(args.output_file_hex);
        process_assembly_file(args.input_file, args.output_file_analyze, args.output_file_hex);
        break;
    case MODE_ANALZE_FILE:
        create_file_if_not_exists(args.output_file_assemble);
        create_file_if_not_exists(args.output_file_hex);
        process_analyze_file(args.input_file, args.output_file_assemble, args.output_file_hex);
        break;
    case MODE_HEX_FILE:
        create_file_if_not_exists(args.output_file_assemble);
        create_file_if_not_exists(args.output_file_analyze);
        process_hex_file(args.input_file, args.output_file_assemble, args.output_file_analyze);
        break;
    case MODE_RUN:
        run_simulator(&args);
        break;
    case MODE_ERROR:
    default:
        log_err("No valid mode specified. Please use -a, -l, -x, or -r to specify the mode.");
        goto error;
    }

error:

    return 0;
}




void get_file_names(const char* input_file,char* base_name_buffer,size_t buffer_size){
    const char* last_dot=strrchr(input_file,'.')?strrchr(input_file,'.'):input_file+strlen(input_file);
    const char* begin_of_base_name=file_base_name(input_file);
    if(buffer_size<(size_t)(last_dot-begin_of_base_name+1)){
        debug("buffer_size:%zu,required size:%zu",buffer_size,(size_t)(last_dot-begin_of_base_name+1));
        debug("last_dot:%s",last_dot);
        debug("begin_of_base_name:%s",begin_of_base_name);
        log_err("Buffer size %zu is not enough for base name extraction",buffer_size);
        return;
    }
    strncpy(base_name_buffer,begin_of_base_name,last_dot-begin_of_base_name);
    debug("Base name extracted:%s",base_name_buffer);
    base_name_buffer[last_dot-begin_of_base_name]='\0';
}

void create_file_if_not_exists(const char* file_name){
    FILE* fp=fopen(file_name,"a+");
    if(fp==NULL){
        log_err("Failed to create file:%s",file_name);
        return;
    }
    fclose(fp);
}