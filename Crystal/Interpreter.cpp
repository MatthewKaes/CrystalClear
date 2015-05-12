#include "Interpreter.h"
#include "Library.h"
#include "Filesystems.h"
#include "IO.h"
#include "OS.h"
#include "Math.h"
#include <stdio.h>
#include <boost\filesystem.hpp>

#define BUFFER_SIZE 0x8000

#define REGISTER_FUNCTION(Name, Func, Args) \
  { Package_Info new_package; \
  new_package.pt = PKG_EXE; \
  new_package.info.arguments = Args; \
  new_package.function = Func; \
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
    new_package.pt = PKG_EXE; \
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
  REGISTER_FUNCTION(print, Crystal_Print, 1);
  REGISTER_FUNCTION(type, Crystal_Type, 1);
  REGISTER_FUNCTION(convert, Crystal_Convert, 2);
  REGISTER_FUNCTION(boolean, Crystal_Boolean, 1);
  REGISTER_FUNCTION(integer, Crystal_Integer, 1);
  REGISTER_FUNCTION(double, Crystal_Double, 1);
  REGISTER_FUNCTION(string, Crystal_String, 1);
  REGISTER_FUNCTION(print_color, Crystal_PrintColor, 2);
  REGISTER_FUNCTION(rand, Crystal_Rand, 1);
  REGISTER_FUNCTION(time, Crystal_Time, 0);
  REGISTER_FUNCTION(input, Crystal_Input, 0);
  REGISTER_FUNCTION(size, Crystal_Size, 1);
  REGISTER_FUNCTION(clone, Crystal_Clone, 1);
  REGISTER_FUNCTION(fork, Crystal_Fork, 1);

  //Math
  REGISTER_FUNCTION(cos, Crystal_Cos, 1);
  REGISTER_FUNCTION(sin, Crystal_Sin, 1)
  REGISTER_FUNCTION(log, Crystal_Log, 1);
  REGISTER_FUNCTION(ln, Crystal_NatrualLog, 1);
  REGISTER_FUNCTION(abs, Crystal_Abs, 1);
  REGISTER_FUNCTION(sum, Crystal_Sum, 1);
  REGISTER_FUNCTION(mean, Crystal_Mean, 1);
  REGISTER_FUNCTION(min, Crystal_Min, 1);
  REGISTER_FUNCTION(max, Crystal_Max, 1);
  REGISTER_FUNCTION(var, Crystal_Var, 1);
  REGISTER_FUNCTION(sd, Crystal_Sd, 1);
  REGISTER_FUNCTION(step, Crystal_Step, 3);

  //Input / Output
  REGISTER_FUNCTION(marshal_open, Crystal_MarshalOpen, 1);
  REGISTER_FUNCTION(marshal_dump, Crystal_MarshalDump, 1);
  REGISTER_FUNCTION(marshal_load, Crystal_MarshalLoad, 1);
  REGISTER_FUNCTION(marshal_close, Crystal_MarshalClose, 0);
  REGISTER_FUNCTION(write_text, Crystal_WriteText, 2);
  REGISTER_FUNCTION(read_text, Crystal_ReadText, 1);

  //Boost Extensions
  REGISTER_FUNCTION(sleep, Crystal_Sleep, 1);

  //Filesystems
  REGISTER_FUNCTION(make_path, Crystal_MakeDirectory, 1);
  REGISTER_FUNCTION(remove_path, Crystal_RemovePath, 1);
  REGISTER_FUNCTION(copy_path, Crystal_CopyPath, 2);
  REGISTER_FUNCTION(path_exists, Crystal_PathExists, 1);
  REGISTER_FUNCTION(is_file, Crystal_IsFile, 1);
  REGISTER_FUNCTION(is_dir, Crystal_IsDir, 1);
  REGISTER_FUNCTION(file_size, Crystal_FileSize, 1);
  REGISTER_FUNCTION(list_files, Crystal_FileList, 1);
  REGISTER_FUNCTION(cry_root, Crystal_CryRoot, 0);

  //OS
  REGISTER_FUNCTION(cry_copyright, Crystal_CrystalCopyright, 0)
  REGISTER_FUNCTION(cry_version, Crystal_CrystalVersion, 0);
  REGISTER_FUNCTION(cry_target, Crystal_CrystalTarget, 0);
  REGISTER_FUNCTION(environ, Crystal_Environ, 1);

  //Hooks to other langauges
  REGISTER_FUNCTION(python, Crystal_Python, 2);

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

  REGISTER_CLASS(File);
  REGISTER_ATTRIBUTE(filename);
  REGISTER_ATTRIBUTE(object);
  REGISTER_METHOD(print, Crystal_Print, 1);
  REGISTER_METHOD(type, Crystal_Type, 1);
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
        printf("CRYSTAL ERROR: function '%s' is defined multiple times in scope '%s'\n", current_class->name);
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
    if(!dot_op && current_class && current_class->lookup.find(Late_Binding(sym->str.c_str())) != current_class->lookup.end())
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
        sym->external = built_in[sym->str.c_str()].function;
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
