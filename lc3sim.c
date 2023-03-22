#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/***************************************************************/
/*                                                             */
/* Files: isaprogram   LC-3 machine language program file     */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void process_instruction();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE 1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x)&0xFFFF)

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/*
  MEMORY[A] stores the word address A
*/

#define WORDS_IN_MEM 0x08000
int MEMORY[WORDS_IN_MEM];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3 State info.                                           */
/***************************************************************/
#define LC_3_REGS 8

int RUN_BIT; /* run bit */

typedef struct System_Latches_Struct
{

  int PC,              /* program counter */
      N,               /* n condition bit */
      Z,               /* z condition bit */
      P;               /* p condition bit */
  int REGS[LC_3_REGS]; /* register file. */
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int INSTRUCTION_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands                    */
/*                                                             */
/***************************************************************/
void help()
{
  printf("----------------LC-3 ISIM Help-----------------------\n");
  printf("go               -  run program to completion         \n");
  printf("run n            -  execute program for n instructions\n");
  printf("mdump low high   -  dump memory from low to high      \n");
  printf("rdump            -  dump the register & bus values    \n");
  printf("?                -  display this help menu            \n");
  printf("quit             -  exit the program                  \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle()
{

  process_instruction();
  CURRENT_LATCHES = NEXT_LATCHES;
  INSTRUCTION_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3 for n cycles                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles)
{
  int i;

  if (RUN_BIT == FALSE)
  {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating for %d cycles...\n\n", num_cycles);
  for (i = 0; i < num_cycles; i++)
  {
    if (CURRENT_LATCHES.PC == 0x0000)
    {
      RUN_BIT = FALSE;
      printf("Simulator halted\n\n");
      break;
    }
    cycle();
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3 until HALTed                 */
/*                                                             */
/***************************************************************/
void go()
{
  if (RUN_BIT == FALSE)
  {
    printf("Can't simulate, Simulator is halted\n\n");
    return;
  }

  printf("Simulating...\n\n");
  while (CURRENT_LATCHES.PC != 0x0000)
    cycle();
  RUN_BIT = FALSE;
  printf("Simulator halted\n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE *dumpsim_file, int start, int stop)
{
  int address; /* this is a address */

  printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  printf("-------------------------------------\n");
  for (address = start; address <= stop; address++)
    printf("  0x%.4x (%d) : 0x%.2x\n", address, address, MEMORY[address]);
  printf("\n");

  /* dump the memory contents into the dumpsim file */
  fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
  fprintf(dumpsim_file, "-------------------------------------\n");
  for (address = start; address <= stop; address++)
    fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x\n", address, address, MEMORY[address]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE *dumpsim_file)
{
  int k;

  printf("\nCurrent register/bus values :\n");
  printf("-------------------------------------\n");
  printf("Instruction Count : %d\n", INSTRUCTION_COUNT);
  printf("PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  printf("Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  printf("\n");

  /* dump the state information into the dumpsim file */
  fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
  fprintf(dumpsim_file, "-------------------------------------\n");
  fprintf(dumpsim_file, "Instruction Count : %d\n", INSTRUCTION_COUNT);
  fprintf(dumpsim_file, "PC                : 0x%.4x\n", CURRENT_LATCHES.PC);
  fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
  fprintf(dumpsim_file, "Registers:\n");
  for (k = 0; k < LC_3_REGS; k++)
    fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
  fprintf(dumpsim_file, "\n");
  fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */
/*                                                             */
/***************************************************************/
void get_command(FILE *dumpsim_file)
{
  char buffer[20];
  int start, stop, cycles;

  printf("LC-3-SIM> ");

  scanf("%s", buffer);
  printf("\n");

  switch (buffer[0])
  {
  case 'G':
  case 'g':
    go();
    break;

  case 'M':
  case 'm':
    scanf("%i %i", &start, &stop);
    mdump(dumpsim_file, start, stop);
    break;

  case '?':
    help();
    break;
  case 'Q':
  case 'q':
    printf("Bye.\n");
    exit(0);

  case 'R':
  case 'r':
    if (buffer[1] == 'd' || buffer[1] == 'D')
      rdump(dumpsim_file);
    else
    {
      scanf("%d", &cycles);
      run(cycles);
    }
    break;

  default:
    printf("Invalid Command\n");
    break;
  }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_memory                                     */
/*                                                             */
/* Purpose   : Zero out the memory array                       */
/*                                                             */
/***************************************************************/
void init_memory()
{
  int i;

  for (i = 0; i < WORDS_IN_MEM; i++)
  {
    MEMORY[i] = 0;
  }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename)
{
  FILE *prog;
  int ii, word, program_base;

  /* Open program file. */
  prog = fopen(program_filename, "r");
  if (prog == NULL)
  {
    printf("Error: Can't open program file %s\n", program_filename);
    exit(-1);
  }

  /* Read in the program. */
  if (fscanf(prog, "%x\n", &word) != EOF)
    program_base = word;
  else
  {
    printf("Error: Program file is empty\n");
    exit(-1);
  }

  ii = 0;
  while (fscanf(prog, "%x\n", &word) != EOF)
  {
    /* Make sure it fits. */
    if (program_base + ii >= WORDS_IN_MEM)
    {
      printf("Error: Program file %s is too long to fit in memory. %x\n",
             program_filename, ii);
      exit(-1);
    }

    /* Write the word to memory array. */
    MEMORY[program_base + ii] = word;
    ii++;
  }

  if (CURRENT_LATCHES.PC == 0)
    CURRENT_LATCHES.PC = program_base;

  printf("Read %d words from program into memory.\n\n", ii);
}

/************************************************************/
/*                                                          */
/* Procedure : initialize                                   */
/*                                                          */
/* Purpose   : Load machine language program                */
/*             and set up initial state of the machine.     */
/*                                                          */
/************************************************************/
void initialize(char *program_filename, int num_prog_files)
{
  int i;

  init_memory();
  for (i = 0; i < num_prog_files; i++)
  {
    load_program(program_filename);
    while (*program_filename++ != '\0')
      ;
  }
  CURRENT_LATCHES.Z = 1;
  NEXT_LATCHES = CURRENT_LATCHES;

  RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[])
{
  FILE *dumpsim_file;

  /* Error Checking */
  if (argc < 2)
  {
    printf("Error: usage: %s <program_file_1> <program_file_2> ...\n",
           argv[0]);
    exit(1);
  }

  printf("LC-3 Simulator\n\n");

  initialize(argv[1], argc - 1);

  if ((dumpsim_file = fopen("dumpsim", "w")) == NULL)
  {
    printf("Error: Can't open dumpsim file\n");
    exit(-1);
  }

  while (1)
    get_command(dumpsim_file);
}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   MEMORY

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */

/***************************************************************/

void SetCC(int DR)
{
  if (NEXT_LATCHES.REGS[DR] == 0)
  {
    NEXT_LATCHES.Z = 1;
    NEXT_LATCHES.N = 0;
    NEXT_LATCHES.P = 0;
  }
  else
  {
    NEXT_LATCHES.Z = 0;
    if (NEXT_LATCHES.REGS[DR] >> 15 == 1)
    {
      NEXT_LATCHES.N = 1;
      NEXT_LATCHES.P = 0;
    }
    else
    {
      NEXT_LATCHES.N = 0;
      NEXT_LATCHES.P = 1;
    }
  }
}

int Binary2Decimal(int num, int bits) // sign extension
{
  int ans = 0;
  ans -= ((num >> (bits - 1)) & 0x1) * pow(2, bits - 1);
  for (int i = 2; i <= bits; i++)
    ans += ((num >> (bits - i)) & 0x1) * pow(2, bits - i);
  return ans;
}

int simulate_add_for_16_bits(int op1, int op2, int bit1, int bit2)
{
  int ans = 0;

  op1 = Binary2Decimal(op1, bit1);
  op2 = Binary2Decimal(op2, bit2);
  ans = op1 + op2;
  ans = ans & 0xFFFF;

  return ans;
}

void process_instruction()
{
  /*  function: process_instruction
   *
   *    Process one instruction at a time
   *       -Fetch one instruction
   *       -Decode
   *       -Execute
   *       -Update NEXT_LATCHES
   */
  int IR = MEMORY[CURRENT_LATCHES.PC];
  CURRENT_LATCHES.PC = CURRENT_LATCHES.PC + 1;

  int opcode = (IR >> 12) & 0xF;

  int DR = (IR >> 9) & 0x7;
  int SR1 = (IR >> 6) & 0x7;
  int SR2 = IR & 0x7;

  int imm5 = IR & 0x1F;

  int PCoffset9 = IR & 0x1FF;
  int n = (IR >> 11) & 0x1;
  int z = (IR >> 10) & 0x1;
  int p = (IR >> 9) & 0x1;

  int BaseR = (IR >> 6) & 0x7;

  int PCoffset11 = IR & 0x7FF;

  int PCoffset6 = IR & 0x3F;

  int SR_NOT = (IR >> 6) & 0x7;

  int SR = (IR >> 9) & 0x7;

  int trapvect8 = IR & 0xFF;

  switch (opcode)
  {
  case 0x1: // ADD
    if (((IR >> 5) & 0x1) == 0)
      NEXT_LATCHES.REGS[DR] = simulate_add_for_16_bits(CURRENT_LATCHES.REGS[SR1], CURRENT_LATCHES.REGS[SR2], 16, 16);
    else
      NEXT_LATCHES.REGS[DR] = simulate_add_for_16_bits(CURRENT_LATCHES.REGS[SR1], imm5, 16, 5);
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x5: // AND
    if (((IR >> 5) & 0x1) == 0)
      NEXT_LATCHES.REGS[DR] = CURRENT_LATCHES.REGS[SR1] & CURRENT_LATCHES.REGS[SR2];
    else
      NEXT_LATCHES.REGS[DR] = CURRENT_LATCHES.REGS[SR1] & (Binary2Decimal(imm5, 5) & 0xFFFF);
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x0: // BR
    if (n == 1 && CURRENT_LATCHES.N == 1)
      NEXT_LATCHES.PC = simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9);
    else if (z == 1 && CURRENT_LATCHES.Z == 1)
      NEXT_LATCHES.PC = simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9);
    else if (p == 1 && CURRENT_LATCHES.P == 1)
      NEXT_LATCHES.PC = simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9);
    else
      NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0xC: // JMP && RET
    NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[BaseR];
    break;

  case 0x4: // JSR
    int temp = CURRENT_LATCHES.PC;
    if (((IR >> 11) & 0x1) == 1)
      NEXT_LATCHES.PC = simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset11, 16, 11);

    else
      NEXT_LATCHES.PC = CURRENT_LATCHES.REGS[BaseR];

    NEXT_LATCHES.REGS[7] = temp;
    break;

  case 0x2: // LD
    NEXT_LATCHES.REGS[DR] = MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9)];
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0xA: // LDI
    NEXT_LATCHES.REGS[DR] = MEMORY[MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9)]];
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x6: // LDR
    NEXT_LATCHES.REGS[DR] = MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.REGS[BaseR], PCoffset6, 16, 6)];
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0xE: // LEA
    NEXT_LATCHES.REGS[DR] = simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x9: // NOT
    NEXT_LATCHES.REGS[DR] = (~CURRENT_LATCHES.REGS[SR_NOT]) & 0xFFFF;
    SetCC(DR);
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x8: // RTI
    break;

  case 0x3: // ST
    MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9)] = CURRENT_LATCHES.REGS[SR];
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0xB: // STI
    MEMORY[MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.PC, PCoffset9, 16, 9)]] = CURRENT_LATCHES.REGS[SR];
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0x7: // STR
    MEMORY[simulate_add_for_16_bits(CURRENT_LATCHES.REGS[BaseR], PCoffset6, 16, 6)] = CURRENT_LATCHES.REGS[SR];
    NEXT_LATCHES.PC = CURRENT_LATCHES.PC;
    break;

  case 0xF: // TRAP
            /*
             You need to implement the TRAP instruction as the LC-3 ISA defines in Appendix A. However, you do not need
            to implement the TRAP routines. The trap vector table will be initialized to all zeroes by the shell code provided.
            Thus, whenever a TRAP instruction is processed, PC will be set to 0. The shell code provided will halt the simulator
            whenever PC becomes 0
             */
    NEXT_LATCHES.PC = 0;
    break;
  }
}
