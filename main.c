#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "functions.h"

void doStuff(InstInfo* f, InstInfo* d, InstInfo*x, InstInfo* m, InstInfo* w);
//int getRS(InstInfo* i) { return ( (i==NULL) ? -1 : i->

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
    decode(d);
    execute(x);
    memory(m);
    writeback(w); 
    int F = (f==NULL) ? -1 : f->signals.rw;
    //printf("%d, ", F);
    int D = (d==NULL) ? -1 : d->signals.rw;
    //printf("%d, ", D);
    int X = (x==NULL) ? -1 : w->signals.rw;
    //printf("%d, ", X);
    int M = (m==NULL) ? -1 : m->signals.rw;
    //printf("%d, ", M);
    int W = (w==NULL) ? -1 : w->signals.rw;
    //printf("%d, \n", W);
    //printf("prelim\n");
    //printf("F: %d D: %d X: %d M: %d W: %d \n", F, D, X, M, W);
    // printf("m) rd: %d rs %d rt %d\n", m->fields.rd, m->fields.rs, m->fields.rt);
    //printf("m) input1: %d input2: %d rt %d\n", m->input1, m->input2, m->fields.rt);
    //setPCWithInfo(w);  
    printP2(f,d,x,m,w,instnum++);
    int memoryResult = w->memout;
    //printf("memoryResult %d, ", memoryResult);
    int executeResult=x->aluout;
    //printf("executeResult %d, ", executeResult);
    int forwardA=0, forwardB=0;
    //printf("%d, ", F);
    int xRegWrite = x->signals.rw;
    //printf("xRegWrite %d, ", xRegWrite);
    int mRegWrite = m->signals.rw;
    //printf("mRegWrite%d, ", mRegWrite);
    int mrd = m->destreg;//fields.rd;
    //printf("mrd %d, ", mrd);
    int xrd = x->destreg;//fields.rd;
    //printf("xrd %d, ", xrd);
    int drs = d->sourcereg;//fields.rs;
    //printf("drs %d, ", drs);
    int drt = d->targetreg;//fields.rt;
    // printf("drt %d, ", drt);
    printf("input1: %d, input2: %d,  aluout: %d, memout: %d\n", x->input1, x->input2, x->aluout, m->memout);
    //printf("xRegWrite: %d mRegWrite: %d mrd: %d xrd: %d drs: %d drt: %d\n", xRegWrite);
    printf("xRegWrite(%d) ==? 1 && xrd(%d) !=? 0 && (xrd(%d) ==? drs(%d))\n", xRegWrite, xrd, xrd, drs);
    if (xRegWrite == 1 && xrd != 0 && (xrd == drs)) { 
      forwardA=2; 
      printf("aluop 1 from prev alu result\n!!!!");}
    
    printf("xRegWrite(%d) ==? 1 && xrd(%d) !=? 0 && (xrd(%d) ==? drt(%d))\n", xRegWrite, xrd, xrd, drt);
    
    if (xRegWrite == 1 && xrd != 0 && (xrd == drt)) { 
      forwardB=2; 
      printf("aluop 2 from prev alu result\n");}
    
    printf("(mRegWrite(%d) ==? 1 && mrd(%d) !=? 0 && !(xRegWrite(%d) ==? 1 && xrd(%d)!=? 0 )) && xrd(%d) != drs(%d) && (mrd(%d) ==? drs(%d))\n", mRegWrite, mrd, xRegWrite, xrd, xrd, drs, mrd, drs); 
    if ((mRegWrite ==1 && mrd != 0 && !(xRegWrite == 1 && xrd!= 0 )) && xrd!=drs && (mrd == drs)){ forwardA=1; printf("aluop 1from prev mem result\n");}
 
    printf("(mRegWrite(%d) ==? 1 && mrd(%d) !=? 0 && !(xRegWrite(%d) ==? 1 && xrd(%d)!=? 0 )) && xrd(%d) != drt(%d) && (mrd(%d) ==? drt(%d))\n", mRegWrite, mrd, xRegWrite, xrd, xrd, drt, mrd, drt); 
    if ((mRegWrite ==1 && mrd != 0 && !(xRegWrite == 1 && xrd!= 0 )) && xrd!=drt && (mrd == drt)){ forwardB=1; printf("aluop 2 from prev mem result\n");}
    printf("A %d, B %d \n", forwardA, forwardB);
    switch (forwardA) {
    case 2:
      d->input1 = executeResult;
      printf("input 1 is alu: %d \n", d->input1);
      break;
    case 1:
      d->input1 = memoryResult;
      printf("input 1 is mem: %d \n", d->input1);
      break;
    default:
      if (sourcereg > 0) { printf("source reg %d: %d\n", d->sourcereg, regfile[d->sourcereg]); }
      printf("input 1 is reg %d: %d\n", d->sourcereg, d->s1data);
      break;
    }

    switch (forwardB) {
    case 2:
      d->input2 = executeResult;
      printf("input 2 is alu: %d \n",d->input2);
      break;
    case 1:
      printf("input 2 is mem: %d\n",d->input2);
      d->input2 = memoryResult;
      break;
    default:
      printf("input 2 is reg %d: %d\n", d->targetreg,d->s2data);
      break;
    }
    //setPCWithInfo(w);
    *w=*m; *m=*x; *x=*d; *d=*f;
    if (i <= maxpc) {
      fetch(f);
      decode(f);
    }
    else {
      *f = *n;
    }
    printf("-------------------------------------------\n");
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
