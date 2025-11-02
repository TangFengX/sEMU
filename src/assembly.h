
#include <stdint.h>
#define MAX_BUFFER_SIZE 1024
#define HEX_CODE_SIZE 8
#define HEX_PER_LINE 8
#define ASSEMBLE_LINE_SIZE 64

void compile_source_file(const char* file_assemble, const char* file_analyze, const char* file_hex) ;

void decompile_hex_file(const char* file_hex, const char* file_assemble, const char* file_analyze) ;

void deassemble_analyze_file(const char* file_analyze, const char* file_assemble,const char* file_hex) ;

void analyze_file_to_hex_file(const char* file_analyze, const char* file_hex) ;

void hex_file_to_analyze_file(const char* file_hex, const char* file_analyze) ;

void analyze_file_to_assemble_file(const char* file_analyze, const char* file_assemble) ;

void assemble_file_to_analyze_file(const char* file_assemble, const char* file_analyze) ;

char* return_pointer_next_to_Nth_tab(char* line_ptr, int n);

typedef struct {
    char hex_code[HEX_CODE_SIZE+1];
    uint32_t index;
    HexCodeList* next;
}HexCodeList;

typedef struct {
    char assemble_line[ASSEMBLE_LINE_SIZE+1];
    uint32_t index;
    AssembleLineList* next;
}AssembleLineList;
