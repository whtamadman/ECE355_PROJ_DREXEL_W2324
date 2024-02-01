#include "Parser.h"

// FIXME, implement this function.
// Here shows an example on how to translate "add x10, x10, x25"

char *Rtype[] = {"add","sub","sll","slt","sltu","xor","srl","sra","or","and","addw","subw","sllw","srlw","sraw"};
int RtypeOpcode[] = {51,51,51,51,51,51,51,51,51,51,59,59,59,59,59};
int Rtype_func3[] = {0,0,1,2,3,4,5,5,6,7,0,0,1,5,5};
int Rtype_func7[] = {0,32,0,0,0,0,0,32,0,0,0,32,0,0,32};

char *Itype[] = {"lb","lh","lw","ld","lbu","lhu","lwu","fence","fence.i","addi","slli","slti","sltiu","xori","srli","srai","ori","andi","addiw","slliw","srliw","sraiw"};
int ItypeOpcode[] = {3,3,3,3,3,3,3,15,15,19,19,19,19,19,19,19,19,19,27,27,27,27};
int Itype_func3[] = {0,1,2,3,4,5,6,0,1,0,1,2,3,4,5,5,6,7,0,1,5,5};
int Itype_imm[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,32,0,0,0,0,0,32};
64 + 32 + 3 = 99
char *SBtype[] = {"beq","bne","blt","bge","bltu","bgeu"};
int SBtypeOpcode[] = {99,99,99,99,99,99};
int SBfunc3[] = {0,1,4,5,6,7};




void loadInstructions(Instruction_Memory *i_mem, const char *trace)
{
    printf("Loading trace file: %s\n", trace);

    FILE *fd = fopen(trace, "r");
    if (fd == NULL)
    {
        perror("Cannot open trace file. \n");
        exit(EXIT_FAILURE);
    }

    // Iterate all the assembly instructions
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    Addr PC = 0; // program counter points to the zeroth location initially.
    int IMEM_index = 0;
    while ((read = getline(&line, &len, fd)) != -1)
    {
        // Assign program counter
        i_mem->instructions[IMEM_index].addr = PC;

        // Extract operation
        char *raw_instr = strtok(line, " ");

        //For R-Types
        for (int i = 0; i < 15; i++) {
            if (strcmp(raw_instr,Rtype[i]) == 0) {
                printf("Function:%s\n\tOpcode:%d\n\tFunc3:%d\n\tFunc7:%d\n",Rtype[i],RtypeOpcode[i],Rtype_func3[i],Rtype_func7[i]);
                parseRType(raw_instr, &(i_mem->instructions[IMEM_index]), RtypeOpcode[i], Rtype_func3[i], Rtype_func7[i]);
                i_mem->last = &(i_mem->instructions[IMEM_index]);
            }
        }
        //For I-Types
        for (int i = 0; i < 22; i++) {
            if (strcmp(raw_instr,Itype[i]) == 0) {
                printf("Function:%s\n\tOpcode:%d\n\tFunc3:%d\n\tImm:%d\n",Itype[i],ItypeOpcode[i],Itype_func3[i],Itype_imm[i]);
                parseIType(raw_instr, &(i_mem->instructions[IMEM_index]), ItypeOpcode[i], Itype_func3[i], Itype_imm[i]);
                i_mem->last = &(i_mem->instructions[IMEM_index]);
            }
        }

        IMEM_index++;
        PC += 4;
    }

    fclose(fd);
}

void parseRType(char *opr, Instruction *instr, int opcode_IN, int funct3_IN, int funct7_IN)
{
    instr->instruction = 0;
    unsigned opcode = opcode_IN;
    unsigned funct3 = funct3_IN;
    unsigned funct7 = funct7_IN;

    char *reg = strtok(NULL, ", ");
    unsigned rd = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_1 = regIndex(reg);

    reg = strtok(NULL, ", ");
    unsigned rs_2 = regIndex(reg);
    reg[strlen(reg)-1] = '\0';

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (rs_2 << (7 + 5 + 3 + 5));
    instr->instruction |= (funct7 << (7 + 5 + 3 + 5 + 5));
}

void parseIType(char *opr, Instruction *instr, int opcode_IN, int funct3_IN, int Imm_IN)
{
    instr->instruction = 0;
    unsigned opcode = opcode_IN;
    unsigned funct3 = funct3_IN;
    unsigned Imm = Imm_IN;
    int load = 0;
    unsigned rd;
    unsigned rs_1;

    //Check if Func is Load
    for (int i = 0; i < 8; i++) {
        if (strcmp(opr, Itype[i]) == 0) {
            load = 1;
        }
    }
    
    //Universal
    char *reg = strtok(NULL, ", ");
    rd = regIndex(reg);
    //Non-Load 
    if (load == 0) {
        reg = strtok(NULL, ", ");
        rs_1 = regIndex(reg);

        reg = strtok(NULL, ", ");
        Imm += atoi(reg);
        reg[strlen(reg)-1] = '\0';
        printf("Imm: %d\n",atoi(reg));
    }
    //Load:
    else if (load == 1) {
        reg = strtok(NULL, "(");
        Imm += atoi(reg);
        printf("Imm: %d\n",atoi(reg));

        reg = strtok(NULL, ")");
        rs_1 = regIndex(reg);
        //reg[strlen(reg)-1] = '\0';
        printf("rs_1= %d\n",rs_1);

    }

    // Contruct instruction
    instr->instruction |= opcode;
    instr->instruction |= (rd << 7);
    instr->instruction |= (funct3 << (7 + 5));
    instr->instruction |= (rs_1 << (7 + 5 + 3));
    instr->instruction |= (Imm << (7 + 5 + 3 + 5));
}

int regIndex(char *reg)
{
    unsigned i = 0;
    for (i; i < NUM_OF_REGS; i++)
    {
        if (strcmp(REGISTER_NAME[i], reg) == 0)
        {
            break;
        }
    }

    return i;
}
