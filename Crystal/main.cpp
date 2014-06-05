#include <stdio.h>

#include "Compiler.h"
#include "Lexicon.h"

//Avalible Machines
#include "Machines\x86_Machine.h"

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

  //char* memory = "Hello World";
  //char* memory2 = (char*)malloc(20);

  Crystal_Symbol x = Crystal_Symbol();

  Crystal_Compiler comp(new x86_Machine);
  //==========================
  // Main
  //==========================
  
  //Testing Garbage Collection
  //comp.Start_Encode("main", 1);
  //comp.Load(0, "Hello World val: ");
  //comp.Load(comp.Addr_Reg(CRY_R0), 3.98);
  //comp.MulC(comp.Addr_Reg(CRY_R0), 5.2);
  //comp.Add(0, comp.Addr_Reg(CRY_R0));
  //comp.Load(comp.Addr_Reg(CRY_R1), " Crystal Clear");
  //comp.Add(0, comp.Addr_Reg(CRY_R1));
  //comp.Print(0);
  //comp.Return(0);
  //comp.End_Encode();

  //Standard Integration Test
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
  //String Testing
  comp.Load(comp.Addr_Reg(CRY_R0), "Str Test: ");
  comp.Add(comp.Addr_Reg(CRY_R0), 3);
  comp.Add(comp.Addr_Reg(CRY_R0), comp.Addr_Reg(CRY_R1));
  comp.Print(comp.Addr_Reg(CRY_R0));
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
  printf("Huzzah! Variable returned in x: \n");
  Crystal_Print(&x);
  getchar();
  return 0;
}
