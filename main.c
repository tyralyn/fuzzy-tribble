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
  int nopnop=0; 
  int MAXPC4 = maxpc + 4;
  int k = 0;
  int zapzap=0;
  int setPC;
  while (i<=MAXPC4) {

    writeback(w);
    memory(m);
    execute(x);
    decode(d);
    setPCWithInfo(w);  
    printP2(f,d,x,m,w,instnum++); 

    printf("-------------------------------------------------\n");
    //printf("PC %d \n", pc);
    int memoryResult = m->memout;//w->memout;
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
    int dmr = x->signals.mr;
    int drd = x->destreg;
    int frs = d->sourcereg;
    int frt = d->targetreg;
    int dBType = f->signals.btype;
    int zapped;
    //if (zapzap == 2) {
      
    //}
    if (zapzap == 1) {
      *w = *m;
      *m = *x;
      *x =*d;
      *d = *nop;
      //i++;
      //k++;
      printf("IN XAAPZAP\n");
      //pc = setPC;
      zapzap=0;
      zapped = 1;
    }
    //    int zapped = 1;
    int guessedNotTaken = pc+1;
    if (dBType == 1 || dBType ==2 || dBType == 3) {
      if (guessedNotTaken != d->aluout) {
	zapzap=1;
	int setPC = d->aluout;
	//setPCWithInfo(d);
      }
    } 

    printf("PC %d , setPC: %d \n", pc, setPC);
    printf("ZAPZAP %d btype %d \n",zapzap, dBType);//zapzap);
    
    // int k = 0;

    //printf("memorytest %d\n", memoryResult);
    if (dmr == 1 && ( drd == frs || drd ==frt) ) {
      nopnop = 1;
      //printf("SETTING NOPNOP! %d\n", nopnop);
    }
    else {
      //printf("NOT SETTING NOPNOP %d\n", nopnop);
    }    
    if (wRegWrite == 1 && wrd != 0 && (wrd == drs)) { 
      forwardA=3; 
      
      //printf("3a %d \n", regfile[wrd]);
    }
    else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drs)) {
    forwardA= 1;
    //printf("1a %d \n", memoryResult);//regfile[mrd]);
      }
      else {
    if (xRegWrite == 1 && xrd !=0 && (xrd==drs)) {
      //printf("2a %d \n", regfile[xrd]);
      forwardA = 2;
    }
      }
    }
    
    if (wRegWrite == 1 && wrd != 0 && (wrd == drt)) { 
      forwardB=3; 
      //printf("3b  %d \n", regfile[wrd]);
    }
    else {
      if (mRegWrite == 1 && mrd !=0 && (mrd == drt)) {
    //printf("1b %d \n", regfile[mrd]);
    forwardB= 1;
      }
      else {
    if (xRegWrite == 1 && xrd !=0 && (xrd==drt)) {
      forwardB = 2;
      //printf("2b  %d \n", regfile[xrd]);
    }
      }
    }
    
    switch (forwardA) {
    case 2: //from execute
      d->input1 = executeResult;
      break;
    case 1: //from mem 
      d->input1 = m->memout;//memoryResult;
      break;
    case 3:
      d->input1 = w->destdata;
      break;
    default:
      break;
    }
    // printf("D %d input1: %d\n", forwardA, d->input1);
    
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
    //printf("D inputs: %d %d\n", d->input1, d->input2);
    // decode(d);
    //writeback(w); 
    // memory(m);
    //decode(d);
    // execute(x);
    //decode(d);
    //memory(m);
    //setPCWithInfo(w);  
    //printP2(f,d,x,m,w,instnum++); 
    i++;
    //d->input1=
    //printf("D inputs: %d %d\n", d->input1, d->input2);
    //decode(d);
    /*if (dBType == 1 || dBType ==2) {
      
      i--;
      k++;
      setPCWithInfo(d);
    }*/     
    //else {
    //    if (dBType == 0) {
    if (nopnop== 0){// & zapzap != 1)  { 
      printf("NO NOP!\n");
      if (zapzap==0)
	*w=*m; *m=*x; *x=*d; *d=*f;
    }
    else {
      //printf("NO NOP\n");
      *w = *m;
      *m = *x;
      *x = *nop;
      //*w=*m; *m=*x; *x=*d; *d=*f;
      i--;
      k++;
      pc--;
    }
    //}
    // if (zapped ==1) {
    //pc = setPC;
    //}
    //}
    if (i <= maxpc){// && dBType ==0) {
      fetch(f);
      decode(f);
    }
    else {
      *f = *n;
    }
    nopnop = 0;
    //    zapzap=0;
  }
  printf("Cycles: %d\nInstructions Executed: %d\n", i+k, maxpc+1);
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
