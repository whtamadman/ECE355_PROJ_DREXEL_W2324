#include "Core.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
unsigned instruction; int is_ld_sd; ControlSignals instruction_CS; Signal rd; Signal rs1; Signal rs2; Signal funct3; Signal funct7; Signal imm; Signal OP; Signal opcode; Signal O_MUX1;

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    core->reg_file[1] = 0;
    core->reg_file[2] = 10; 
    core->reg_file[3] = -15;
    core->reg_file[4] = 20;
    core->reg_file[5] = 30;
    core->reg_file[6] = -35;
    core->data_mem[40] = -63;
    core->data_mem[48] = 63;
    return core;
}

unsigned int instruct_split(unsigned int instruct, int start, int length) {
    char binaryString[33];
    sprintf(binaryString, "%0*u", 32, instruct);
    int index = 31;
    while (instruct > 0 || index >= 31 - length) {
        binaryString[index--] = (instruct% 2) + '0';
        instruct/= 2;
    }
    unsigned int maxValue = (1u << length) - 1;
    unsigned int fieldValue = 0;
    int i;
    for (i = start; i < start+ length; i++) {
        fieldValue = (fieldValue << 1) | (binaryString[i] - '0');
    }
    return fieldValue; //opcode last
}

unsigned extractImmediate(unsigned int decimalValue) {
  char binaryString[33];  // Assuming 32-bit integer + 1 for null terminator
    sprintf(binaryString, "%0*u", 32, decimalValue);  // Initialize the string with spaces
    int index = 31;
    while (decimalValue > 0) {
        binaryString[index--] = (decimalValue % 2) + '0';
        decimalValue /= 2;
    }
    
    char firstImm[2];
    char secondImm[7];
    char thirdImm[5];
    char fourthImm[2];
    char Imm[15];

    strncpy(firstImm, binaryString, 1);
    firstImm[1] = '\0';
    strncpy(secondImm, binaryString + 1, 6);
    secondImm[6] = '\0';
    strncpy(thirdImm, binaryString + 20, 4);
    thirdImm[4] = '\0';
    strncpy(fourthImm, binaryString + 24, 1);
    fourthImm[1] = '\0';

    // firstImm[7] = '\0';
    // secondImm[5] = '\0';
    strcpy(Imm, firstImm);
    strcat(Imm, secondImm);
    strcat(Imm, thirdImm);
    strcat(Imm, fourthImm);

    int length = strlen(Imm);
    int isNegative = 0;
    int i;

    if (Imm[0] == '1') {
        isNegative = 1;
        for (i = length - 1; i >= 0; i--) {
            if (Imm[i] == '0') {
                Imm[i] = '1';
            } else {
                Imm[i] = '0';
            }
        }
        int carry = 1;
        for (i = length - 1; i >= 0; i--) {
            if (Imm[i]=='0' && carry == 1) {
                Imm[i] = '1';
                carry = 0;
            } else if (Imm[i]=='1' && carry == 1) {
                Imm[i] = '0';
            }
        }
    }
    int decimal = 0;
    for (i = length - 1; i >= 0; i--) {
        if (Imm[i]=='1') {
            decimal += pow(2, length - 1 - i);
        }
    }
    
    if (isNegative) {
        decimal = -decimal;
    }
    
    //printf("%d\n",decimal-1);

    return (decimal-1);
}

unsigned int fetch(Core* core) {
    instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    printf("INSTRU: %d\n", instruction);
    return instruction;
}

ControlSignals decode(Core *core) {
    opcode = instruct_split(instruction,25, 7);
    printf("Opcode: %ld\n",opcode);
    if (opcode == 51){
        rd = instruct_split(instruction,20,5);
        rs1 = instruct_split(instruction,12,5);
        rs2 = instruct_split(instruction,7,5);
        funct3 = instruct_split(instruction,17,3);
        funct7 = instruct_split(instruction,0,6);
        printf("rd: %ld, rs1: %ld, funt3: %ld, rs2: %ld, funct7: %ld\n",rd,rs1,funct3,rs2,funct7);
    }

    else if (opcode == 3 || opcode == 15 || opcode == 19 || opcode == 27){
        imm = ImmeGen(instruct_split(instruction,0,12)); 
        rs1 = instruct_split(instruction,12,5);
        funct3 = instruct_split(instruction,17,3);
        rd = instruct_split(instruction,20,5);
        if (opcode == 3) {
            is_ld_sd = 1;
        }
        printf("rd: %ld, rs1: %ld, funt3: %ld, imm: %ld\n",rd,rs1,funct3,imm);
    }

    else if (opcode == 99) {
        imm = ImmeGen(extractImmediate(instruction));
        rs1 = instruct_split(instruction,12,5);
        rs2 = instruct_split(instruction,7,5);
        funct3 = instruct_split(instruction,17,3);
        printf("rs1: %ld, funt3: %ld, rs2: %ld, imm: %ld\n",rs1,funct3,rs2,imm);
    }

    ControlUnit(opcode,&instruction_CS);
    
    O_MUX1 = MUX(instruction_CS.ALUSrc,rs2,imm);

    OP = ALUControlUnit(instruction_CS.ALUOp,funct7,funct3);

    printf("OP: %ld\n",OP);

    return instruction_CS;
}
uint32_t temp = 0; 
uint32_t zero = 0;
int execute(Core *core) {
    //R-Type
    if (opcode == 51){
        ALU(core->reg_file[rs1],core->reg_file[O_MUX1],OP,&temp,&zero);
        return 0;
    }
    // Other I-types
    else if (opcode == 15 || opcode == 19 || opcode == 27) {
        ALU(core->reg_file[rs1],imm,OP,&temp,&zero);
        return 0;
    }
    // Branch
    else if (opcode == 99) {
        uint32_t temp = 0; 
        uint32_t zero = 0;
        ALU(core->reg_file[rs1],core->reg_file[rs2],OP,&temp,&zero);
        int OMUX_3 = MUX(zero,core->PC+= 4,core->PC+=imm);
        printf("%ld vs %ld\n",core->reg_file[rs1],core->reg_file[rs2]);
        if (core->reg_file[rs1] != core->reg_file[rs2]){
            core->PC = OMUX_3;
            printf("----------------Start of Next Loop--------------------\n\n");
        }
        return 1;
    }
    //LD or SD
    else if (is_ld_sd == 1 && opcode == 3){
        //Init Array
        if (instruction_CS.MemRead == 1){ 
            printf("Ld/Sd Detected\n");
        }
        return 0;
    }
}

void memory_access(Core *core) {
    if (is_ld_sd == 1 && opcode == 3){
        if (instruction_CS.MemRead == 1){ 
            printf("Before Change:%p\n",&core->data_mem[rs1]);
            int ldtemp = core->reg_file[rs1] + imm/8;
            printf("x%d = data_mem[%d]: %p\n",rd, ldtemp, &core->data_mem[rs1]);
        }
    }
}

void write_back(Core *core) {
    if (opcode == 51){
        int O_MUX2 = MUX(instruction_CS.MemtoReg,temp,NULL);
        core->reg_file[rd] = temp;
        printf("R-Type Memory Address: %p\n",rs1,(void*)&core->reg_file[rd]);
        printf("%ld:%ld\n\n",rd,core->reg_file[rd]);
    }
    else if (opcode == 15 || opcode == 19 || opcode == 27) {
        int O_MUX2 = MUX(instruction_CS.MemtoReg,temp,0);
        core->reg_file[rd] = temp;
        printf("I-Type Memory Address: %p\n",rd);
        printf("%ld:%ld\n\n",rd,core->reg_file[rd]);
    }
    else if (is_ld_sd == 1 && opcode == 3){
        core->reg_file[rd] = core->data_mem[rs1];
        printf("%ld:%ld\n\n",rd,core->reg_file[rd]);
    }
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
    fetch(core);
    decode(core);
    if (execute(core) == 0){
        memory_access(core);
        write_back(core);
        core->PC += 4;
    }
    ++core->clk;
    // Are we reaching the final instruction?
    if (core->PC > core->instr_mem->last->addr)
    {
        return false;
    }
    return true;
}

//REVIEW
// FIXME (1). Control Unit. Refer to Figure 4.18.
void ControlUnit(Signal input,
                 ControlSignals *signals)
{
    if (input == 51)
    {
        signals->ALUSrc = 0;
        signals->MemtoReg = 0;
        signals->RegWrite = 1;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 2;
    }
    // For I-Type 3,15,19,27
    if (input == 3 || input == 15 || input == 19 || input == 27)
    {
        signals->ALUSrc = 1;
        signals->MemtoReg = 1;
        signals->RegWrite = 1;
        signals->MemRead = 1;
        signals->MemWrite = 0;
        signals->Branch = 0;
        signals->ALUOp = 0;
    }
    // For SB-Type 99
    if (input == 99) {
        signals->ALUSrc = 0;
        signals->RegWrite = 0;
        signals->MemRead = 0;
        signals->MemWrite = 0;
        signals->Branch = 1;
        signals->ALUOp = 1;
    }
}

// REVIEW
// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{   //ld and sd
    printf("ALUOP:%ld, F7:%ld, F3:%ld\n",ALUOp,Funct7,Funct3);
    if (ALUOp == 0 && Funct7 == 0 && Funct3 == 3) {return 2;}
    //beq
    else if (ALUOp == 1 && Funct7 == 0 && Funct3 == 0){return 6;}
    // For add
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0){return 2;}
    // R-Type Subf
    else if (ALUOp == 2 && Funct7 == 16 && Funct3 == 0){return 6;}
    // R-Type AND
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7){return 0;}
    // R-Type OR
    else if (ALUOp == 1 && Funct7 == 0 && Funct3 == 6){return 1;}
    // SLLI
    else if (ALUOp == 0 && Funct3 == 1 && Funct7 == 0) {return 61;}
    // Addi
    else if (ALUOp == 0 && Funct3 == 0 && Funct7 == 0) {return 2;}
    //bne
    else if (ALUOp == 0 && Funct3 == 0 && Funct7 == 1) {return 71;}
}

//REVIEW
// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input)
{
    input = (int64_t)input;
}

// FIXME (4). ALU
void ALU(Signal input_0, //4
         Signal input_1, //4
         Signal ALU_ctrl_signal, //2
         Signal *ALU_result, 
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
    }

    else if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
    }

    else if (ALU_ctrl_signal == 61)
    {
        *ALU_result = input_0 << input_1;
    }
    // For AND (R-type)
    else if (ALU_ctrl_signal == 0) 
    {
        *ALU_result = (input_0 & input_1);
    }
    // For OR (R-type)
    else if (ALU_ctrl_signal == 1)
    {
        *ALU_result = (input_0 | input_1);
    }
    //BNE
    else if (ALU_ctrl_signal == 71)
    {
        *ALU_result = (input_0 != input_1);
    }
    if (*ALU_result == 0) { *zero = 1; } else { *zero = 0;}
}

// (4). MUX
Signal MUX(Signal sel,
           Signal input_0,
           Signal input_1)
{
    if (sel == 0) { return input_0; } else { return input_1; }
}

// (5). Add
Signal Add(Signal input_0,
           Signal input_1)
{
    return (input_0 + input_1);
}

// (6). ShiftLeft1
Signal ShiftLeft1(Signal input)
{
    return input << 1;
}
