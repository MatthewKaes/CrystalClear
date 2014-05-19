#include <stdio.h>

#include "Compiler.h"
#include "Lexicon.h"

Crystal_Symbol create()
{
  Crystal_Symbol x;
  x.i32 = 10;
  x.type = CRY_INT;
  return x;
}

void Int_Mul(Crystal_Symbol* y, Crystal_Symbol* x)
{
  y->i32 *= x->i32;
}

int __cdecl main(int argc, char **argv)
{
  //const char* text = "\"word mate\" 2.0 + 4 . ";
  //Crystal_Data read;
  //do{
  //  read = Create_Symbol(&text);
  //}while(read.str.size());
  
  Crystal_Symbol x = Crystal_Symbol();
  //Crystal_Symbol z;
  //Crystal_Symbol* y = &z;
  //y->LOWWER = x.LOWWER;
  //y->UPPER = x.UPPER;
  //y->type = x.type;

  const char* wor = "Hello world";
  const char* var = wor;

  Crystal_Compiler comp;
  //==========================
  // Main
  //==========================
  comp.Start_Encode("main", 5);
  comp.Load(0, 14);
  comp.Load(1, 20);
  comp.Load(2, 105.6);
  comp.Load(4, "Hello World");
  comp.Copy(3 , 0);
  comp.Mul(3, 1);
  comp.Mul(3, 2);
  comp.MulC(3, -0.0054);
  comp.AddC(3, -10);
  //System Print
  comp.Print(4);
  comp.Print(0);
  comp.Print(1);
  comp.Print(2);
  comp.Print(3);
  //Manual Print call.
  comp.Push(1);
  comp.Push(0);
  comp.Call(Int_Mul);
  comp.Pop(2);
  comp.Call("mag", comp.Addr_Reg(CRY_R0));
  //Return!
  comp.Print(comp.Addr_Reg(CRY_R0));
  //comp.Copy(0, comp.Addr_Reg(CRY_R0));
  comp.Return(comp.Addr_Reg(CRY_R0));
  comp.End_Encode();
  //==========================
  // mag
  //==========================
  comp.Start_Encode("mag", 1);
  comp.Load(0, "In sub function");
  //System Print
  //comp.Print(0);
  //Return!
  comp.Return(0);
  comp.End_Encode();
  //========================
  comp.Linker();
  comp.Execute(&x);
  printf("Huzzah! Variable 3 returned in x: \n");
  Crystal_Print(&x);
  printf("%d, %x\n", sizeof(Crystal_Symbol), sizeof(Crystal_Symbol));
  printf("%d, %x\n", sizeof(Crystal_Symbol*), sizeof(Crystal_Symbol*));
  printf("%d, %x\n", sizeof(double), sizeof(double));
  printf("%d, %x\n", 0x30, 0x30);
  getchar();
  return 0;
}