#include <stdio.h>
#include <list>
#include <boost\filesystem.hpp>
#include <boost\date_time.hpp>

#include "Compiler.h"
#include "Interpreter.h"
#include "Lexicon.h"
#include "Helper.h"
#include "Garbage_Collector.h"

#if INCLUDE_PYTHON
#include <boost\python.hpp>
#endif

//Avalible Machines
#include "Machines\x86_Machine.h"

static char* CRY_BASE = 0;
const char* CRY_ROOT = 0;

extern GC global_gc;
extern std::vector<Class_Info*> Class_Listing;

typedef void (*CRY_EXPORT)();

void Usage()
{
  printf("USAGE: CRYSTAL target [--c 'output']\n");
  printf("  target  : The target of the crystal compiler. \n");
  printf(" [output] : If provided, a compiled Crystal File will be produced. \n");
  printf("            An output file can only be specified if target is a directory.\n");
  printf("Compiles and runs a crystal program for a collection of crystal source code.\n");
  printf("If the target is a directory source code will be pulled form the target directory\n");
  printf("and aggragated for use. If the target is a compiled crystal file then the crystal\n\n");
  printf("the program is used for the entire execution.\n");
}

int Process_Root(Crystal_Compiler* comp, const char* target)
{
  if (!boost::filesystem::exists(target))
  {
    printf("CRYSTAL ERROR: Try target is does not exist. The target must be a directory or a \".crystal\" file.\n\n");
  }

  if (boost::filesystem::is_directory(target))
  {
    //Set the root for other functions to use
    unsigned size = strlen(target);
    CRY_BASE = strcpy(new char[size + 2], target);
    if(CRY_BASE[size - 1] != '\\')
    {
      CRY_BASE[size] = '\\';
      CRY_BASE[size + 1] = '\0';
    }
    CRY_ROOT = CRY_BASE;

    //Create the interpreter
    boost::filesystem::path root(target);
    Crystal_Interpreter interpreter(comp);

    int Files_Read = 0;
    // Load all of the files into memory.
    for (boost::filesystem::directory_iterator it(root); it != boost::filesystem::directory_iterator(); ++it)
    {
      if(boost::filesystem::is_regular_file(it->path()))
      {
        if(!it->path().extension().compare(std::string(".cry")))
        {
          Files_Read++;
          interpreter.Cache_Code(it->path().generic_string().c_str());
        }
      }
    }
    
    if(!Files_Read)
    {
      printf("CRYSTAL ERROR: Root project empty. The Root project must contain one or more files. \n\n");
      return 1;
    }

    // Interpret what the execution is and what the code does.
    interpreter.Interpret();

    // Link together all of the interpreted packages.
    comp->Linker();
  }
  else
  {
    comp->Read_Binary(target);
  }

  return 0;
 }

int main(int argc, const char **argv)
{

  //Set up random
  srand(static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds()));
  
  //Set up Python
#if INCLUDE_PYTHON
  Py_Initialize();
#endif

  if(argc > 1 && strcmp(argv[1], "\?"))
  {
    Crystal_Compiler comp(new x86_Machine);

    if(Process_Root(&comp, argv[1]))
    {
      printf("CRYSTAL ERROR: Unable to process the project root directory.\n");
      return 1;
    }
    
    //========================
    //If requested use the linker to write out a "crystal" file.
    //========================
    if(argc == 4 && (!strcmp(argv[2], "--c") || !strcmp(argv[2], "--C")))
    {
      comp.Write_Binary(argv[3]);
      return 0;
    }
    
    //========================
    //Execute packages and return terminating crystal
    //========================
    //variable in x
    Crystal_Symbol exit_sym;
    memset(&exit_sym, 0, sizeof(exit_sym));

    std::vector<Crystal_Symbol> crystal_args;
    for (int i = 2; i < argc; i++)
    {
      Crystal_Symbol var;
      var.type = CRY_TEXT;
      var.text = argv[i];
      crystal_args.push_back(var);
    }

    if (comp.Execute(&exit_sym, &crystal_args))
    {
      printf("CRYSTAL ERROR: could not file entry point.\nA file needs to include a \"main\" package.\n");
      return 1;
    }

    //Print the exit sym
    printf("\nCrystal Clear exited with symbol:\n");
    Crystal_Print(&exit_sym, &exit_sym);
  }
  else
  {
    Usage();
  }  

  //Clean up code

  //Release Python
#if INCLUDE_PYTHON
  Py_Finalize();
#endif
  //Free extensions
  for(unsigned i = 0; i < Crystal_Interpreter::Extension_Libs.size(); i++)
  {          
    CRY_EXPORT release = (CRY_EXPORT)GetProcAddress(Crystal_Interpreter::Extension_Libs[i], "CrystalRelease");
    if(release)
    {
      //Call the CrystalRelease function which contains all
      //of the clean up logic for the dll
      release();
    }
    FreeLibrary(Crystal_Interpreter::Extension_Libs[i]);
  }

  //Free Class Objects
  for(unsigned i = 0; i < Class_Listing.size(); i++)
  {
    delete Class_Listing[i];
  }

  delete[] CRY_BASE;
}
