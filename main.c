#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

void doStuff(InstInfo* f, InstInfo* d, InstInfo*x, InstInfo* m, InstInfo* w);
int getRS(InstInfo* i) { return ( (i==NULL) ? -1 : i->fields.rs);  }
int getRT(InstInfo* i) { return ( (i==NULL) ? -1 : i->fields.rt);  }
int getRD(InstInfo* i) { return ( (i==NULL) ? -1 : i->fields.rd);  }

int main(int argc, char *argv[])
{
  
  InstInfo curInst;
  //InstInfo* instArray[4];
  //int top = 0;
  //  InstInfo *f = &curInst;
  int instnum = 0;
  int maxpc;
  FILE *program;
  int i;
  for (i =0 ; i < 32; i++) {
   regfile[i]=0;
  }
  
  if (argc != 2)
    {
      printf("Usage: sim filename\n");
      exit(0);
    }
  
  maxpc = load(argv[1]) - 1;
  InstInfo* n; n= malloc(sizeof(*n));
  InstInfo* f; f=malloc(sizeof(*f));
  InstInfo* d; d=malloc(sizeof(*d));
  InstInfo* x; x=malloc(sizeof(*x));
  InstInfo* m; m=malloc(sizeof(*m));
  InstInfo* w; w=malloc(sizeof(*w));
  
  fetch(f);
  decode(f);
  i = 0;

    
  while (i<=maxpc+4) {

    i++;
    //decode(d);
    // execute(x);
    //memory(m);
    writeback(w); 
    memory(m);
    int k;
    int memoryOutput;
    if (m->signals.rw != 0) {
      switch(m->signals.rdst) {
      case 0:
	k = regfile[m->targetreg];
	break;
      case 1:
	k = regfile[m->destreg];
	break;
      case 2:
	k = regfile[31];
	break;
      default:
	break;
      }
    }
    memoryOutput = k;
    execute(x);
    int executeOutput = x->aluout;
    decode(d);
    int decodeOutput1=d->s1data;
    int decodeOutput2=d->s2data;
    setPCWithInfo(w);  
    printP2(f,d,x,m,w,instnum++);
    
    int aluInput1, aluInput2;
    int forwardA = 0, forwardB = 0;
    if (x->signals.rw == 1 && x->fields.rd != -1 && x->fields.rd == d->fields.rs) {
      forwardA = 2;
    } 
    if (x->signals.rw == 1 && x->fields.rd != -1 && x->fields.rd == d->fields.rt) {
      forwardB = 2;
    } 
   
  //m hazard: u r in decode and you need somethng in memory
    if (m->signals.rw == 1 && (m->fields.rd != -1) && !(x ->signals.rw == 1 && ( x->fields.rd != -1 )) && (x->fields.rd != d->fields.rs) &&  (m->fields.rd == d->fields.rs ))  {
      forwardA = 1;
      //printf("m hazard a: rd: %d rs = %d \n", m->fields.rd, d->fields.rs);
    } 

    if (m->signals.rw == 1 && (m->fields.rd != -1) && !(x ->signals.rw == 1 && ( x->fields.rd != -1 )) && (x->fields.rd != d->fields.rt) &&  (m->fields.rd == d->fields.rt ))  {
      forwardB = 1;
      //printf("m hazard b: rd: %d rt = %d \n", m->fields.rd, d->fields.rt);
    }
    switch (forwardA) {
    case 0:
      aluInput1=decodeOutput1;
      break;
    case 1:
      aluInput1=executeOutput;
      break;
    case 2:
      aluInput1=memoryOutput;
      break;
    default:
      break;
    }

    switch (forwardB) {
    case 0:
      aluInput2=decodeOutput1;
      break;
    case 1:
      aluInput2=executeOutput;
      break;
    case 2:
      aluInput2=memoryOutput;
      break;
    default:
      break;
    }
    
    d->input1=aluInput1;
    d->input2=aluInput2;
    *w=*m; *m=*x; *x=*d; *d=*f;


    
    if (i <= maxpc) {
      fetch(f);
      decode(f);
    }
    else {
      *f = *n;
    }
  }
  printf("Cycles: %d\nInstructions Executed: %d\n", i, maxpc+1);
  free(d);  free(x);  free(m);  free(w);  free(n); free(f);
  exit(0);
}


void doStuff(InstInfo* FI, InstInfo* DI, InstInfo* XI, InstInfo* MI, InstInfo* WI) {
  //fetch(FI);
  //printf("%s\n", FI->string);
  decode(DI);
  execute(XI);
  memory(MI);
  writeback(WI); 
  setPCWithInfo(MI);
}


/*
 * print out the loaded instructions.  This is to verify your loader
 * works.
 */
void printLoad(int max)
{
  int i;
  for(i=0;i<=max;i++)
    printf("%d\n",instmem[i]);
}

/* print
 *
 * prints out the state of the simulator after each instruction
 */
void print(InstInfo *inst, int count)
{
  int i, j;
  printf("Instruction %d: %d\n",count,inst->inst);
  printf("%s\n\n",inst->string);
  printf("Fields:\n rd: %d\nrs: %d\nrt: %d\nimm: %d\n\n",
	 inst->fields.rd, inst->fields.rs, inst->fields.rt, inst->fields.imm);
  printf("Control Bits:\nalu: %d\nmw: %d\nmr: %d\nmtr: %d\nasrc: %d\nbt: %d\nrdst: %d\nrw: %d\n\n",
	 inst->signals.aluop, inst->signals.mw, inst->signals.mr, inst->signals.mtr, inst->signals.asrc,
	 inst->signals.btype, inst->signals.rdst, inst->signals.rw);
  // if we are using the alu result, print out the alu output
  if (inst->signals.mtr == 0)
    printf("ALU Result: %d\n\n",inst->aluout);
  else
    printf("ALU Result: X\n\n",inst->aluout);
  // if we are using the mem result, print out the mem value
  if (inst->signals.mr == 1)
    printf("Mem Result: %d\n\n",inst->memout);
  else
    printf("Mem Result: X\n\n");
  for(i=0;i<8;i++)
    {
      for(j=0;j<32;j+=8)
	printf("$%d: %4d ",i+j,regfile[i+j]);
      printf("\n");
    }
  printf("\n");
}

