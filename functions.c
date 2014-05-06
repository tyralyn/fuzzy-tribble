#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

// these are the structures used in this simulator


// global variables
// register file
int regfile[32];
// instruction memory
int instmem[100];  // only support 100 static instructions
// data memory
int datamem[1024];
// program counter
int pc;

/* load
 *
 * Given the filename, which is a text file that 
 * contains the instructions stored as integers 
 *
 * You will need to load it into your global structure that 
 * stores all of the instructions.
 *
 * The return value is the maxpc - the number of instructions
 * in the file
 */
int load(char *filename)
{
    FILE *file;
    int* fchar;
    char stringy[10000];
    int decval, opcode;
    file = fopen(filename, "r"); // read file
    if (file == NULL)
        return 0;
    int maxpc = 0; pc=0;
    while ((fgets(stringy, 100000, file))!= NULL) {
      decval = atoi(stringy);
      instmem[maxpc]=decval;
      maxpc++;
    }
    return maxpc;    
}


/* fetch
 *
 * This fetches the next instruction and updates the program counter.
 * "fetching" means filling in the inst field of the instruction.
 */
void fetch(InstInfo *inst)
{
  // loop through instruction array (instmem) using inst.pc
  inst->pc = pc;//program counter of inst is pc of machine
  int instruction = instmem[inst->pc]; //get instruction int from instmem
  //printf("fetch %d \n", inst->inst);//debugging for fetch
  inst->inst = instruction;//set inst's instruction int to the one from instmem
  // printf("fetch %d\n", inst->inst);
  pc++; //increment program counter of whole simulation  
}

/* decode
 *
 * This decodes an instruction.  It looks at the inst field of the 
 * instruction.  Then it decodes the fields into the fields data 
 * member.  The first one is given to you.
 *
 * Then it checks the op code.  Depending on what the opcode is, it
 * fills in all of the signals for that instruction.
 */
void decode(InstInfo *instruction)
{
  // fill in the signals and fields
  int val = instruction->inst;
  
  //conversion of integer to bitstring for debugging purposes
  int pos, binVal;
  char bitstring[32];
  for (pos = 31; pos >= 0; pos--) {
    binVal=val >> pos;
    if (binVal & 1)
      bitstring[31-pos]='1';
    else
      bitstring[31-pos]='0';
   }
  //printf("decode %d\n", instruction->inst);
  
  //printing to test getting op
  int op, test, func, testImm;
  binVal = 0;
  char opstring[6];
  test = (val) & 0x3f;
  op=(val >> 26) & 0x03f;
  testImm=val & 0x3ffffff;
  for (pos = 26; pos >=0; pos--) {
    binVal = testImm >> pos;
    if (binVal & 1) 
      opstring[26-pos]='1';
    else
      opstring[26-pos]='0';
  }
  // printf("op %d in binary: %s\n", testImm, opstring);
  int jalImm = ((val & 0x3FFFFFF) << 6) >> 6;
  // fill in the rest of the fields here
  instruction->fields.op = op;
  instruction->fields.rd = (val >> 11) & 0x1F;
  instruction->fields.rs = (val >> 21) & 0x1F;
  instruction->fields.rt = (val >> 16) & 0x1F;
  instruction->fields.imm = ((val & 0xFFFF) << 16) >> 16;//(hal << 16) & 0x7FFF;
  instruction->fields.func = val & 0x3f;

  //printf("everything) op: %d rs: %d rt: %d rd: %d func: %d\n", instruction->fields.op, instruction->fields.rs, instruction->fields.rt, instruction->fields.rd, instruction->fields.func);
  
  //dividing possibilities by R, I, and J format
  if (op == 33 || op == 41) { //R-format
    instruction->destreg=instruction->fields.rd;
    instruction->sourcereg=instruction->fields.rs;
    instruction->targetreg=instruction->fields.rt;
    
    instruction->s1data=regfile[instruction->sourcereg];
    instruction->s2data=regfile[instruction->targetreg];
    instruction->input1=instruction->s1data;
    instruction->input2=instruction->s2data;
    switch(op) {
    case 33: //and, sub, sgt
      instruction->signals.mw=0;
      instruction->signals.mtr=0;
      instruction->signals.mr=0;
      instruction->signals.asrc=0;
      instruction->signals.btype=0;
      instruction->signals.rdst=1;
      instruction->signals.rw=1;
      switch(instruction->fields.func) {
      case 40: //and
	instruction->signals.aluop=2;
	sprintf(instruction->string,"and $%d, $%d, $%d",
		instruction->fields.rd, instruction->fields.rs, 
		instruction->fields.rt);
	instruction->destreg = instruction->fields.rd;
	break;
      case 24: //sub
	instruction->signals.aluop=1;
	sprintf(instruction->string,"sub $%d, $%d, $%d",
		instruction->fields.rd, instruction->fields.rs, 
		instruction->fields.rt);
	instruction->destreg = instruction->fields.rd;
	break;
      case 10: //sgt
	instruction->signals.aluop=6;
	sprintf(instruction->string,"sgt $%d, $%d, $%d",
		instruction->fields.rd, instruction->fields.rs, 
		instruction->fields.rt);
	instruction->destreg = instruction->fields.rd;
	break;
      default:
	//printf("wtf1 %d \n", func);
	break;
      }
      break;
    case 41: //jr
      instruction->signals.aluop=-1;
      instruction->signals.mw=0;
      instruction->signals.mtr=-1;
      instruction->signals.mr=0;
      instruction->signals.asrc=-1;
      instruction->signals.btype=2;
      instruction->signals.rdst=-1;
      instruction->signals.rw=0;
      instruction->sourcereg = instruction->fields.rs;
      instruction->targetreg = -1;
      instruction->destreg = -1;
      sprintf(instruction->string,"jr $%d",
	      instruction->fields.rs);
      instruction->destreg = -1;//instruction->fields.rd;
      break;
    default:
      //printf("wtf2 \n");
      break;
    }
  }
  else if (op ==48 || op == 18 || op ==19 || op == 14) { //I-format
    instruction->destreg=instruction->fields.rt;
    instruction->fields.rd = -1;
    /* printf("i format: rs: %d rt: %d rd: %d destreg: %d imm: %d\n", instruction->fields.rs, instruction->fields.rt, instruction->fields.rd, instruction->destreg, instruction->fields.imm); */
    instruction->targetreg=-1;
    instruction->sourcereg=instruction->fields.rs;
    instruction->s1data=regfile[instruction->sourcereg];
    instruction->s2data=instruction->fields.imm;//regfile[instruction->targetreg];
    instruction->input1=regfile[instruction->fields.rs];//s1data;
    instruction->input2=instruction->fields.imm;
    switch (op) {
    case 48: //addi
      instruction->signals.aluop=0;
      instruction->signals.mw=0;
      instruction->signals.mtr=0;
      instruction->signals.mr=0;
      instruction->signals.asrc=1;
      instruction->signals.btype=0;
      instruction->signals.rdst=0;
      instruction->signals.rw=1;
      sprintf(instruction->string,"addi $%d, $%d, %d",
          instruction->fields.rt, instruction->fields.rs, 
          instruction->fields.imm);
      //instruction->destreg = instruction->fields.rt;
      break;
    case 18: //lw
      instruction->signals.aluop=0;
      instruction->signals.mw=0;
      instruction->signals.mtr=1;
      instruction->signals.mr=1;
      instruction->signals.asrc=1;
      instruction->signals.btype=0;
      instruction->signals.rdst=0;
      instruction->signals.rw=1;
      sprintf(instruction->string,"lw $%d, %d($%d)",
          //c switched rs and rt
          instruction->fields.rt, instruction->fields.imm, 
	  instruction->fields.rs);
      instruction->destreg = instruction->fields.rt;
      //printf("LW: rs(%d): %d rt(%d): %d rd(%d): %d imm: %d\n", instruction->fields.rs, regfile[instruction->fields.rs],instruction->fields.rt, regfile[instruction->fields.rt], instruction->fields.rd, regfile[instruction->fields.rd], instruction->fields.imm); 
      break;
    case 19: //sw
      instruction->signals.aluop=0;
      instruction->signals.mw=1;
      instruction->signals.mtr=-1;
      instruction->signals.mr=0;
      instruction->signals.asrc=1;
      instruction->signals.btype=0;
      instruction->signals.rdst=-1;
      instruction->signals.rw=0;
      sprintf(instruction->string,"sw $%d,%d($%d)",
          instruction->fields.rt, instruction->fields.imm, 
          instruction->fields.rs);
      instruction->destreg = -1;//instruction->fields.rd;
      instruction->targetreg = instruction->fields.rt;
      /* printf("SW: rs(%d): %d rt(%d): %d rd(%d): %d imm: %d\n", instruction->fields.rs, regfile[instruction->fields.rs],instruction->fields.rt, regfile[instruction->fields.rt], instruction->fields.rd, regfile[instruction->fields.rd], instruction->fields.imm); */
      break;
    case 14: //blt
      instruction->signals.aluop=7;
      instruction->signals.mw=0;
      instruction->signals.mtr=-1; //c changed
      instruction->signals.mr=0;
      instruction->signals.asrc=0;
      instruction->signals.btype=3;
      instruction->signals.rdst=-1;
      instruction->signals.rw=0;
      sprintf(instruction->string,"blt $%d, $%d, %d",
          instruction->fields.rs, instruction->fields.rt, 
          instruction->fields.imm);
      instruction->destreg = -1;//instruction->fields.rd;
      instruction->input2=instruction->s2data;

	// changed condition to decode
      if (instruction->input1 < instruction->input2)
	instruction->aluout = 1;
      else
	instruction->aluout = 0;
	break;
    default:
      break;
    }
  }
  else if (op == 10) { //I-format
    int address;
    address = (val) & 0x3FFFFFF;
    // printf("J-items %d", address);
    instruction->fields.imm = testImm;//jalImm << 2;
    //switch(op) {
    // case 10: //jal
    instruction->sourcereg = instruction->targetreg = instruction->destreg = -1;
    instruction->signals.aluop=-1;
    instruction->signals.mw=0;
    instruction->signals.mtr=2;
    instruction->signals.mr=0;
    instruction->signals.asrc=-1;
    instruction->signals.btype=01;
    instruction->signals.rdst=2;
    instruction->signals.rw=1;
    sprintf(instruction->string, "jal %d", instruction->fields.imm);
    instruction->destreg=31;
    //break;
    // default:
      //   break;
  }
}

/* execute
 *
 * This fills in the aluout value into the instruction and destdata
 */

void execute(InstInfo *instruction)
{
  int k;
  int i1=instruction->input1, i2=instruction->input2;
  switch(instruction->signals.aluop) {
  case 0: //addi, load word, save word
    if (instruction->fields.op==18 || instruction->fields.op == 19) {
       instruction->aluout = instruction->fields.imm +regfile [instruction->fields.rs];
      break;
    }
    k=i1+i2;
    instruction->aluout=k;
    
    //printf("input1 %d input2 %d output %d \n", instruction->input1, instruction->input2, instruction->aluout);
    break;
  case 2: //and
    k=i1 & i2;
    instruction->aluout=k;
    break;
  case 1: //sub
    k=i1-i2;
    instruction->aluout=k;
    break;
  case 6: //sgt
    k=(i1>i2) ? 1 : 0;
    instruction->aluout=k;
    break;
  case 7: //blt
    //k=(i1<i2) ? 1 : 0;
    break;
  default: //don't care for jal, jr
    break;
  }
  // printf("execute %d\n", instruction->inst);
  //instruction->aluout=k;
}

/* memory
 *
 * If this is a load or a store, perform the memory operation
 */
void memory(InstInfo *instruction)
{
  //printf("memory %d\n", instruction->inst);
  if (instruction->signals.mr==1) { //lw
    instruction->memout=datamem[instruction->aluout];
    /* //printf("store word memout%d aluout %d \n", instruction->memout, instruction->aluout); */
  }
  if (instruction->signals.mw==1) { //sw
    datamem[instruction->aluout]=regfile[instruction->targetreg];
    /* printf("store word: content %d datamemIndex %d \n",regfile[instruction->targetreg] , instruction->aluout);  */
    /* printf("input1: %d input2: %d aluout: %d\n", instruction->input1, instruction->input2, instruction->aluout);  */

  }
  switch(instruction->signals.mtr) {
  case(0):
    instruction->destdata=instruction->aluout;// regfile[instruction->destreg]=instruction->aluout;
    break;
  case(1): //load word
    instruction->destdata=regfile[instruction->destreg];//instruction->memout;
    //printf("targetreg %d %d \n", instruction->destreg, instruction->string);
    //printf("what is stored in memory stage %d\n", instruction->destdata);
    break;
  case(2): //jal
    instruction->destdata=pc+1;
    regfile[31]=pc+1;
    break;
  default:
    break;
  }
}

/* writeback
 *
 * If a register file is supposed to be written, write to it now
 */
void writeback(InstInfo *instruction)
{
  int k;
  //printf("instruction->aluout = instruction->fields.imm +regfile [instruction->fields.rs]; 
  //printf("writeback %d\n", instruction->inst);
  //printf("SW: rs(%d): %d rt(%d): %d rd(%d): %d imm: %d\n", instruction->fields.rs, regfile[instruction->fields.rs],instruction->fields.rt, regfile[instruction->fields.rt], instruction->fields.rd, regfile[instruction->fields.rd], instruction->fields.imm);
  //printf("aluout: %d imm:%d rscontent: %d",instruction->aluout, instruction->fields.imm, regfile [instruction->fields.rs]);
  if (instruction->signals.rw==0)
    return;
  switch (instruction->signals.rdst) {
  case 0: 
    //printf("op field: %d memout: %d aluout: %d \n", instruction->fields.op,instruction->memout, instruction->aluout);
    if (instruction->fields.op == 18)
      regfile[instruction->fields.rt]=instruction->memout;
    else
      regfile[instruction->destreg]=instruction->aluout;
    break;
  case 2:
    regfile[31]=pc+1;
    break;
  case 1:
    regfile[instruction->destreg]=instruction->aluout;
    break;
  default:
    break;
  }
}


/*
 *
 *
 */

void setPCWithInfo(InstInfo *instruction) {
  switch (instruction->signals.btype) {
  case 0:
    break;
  case 1:
    regfile[31]=pc;
    pc=instruction->fields.imm;
    break;
  case 2:
    pc=regfile[instruction->sourcereg];
    break;
  case 3:
    if (instruction->aluout == 1) 
      pc+=instruction->fields.imm;
    break;
  default:
    break;
  }
}
