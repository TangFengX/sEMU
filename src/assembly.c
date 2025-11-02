#include "assembly.h"
#include "dbg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



void compile_source_file(const char* file_assemble, const char* file_analyze, const char* file_hex) {

}

void analyze_file_to_hex_file(const char* file_analyze, const char* file_hex) {
    FILE* analyze_fp = fopen(file_analyze, "r");
    
    char line[MAX_BUFFER_SIZE];
    char hex_code[HEX_CODE_SIZE];

    HexCodeList* hex_list_head = NULL,* hex_list_tail = NULL;

    hex_list_head = (HexCodeList*)malloc(sizeof(HexCodeList));
    hex_list_tail = hex_list_head;

    check_mem(hex_list_head);
    check_mem(analyze_fp);

    log_info("Reading analyze file: %s", file_base_name(file_analyze));

    int line_count = 0;//提取的编码行号从0开始
    int file_line_index=0;//文件行号从1开始

    while(fgets(line, sizeof(line), analyze_fp)!=NULL){
        file_line_index++;
        size_t len = strlen(line);

        line[strcspn(line, "\r\n")] = '\0';

        debug("Line %d : %s", line_count, line);

        char* begin_of_hex=NULL;
        HexCodeList* new_hex_node=NULL;

        begin_of_hex=return_pointer_next_to_Nth_tab(line,1);
        if(begin_of_hex==NULL){
            log_info("No hex code found in line %d ", file_line_index);
            continue;
        }
        
        

        while(1){
            if(*begin_of_hex=='\0'){
                log_info("No hex code found in line %d ", file_line_index);
                break;
            }
            else if(*begin_of_hex==':'||*begin_of_hex=='\t'||*begin_of_hex==' '){
                begin_of_hex++;
            }
            else{
                new_hex_node=(HexCodeList*)malloc(sizeof(HexCodeList));
                check_mem(new_hex_node);
                strncpy(new_hex_node->hex_code, begin_of_hex, HEX_CODE_SIZE);

                new_hex_node->index=line_count;
                new_hex_node->hex_code[HEX_CODE_SIZE]='\0';
                debug("Hex Code %d: %s",line_count, new_hex_node->hex_code);

                line_count++;
                new_hex_node->next=NULL;
                hex_list_tail->next=new_hex_node;
                hex_list_tail=new_hex_node;
                break;
            }
        }
    }
    fclose(analyze_fp);
    log_info("Analyze file %s read successfully.", file_base_name(file_analyze));
    int index_length=1;
    int max_index=16;
    
    while(1){
        if(max_index>=line_count){
            break;
        }
        else{
            index_length++;
            max_index*=16;
        }
    }
    FILE* assemble_fp = fopen(file_hex, "w");
    check_mem(assemble_fp);
    log_info("Writing hex file: %s", file_base_name(file_hex));
    HexCodeList* current_hex_node=hex_list_head->next;

    fprintf(assemble_fp, "v3.0 hex words addressed\n");

    int current_line_index_hex_file=0;
    
    while(current_hex_node!=NULL){

        fprintf(assemble_fp, "%0*x:",index_length,current_line_index_hex_file);
        for(int i=0;i<HEX_PER_LINE&&current_hex_node!=NULL;i++){
            fprintf(assemble_fp, " %s", current_hex_node->hex_code);
            current_line_index_hex_file++;
            current_hex_node=current_hex_node->next;
        }
        fprintf(assemble_fp, "\n");
    }
    fclose(assemble_fp);
    log_info("Hex file %s written successfully.", file_base_name(file_hex));
    
    error:
    while(hex_list_head!=NULL){
        HexCodeList* temp=hex_list_head;
        hex_list_head=hex_list_head->next;
        free(temp);
    }
    return;
}



void analyze_file_to_assemble_file(const char* file_analyze, const char* file_assemble) {
    FILE* analyze_fp = fopen(file_analyze, "r");
    
    char line[MAX_BUFFER_SIZE];
    char assemble_line[ASSEMBLE_LINE_SIZE];

    AssembleLineList* assemble_list_head = NULL,* assemble_list_tail = NULL;

    assemble_list_head = (AssembleLineList*)malloc(sizeof(AssembleLineList));
    assemble_list_tail = assemble_list_head;

    check_mem(assemble_list_head);
    check_mem(analyze_fp);

    log_info("Reading analyze file: %s", file_base_name(file_analyze));

    int line_count = 0;
    int file_line_index=0;

    while(fgets(line, sizeof(line), analyze_fp)!=NULL){
        file_line_index++;
        size_t len = strlen(line);

        line[strcspn(line, "\r\n")] = '\0';

        debug("Line %d : %s", line_count, line);

        char* begin_of_assemble=NULL;
        AssembleLineList* new_assemble_node=NULL;

        begin_of_assemble=return_pointer_next_to_Nth_tab(line,2);
        if(begin_of_assemble==NULL){
            log_info("No assemble code found in line %d ", file_line_index);
            continue;
        }
        
        while(1){
            if(*begin_of_assemble=='\0'){
                log_info("No assemble code found in line %d ", file_line_index);
                break;
            }
            else{
                new_assemble_node=(AssembleLineList*)malloc(sizeof(AssembleLineList));
                check_mem(new_assemble_node);
                strncpy(new_assemble_node->assemble_line, begin_of_assemble, ASSEMBLE_LINE_SIZE);

                new_assemble_node->index=line_count;
                new_assemble_node->assemble_line[ASSEMBLE_LINE_SIZE]='\0';
                debug("Assemble Line %d: %s",line_count, new_assemble_node->assemble_line);

                line_count++;
                new_assemble_node->next=NULL;
                assemble_list_tail->next=new_assemble_node;
                assemble_list_tail=new_assemble_node;
                break;
            }
        }
    }
    fclose(analyze_fp);
    log_info("Analyze file %s read successfully.", file_base_name(file_analyze));
    FILE* assemble_fp = fopen(file_assemble, "w");
    check_mem(assemble_fp);
    log_info("Writing assemble file: %s", file_base_name(file_assemble));
    AssembleLineList* current_assemble_node=assemble_list_head->next;

    while(current_assemble_node!=NULL){
        fprintf(assemble_fp, "%s\n", current_assemble_node->assemble_line);
        current_assemble_node=current_assemble_node->next;
    }
    fclose(assemble_fp);
    log_info("Assemble file %s written successfully.", file_base_name(file_assemble));
    error:
    while(assemble_list_head!=NULL){
        AssembleLineList* temp=assemble_list_head;
        assemble_list_head=assemble_list_head->next;
        free(temp);
    }
}

char* return_pointer_next_to_Nth_tab(char* line_ptr, int n){
    char* current_ptr=line_ptr;
    for(int i=0;i<n;i++){
        current_ptr=strchr(current_ptr,'\t');
        if(current_ptr==NULL){
            return NULL;
        }
        current_ptr=current_ptr+1;
    }
    return current_ptr;
}