#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include "emu.h"
#include "main.h"
#include "dbg.h"
#include "assembly.h"

// Set PC and counter to zeor.Malloc new scpu instance if scpu_inst is null.
scpu *scpu_init(scpu *scpu_inst)
{
    if (!scpu_inst)
    {
        scpu_inst = (scpu *)malloc(sizeof(scpu));
        check_mem(scpu_inst);
        scpu_inst->ram = (uint8_t *)malloc((1 << RAM_ADDRESS_BIT_WIDTH_DEFAULT) * (RAM_DATA_BIT_WIDTH_DEFAULT / 8) * sizeof(uint8_t));
        scpu_inst->gpr = (uint32_t *)malloc(REGISTER_COUNT_DEFAULT * sizeof(uint32_t));
        scpu_inst->rom = (uint8_t *)malloc((1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT) * (ROM_DATA_BIT_WIDTH_DEFAULT / 8) * sizeof(uint8_t));
        check_mem(scpu_inst->ram);
        check_mem(scpu_inst->gpr);
        check_mem(scpu_inst->rom);
    }
    for (int i = 0; i < REGISTER_COUNT_DEFAULT; i++)
    {
        scpu_inst->gpr[i] = 0;
    }

    for (int i = 0; i < (1 << RAM_ADDRESS_BIT_WIDTH_DEFAULT) * (RAM_DATA_BIT_WIDTH_DEFAULT / 8); i++)
    {
        scpu_inst->ram[i] = 0;
    }
    for (int i = 0; i < (1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT) * (ROM_DATA_BIT_WIDTH_DEFAULT / 8); i++)
    {
        scpu_inst->rom[i] = 0;
    }
    scpu_inst->gpr_changed = 0;
    scpu_inst->PC = 0;
    scpu_inst->counter = 0;
    return scpu_inst;
error:
    return NULL;
}

// Free scpu instance
void scpu_free(scpu *scpu_inst)
{
    if (scpu_inst)
    {
        free(scpu_inst->ram);
        free(scpu_inst->rom);
        free(scpu_inst->gpr);
        free(scpu_inst);
    }
}

// execute instruction
int execute_instruction(scpu *scpu_inst)
{
    uint32_t inst;
    memcpy(&inst, scpu_inst->rom + scpu_inst->PC, sizeof(uint32_t));
    uint32_t opcode = inst & 0x7F;
    uint32_t rd = (inst >> 7) & 0x1F;
    uint32_t funct3 = (inst >> 12) & 0x7;
    uint32_t rs1 = (inst >> 15) & 0x1F;
    uint32_t rs2 = (inst >> 20) & 0x1F;
    uint32_t imm = 0;

    switch (opcode)
    {
    case 0b0110111: // LUI

        imm = (uint32_t)(inst >> 12);
        debug("[PC:%x][COUNTER:%d]\tlui\t%s,0x%x", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm);
        if (rd != 0)
        {
            scpu_inst->gpr[rd] = imm << 12;
            scpu_inst->gpr_changed |= 1 << rd;
        }
        scpu_inst->PC += 4;
        break;
    case 0b0010011: // ADDI

        imm = (int)((inst) >> 20);
        if (imm & 0x800)
            imm |= 0xFFFFF000;

        debug("[PC:%x][COUNTER:%d]\taddi\t%s,%s,%d", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), reg_to_name(rs1), imm);
        if (rd != 0)
        {
            scpu_inst->gpr[rd] = scpu_inst->gpr[rs1] + imm;
            scpu_inst->gpr_changed |= 1 << rd;
        }
        scpu_inst->PC += 4;
        break;
    case 0b0110011: // ADD

        debug("[PC:%x][COUNTER:%d]\tadd\t%s,%s,%s", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), reg_to_name(rs1), reg_to_name(rs2));
        if (rd != 0)
        {
            scpu_inst->gpr[rd] = scpu_inst->gpr[rs1] + scpu_inst->gpr[rs2];
            scpu_inst->gpr_changed |= 1 << rd;
        }
        scpu_inst->PC += 4;
        break;
    case 0b0000011: // LW/LBU

        imm = (int)(inst) >> 20;
        if (imm & 0x800)
            imm |= 0xFFFFF000;

        if (funct3 == 0b010)
        {
            debug("[PC:%x][COUNTER:%d]\tlw\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 0x3)
            {
                log_err_goto("Address misaligned exception: 0x%x", addr32);
            }
            addrn=addr32_to_addrn(addr32,RAM);
            if(rd!=0){
                memcpy(&scpu_inst->gpr[rd], scpu_inst->ram + addrn, sizeof(uint32_t));
            scpu_inst->gpr_changed |= 1 << rd;
            debug("load %08x from %08x",scpu_inst->gpr[rd],addrn);
            
        }
            
        }

        else if (funct3 == 0b100)
        {
            debug("[PC:%x][COUNTER:%d]\tlbu\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            addrn=addr32_to_addrn(addr32,RAM);
            if(rd!=0){
                scpu_inst->gpr[rd] = (uint32_t)scpu_inst->ram[addrn];
                scpu_inst->gpr_changed |= 1 << rd;
                debug("load %08x from %08x",scpu_inst->gpr[rd],addrn);
            }
            
        }
        else
        {
            log_err_goto("Unknown funct3 for opcode 0000011: %d", funct3);
        }
        scpu_inst->PC += 4;
        break;
    case 0b0100011: // SW/SB

        imm = ((inst >> 25) << 5) | ((inst >> 7) & 0x1F);
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        if (funct3 == 0b010)
        {
            debug("[PC:%x][COUNTER:%d]\tsw\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rs2), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 0x3)
            {
                log_err_goto("Address misaligned exception: 0x%x", addr32);
            }
            addrn=addr32_to_addrn(addr32,RAM);
            memcpy(scpu_inst->ram + addrn, &scpu_inst->gpr[rs2], sizeof(uint32_t));
            debug("save %08x to %08x",scpu_inst->gpr[rs2],addrn);
        }

        else if (funct3 == 0b000)
        {
            debug("[PC:%x][COUNTER:%d]\tsb\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rs2), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            addrn=addr32_to_addrn(addr32,RAM);
            memcpy(scpu_inst->ram + addrn, &scpu_inst->gpr[rs2], sizeof(uint8_t));
            debug("save %08x to %08x",scpu_inst->gpr[rs2],addrn);
        }
        else
        {
            log_err_goto("Unknown funct3 for opcode 0100011: %d", funct3);
        }
        scpu_inst->PC += 4;
        break;
    case 0b1100111: // JALR
        imm = (int)(inst) >> 20;
        if (imm & 0x800)
            imm |= 0xFFFFF000;
        debug("[PC:%x][COUNTER:%d]\tjalr\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
        if(rd!=0){
            scpu_inst->gpr[rd] = scpu_inst->PC + 4;
            scpu_inst->gpr_changed |= 1 << rd;
        }
        
        scpu_inst->PC = scpu_inst->gpr[rs1] + imm;

        break;
    default:
        log_err_goto("Unknown opcode: %d", opcode);
        break;
    }
    scpu_inst->counter++;
    return 0;
error:
    return -1;
}

void run_simulator(const ProgramArgs *args)
{
    scpu *inst = scpu_init(NULL);
    check_mem(inst);
    load_instruction_to_rom(args->input_file, inst);
    int cycles = args->target_cycles, i = 0;
    int flag = 0;
    while (flag == 0 && i < cycles)
    {
        flag = execute_instruction(inst);
        i++;
    }
    save_img(inst, args, ROM);
    save_img(inst, args, RAM);
    save_img(inst, args, GPR);
    
error:
    scpu_free(inst);
    
}

void load_instruction_to_rom(const char *file_name, scpu *scpu_inst)
{
    uint32_t *buffer = (uint32_t *)malloc((1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT) * sizeof(uint32_t));
    FILE *hex_fp = fopen(file_name, "r");
    check_mem(hex_fp);
    check_mem(scpu_inst);
    log_info("Loading instructions from file: %s", file_name);
    int file_line_index = 0;
    char line[BUFFER_SIZE];
    char hex_line[HEX_CODE_SIZE + 1];
    uint32_t *rom_pointer = buffer;
    uint32_t tmp;
    while (fgets(line, sizeof(line), hex_fp) != NULL)
    {
        file_line_index++;
        line[strcspn(line, "\r\n")] = '\0';
        debug("Line %d in file : %s", file_line_index, line);
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
                tmp = hex_string_to_int(hex_line);
                if (tmp)
                {
                    *rom_pointer = tmp;
                    debug("write\t%08x : %08x\tto ROM", (rom_pointer - buffer) * 4, *rom_pointer);
                    rom_pointer++;
                    
                }
                hex_begin = return_pointer_next_to_the_next_space(hex_begin);
            }
        }
    }
    while (rom_pointer - buffer < 1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT)
    {
        *rom_pointer = 0;
        rom_pointer++;
    }
    if (scpu_inst)
    {
        memcpy(scpu_inst->rom, buffer, (1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT) * sizeof(uint32_t));
    }

error:
    free(buffer);
    fclose(hex_fp);

    return;
}

uint32_t hex_string_to_int(const char *hex_str)
{
    char *endptr;
    long int value_long;
    uint32_t value_int;

    value_long = strtol(hex_str, &endptr, 16);

    if ((errno == ERANGE && (value_long == LONG_MAX || value_long == LONG_MIN)) || (errno != 0 && value_long == 0))
    {
        log_err_goto("strtol 转换错误或溢出");
    }

    if (endptr == hex_str)
    {
        log_err_goto("错误: 未找到任何有效的数字\n");
    }

    if (*endptr != '\0')
    {
        log_err_goto("警告: 字符串中包含未转换的字符: %s\n", endptr);
    }

    value_int = (uint32_t)value_long;
    return value_int;
error:
    return 0;
}

void save_img(scpu *scpu_inst, const ProgramArgs *args, Storage obj)
{
    uint32_t *data;
    uint32_t n;
    char *output_file;
    FILE *output_file_fp;
    int index_length;
    char *obj_name;
    switch (obj)
    {
    case RAM:
        n = (1 << RAM_ADDRESS_BIT_WIDTH_DEFAULT) * (RAM_DATA_BIT_WIDTH_DEFAULT / 8)>>2;
        data = (uint32_t *)malloc(n * sizeof(uint32_t));
        memcpy(data, scpu_inst->ram, n * sizeof(uint32_t));
        sprintf(args->output_file_ram_img, "%s.ramimg", args->file_name);
        output_file = args->output_file_ram_img;
        index_length = (RAM_ADDRESS_BIT_WIDTH_DEFAULT - 1) / 4 + 1;
        obj_name = "RAM";
        break;
    case ROM:
        n = (1 << ROM_ADDRESS_BIT_WIDTH_DEFAULT) * (ROM_DATA_BIT_WIDTH_DEFAULT / 8)>>2;
        data = (uint32_t *)malloc(n * sizeof(uint32_t));
        memcpy(data, scpu_inst->rom, n * sizeof(uint32_t));
        sprintf(args->output_file_rom_img, "%s.romimg", args->file_name);
        output_file = args->output_file_rom_img;
        index_length = (ROM_ADDRESS_BIT_WIDTH_DEFAULT - 1) / 4 + 1;
        obj_name = "ROM";
        break;
    case GPR:
        n = REGISTER_COUNT_DEFAULT;
        
        data = (uint32_t *)malloc(n * sizeof(uint32_t));
        memcpy(data, scpu_inst->gpr, n * sizeof(uint32_t));
        sprintf(args->output_file_gpr_img, "%s.gprimg", args->file_name);
        output_file = args->output_file_gpr_img;
        index_length = 2;
        obj_name = "GPR";
        break;
    default:
        log_err_goto("未知的存储部件");
    }
    create_file_if_not_exists(output_file);
    output_file_fp = fopen(output_file, "w");
    check_mem(output_file_fp);
    log_info("Writing img file: %s", file_base_name(output_file));
    fprintf(output_file_fp, "v3.0 hex words addressed %s\n", obj_name);
    int current_line_index = 0;
    switch (obj)
    {
    case ROM:
    case RAM:
        for (int i = 0; i < n / HEX_PER_LINE + 1; i++)
        {
            fprintf(output_file_fp, "%0*x:", index_length, current_line_index);
            for (int j = 0; j < HEX_PER_LINE && i * HEX_PER_LINE + j < n; j++)
            {
                uint32_t *int32p = data + i * HEX_PER_LINE + j;
                fprintf(output_file_fp, " %0*x", 8, *int32p);
                current_line_index++;
            }
            fprintf(output_file_fp, "\n");
        }
        break;
    case GPR:
        for (int i = 0; i < n; i++)
        {
            fprintf(output_file_fp, "%2d\t%s\t\t:\t%08x\n", i, reg_to_name(i), data[i]);
        }
        break;
    default:
        log_err_goto("未知的存储部件");
    }

    log_info("Successfully write img file: %s", file_base_name(output_file));
error:
    
    fclose(output_file_fp);
    
    free(data);
    
}

uint32_t addr32_to_addrn(uint32_t addr32, Storage obj)
{
    uint32_t limit;
    if (obj == RAM)
    {
        limit = 1 << (RAM_ADDRESS_BIT_WIDTH_DEFAULT + log2_int32(RAM_DATA_BIT_WIDTH_DEFAULT / 8));
    }
    else if (obj == ROM)
    {
        limit = 1 << (ROM_ADDRESS_BIT_WIDTH_DEFAULT + log2_int32(ROM_DATA_BIT_WIDTH_DEFAULT / 8));
    }
    else
    {
        log_err_goto("Unkown hardware");
    }
    if (addr32 & 1 << 31)
    {
        addr32 = addr32+limit;
    }

    if (addr32 >= limit)
        log_err("address %08x out of boundary %08x", addr32, limit);
    return ((limit<<1)-1) & addr32;

error:
    return 0;
}

uint32_t log2_int32(uint32_t n){
    uint32_t r=0;
    if(!n){
        return 0;
    }
    while (n > 1) {
        n >>= 1; 
        r++;     
    }

    return r;
}