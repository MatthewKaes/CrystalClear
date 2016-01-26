#include "Interpreter.h"
#include "Core.h"
#include "Library.h"
#include "Filesystems.h"
#include "IO.h"
#include "OS.h"
#include "Math.h"
#include <stdio.h>
#include <boost\filesystem.hpp>

std::unordered_map<std::string, Package_Info> built_in;

// The rate at which code is read into buffered memory.
#define BUFFER_SIZE 0x8000

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
  built_in[#Name] = new_package; } 

#define REGISTER_CLASS(Name) \
  current_class = new Class_Info; \
  current_class->name = #Name; \
  current_class->id = Class_Listing.size(); \
  Class_Listing.push_back(current_class); \
  new_package.pt = PKG_CLASS; \
  new_package.ID = Class_Listing.size() - 1; \
  packages[#Name] = new_package;

#define REGISTER_METHOD(Meth, func, args) \
  { unsigned LB_ID = Late_Binding(#Meth); \
  if(current_class->attributes_loc.find(LB_ID) == current_class->attributes_loc.end()) \
  { \
    Package_Info new_package; \
    new_package.pt = PKG_EXT; \
    new_package.info.arguments = args; \
    new_package.function = func; \
    current_class->lookup[LB_ID] = new_package; \
  } }

#define REGISTER_GLOBAL_METHOD(Meth, func, args) \
  for(unsigned i = 0; i < Class_Listing.size(); i++) \
  { \
    Class_Info* current_class = Class_Listing[i]; \
    REGISTER_METHOD(Meth, func, args); \
  } \

#define REGISTER_ATTRIBUTE(Attr) \
  { unsigned LB_ID = Late_Binding("@" ## #Attr); \
  if(current_class->attributes_loc.find(LB_ID) == current_class->attributes_loc.end()) \
  { \
    current_class->attributes_loc[LB_ID] = current_class->attributes.size(); \
    current_class->attributes.push_back("@" ## #Attr); \
  } }

typedef int (*CRY_EXPORT)(std::unordered_map<std::string, Package_Info>*, const char*);

extern const char* CRY_ROOT;

// Late binding indexes for lookups.
std::unordered_map<std::string, unsigned> late_bindings;

// Registry for all classes and their contained attributes and functions.
std::vector<Class_Info*> Class_Listing;

// All of the externally loaded functions.
std::vector<HINSTANCE> Crystal_Interpreter::Extension_Libs;

Crystal_Interpreter::Crystal_Interpreter(Crystal_Compiler* compiler)
{
  comp = compiler;
  code_cache.assign(" ");
  Populate_BIP();
  Populate_Base_Classes();
  Populate_BIC();
}

Crystal_Interpreter::~Crystal_Interpreter(){}

void Crystal_Interpreter::Populate_BIP()
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
  REGISTER_FUNCTION(min, Crystal_Min, 1, Finds the minimum value in a container. Returns the value in the form of a DOUBLE., "DOUBLE The max value in the container.", "ARRAY The container to pull the max value from.");
  REGISTER_FUNCTION(max, Crystal_Max, 1, Finds the max value in a container. Returns the value in the form of a DOUBLE., "DOUBLE The min value in the container.", "ARRAY The container to pull the min value from.");
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
  REGISTER_FUNCTION(make_path, Crystal_MakeDirectory, 1, Creates a directory with the given path. Parent directories will be created as needed., "BOOLEAN True if the path was succesfully created.", "STRING The full path that needs to be created.");
  REGISTER_FUNCTION(remove_path, Crystal_RemovePath, 1, Removes a path from disk. If the path is a directory all files and subdirectories are also removed., "BOOLEAN True if the path was succesfully deleted.", "STRING The path to remove.");
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

#if _DEBUG
  //Load extensions
  //Get the debug version of the dll.
  boost::filesystem::path externals("Extensions\\Debug\\External");
#else
  //Load extensions
  boost::filesystem::path externals("Extensions\\External");
#endif
  if(boost::filesystem::exists(externals) && boost::filesystem::is_directory(externals))
  {
    for (boost::filesystem::directory_iterator it(externals); it != boost::filesystem::directory_iterator(); ++it)
    {
      if(!it->path().extension().compare(std::string(".dll")))
      {
        wchar_t name[256];
        mbstowcs( name, it->path().string().c_str(), it->path().string().length());
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
  if(boost::filesystem::exists(extensions) && boost::filesystem::is_directory(extensions))
  {
    for (boost::filesystem::directory_iterator it(extensions); it != boost::filesystem::directory_iterator(); ++it)
    {
      if(boost::filesystem::is_regular_file(it->path()))
      {
        if(!it->path().extension().compare(std::string(".dll")))
        {
          wchar_t name[256];
          mbstowcs( name, it->path().string().c_str(), it->path().string().length());
          name[it->path().string().length()] = 0;
          HINSTANCE Crystal_Lib = LoadLibrary(name);
          CRY_EXPORT import = (CRY_EXPORT)GetProcAddress(Crystal_Lib, "CrystalExports");
          if(import)
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
}

void Crystal_Interpreter::Populate_Base_Classes()
{
  Class_Info* current_class;
  Package_Info new_package;
  REGISTER_CLASS(Nil);
  REGISTER_CLASS(Bool);
  REGISTER_CLASS(Int);
  REGISTER_CLASS(Int64);
  REGISTER_CLASS(Double);
  REGISTER_CLASS(Text);
}

void Crystal_Interpreter::Populate_BIC()
{
  Class_Info* current_class;
  Package_Info new_package;

  // Initalize Function
  Late_Binding("init");

  REGISTER_CLASS(File);
  REGISTER_ATTRIBUTE(filename);
  REGISTER_ATTRIBUTE(object);
}

void Crystal_Interpreter::Cache_Code(const char* filename)
{
  FILE* source;
  char buffer[BUFFER_SIZE + 1];
  int bytes_read;

  //open the file for reading.
  source = fopen(filename, "r");
  if(!source)
  {
    printf("CRYSTAL ERROR: failed to load %s into code cache.", filename);
    return;
  }

  //Read all the code into the cache
  do {
    bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, source);
    buffer[bytes_read] = 0;
    code_cache.append(buffer);
  } while (bytes_read == BUFFER_SIZE);
  
  code_cache.push_back('\n');
  //close the file
  fclose(source);
}

void Crystal_Interpreter::Interpret()
{
  // Format the code for the lexiconer
  Format_Code();
  
  // Process all the code's syntax
  Process_Lookups();
  
  // Add all global methods for all types of objects
  Add_Global_Methods();

  // Process all the code's executable logic
  Process_Logic();
}

void Crystal_Interpreter::Format_Code()
{
  const char* code_ptr = code_cache.c_str();
  code_out.clear();
  while(*code_ptr)
  {
    switch(*code_ptr)
    {
    case '\n':
      if(code_out.size() > 2)
      {
        if(code_out[code_out.size() - 2] == '_')
        {
          code_out.pop_back();
          code_out.pop_back();
          code_ptr++;
          continue;
        }
        if(code_out[code_out.size() - 2] == ',')
        {
          code_ptr++;
          continue;
        }
        if(code_out[code_out.size() - 2] == '\n')
        {
          code_ptr++;
          continue;
        }
      }

      code_out.push_back(*code_ptr++);
      break;
    case ' ':
      code_ptr++;
      continue;
    case '#':
      code_ptr++;
      while(*code_ptr != '\n' && *code_ptr != '\0' && *code_ptr != '#')
      {
        code_ptr++;
      }
      if(*code_ptr == '#')
      {
        code_ptr++;
      }
      continue;
    case '"':
      code_out.push_back(*code_ptr++);
      // Contents of the string
      while(*code_ptr != '"')
      {
        // Escape codes
        if(*code_ptr == '\\')
        {
          code_ptr++;
          switch(*code_ptr)
          {
          case 'n':
          case 'N':
            code_out.push_back('\n');
            break;
          case 't':
          case 'T':
            code_out.push_back('\t');
            break;
          case '\\':
            code_out.push_back('\\');
            code_out.push_back('\\');
            break;
          case '\"':
            code_out.push_back('\\');
            code_out.push_back('\"');
            break;
          }
          code_ptr++;
        }
        else
        {
          code_out.push_back(*code_ptr++);
        }
      }
      code_out.push_back(*code_ptr++);
      break;     
    case '<':
      if(*(code_ptr + 1) == '>')
      {
        code_out.push_back(*code_ptr++);
      }
      code_out.push_back(*code_ptr++);
      break;
    case '.':
      while(*code_ptr == '.')
      {
        code_out.push_back(*code_ptr++);
      }
      break;
    case '-':
      if(is_op(code_out[code_out.size() - 2]))
      {
        code_out.push_back(*code_ptr++);
        continue;
      }
    case '=':    
    case '|':
    case '&':   
    case '>':
    case '+':
    case '*':
    case '/':
    case '^':
    case '%':
    case '!':
      if(*(code_ptr + 1) == *code_ptr || *(code_ptr + 1) == '=')
      {
        code_out.push_back(*code_ptr++);
      }
      code_out.push_back(*code_ptr++);
      break;
    default:
      if(is_number(*code_ptr))
      {
        bool dot = false;
        while(is_number(*code_ptr) || (*code_ptr == '.' && !dot))
        {
          if(*code_ptr == '.')
          {
            if(!is_number(*(code_ptr + 1)))
            {
              break;
            }
            dot = true;
          }
          code_out.push_back(*code_ptr++);
        }
      }
      else if(!is_symbol(*code_ptr))
      {
        while(is_number(*code_ptr) || (!is_symbol(*code_ptr) && *code_ptr != '\0' && *code_ptr != '#'))
        {
          code_out.push_back(*code_ptr++);
        }
      }
      else
      {
        code_out.push_back(*code_ptr++);
      }
      break;
    }
    code_out.push_back(' ');
  }
}

void Crystal_Interpreter::Process_Lookups()
{
  const char* package_code = code_out.c_str();
  Crystal_Data pkg, sym, last_sym;
  Class_Info* current_class = NULL;
  unsigned scope = 0;

  while(Create_Symbol(&package_code, &sym))
  {
    if(!sym.str.compare("def"))
    {
      Create_Symbol(&package_code, &pkg);

      if(scope != 0 && scope != 1)
      {
        printf("CRYSTAL ERROR: package '%s' is defined inside a scope other then a class or GLOBAL.", pkg.str.c_str());
      }
      else if(current_class == NULL && packages.find(pkg.str.c_str()) != packages.end())
      {
        printf("CRYSTAL ERROR: package '%s' is defined multiple times in scope 'GLOBAL'\n", pkg.str.c_str());
      }
      else if(current_class != NULL && current_class->lookup.find(Late_Binding(pkg.str.c_str())) != current_class->lookup.end())
      {
        printf("CRYSTAL ERROR: function '%s' is defined multiple times in scope '%s'\n", pkg.str.c_str(), current_class->name.c_str());
      }
      else
      {
        scope += 1;

        //Define a new package
        Package_Info new_package;
        new_package.pt = PKG_EXE;
        new_package.info.arguments = 0;
      
        //Get the arguments
        Create_Symbol(&package_code, &sym); 
        while(sym.str[0] != '\n')
        {
          if(sym.str[0] != '(' && sym.str[0] != ')' && sym.str[0] != ',')
          {
            new_package.info.arguments++;
          }
          Create_Symbol(&package_code, &sym);
        }

        if(current_class == NULL)
        {
          packages[pkg.str.c_str()] = new_package;
        }
        else
        {
          current_class->lookup[Late_Binding(pkg.str.c_str())] = new_package;
        }
      }
    }
    else if(!sym.str.compare("class"))
    {     
      Create_Symbol(&package_code, &pkg);
      if(packages.find(pkg.str.c_str()) != packages.end())
      {
        printf("CRYSTAL ERROR: package '%s' is defined multiple times.", pkg.str.c_str());
      }
      else if(scope != 0)
      {
        printf("CRYSTAL ERROR: class '%s' is defined inside a scope other then GLOBAL.", pkg.str.c_str());
      }
      else
      {
        scope += 1;

        //Define a new class
        current_class = new Class_Info;

        current_class->name = pkg.str.c_str();
        current_class->id = Class_Listing.size();
        Class_Listing.push_back(current_class);
        
        //Define a new package
        Package_Info new_package;
        new_package.pt = PKG_CLASS;
        new_package.ID = Class_Listing.size() - 1;
        
        packages[pkg.str.c_str()] = new_package;
      }
    }
    else if(!sym.str.compare("end"))
    {
      scope -= 1;

      if(scope == 0 && current_class)
      {
        current_class = NULL;
      }
    }      
    else if(!sym.str.compare("if") || !sym.str.compare("while"))
    {
      scope += 1;
    }
    else if(current_class && sym.str[0] == '@' && last_sym.str.compare("."))
    {
      unsigned LB_ID = Late_Binding(sym.str.c_str());
      if(current_class->attributes_loc.find(LB_ID) == current_class->attributes_loc.end())
      {
        current_class->attributes_loc[LB_ID] = current_class->attributes.size();
        current_class->attributes.push_back(sym.str);
      }
    }

    last_sym = sym;
  }

  if(scope != 0)
  {
    printf("CRYSTAL ERROR: Hanging scope. Missing %d 'end' keywords.", scope);
  }
}

void Crystal_Interpreter::Add_Global_Methods()
{
  REGISTER_GLOBAL_METHOD(type, Crystal_Type, 1)
  REGISTER_GLOBAL_METHOD(int, Crystal_Integer, 1);
  REGISTER_GLOBAL_METHOD(bool, Crystal_Boolean, 1);
  REGISTER_GLOBAL_METHOD(double, Crystal_Double, 1);
  REGISTER_GLOBAL_METHOD(string, Crystal_String, 1);
  REGISTER_GLOBAL_METHOD(nil?, Crystal_NilCheck, 1);
  REGISTER_GLOBAL_METHOD(print, Crystal_Print, 1)
  REGISTER_GLOBAL_METHOD(print_color, Crystal_PrintColor, 2);
}

void Crystal_Interpreter::Process_Logic()
{
  const char* package_code = code_out.c_str();
  Crystal_Data sym;
  Class_Info* current_class = NULL;

  while(*package_code)
  {
    Create_Symbol(&package_code, &sym);
    if(!sym.str.compare("def"))
    {
      Process_Package(&package_code, current_class);
    }
    else if(!sym.str.compare("class"))
    {
      Create_Symbol(&package_code, &sym);
      current_class = Class_Listing[packages[sym.str.c_str()].ID];
    }
    else if(!sym.str.compare("end"))
    {
      current_class = NULL;
    }
  }
}

void Crystal_Interpreter::Process_Package(const char** code, Class_Info* current_class)
{
  Syntax_Tree syntax;
  Crystal_Data entry, sym;
  std::unordered_map<std::string, unsigned> local_map;
  unsigned scope = 1;
  unsigned precedence = 0;
  unsigned arguments = 0;
  bool dot_op = false;
  
  // Get the function signature data.
  Create_Symbol(code, &entry);
  Create_Symbol(code, &sym);

  // Set up the "this" object
  if(current_class)
  {
    local_map["this"] = 0;
  }

  while(sym.str[0] != '\n')
  {
    if(sym.str[0] != '(' && sym.str[0] != ')' && sym.str[0] != ',')
    {
      unsigned index = local_map.size();
      local_map[sym.str] = index;
      arguments++;
    }
    Create_Symbol(code, &sym);
  }

  // Evaluating the contents of the package.
  while(scope)
  {
    Create_Symbol(code, &sym);

    if(sym.type != DAT_STRING)
    {
      if(sym.str[0] == '\n')
      {
        precedence = 0;
        syntax.Evaluate();
        continue;
      } 
      else if(sym.str[0] == '[')
      {
        precedence++;
        sym.type = DAT_OP;

        // If the i32 segment remains zero it's a bracket operator
        // if it is set to -1 in the syntax tree it's an array
        // constructor.
        sym.i32 = 0;
      }
      else if(sym.str[0] == '(')
      {
        precedence++;
        continue;
      }
      else if(sym.str[0] == ')' || sym.str[0] == ']')
      {
        precedence--;
        continue;
      }
      else if(!sym.str.compare("end"))
      {
        scope -= 1;
        if(scope == 0)
          continue;
        sym.type = DAT_STATEMENT;
      }
      else if(!sym.str.compare("if") || !sym.str.compare("while"))
      {
        scope += 1;
        sym.type = DAT_STATEMENT;
      }
      else if(!sym.str.compare("else") || !sym.str.compare("elsif"))
      {
        sym.type = DAT_STATEMENT;
      }
    }
    
    // Special Symbols
    Special_Processing(&sym);

    // Look up nodes that need are unknown
    dot_op = Lookup_Processing(&sym, &local_map, dot_op, current_class);

    // Creating the node
    Syntax_Node* new_node = syntax.Acquire_Node();
    *new_node->Acquire() = sym;

    // Precedence
    if(sym.type == DAT_OP)
      new_node->priority = Get_Precedence(sym.str.c_str()) + Get_Precedence(NULL) * precedence;
    else if(sym.type == DAT_FUNCTION || sym.type == DAT_BIFUNCTION || sym.type == DAT_OBJFUNCTION || sym.type == DAT_INTFUNCTION)
      new_node->priority = Get_Precedence("f") + Get_Precedence(NULL) * precedence;
    else if(sym.type == DAT_STATEMENT)
      new_node->priority = Get_Precedence("k") + Get_Precedence(NULL) * precedence;
    else
      new_node->priority = Get_Precedence(NULL) * (precedence + 1);

    // Process node
    syntax.Process(new_node);
  }

  // Start the encoding
  comp->Start_Encode(entry.str.c_str(), local_map.size(), syntax.Get_Depth(), arguments, current_class, Late_Binding(entry.str.c_str()));
  
  // Process all the bytecode blocks
  std::vector<Bytecode>* codes = syntax.Get_Bytecodes();
  for(unsigned i = 0; i < codes->size(); i++)
  {
    (*codes)[i].Execute(comp);
  }
  
  // End the encoding process and reset the syntax tree
  comp->End_Encode();
  syntax.Reset();
}

bool Crystal_Interpreter::Lookup_Processing(Crystal_Data* sym, std::unordered_map<std::string, unsigned>* local_map, bool dot_op, Class_Info* current_class)
{
  // Processing Lookups
  if(sym->type == DAT_LOOKUP)
  {
    // Class function
    bool internal_function = false;
    if(!dot_op && current_class)
    {
      auto block = current_class->lookup.find(Late_Binding(sym->str.c_str()));

      // Check if the function exist and if it's alright for internal use.
      if(block != current_class->lookup.end() && block->second.pt != PKG_EXT)
      {
        internal_function = true;
      }
    }
    
    if(internal_function)
    {
      sym->i32 = Late_Binding(sym->str.c_str());
      sym->type = DAT_INTFUNCTION;
    }
    else if(dot_op && sym->str.c_str()[0] != '@')
    {
      sym->i32 = Late_Binding(sym->str.c_str());
      sym->type = DAT_OBJFUNCTION;
    }
    // Class attribute
    else if(!dot_op && sym->str.c_str()[0] == '@')
    {
      sym->i32 = current_class->attributes_loc[Late_Binding(sym->str.c_str())];
      sym->type = DAT_ATTRIBUTE;
    }
    // Crystal packages look up
    else if(packages.find(sym->str.c_str()) != packages.end())
    {
      // Globally defined functions
      if(packages[sym->str.c_str()].pt == PKG_EXE)
      {
        sym->type = DAT_FUNCTION;
        sym->i32 = packages[sym->str.c_str()].info.arguments;
      }
      // Class packages
      else if(packages[sym->str.c_str()].pt == PKG_CLASS)
      {
        sym->type = DAT_CLASS;
        sym->i32 = packages[sym->str.c_str()].ID;
      }
    }
    // Look up for built in functions
    else if(built_in.find(sym->str.c_str()) != built_in.end())
    {
      if(built_in[sym->str.c_str()].pt == PKG_EXE)
      {
        sym->type = DAT_BIFUNCTION;
        sym->i32 = built_in[sym->str.c_str()].info.arguments;
        sym->str = built_in[sym->str.c_str()].name;
      }
    }
    // Look up for locals
    else if(local_map->find(sym->str.c_str()) != local_map->end())
    {
      sym->type = DAT_LOCAL;
      sym->i32 = (*local_map)[sym->str.c_str()];
    }
    // Add the symbol to the local map
    else
    {
      sym->type = DAT_LOCAL;
      sym->i32 = local_map->size();
      (*local_map)[sym->str.c_str()] = sym->i32;
    }
  }

  // Processing dot operators for look ups
  if(!sym->str.compare("."))
  {
    return true;
  }
  return false;
}

void Crystal_Interpreter::Special_Processing(Crystal_Data* sym)
{
  //Special symbol handling
  if(sym->type != DAT_STRING)
  {
    if(!sym->str.compare("return"))
    {
      sym->type = DAT_STATEMENT;
    }
    else if(!sym->str.compare("nil"))
    {
      sym->type = DAT_NIL;
    }
    else if(!sym->str.compare("PI"))
    {
      sym->type = DAT_DOUBLE;
      sym->d = PI;
    }
    else if(!sym->str.compare("E"))
    {
      sym->type = DAT_DOUBLE;
      sym->d = C_E;
    }
    else if(!sym->str.compare("and"))
    {
      sym->str.assign("&&");
      sym->type = DAT_OP;
    }
    else if(!sym->str.compare("or"))
    {
      sym->str.assign("||");
      sym->type = DAT_OP;
    }
  }
}

unsigned Crystal_Interpreter::Late_Binding(const char* id, bool exclude)
{
  std::unordered_map<std::string, unsigned>::iterator index;
  
  index = late_bindings.find(id);

  if(index != late_bindings.end())
  {
    return index->second;
  }
  else
  {
    if(exclude)
      return -1;

    late_bindings[id] = late_bindings.size();
    return late_bindings.size() - 1;
  }
}
