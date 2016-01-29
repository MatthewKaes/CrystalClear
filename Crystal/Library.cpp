#include "Linker.h"
#include <windows.h>

#include "Native.h"
#include "Function.h"
#include "Filesystems.h"
#include "Helper.h"
#include "Garbage_Collector.h"
#include "Obscure.h"
#include "Core.h"
#include "Math.h"
#include "OS.h"
#include "IO.h"


#define REGISTER_SUPPORT(function) supported_functions[#function] = function;


#define REGISTER_FUNCTION(Name, Func, Args, Info, Ret, ...) \
    { Package_Info new_package; \
      new_package.pt = PKG_EXE; \
      new_package.info.arguments = Args; \
      new_package.function = Func; \
      new_package.name = #Func; \
      new_package.discript = #Info; \
      new_package.returndis = Ret; \
      new_package.lookup = #Name; \
      new_package.argtext = { __VA_ARGS__ }; \
      built_in[#Name] = new_package; \
      supported_functions[#Func] = Func; \
    } 

Crystal_Library::Crystal_Library()
{

  // Standard Library
  REGISTER_SUPPORT(memset);
  REGISTER_SUPPORT(calloc);
  REGISTER_SUPPORT(malloc);

  // Linking Support Functionallity
  REGISTER_SUPPORT(Copy_Ref);
  REGISTER_SUPPORT(This_Copy);
  REGISTER_SUPPORT(Stack_Copy);
  REGISTER_SUPPORT(Parse_Int);
  REGISTER_SUPPORT(Parse_Bool);
  REGISTER_SUPPORT(Parse_Double);
  REGISTER_SUPPORT(Parse_String);
  REGISTER_SUPPORT(Fast_strcmp);
  REGISTER_SUPPORT(Fast_pointercmp);
  REGISTER_SUPPORT(Late_Func_Binding);
  REGISTER_SUPPORT(Late_Attr_Binding);
  REGISTER_SUPPORT(Late_Func_Binding_Ref);
  REGISTER_SUPPORT(Late_Attr_Binding_Ref);

  // Linking Complex Support Functionallity
  REGISTER_SUPPORT(Construct_Class);
  REGISTER_SUPPORT(Construct_Array);
  REGISTER_SUPPORT(Construct_String);
  REGISTER_SUPPORT(Array_Add_Nil);
  REGISTER_SUPPORT(Push_Int);
  REGISTER_SUPPORT(Push_Double);
  REGISTER_SUPPORT(Push_Text);
  REGISTER_SUPPORT(Val_Binding);

  // Linking Garbage Collection
  REGISTER_SUPPORT(GC_Branch);
  REGISTER_SUPPORT(GC_Extend_Generation);
  REGISTER_SUPPORT(GC_Collect);

  // Linking Dynamic Math Support
  REGISTER_SUPPORT(Obscure_Addition);
  REGISTER_SUPPORT(Obscure_Subtraction);
  REGISTER_SUPPORT(Obscure_Multiplication);
  REGISTER_SUPPORT(Obscure_Division);
  REGISTER_SUPPORT(Obscure_Modulo);
  REGISTER_SUPPORT(Obscure_Power);
  REGISTER_SUPPORT(Obscure_Equal);
  REGISTER_SUPPORT(Obscure_Diffrence);
  REGISTER_SUPPORT(Obscure_Less);
  REGISTER_SUPPORT(Obscure_Greater);
  REGISTER_SUPPORT(Obscure_Less_Equal);
  REGISTER_SUPPORT(Obscure_Greater_Equal);
  REGISTER_SUPPORT(Obscure_AdditionR);
  REGISTER_SUPPORT(Obscure_SubtractionR);
  REGISTER_SUPPORT(Obscure_DivisionR);
  REGISTER_SUPPORT(Obscure_ModuloR);
  REGISTER_SUPPORT(Obscure_PowerR);

  Populate_Built_In();
}

void Crystal_Library::Populate_Built_In()
{
  //Define all executable packages
  REGISTER_FUNCTION(print, Crystal_Print, 1, Prints out a line of text., "INTEGER The number of characters printed.", "VALUE The value to print out.");
  REGISTER_FUNCTION(type, Crystal_Type, 1, Returns a string value of the object type., "STRING The string representation of the type.", "VALUE A value to check the type of.");
  REGISTER_FUNCTION(convert, Crystal_Convert, 2, Converts a value into another type., "VALUE The converted value.", "VALUE The value to be converted.", "STRING The string representation of the type. EX: \"INTEGER\", \"STRING\".");
  REGISTER_FUNCTION(boolean, Crystal_Boolean, 1, Converts a value into a boolean., "BOOLEAN The converted value.", "VALUE The value to be converted.");
  REGISTER_FUNCTION(integer, Crystal_Integer, 1, Converts a value into an integer., "INTEGER The converted value.", "VALUE The value to be converted.");
  REGISTER_FUNCTION(double, Crystal_Double, 1, Converts a value into a double., "DOUBLE The converted value.", "VALUE The value to be converted.");
  REGISTER_FUNCTION(string, Crystal_String, 1, Converts a value into a string., "STRING The converted value.", "VALUE The value to be converted.");
  REGISTER_FUNCTION(print_color, Crystal_PrintColor, 2, Prints out a line of text with a given color., "INTEGER The number of characters printed.", "VALUE The value to print out.", "INTEGER The color value to use. (OS Dependant)");
  REGISTER_FUNCTION(rand, Crystal_Rand, 1, Generates a random value upto but excluding the given number., "INTEGER A randomly generated integer between 0 and the passed value minus one.", "INTEGER One More then the max value to be generated.");
  REGISTER_FUNCTION(time, Crystal_Time, 0, Returns the current CPU time in miliseconds., "INTEGER The current CPU time in miliseconds.");
  REGISTER_FUNCTION(input, Crystal_Input, 0, Halts input to get a line of input from the user in the form of a string., "STRING The line of text entered in by the user.");
  REGISTER_FUNCTION(size, Crystal_Size, 1, Returns the size of a given container., "INTEGER The number of objects in the root of the container.", "ARRAY The container you want the size of.");
  REGISTER_FUNCTION(clone, Crystal_Clone, 1, Copies a container or object and preserves all of the references inside., "VALUE The cloned object. If the object contains any references the object will be referenced by both the cloned object and the original.", "VALUE The object that you want to clone.");
  REGISTER_FUNCTION(fork, Crystal_Fork, 1, Copies a container or object and all references recursively., "VALUE The forked object. If the object contains any references they will also be forked.", "VALUE The object that you want to fork.");

  //Math
  REGISTER_FUNCTION(cos, Crystal_Cos, 1, Takes an angle in radians calculates given cosine., "DOUBLE The calcualed cosine value.", "DOUBLE The value to take the cosine of.");
  REGISTER_FUNCTION(sin, Crystal_Sin, 1, Takes an angle in radians calculates given sine., "DOUBLE The calcualed sine value.", "DOUBLE The value to take the sine of.");
  REGISTER_FUNCTION(log, Crystal_Log, 1, Calcualtes the base 10 log of a value., "DOUBLE The calcualed log value.", "DOUBLE The value to take the log base 10 of.");
  REGISTER_FUNCTION(ln, Crystal_NatrualLog, 1, Calculates the natural lag of a value., "DOUBLE The calcualed log value.", "DOUBLE The value to take the natural log of.");
  REGISTER_FUNCTION(abs, Crystal_Abs, 1, Returns the absolute value of an object., "DOUBLE The associated possitive numerical value of the passed object.", "DOUBLE The value to take the absolute value of.");
  REGISTER_FUNCTION(sum, Crystal_Sum, 1, Calculates the sum of all values in a container., "DOUBLE The sum of all objects in the container.", "ARRAY The container to preform the summation on.");
  REGISTER_FUNCTION(mean, Crystal_Mean, 1, Calculates the mean of all values of a container., "DOUBLE The mean of all objects in the container.", "ARRAY The container to preform the mean operation on.");
  REGISTER_FUNCTION(min, Crystal_Min, 1, Finds the minimum value in a container.Returns the value in the form of a DOUBLE., "DOUBLE The max value in the container.", "ARRAY The container to pull the max value from.");
  REGISTER_FUNCTION(max, Crystal_Max, 1, Finds the max value in a container.Returns the value in the form of a DOUBLE., "DOUBLE The min value in the container.", "ARRAY The container to pull the min value from.");
  REGISTER_FUNCTION(var, Crystal_Var, 1, Calculates the variance of the values in a container., "DOUBLE The variance of values in the container.", "ARRAY The container to check the variance of.");
  REGISTER_FUNCTION(sd, Crystal_Sd, 1, Calculates the standard deviation of the values in a container., "DOUBLE The standard deviation of values in the container.", "ARRAY The container to check the standard deviation of.");
  REGISTER_FUNCTION(step, Crystal_Step, 3, Generates a range from a given start and end value using a step., "ARRAY A container with all of the values in the generated range in order of generation.", "INTEGER Start value in the range to generate.", "INTEGER End value in the range to generate.", "INTEGER The incremental step to use to generate the range.");

  //Input / Output
  REGISTER_FUNCTION(marshal_open, Crystal_MarshalOpen, 1, Opens a file for the serialization of Crystal Symbols., "BOOLEAN True if the file was opened for marshaling.", "STRING The path to the marshal file to open.");
  REGISTER_FUNCTION(marshal_dump, Crystal_MarshalDump, 1, Serializes a Crystal Symbol into an open marshal file., "BOOLEAN True object was marshaled.", "VALUE A value to be seralized by the marshaler.");
  REGISTER_FUNCTION(marshal_load, Crystal_MarshalLoad, 1, Loads all of the symbols in a marshal file in to a container., "ARRAY A container with all of the marshled objects in the order they were marshled.", "STRING The path to the marshal file to load.");
  REGISTER_FUNCTION(marshal_close, Crystal_MarshalClose, 0, Closes and writes to disk the current marshal file., "BOOLEAN True if the file was properly saved to disk.");
  REGISTER_FUNCTION(write_text, Crystal_WriteText, 2, Converts a symbol to a string and appends the text to a file., "BOOLEAN True the text was written correctly to disk.", "STRING The path to the text file to write to.", "STRING The text to be written to the text file.");
  REGISTER_FUNCTION(read_text, Crystal_ReadText, 1, Reads a text file into a string object., "STRING The entire contents of the text file. If the file failed to load an empty string will be returned.", "STRING The path to the text file to read from.");

  //Boost Extensions
  REGISTER_FUNCTION(sleep, Crystal_Sleep, 1, Pauses execution for a provided number of miliseconds., "NIL This function doesn't produce any output.", "INTEGER The number of miliseconds to wait. Multiply values by 1000 to get seconds.");

  //Filesystems
  REGISTER_FUNCTION(make_path, Crystal_MakeDirectory, 1, Creates a directory with the given path.Parent directories will be created as needed., "BOOLEAN True if the path was succesfully created.", "STRING The full path that needs to be created.");
  REGISTER_FUNCTION(remove_path, Crystal_RemovePath, 1, Removes a path from disk.If the path is a directory all files and subdirectories are also removed., "BOOLEAN True if the path was succesfully deleted.", "STRING The path to remove.");
  REGISTER_FUNCTION(copy_path, Crystal_CopyPath, 2, Copies a path to a given location., "BOOLEAN True if the path was successfully copied.", "STRING The sorce file path.", "STRING the destination path.");
  REGISTER_FUNCTION(path_exists, Crystal_PathExists, 1, Checks to see if a path exists on disk, "BOOLEAN True if the path exist on disk.", "STRING The path to check.");
  REGISTER_FUNCTION(is_file, Crystal_IsFile, 1, Checks if a path is a file., "BOOLEAN True if the path exist and is a file.", "STRING The path to check.");
  REGISTER_FUNCTION(is_dir, Crystal_IsDir, 1, Checks if a path is a directory., "BOOLEAN True if the path exist and is a directory.", "STRING The path to check.");
  REGISTER_FUNCTION(file_size, Crystal_FileSize, 1, Provides the size of a given file on disk., "INTEGER The size of the file. Currently supports only up to 4GB sized files.");
  REGISTER_FUNCTION(list_files, Crystal_FileList, 1, Returns a list of all files in directories in a given path in a container., "ARRAY A container containing all of the objects in the provided path.", "STRING The path to examine. If the path doesn't exist an empty list is returned.");
  REGISTER_FUNCTION(cry_root, Crystal_CryRoot, 0, The root directory used for the current execution., "STRING The running directory the application is currently running from.");

  //OS
  REGISTER_FUNCTION(cry_copyright, Crystal_CrystalCopyright, 0, Prints out the Crystal Clear Copyright., "STRING The copyright string for Crystal Clear.");
  REGISTER_FUNCTION(cry_version, Crystal_CrystalVersion, 0, Returns a string of the current Crystal Clear Version., "STRING The three part version of the code base running.");
  REGISTER_FUNCTION(cry_linker, Crystal_CrystalLinker, 0, Returns the version of the Crystal Linker., "STRING The three part version of the linker used. Exported Crystal files only run with compatiable linker versions.");
  REGISTER_FUNCTION(cry_compiler, Crystal_CrystalCompiler, 0, Returns the version of the Crystal Compiler., "STRING The three part version of the compiler used. The built in functionality provided is compiler version dependent.");
  REGISTER_FUNCTION(cry_target, Crystal_CrystalTarget, 0, Returns the target machine Crystal Clear is running on., "STRING The target architecture that this version of Crystal Clear was built for.");
  REGISTER_FUNCTION(help, Crystal_Help, 1, List all commands supported by this version of Crystal Clear., "ARRAY Returns an array containing all of the functions avalible or NIL if a command's help is listed.", "STRING The command to print out help for. If no proper command is passed all commands are listed.");
  REGISTER_FUNCTION(environ, Crystal_Environ, 1, Expand a given enviornment variable if it exist., "STRING The value of the expanded enviornment variable. If the variable doesn't exist an empty string is returned.", "STRING The enviornment variable to expand.");

  //Hooks to other langauges
  REGISTER_FUNCTION(python, Crystal_Python, 2, Runs a python script and translates any return values., "VALUE The returned value from the python script.", "STRING The path to the python script to run.", "STRING The python function to run. If \"nil\" is passed then the GLOBAL function is run.");

  /*
#if _DEBUG
  //Load extensions
  //Get the debug version of the dll.
  boost::filesystem::path externals("Extensions\\Debug\\External");
#else
  //Load extensions
  boost::filesystem::path externals("Extensions\\External");
#endif
  if (boost::filesystem::exists(externals) && boost::filesystem::is_directory(externals))
  {
    for (boost::filesystem::directory_iterator it(externals); it != boost::filesystem::directory_iterator(); ++it)
    {
      if (!it->path().extension().compare(std::string(".dll")))
      {
        wchar_t name[256];
        mbstowcs(name, it->path().string().c_str(), it->path().string().length());
        name[it->path().string().length()] = 0;
        HINSTANCE Crystal_Lib = LoadLibrary(name);
      }
    }
  }

#if USE_CRYSTAL_EXTENSIONS
#if _DEBUG
  //Load extensions
  //Get the debug version of the dll.
  boost::filesystem::path extensions("Extensions\\Debug");
#else
  //Load extensions
  boost::filesystem::path extensions("Extensions");
#endif
  if (boost::filesystem::exists(extensions) && boost::filesystem::is_directory(extensions))
  {
    for (boost::filesystem::directory_iterator it(extensions); it != boost::filesystem::directory_iterator(); ++it)
    {
      if (boost::filesystem::is_regular_file(it->path()))
      {
        if (!it->path().extension().compare(std::string(".dll")))
        {
          wchar_t name[256];
          mbstowcs(name, it->path().string().c_str(), it->path().string().length());
          name[it->path().string().length()] = 0;
          HINSTANCE Crystal_Lib = LoadLibrary(name);
          CRY_EXPORT import = (CRY_EXPORT)GetProcAddress(Crystal_Lib, "CrystalExports");
          if (import)
          {
            //Call the CrystalExports function which contains all the REGISTER_FUNCTION
            //calls that builds up the built_in object;
            import(&built_in, CRY_ROOT);
            //Add the library to save it
            Extension_Libs.push_back(Crystal_Lib);
          }
        }
      }
    }
  }
#endif
  */
}