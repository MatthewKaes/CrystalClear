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
  
  //Test pre package
  //==========================
  // GC_test Package
  //==========================
  //Testing Garbage Collection
  comp.Start_Encode("GC_test", 1);
  //Print Package Name
  comp.Load(comp.Addr_Reg(CRY_R3), "\nPACKAGE: GC_test");
  comp.Print(comp.Addr_Reg(CRY_R3));
  //Preform some string manpulations
  comp.Load(0, "Hello World val: ");
  comp.Load(comp.Addr_Reg(CRY_R0), 3.98);
  comp.MulC(comp.Addr_Reg(CRY_R0), 5.2);
  comp.Add(0, comp.Addr_Reg(CRY_R0));
  comp.Load(comp.Addr_Reg(CRY_R1), " in Crystal Clear");
  comp.Add(0, comp.Addr_Reg(CRY_R1));
  comp.Print(0);
  //Test Cleanup
  comp.Load(comp.Addr_Reg(CRY_R3), "END PACKAGE: GC_test\n");
  comp.Print(comp.Addr_Reg(CRY_R3));
  comp.Return(0);
  comp.End_Encode();

  //Test entry point
  //==========================
  // Main
  //==========================
  //Standard Integration Test
  comp.Start_Encode("main", 5);
  //Print Package Name
  comp.Load(comp.Addr_Reg(CRY_R3), "\nPACKAGE: main");
  comp.Print(comp.Addr_Reg(CRY_R3));
  //Loading diffrent values
  comp.Load(0, 14);
  comp.Load(1, 20);
  comp.Load(2, 105.6);
  comp.Load(4, "Hello World");
  //Copying one value to another.
  comp.Copy(3 , 0);
  //Math with symbols also testing for dynamics
  comp.Mul(3, 1);
  comp.Mul(3, 2);
  //Testing math with constants
  comp.MulC(3, -0.0054);
  comp.AddC(3, -10);
  //System Print
  //Testing that diffrent types print properly
  comp.Print(4);
  comp.Print(0);
  comp.Print(1);
  comp.Print(2);
  comp.Print(3);
  //String Testing
  comp.Load(comp.Addr_Reg(CRY_R0), "Str Test: ");
  comp.Add(comp.Addr_Reg(CRY_R0), 3);
  comp.AddC(comp.Addr_Reg(CRY_R0), " adding const str ");
  comp.Add(comp.Addr_Reg(CRY_R0), comp.Addr_Reg(CRY_R1));
  comp.AddC(comp.Addr_Reg(CRY_R0), " adding const val ");
  comp.AddC(comp.Addr_Reg(CRY_R0), 317000);
  comp.Print(comp.Addr_Reg(CRY_R0));
  //Load an arbitrary constant
  comp.Load(comp.Addr_Reg(CRY_R0), 420.50);
  comp.AddC(comp.Addr_Reg(CRY_R0), " added to a test string!");
  comp.Print(comp.Addr_Reg(CRY_R0));
  //Simple compile time Clear Hook
  comp.Push(1);
  comp.Push(0);
  comp.Call(Int_Mul);
  comp.Pop(2);
  //Call simple package and store the return value
  //In Crystal Register 0
  comp.Call("simple", comp.Addr_Reg(CRY_R0));
  //Test naked package call negating return value
  comp.Call("GC_test");
  //Return!
  comp.Print(comp.Addr_Reg(CRY_R0));
  comp.Return(comp.Addr_Reg(CRY_R0));
  comp.End_Encode();

  //Test post package
  //==========================
  // simple Package
  //==========================
  comp.Start_Encode("simple", 1);
  //Print Package Name
  comp.Load(comp.Addr_Reg(CRY_R3), "\nPACKAGE: simple");
  comp.Print(comp.Addr_Reg(CRY_R3));
  //Load a value and test returning
  comp.Load(0, "In sub function");
  //End package
  comp.Load(comp.Addr_Reg(CRY_R3), "END PACKAGE: Simple\n");
  comp.Print(comp.Addr_Reg(CRY_R3));
  comp.Return(0);
  comp.End_Encode();

  //========================
  //Link all packages
  comp.Linker();
  //Execute packages and return terminating crystal
  //variable in x
  comp.Execute(&x);
  printf("Huzzah! Variable returned in x: \n");
  Crystal_Print(&x);
  getchar();
  return 0;
}
