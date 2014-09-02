#include <stdio.h>
#include <boost\filesystem.hpp>

#include "Compiler.h"
#include "Interpreter.h"
#include "Lexicon.h"

//Avalible Machines
#include "Machines\x86_Machine.h"

int Process_Root(Crystal_Compiler* comp, const char* rootdir)
{
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
