#include "Core.h"

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

    return core;
}

int extract_bits_and_convert_to_decimal(int value, int start, int end) {
    int decimal_value = 0;
    int bit_position = 1; // Represents 2^0

    // Iterate through each bit from start to end
    for (int i = start; i <= end; i++) {
        // If the bit is set, add the corresponding value to decimal_value
        if ((value >> i) & 1) {
            decimal_value += bit_position;
        }
        // Increment bit_position to represent the next power of 2
        bit_position <<= 1;
    }
    return decimal_value;
}

// FIXME, implement this function
bool tickFunc(Core *core)
{
    // Steps may include
    // (Step 1) Reading instruction from instruction memory
    unsigned instruction = core->instr_mem->instructions[core->PC / 4].instruction;
    Signal opcode = extract_bits_and_convert_to_decimal(instruction, 0, 6);
    ControlSignals instruction_CS;
    ControlUnit(opcode,&instruction_CS);

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
    printf("%ld\n",input);   
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
{   // ld and sd
    if (ALUOp == 0 && Funct7 == 0 && Funct3 == 0) {
        return 2;
    }
    // beq
    if (ALUOp == 1 && Funct7 == 0 && Funct3 == 0){
        return 6;
    }
    // For add
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 0)
    {
        return 2;
    }
    // R-Type Sub
    else if (ALUOp == 2 && Funct7 == 32 && Funct3 == 0)
    {
        return 6;
    }
    // R-Type AND
    else if (ALUOp == 2 && Funct7 == 0 && Funct3 == 7)
    {
        return 0;
    }
    // R-Type OR
    else if (ALUOp == 1 && Funct7 == 0 && Funct3 == 6)
    {
        return 1;
    }
    
}
//REVIEW
// FIXME (3). Imme. Generator
Signal ImmeGen(Signal input)
{
    input = (int32_t)input;
}

// FIXME (4). ALU
void ALU(Signal input_0,
         Signal input_1,
         Signal ALU_ctrl_signal,
         Signal *ALU_result,
         Signal *zero)
{
    // For addition
    if (ALU_ctrl_signal == 2)
    {
        *ALU_result = (input_0 + input_1);
    }
    // For subtraction
    else if (ALU_ctrl_signal == 6)
    {
        *ALU_result = (input_0 - input_1);
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
    // zero flag
    if (*ALU_result == 0) { *zero = 1; } else { *zero = 0; }

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
