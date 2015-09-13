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

int Process_Root(Crystal_Compiler* comp, const char* rootdir)
{
  //Set the root for other functions to use
  unsigned size = strlen(rootdir);
  CRY_BASE = strcpy(new char[size + 2], rootdir);
  if(CRY_BASE[size - 1] != '\\')
  {
    CRY_BASE[size] = '\\';
    CRY_BASE[size + 1] = '\0';
  }
  CRY_ROOT = CRY_BASE;

  //Create the interpreter
  Crystal_Interpreter interpreter(comp);
  boost::filesystem::path root(rootdir);
  int Files_Read = 0;

  if(!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root))
  {
    printf("CRYSTAL ERROR: Root path is not a directory. Crystal must be pointed to a directory to run. \n\n");
    return 1;
  }

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

  interpreter.Interpret();
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

  if(argc > 1)
  {
    Crystal_Compiler comp(new x86_Machine);

    if(Process_Root(&comp, argv[1]))
    {
      printf("CRYSTAL ERROR: Unable to process the project root directory.\n");
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
    printf("USAGE: CRYSTAL directory\n");
    printf("  directory: The root directory of the target crystal project. \n\n");
    printf("Compiles and runs a crystal program for a collection of crystal source code.\n");
    printf("Source code will be pulled for the target directory and aggragated for use.\n\n");
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
