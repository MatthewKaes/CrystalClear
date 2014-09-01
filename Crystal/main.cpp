#include <stdio.h>

#include "Compiler.h"
#include "Interpreter.h"
#include "Lexicon.h"

//Avalible Machines
#include "Machines\x86_Machine.h"

int Process_Root(Crystal_Compiler* comp, const char* rootdir)
{
    Crystal_Interpreter interpreter(comp);
    HANDLE dirhand;
    WIN32_FIND_DATAA data;
     
    SecureZeroMemory(&dirhand, sizeof(HANDLE));
    SecureZeroMemory(&data, sizeof(WIN32_FIND_DATA));
     
    int Files_Read = 0;
     
    dirhand = FindFirstFileA(rootdir, &data);
    bool processing = (dirhand != INVALID_HANDLE_VALUE);
     
    while(processing)
    {
		  ++Files_Read;
		  if(strcmp((char*)data.cFileName, ".") && strcmp((char*)data.cFileName, ".."))
		  {
        interpreter.Cache_Code((char*)data.cFileName);
		  }
     
		  int cont = FindNextFileA(dirhand, &data);
		  processing = (cont != 0);
    }
    if(!Files_Read)
    {
      printf("CRYSTAL ERROR: Root project empty. The Root project must contain one or more files. \n\n");
      return 1;
    }
    interpreter.Interpret();
    return 0;
 }

int main(int argc, const char **argv)
{
  if(argc > 1)
  {
    Crystal_Compiler comp(new x86_Machine);

    if(Process_Root(&comp, argv[1]))
    {
      return 1;
    }
    
    //========================
    //Link all packages
    //========================
    comp.Linker();
    
    //========================
    //Execute packages and return terminating crystal
    //========================
    //variable in x
    Crystal_Symbol exit_sym;
    if(comp.Execute(&exit_sym))
    {
      printf("CRYSTAL ERROR: could not file entry point.\nAt least one file needs to include a \"main\" package.\n");
    }

    //Print the exit sym
    Crystal_Print(&exit_sym);
  }
  else
  {
    printf("USAGE: CRYSTAL directory\n");
    printf("  directory: The root directory of the target crystal project. \n\n");
    printf("Compiles and runs a crystal program for a collection of crystal source code.\n");
    printf("Source code will be pulled for the target directory and aggragated for use.\n\n");
  }
}
