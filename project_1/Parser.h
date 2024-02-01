#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Instruction_Memory.h"
#include "Registers.h"

void loadInstructions(Instruction_Memory *i_mem, const char *trace);
void parseRType(char *opr, Instruction *instr, int opcode_IN, int funct3_IN, int funct7_IN);
int regIndex(char *reg);
