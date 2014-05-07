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
  InstInfo* nop = malloc(sizeof(*nop));
  nop->inst = 31;
  int nopnopbool = 0;
  int nopnopnext = 0;
  int nopnopnext1=0;
  int nopnopnext2=0;
  int nopnopnext3=0;
  while (i<=maxpc+4) {
    decode(d);
    
    if(nopnopnext1 ==1){
      InstInfo* nullInfo = malloc(sizeof(*nullInfo));
      //maxpc++;
      i--;
      *w = *m;
      *m = *x;
      *x = *nullInfo;
      //nopnopnext=0;
      nopnopnext1=0;
    }
    writeback(w); 
    execute(x);
    memory(m);
    setPCWithInfo(w);  
    if(nopnopnext1==1)
      {
	nopnopnext=1;
	nopnopnext1=0;
      }
    if(nopnopnext2==1)
      {
	nopnopnext1=1;
	nopnopnext2=0;
      }
    if(nopnopnext3==1)
      {
	nopnopnext2=1;
	nopnopnext3=0;
      }
    printP2(f,d,x,m,w,instnum++); 
    //continue;
    i++;
    /* printP2(f,d,x,m,w,instnum++); */
    int memoryResult = m->aluout;//w->memout;
    int executeResult=x->aluout;
    int forwardA=0, forwardB=0;
    int xRegWrite = x->signals.rw;
    int mRegWrite = m->signals.rw;
    int wRegWrite = m->signals.rw;
    int wrd = w->destreg;
    int mrd = m->destreg;
    int xrd = x->destreg;
    int drs = d->sourcereg;
    int drt = d->targetreg;
    /*
      if (wRegWrite == 1 && wrd != 0 && (wrd == drs)) { 
      forwardA=2; 
      //     else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drs)) {
      forwardA= 1;
      }
      //else {
      if (xRegWrite == 1 && xrd !=0 && (xrd==drs)) {
      forwardA = 3;
      // }
      // }
      }
      }
      
      
      if (wRegWrite == 1 && wrd != 0 && (wrd == drt)) { 
      forwardB=2; 
      //else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drt)) {
      forwardB= 1;
      }
      //else {
      if (xRegWrite == 1 && xrd !=0 && (xrd==drt)) {
      forwardB = 3;
      //}
      //     }
      }
      }88888888888888*/
    if (xRegWrite == 1 && xrd != 0 && (xrd == drs)) { 
      forwardA=2; }
    else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drs)) {
	forwardA= 1;
      }
      else {
	if (wRegWrite == 1 && wrd !=0 && (wrd==drs)) {
	  forwardA = 3;
	}
      }
    }
    
    if (xRegWrite == 1 && xrd != 0 && (xrd == drt)) { 
      forwardB=2; }
    else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drt)) {
	forwardB= 1;
      }
      else {
	if (wRegWrite == 1 && wrd !=0 && (wrd==drt)) {
	  forwardB = 3;
	}
      }
    }
    
    switch (forwardA) {
    case 2: //from execute
      d->input1 = executeResult;
      break;
    case 1: //from mem
      d->input1 = memoryResult;
      break;
    case 3:
      d->input1 = w->destdata;
      break;
    default:
      break;
    }
    
    switch (forwardB) {
    case 2:
      d->input2 = executeResult;
      break;
    case 1:
      d->input2 = memoryResult;
      break;
    case 3:
      d->input2 = w->destdata;
      break;
    default:
      break;
    }
    if (nopnopnext1!= 1) 
      {*w=*m; *m=*x; *x=*d; *d=*f;}
    
    if (i <= maxpc) {
      fetch(f);
      decode(f);
    }
    else {
      *f = *n;
    }
    
    if (d->signals.mr == 1 && (d->fields.rt == f->sourcereg || d->fields.rt == f->targetreg)) {
      //printf("nop nop who is tehre\n");
      nopnopbool = 1;
      //InstInfo* nullInfo = malloc(sizeof(*nullInfo));
      nopnopnext3=1;
      //execute(d);
      // *f = *f;
      //*d = *d;
      //*x=*x;
      //  printP2(f,d,nullInfo,nullInfo,nullInfo,instnum++); }
    }
    else
      nopnopnext=0;
    // printf("-------------------------------------------\n");
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
