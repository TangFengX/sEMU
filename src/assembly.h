#ifndef __ASSEMBLY_H__
#define __ASSEMBLY_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "main.h"
#define MAX_BUFFER_SIZE 256
#define HEX_CODE_SIZE 8
#define HEX_PER_LINE 8
#define ASSEMBLE_LINE_SIZE 64

void process_assembly_file(const char *file_assembly, const char *file_analyze, const char *file_hex);

void process_hex_file(const char *file_hex, const char *file_assembly, const char *file_analyze);

void process_analyze_file(const char *file_analyze, const char *file_assembly, const char *file_hex);

void analyze_file_to_hex_file(const char *file_analyze, const char *file_hex);

void hex_file_to_analyze_file(const char *file_hex, const char *file_analyze);

void analyze_file_to_assembly_file(const char *file_analyze, const char *file_assembly);

void assembly_file_to_analyze_file(const char *file_assembly, const char *file_analyze);

void compile_assemble_code_line_to_hex_code(const char *assemble_line, char *hex_code);

void decompile_hex_code_to_assemble_code_line(const char *hex_code, char *assemble_line);

typedef struct HexCodeList HexCodeList;

struct HexCodeList
{
    char hex_code[HEX_CODE_SIZE + 1];
    int index;
    HexCodeList *next;
} ;

typedef struct AnalyzeLineList AnalyzeLineList;
struct AnalyzeLineList
{
    char analyze_line[ASSEMBLE_LINE_SIZE + 1];
    int index;
    AnalyzeLineList *next;
} ;

typedef struct AssembleLineList AssembleLineList;
struct AssembleLineList
{
    char assemble_line[MAX_BUFFER_SIZE];
    int index;
    AssembleLineList *next;
} ;

typedef struct
{
    const char *name;
    int id;
} Reg;
static const Reg reg_table[] = {
    {"zero", 0},
    {"ra", 1},
    {"sp", 2},
    {"gp", 3},
    {"tp", 4},
    {"t0", 5},
    {"t1", 6},
    {"t2", 7},
    {"s0", 8},
    {"fp", 8},
    {"s1", 9},
    {"a0", 10},
    {"a1", 11},
    {"a2", 12},
    {"a3", 13},
    {"a4", 14},
    {"a5", 15},
    {"a6", 16},
    {"a7", 17},
    {"s2", 18},
    {"s3", 19},
    {"s4", 20},
    {"s5", 21},
    {"s6", 22},
    {"s7", 23},
    {"s8", 24},
    {"s9", 25},
    {"s10", 26},
    {"s11", 27},
    {"t3", 28},
    {"t4", 29},
    {"t5", 30},
    {"t6", 31},
};

static char *return_pointer_next_to_Nth_tab(char *line_ptr, int n)
{
    char *current_ptr = line_ptr;
    for (int i = 0; i < n; i++)
    {
        current_ptr = strchr(current_ptr, '\t');
        debug("Tab %d found at position: %ld", i + 1, current_ptr ? (long)(current_ptr - line_ptr) : -1);
        if (current_ptr == NULL)
        {
            return NULL;
        }
        current_ptr = current_ptr + 1;
    }
    return current_ptr;
}

char *return_pointer_next_to_the_next_space(char *line_ptr)
{
    char *p = line_ptr;

    
    while (*p == ' ' || *p == '\t')
        p++;

    
    while (*p != '\0' && *p != ' ' && *p != '\t')
        p++;

    while (*p == ' ' || *p == '\t')
        p++;

    
    if (*p == '\0')
        return NULL;

    
    return p;
}




static int reg_from_name(const char *name)
{
    int i;
    for (i = 0; i < (int)(sizeof(reg_table) / sizeof(reg_table[0])); ++i)
        if (strcmp(name, reg_table[i].name) == 0)
            return reg_table[i].id;
    if (name[0] == 'x')
    {
        int name_id = atoi(name + 1);
        debug("Register name %s parsed as x%d", name, name_id);
        return name_id;
    }
    log_err_goto("Unknown register name: %s", name);
error:
    return -1;
}

static const char *reg_to_name(int id)
{
    int i;
    for (i = 0; i < (int)(sizeof(reg_table) / sizeof(reg_table[0])); ++i)
        if (reg_table[i].id == id)
        {
            debug("Register id %d converted to name %s", id, reg_table[i].name);
            return reg_table[i].name;
        }
    static char buf[5] = "err";
    log_err("Unknown register id: %d", id);
    return buf;
}

// 去除字符串前后空格
static void trim(char *s)
{
    char *p = s;
    while (isspace((unsigned char)*p))
        p++;
    memmove(s, p, strlen(p) + 1);
    int i;
    for (i = strlen(s) - 1; i >= 0 && isspace((unsigned char)s[i]); --i)
        s[i] = '\0';
}

static int parse_imm(const char *s)
{
    while (isspace((unsigned char)*s))
        s++;

    // -0x10 / +0x10
    if ((s[0] == '-' || s[0] == '+') && s[1] == '0' &&
        (s[2] == 'x' || s[2] == 'X'))
    {
        long val = strtol(s + 1, NULL, 16);
        return (s[0] == '-') ? -(int)val : (int)val;
    }
    // 普通 0x10
    if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
        return (int)strtol(s, NULL, 16);
    // 十进制
    return atoi(s);
}


#endif