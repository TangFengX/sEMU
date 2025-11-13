#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
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
        debug("[PC:%d][COUNTER:%d]\tlui\t%s,0x%x", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm);
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

        debug("[PC:%d][COUNTER:%d]\taddi\t%s,%s,%d", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), reg_to_name(rs1), imm);
        if (rd != 0)
        {
            scpu_inst->gpr[rd] = scpu_inst->gpr[rs1] + imm;
            scpu_inst->gpr_changed |= 1 << rd;
        }
        scpu_inst->PC += 4;
        break;
    case 0b0110011: // ADD

        debug("[PC:%d][COUNTER:%d]\tadd\t%s,%s,%s", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), reg_to_name(rs1), reg_to_name(rs2));
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
            debug("[PC:%d][COUNTER:%d]\tlw\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 0x3)
            {
                log_err_goto("Address misaligned exception: 0x%x", addr32);
            }
            if (addr32 & 1 << 15)
            {
                addr32 = ~addr32 + 1;
            }
            addrn = (~(0xFFFFFFFF << RAM_ADDRESS_BIT_WIDTH_DEFAULT)) & addr32;
            memcpy(&scpu_inst->gpr[rd], scpu_inst->ram + addrn, sizeof(uint32_t));
            scpu_inst->gpr_changed |= 1 << rd;
        }

        else if (funct3 == 0b100)
        {
            debug("[PC:%d][COUNTER:%d]\tlbu\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 1 << 15)
            {
                addr32 = ~addr32 + 1;
            }
            addrn = (~(0xFFFFFFFF << RAM_ADDRESS_BIT_WIDTH_DEFAULT)) & addr32;
            scpu_inst->gpr[rd] = (uint32_t)scpu_inst->ram[addrn];
            scpu_inst->gpr_changed |= 1 << rd;
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
            debug("[PC:%d][COUNTER:%d]\tsw\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rs2), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 0x3)
            {
                log_err_goto("Address misaligned exception: 0x%x", addr32);
            }
            if (addr32 & 1 << 15)
            {
                addr32 = ~addr32 + 1;
            }
            addrn = (~(0xFFFFFFFF << RAM_ADDRESS_BIT_WIDTH_DEFAULT)) & addr32;
            memcpy(scpu_inst->ram + addrn, &scpu_inst->gpr[rs2], sizeof(uint32_t));
        }

        else if (funct3 == 0b000)
        {
            debug("[PC:%d][COUNTER:%d]\tsb\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rs2), imm, reg_to_name(rs1));
            uint32_t addr32 = scpu_inst->gpr[rs1] + imm;
            uint32_t addrn;
            if (addr32 & 1 << 15)
            {
                addr32 = ~addr32 + 1;
            }
            addrn = (~(0xFFFFFFFF << RAM_ADDRESS_BIT_WIDTH_DEFAULT)) & addr32;
            memcpy(scpu_inst->ram + addrn, &scpu_inst->gpr[rs2], sizeof(uint8_t));
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
        debug("[PC:%d][COUNTER:%d]\tjalr\t%s,%d(%s)", scpu_inst->PC, scpu_inst->counter, reg_to_name(rd), imm, reg_to_name(rs1));
        scpu_inst->gpr[rd] = scpu_inst->PC + 4;
        scpu_inst->PC = scpu_inst->gpr[rs1] + imm;

        break;
    default:
        log_err_goto("Unknown opcode: %d", opcode);
        break;
    }
return 0;
error:
return -1;
}

void run_simulator(const ProgramArgs *args)
{   
    scpu* inst=scpu_init(NULL);
    load_instruction_to_rom(args->input_file,inst);
    int cycles=args->target_cycles,c=0;
    int flag=0;
    while(flag==0&&c<cycles){
        flag=execute_instruction(inst);
        c++;
    }
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
                *rom_pointer = hex_string_to_int(hex_line);
                rom_pointer++;
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

    if (value_long > INT_MAX || value_long < INT_MIN)
    {
        log_err_goto("警告: 转换后的值超出 int 的范围\n");
    }
    value_int = (uint32_t)value_long;
    return value_int;
error:
    return 0;
}
