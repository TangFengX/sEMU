#include "assembly.h"
#include "dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_assembly_file(const char *file_assembly, const char *file_analyze, const char *file_hex)
{
    assembly_file_to_analyze_file(file_assembly, file_analyze);
    analyze_file_to_hex_file(file_analyze, file_hex);
}

void process_hex_file(const char *file_hex, const char *file_assembly, const char *file_analyze)
{
    hex_file_to_analyze_file(file_hex, file_analyze);
    analyze_file_to_assembly_file(file_analyze, file_assembly);
}

void process_analyze_file(const char *file_analyze, const char *file_assembly, const char *file_hex)
{
    analyze_file_to_assembly_file(file_analyze, file_assembly);
    analyze_file_to_hex_file(file_analyze, file_hex);
}

void analyze_file_to_hex_file(const char *file_analyze, const char *file_hex)
{
    FILE *analyze_fp = fopen(file_analyze, "r");

    char line[MAX_BUFFER_SIZE];
    char hex_code[HEX_CODE_SIZE];

    HexCodeList *hex_list_head = NULL, *hex_list_tail = NULL;

    hex_list_head = (HexCodeList *)malloc(sizeof(HexCodeList));
    check_mem(hex_list_head);
    hex_list_head->next = NULL;

    hex_list_tail = hex_list_head;

    
    check_mem(analyze_fp);

    log_info("Reading analyze file: %s", file_base_name(file_analyze));

    int line_count = 0;      // 提取的编码行号从0开始
    int file_line_index = 0; // 文件行号从1开始

    while (fgets(line, sizeof(line), analyze_fp) != NULL)
    {
        file_line_index++;

        line[strcspn(line, "\r\n")] = '\0';

        debug("Line %d : %s", line_count, line);

        char *begin_of_hex = NULL;
        HexCodeList *new_hex_node = NULL;

        begin_of_hex = return_pointer_next_to_Nth_tab(line, 1);
        if (begin_of_hex == NULL)
        {
            log_info("No hex code found in line %d ", file_line_index);
            continue;
        }

        while (1)
        {
            if (*begin_of_hex == '\0')
            {
                log_info("No hex code found in line %d ", file_line_index);
                break;
            }
            else if (*begin_of_hex == ':' || *begin_of_hex == '\t' || *begin_of_hex == ' ')
            {
                begin_of_hex++;
            }
            else
            {
                new_hex_node = (HexCodeList *)malloc(sizeof(HexCodeList));
                check_mem(new_hex_node);
                
                strncpy(new_hex_node->hex_code, begin_of_hex, HEX_CODE_SIZE);

                new_hex_node->index = line_count;
                new_hex_node->hex_code[HEX_CODE_SIZE] = '\0';
                debug("Hex Code %d: %s", line_count, new_hex_node->hex_code);

                line_count++;
                new_hex_node->next = NULL;
                hex_list_tail->next = new_hex_node;
                hex_list_tail = new_hex_node;
                break;
            }
        }
    }
    fclose(analyze_fp);
    log_info("Analyze file %s read successfully.", file_base_name(file_analyze));
    int index_length = 1;
    int max_index = 16;

    while (1)
    {
        if (max_index >= line_count)
        {
            break;
        }
        else
        {
            index_length++;
            max_index *= 16;
        }
    }
    FILE *assemble_fp = fopen(file_hex, "w");
    check_mem(assemble_fp);
    log_info("Writing hex file: %s", file_base_name(file_hex));
    HexCodeList *current_hex_node = hex_list_head->next;

    fprintf(assemble_fp, "v3.0 hex words addressed\n");

    int current_line_index_hex_file = 0;

    while (current_hex_node != NULL)
    {

        fprintf(assemble_fp, "%0*x:", index_length, current_line_index_hex_file);
        for (int i = 0; i < HEX_PER_LINE && current_hex_node != NULL; i++)
        {
            fprintf(assemble_fp, " %s", current_hex_node->hex_code);
            current_line_index_hex_file++;
            current_hex_node = current_hex_node->next;
        }
        fprintf(assemble_fp, "\n");
    }
    fclose(assemble_fp);
    log_info("Hex file %s written successfully.", file_base_name(file_hex));

error:
    while (hex_list_head != NULL)
    {
        HexCodeList *temp = hex_list_head;
        hex_list_head = hex_list_head->next;
        free(temp);
    }
    return;
}

void analyze_file_to_assembly_file(const char *file_analyze, const char *file_assembly)
{
    FILE *analyze_fp = fopen(file_analyze, "r");

    char line[MAX_BUFFER_SIZE];
    char assemble_line[ASSEMBLE_LINE_SIZE];

    AssembleLineList *assemble_list_head = NULL, *assemble_list_tail = NULL;

    assemble_list_head = (AssembleLineList *)malloc(sizeof(AssembleLineList));
    check_mem(assemble_list_head);
    assemble_list_head->next = NULL;
    assemble_list_tail = assemble_list_head;

    
    check_mem(analyze_fp);

    log_info("Reading analyze file: %s", file_base_name(file_analyze));

    int line_count = 0;
    int file_line_index = 0;

    while (fgets(line, sizeof(line), analyze_fp) != NULL)
    {
        file_line_index++;
        

        line[strcspn(line, "\r\n")] = '\0';

        debug("Line %d : %s", line_count, line);

        char *begin_of_assemble = NULL;
        AssembleLineList *new_assemble_node = NULL;

        begin_of_assemble = return_pointer_next_to_Nth_tab(line, 2);
        if (begin_of_assemble == NULL)
        {
            log_info("No assemble code found in line %d ", file_line_index);
            continue;
        }

        while (1)
        {
            if (*begin_of_assemble == '\0')
            {
                log_info("No assemble code found in line %d ", file_line_index);
                break;
            }
            else
            {
                new_assemble_node = (AssembleLineList *)malloc(sizeof(AssembleLineList));
                check_mem(new_assemble_node);

                begin_of_assemble[strcspn(begin_of_assemble, " #")] = '\0';
                begin_of_assemble[strcspn(begin_of_assemble, "\t")] = ' ';
                strncpy(new_assemble_node->assemble_line, begin_of_assemble, ASSEMBLE_LINE_SIZE);
                new_assemble_node->index = line_count;
                new_assemble_node->assemble_line[ASSEMBLE_LINE_SIZE] = '\0';

                debug("Assemble Line %d: %s", line_count, new_assemble_node->assemble_line);
                char* space_pos = strchr(new_assemble_node->assemble_line, ' ');
                if (space_pos != NULL) {
                    *space_pos = '\t';
                }
                

                line_count++;
                new_assemble_node->next = NULL;
                assemble_list_tail->next = new_assemble_node;
                assemble_list_tail = new_assemble_node;
                break;
            }
        }
    }
    fclose(analyze_fp);
    log_info("Analyze file %s read successfully.", file_base_name(file_analyze));
    FILE *assemble_fp = fopen(file_assembly, "w");
    check_mem(assemble_fp);
    log_info("Writing assemble file: %s", file_base_name(file_assembly));
    AssembleLineList *current_assemble_node = assemble_list_head->next;

    while (current_assemble_node != NULL)
    {
        fprintf(assemble_fp, "%s\n", current_assemble_node->assemble_line);
        //debug("Writing assemble line %d: %s", current_assemble_node->index, current_assemble_node->assemble_line);
        current_assemble_node = current_assemble_node->next;
    }
    fclose(assemble_fp);
    log_info("Assemble file %s written successfully.", file_base_name(file_assembly));
error:
    while (assemble_list_head != NULL)
    {
        AssembleLineList *temp = assemble_list_head;
        assemble_list_head = assemble_list_head->next;
        free(temp);
    }
}

void compile_assemble_code_line_to_hex_code(const char *assemble_line, char *hex_code)
{
    char op[16] = {0}, a1[16] = {0}, a2[16] = {0}, a3[16] = {0};
    unsigned opcode = 0, funct3 = 0, funct7 = 0;
    int rd = 0, rs1 = 0, rs2 = 0, imm = 0;
    char line[ASSEMBLE_LINE_SIZE + 1];
    strncpy(line, assemble_line, ASSEMBLE_LINE_SIZE);
    line[ASSEMBLE_LINE_SIZE] = '\0';

    int i;
    for (i = 0; line[i]; ++i)
        if (line[i] == ',' || line[i] == '(' || line[i] == ')')
            line[i] = ' ';

    sscanf(line, "%15s %15s %15s %15s", op, a1, a2, a3);
    trim(op);
    trim(a1);
    trim(a2);
    trim(a3);

    if (strcmp(op, "lui") == 0)
    {
        rd = reg_from_name(a1);
        imm = parse_imm(a2);
        opcode = 0b0110111;
        unsigned inst = ((imm & 0xFFFFF) << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
        log_info("%s -> %s", assemble_line, hex_code);
    }
    else if (strcmp(op, "addi") == 0)
    {
        rd = reg_from_name(a1);
        rs1 = reg_from_name(a2);
        imm = parse_imm(a3);
        opcode = 0b0010011;
        funct3 = 0b000;
        unsigned inst = ((imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
        log_info("%s -> %s", assemble_line, hex_code);
    }
    else if (strcmp(op, "add") == 0)
    {
        rd = reg_from_name(a1);
        rs1 = reg_from_name(a2);
        rs2 = reg_from_name(a3);
        opcode = 0b0110011;
        funct3 = 0b000;
        funct7 = 0b0000000;
        unsigned inst = (funct7 << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else if (strcmp(op, "lw") == 0)
    {
        rd = reg_from_name(a1);
        imm = parse_imm(a2);
        rs1 = reg_from_name(a3);
        opcode = 0b0000011;
        funct3 = 0b010;
        unsigned inst = ((imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else if (strcmp(op, "lbu") == 0)
    {
        rd = reg_from_name(a1);
        imm = parse_imm(a2);
        rs1 = reg_from_name(a3);
        opcode = 0b0000011;
        funct3 = 0b100;
        unsigned inst = ((imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else if (strcmp(op, "sw") == 0)
    {
        rs2 = reg_from_name(a1);
        imm = parse_imm(a2);
        rs1 = reg_from_name(a3);
        opcode = 0b0100011;
        funct3 = 0b010;
        unsigned immhi = (imm >> 5) & 0x7F, immlo = imm & 0x1F;
        unsigned inst = (immhi << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (immlo << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else if (strcmp(op, "sb") == 0)
    {
        rs2 = reg_from_name(a1);
        imm = parse_imm(a2);
        rs1 = reg_from_name(a3);
        opcode = 0b0100011;
        funct3 = 0b000;
        unsigned immhi = (imm >> 5) & 0x7F, immlo = imm & 0x1F;
        unsigned inst = (immhi << 25) | (rs2 << 20) | (rs1 << 15) | (funct3 << 12) | (immlo << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else if (strcmp(op, "jalr") == 0)
    {
        rd = reg_from_name(a1);
        imm = parse_imm(a2);
        rs1 = reg_from_name(a3);
        opcode = 0b1100111;
        funct3 = 0b000;
        unsigned inst = ((imm & 0xFFF) << 20) | (rs1 << 15) | (funct3 << 12) | (rd << 7) | opcode;
        sprintf(hex_code, "%08x", inst);
    }
    else
    {
        log_err_goto("Unknown assemble operation: %s", assemble_line);
        strcpy(hex_code, "00000000");
    }
    log_info("%s -> %s", assemble_line, hex_code);
error:
    return;
}

// ------------------- HEX -> 汇编 -------------------
void decompile_hex_code_to_assemble_code_line(const char *hex_code, char *assemble_line)
{
    unsigned inst = (unsigned)strtoul(hex_code, NULL, 16);
    unsigned opcode = inst & 0x7F;
    unsigned rd = (inst >> 7) & 0x1F;
    unsigned funct3 = (inst >> 12) & 0x7;
    unsigned rs1 = (inst >> 15) & 0x1F;
    unsigned rs2 = (inst >> 20) & 0x1F;
    //unsigned funct7 = (inst >> 25) & 0x7F;
    int imm = 0;

    switch (opcode)
    {
    case 0b0110111: // LUI
        imm = (int)(inst >> 12) << 12;
        sprintf(assemble_line, "lui\t%s,0x%x", reg_to_name(rd), imm >> 12);
        log_info("%s -> %s", hex_code, assemble_line);
        break;
    case 0b0010011: // ADDI
        imm = (int)(inst) >> 20;
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        sprintf(assemble_line, "addi\t%s,%s,%d", reg_to_name(rd), reg_to_name(rs1), imm);
        log_info("%s -> %s", hex_code, assemble_line);
        break;
    case 0b0110011: // ADD
        sprintf(assemble_line, "add\t%s,%s,%s", reg_to_name(rd), reg_to_name(rs1), reg_to_name(rs2));
        log_info("%s -> %s", hex_code, assemble_line);
        break;
    case 0b0000011: // LW/LBU
        imm = (int)(inst) >> 20;
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        if (funct3 == 0b010)
        {
            sprintf(assemble_line, "lw\t%s,%d(%s)", reg_to_name(rd), imm, reg_to_name(rs1));
            log_info("%s -> %s", hex_code, assemble_line);
        }
        else if (funct3 == 0b100)
        {
            sprintf(assemble_line, "lbu\t%s,%d(%s)", reg_to_name(rd), imm, reg_to_name(rs1));
            log_info("%s -> %s", hex_code, assemble_line);
        }
        else
        {
            strcpy(assemble_line, "unknown");
            log_err_goto("Unknown funct3 for opcode 0000011: %d", funct3);
        }
        break;
    case 0b0100011: // SW/SB
        imm = ((inst >> 25) << 5) | ((inst >> 7) & 0x1F);
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        if (funct3 == 0b010)
        {
            sprintf(assemble_line, "sw\t%s,%d(%s)", reg_to_name(rs2), imm, reg_to_name(rs1));
            log_info("%s -> %s", hex_code, assemble_line);
        }

        else if (funct3 == 0b000)
        {
            sprintf(assemble_line, "sb\t%s,%d(%s)", reg_to_name(rs2), imm, reg_to_name(rs1));
            log_info("%s -> %s", hex_code, assemble_line);
        }
        else
        {
            strcpy(assemble_line, "unknown");
            log_err_goto("Unknown funct3 for opcode 0100011: %d", funct3);
        }
        break;
    case 0b1100111: // JALR
        imm = (int)(inst) >> 20;
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        sprintf(assemble_line, "jalr\t%s,%d(%s)", reg_to_name(rd), imm, reg_to_name(rs1));
        log_info("%s -> %s", hex_code, assemble_line);
        break;
    default:
        sprintf(assemble_line, "unknown");
        log_err_goto("Unknown opcode: %d", opcode);
        break;
    }
    error:
    return;
}

void hex_file_to_analyze_file(const char *file_hex, const char *file_analyze)
{
    FILE *hex_fp = fopen(file_hex, "r");
    char line[MAX_BUFFER_SIZE];
    char hex_line[HEX_CODE_SIZE + 1];
    char assemble_line[ASSEMBLE_LINE_SIZE + 1];
    AnalyzeLineList *analyze_list_head = NULL, *analyze_list_tail = NULL;
    analyze_list_head = (AnalyzeLineList *)malloc(sizeof(AnalyzeLineList));
    check_mem(analyze_list_head);
    analyze_list_head->next = NULL;
    analyze_list_tail = analyze_list_head;
    
    check_mem(hex_fp);
    log_info("Reading hex file: %s", file_base_name(file_hex));
    int line_count = 0;      // 提取的编码行号从0开始
    int file_line_index = 0; // 文件行号从1开始
    while (fgets(line, sizeof(line), hex_fp) != NULL)
    {
        file_line_index++;
        
        line[strcspn(line, "\r\n")] = '\0';
        debug("Line %d : %s", file_line_index, line);
        //char *begin_of_hex = NULL;
        
        char *hex_begin = return_pointer_next_to_the_next_space(line);
        while (1)
        {
            if (hex_begin == NULL)
            {
                break;
            }
            else
            {
                strncpy(hex_line, hex_begin, HEX_CODE_SIZE);
                hex_line[HEX_CODE_SIZE] = '\0';
                debug("Hex Code extracted: %s", hex_line);
                decompile_hex_code_to_assemble_code_line(hex_line, assemble_line);
                if (strcmp(assemble_line, "unknown") == 0)
                {
                    log_err("Failed to decompile hex code %s at line %d", hex_line, file_line_index);
                    break;
                }
                AnalyzeLineList *new_analyze_node = (AnalyzeLineList *)malloc(sizeof(AnalyzeLineList));
                check_mem(new_analyze_node);
                
                
                snprintf(new_analyze_node->analyze_line, MAX_BUFFER_SIZE, "%8x:\t%s          \t%s", line_count, hex_line, assemble_line);
                line_count += 4;
                analyze_list_tail->next = new_analyze_node;
                analyze_list_tail = new_analyze_node;
                hex_begin = return_pointer_next_to_the_next_space(hex_begin);
            }
        }
    }
    fclose(hex_fp);
    log_info("Hex file %s read successfully.", file_base_name(file_hex));
    FILE *analyze_fp = fopen(file_analyze, "w");
    check_mem(analyze_fp);
    log_info("Writing analyze file: %s", file_base_name(file_analyze));
    AnalyzeLineList *current_analyze_node = analyze_list_head->next;
    while (current_analyze_node != NULL)
    {
        fprintf(analyze_fp, "%s\n", current_analyze_node->analyze_line);
        current_analyze_node = current_analyze_node->next;
    }
    fclose(analyze_fp);
    log_info("Analyze file %s written successfully.", file_base_name(file_analyze));
error:
    while (analyze_list_head != NULL)
    {
        AnalyzeLineList *temp = analyze_list_head;
        analyze_list_head = analyze_list_head->next;
        free(temp);
    }
}

void assembly_file_to_analyze_file(const char *file_assembly, const char *file_analyze)
{
    FILE *assemble_fp = fopen(file_assembly, "r");
    char line[ASSEMBLE_LINE_SIZE + 1];
    char hex_code[HEX_CODE_SIZE + 1];
    char assemble_line[ASSEMBLE_LINE_SIZE + 1];
    AnalyzeLineList *analyze_list_head = NULL, *analyze_list_tail = NULL;
    analyze_list_head = (AnalyzeLineList *)malloc(sizeof(AnalyzeLineList));
    check_mem(analyze_list_head);
    analyze_list_head->next = NULL;

    analyze_list_tail = analyze_list_head;

    check_mem(assemble_fp);
    log_info("Reading assemble file: %s", file_base_name(file_assembly));
    int line_count = 0;      // 提取的编码行号从0开始
    int file_line_index = 0; // 文件行号从1开始
    while (fgets(line, sizeof(line), assemble_fp) != NULL)
    {
        file_line_index++;
        
        line[strcspn(line, "\r\n")] = '\0';
        debug("Line %d : %s", file_line_index, line);
        compile_assemble_code_line_to_hex_code(line, hex_code);
        if (strcmp(hex_code, "00000000") == 0)
        {
            log_err_goto("Failed to compile assemble line at line %d: %s", file_line_index, line);
        }
        AnalyzeLineList *new_analyze_node = (AnalyzeLineList *)malloc(sizeof(AnalyzeLineList));
        check_mem(new_analyze_node);
        char* space_pos = strchr(line, ' ');
        if (space_pos != NULL) {
            *space_pos = '\t';
        }
        snprintf(new_analyze_node->analyze_line, MAX_BUFFER_SIZE, "%8x:\t%8s          \t%s", line_count, hex_code, line);
        line_count += 4;
        new_analyze_node->next = NULL;
        analyze_list_tail->next = new_analyze_node;
        analyze_list_tail = new_analyze_node;
    }
    fclose(assemble_fp);
    log_info("Assemble file %s read successfully.", file_base_name(file_assembly));
    FILE *analyze_fp = fopen(file_analyze, "w");
    check_mem(analyze_fp);
    log_info("Writing analyze file: %s", file_base_name(file_analyze));
    AnalyzeLineList *current_analyze_node = analyze_list_head->next;
    while (current_analyze_node != NULL)
    {
        fprintf(analyze_fp, "%s\n", current_analyze_node->analyze_line);
        current_analyze_node = current_analyze_node->next;
    }
    fclose(analyze_fp);
    log_info("Analyze file %s written successfully.", file_base_name(file_analyze));
error:
    while (analyze_list_head != NULL)
    {
        AnalyzeLineList *temp = analyze_list_head;
        analyze_list_head = analyze_list_head->next;
        free(temp);
    }
}
