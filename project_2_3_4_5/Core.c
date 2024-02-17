#include "Core.h"
#include <stdio.h>
#include <stdlib.h>

Core *initCore(Instruction_Memory *i_mem)
{
    Core *core = (Core *)malloc(sizeof(Core));
    core->clk = 0;
    core->PC = 0;
    core->instr_mem = i_mem;
    core->tick = tickFunc;

    for (int i = 0; i < 32; i++) {
        core->reg_file[i] = 0;
    }

    for (int i = 0; i < 32; i++) {
        core->data_mem[i] = 0; 
    }

    core->reg_file[10] = 4;
    core->reg_file[25] = 4;
    core->reg_file[11] = 2;
    core->reg_file[22] = 10;
    return core;
}

unsigned int instruct_split(unsigned int instruct, int start, int length) {
    char binaryString[33];
    sprintf(binaryString, "%0*lu", 32, instruct);
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

// FIXME, implement this function
bool tickFunc(Core *core)
{
    // Steps may include
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    printf("%d\n",instruction);
    instruction = 1774995; 
    Signal rd, rs1, rs2, funct3, funct7, imm;
    Signal opcode = instruct_split(instruction,25, 7);
    printf("Opcode: %d\n",opcode);
    Signal temp, zero;
    int is_ld__sd = 0;
    ControlSignals instruction_CS;

    int test[32]; 

    if (opcode == 51){
        rd = instruct_split(instruction,20,5);
        rs1 = instruct_split(instruction,12,5);
        rs2 = instruct_split(instruction,7,5);
        funct3 = instruct_split(instruction,17,3);
        funct7 = instruct_split(instruction,0,6);
    }

    if (opcode == 3 || opcode == 15 || opcode == 19 || opcode == 27){
        imm = ImmeGen(instruct_split(instruction,0,12)); 
        rs1 = instruct_split(instruction,12,5);
        funct3 = instruct_split(instruction,17,3);
        rd = instruct_split(instruction,20,5);
        if (opcode == 3) {
            is_ld__sd = 1;
        }
    }

    if (opcode == 99) {

    }

    printf("rd: %d, rs1: %d, funt3: %d, imm: %d\n",rd,rs1,funct3,imm);

    ControlUnit(opcode,&instruction_CS);
    
    Signal O_MUX1 = MUX(instruction_CS.ALUSrc,rs2,imm);

    Signal OP = ALUControlUnit(instruction_CS.ALUOp,funct7,funct3);

    printf("OP: %d\n",OP);
    //R-Type
    if (opcode == 51){
        ALU(core->reg_file[rs1],core->reg_file[O_MUX1],OP,&temp,&zero);
        int O_MUX2 = MUX(instruction_CS.MemtoReg,temp,0);
        core->reg_file[rd] = temp;
    }
    // Other I-types
    else if (opcode == 15 || opcode == 19 || opcode == 27) {
        ALU(core->reg_file[rs1],imm,OP,&temp,&zero);
        int O_MUX2 = MUX(instruction_CS.MemtoReg,temp,0);
        core->reg_file[rd] = temp;
    }
    //LD or SD
    if (is_ld__sd == 1 && opcode == 3){
        if (instruction_CS.MemRead == 1){
            temp = core->data_mem[temp]; 
            core->reg_file[rd] = temp;
        }
    }
    printf("%d:%d\n",rd,core->reg_file[rd]);

    // (Step N) Increment PC. FIXME, is it correct to always increment PC by 4?!
    core->PC += 4;

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
        signals->MemWrite = 1;
        signals->Branch = 0;
        signals->ALUOp = 1;
    }
}

// REVIEW
// FIXME (2). ALU Control Unit. Refer to Figure 4.12.
Signal ALUControlUnit(Signal ALUOp,
                      Signal Funct7,
                      Signal Funct3)
{   //ld and sd
    printf("ALUOP:%d, F7:%d, F3:%d\n",ALUOp,Funct7,Funct3);
    if (ALUOp == 0 && Funct7 == 0 && Funct3 == 0) {
        return 2;
    }
    //beq
    if (ALUOp == 1 && Funct7 == 0 && Funct3 == 0){
        return 6;
    }
    // For add
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }
    // R-Type Sub
    else if (ALUOp == 2 && Funct7 == 16 && Funct3 == 0)
    {
        return 6;
    }
    // R-Type Add
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    // R-Type OR
    else if (ALUOp == 1 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }
    // SLLI
    else if (ALUOp == 0 && Funct3 == 1) 
    {
        return 61;
    }
    
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
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }

    else if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }

    else if (ALU_ctrl_signal == 61)
    {
        *ALU_result = input_0 << input_1;
        if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }
    }
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
