#ifndef __EMU_H__
#define __EMU_H__
#include "main.h"


#define RAM_ADDRESS_BIT_WIDTH_DEFAULT 20 //any
#define RAM_DATA_BIT_WIDTH_DEFAULT 32 //n*8
#define ROM_ADDRESS_BIT_WIDTH_DEFAULT 20 //any
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

typedef enum{
    RAM,
    ROM,
    GPR
} Storage;

scpu* scpu_init(scpu* scpu_inst); 

void scpu_free(scpu* scpu_inst);

void run_simulator(const ProgramArgs *args);

void load_instruction_to_rom(const char *file_name,scpu* scpu_inst);

uint32_t hex_string_to_int(const char *hex_str);

int execute_instruction(scpu* scpu_inst);

void save_img(scpu* scpu_inst,const ProgramArgs *args,Storage obj);

uint32_t addr32_to_addrn(uint32_t addr32,Storage obj);

uint32_t log2_int32(uint32_t n);



#endif