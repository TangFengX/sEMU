#ifndef __EMU_H__
#define __EMU_H__
#include "main.h"


#define RAM_ADDRESS_BIT_WIDTH_DEFAULT 16 //any
#define RAM_DATA_BIT_WIDTH_DEFAULT 32 //n*8
#define ROM_ADDRESS_BIT_WIDTH_DEFAULT 16 //any
#define ROM_DATA_BIT_WIDTH_DEFAULT 32 //n*8
#define REGISTER_COUNT_DEFAULT 32 
#define REGISTER_BIT_WIDTH_DEFAULT 32 
#define BUFFER_SIZE 1000


typedef struct scpu scpu;
struct scpu{
    uint8_t* ram;
    uint8_t* rom;
    uint32_t* gpr;
    uint32_t gpr_changed;
    uint32_t PC;
    uint16_t counter;
};

scpu* scpu_init(scpu* scpu_inst); 

void scpu_free(scpu* scpu_inst);

void run_simulator(const ProgramArgs *args);

void load_instruction_to_rom(const char *file_name,scpu* scpu_inst);

int hex_string_to_int(const char *hex_str);

int execute_instruction(scpu* scpu_inst);

int log2(int n){
    int r=-1;
    if(n>0){n/=2;r+=1;}
    return r;
}
#endif