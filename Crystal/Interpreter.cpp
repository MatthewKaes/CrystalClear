#include "Interpreter.h"
#include "Library.h"
#include "Crystal_Math.h"
#include <stdio.h>

#define BUFFER_SIZE 0x8000

#define REGISTER_FUNCTION(Name, Func, Args) \
  { Package_Info new_package; \
  new_package.pt = PGK_EXE; \
  new_package.info.arguments = Args; \
  new_package.function = Func; \
  built_in[#Name] = new_package; } 

Crystal_Interpreter::Crystal_Interpreter(Crystal_Compiler* compiler)
{
  comp = compiler;
  code_cache.assign(" ");
  Populate_BIP();
}
Crystal_Interpreter::~Crystal_Interpreter(){}
void Crystal_Interpreter::Populate_BIP()
{
  //Define all executable packages
  REGISTER_FUNCTION(print, Crystal_Print, 1)
  REGISTER_FUNCTION(print_color, Crystal_PrintColor, 2);
  REGISTER_FUNCTION(rand, Crystal_Rand, 1);
  REGISTER_FUNCTION(time, Crystal_Time, 0);
  REGISTER_FUNCTION(cos, Crystal_Cos, 1);
  REGISTER_FUNCTION(sin, Crystal_Sin, 1);
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
  //Format the code for the lexiconer
  Format_Code();
  
  //Process all the code's syntax
  Lookup_Packages();

  //Process all the code's syntax
  Process_Code();
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
      if(code_out.size() > 1 && code_out[code_out.size() - 2] == '\n')
      {
        code_ptr++;
        continue;
      }
      else
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
      //Contents
      while(*code_ptr != '"')
      {
        //escape code
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
    case '=':     
    case '<':   
    case '>':    
    case '|':
    case '&':
    case '+':
    case '-':
    case '*':
    case '/':
    case '%':
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
void Crystal_Interpreter::Lookup_Packages()
{
  Crystal_Data pkg, sym;
  unsigned offset = 0;
  offset = code_out.find("def ", offset);
  while(offset != std::string::npos)
  {
    offset += strlen("def ");
    const char* ptr = code_out.c_str() + offset;
    Create_Symbol(&ptr, &pkg);
    if(packages.find(pkg.str.c_str()) != packages.end())
    {
      printf("CRYSTAL ERROR: package %s is defined multiple times.", pkg.str.c_str());
    }
    else
    {
      //Define a new package
      Package_Info new_package;
      new_package.pt = PGK_EXE;
      new_package.info.arguments = 0;
      
      //Get the arguments
      Create_Symbol(&ptr, &sym); 
      while(sym.str[0] != '\n')
      {
        if(sym.str[0] != '(' && sym.str[0] != ')')
        {
          new_package.info.arguments++;
        }
        Create_Symbol(&ptr, &sym);
      }

      packages[pkg.str.c_str()] = new_package; 
    }
    offset = code_out.find("def ", offset);
  }
}
void Crystal_Interpreter::Process_Code()
{
  const char* package_code = code_out.c_str();
  Crystal_Data sym;
  while(*package_code)
  {
    Create_Symbol(&package_code, &sym);
    if(!sym.str.compare("def"))
      Process_Package(package_code);
  }
}
void Crystal_Interpreter::Process_Package(const char* code)
{
  Syntax_Tree syntax;
  Crystal_Data entry, sym;
  std::unordered_map<std::string, unsigned> local_map;
  const char* package_code = code;
  unsigned scope = 1;
  unsigned precedence = 0;
  unsigned arguments = 0;
  
  //Get the function signature data.
  Create_Symbol(&package_code, &entry);
  Create_Symbol(&package_code, &sym);
  while(sym.str[0] != '\n')
  {
    if(sym.str[0] != '(' && sym.str[0] != ')')
    {
      unsigned index = local_map.size();
      local_map[sym.str] = index;
      arguments++;
    }
    Create_Symbol(&package_code, &sym);
  }
  //Evaluating the contents of the package.
  while(scope)
  {
    Create_Symbol(&package_code, &sym);

    //Special symbol handling
    if(sym.type != DAT_STRING)
    {
      if(sym.str[0] == '\n')
      {
        precedence = 0;
        syntax.Evaluate();
        continue;
      }
      else if(sym.str[0] == '(' || sym.str[0] == '[')
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
        continue;
      }
      //Statments
      else if(!sym.str.compare("return"))
      {
        sym.type = DAT_STATEMENT;
      }
    }
    
    //Look up nodes that need are unknown
    if(sym.type == DAT_LOOKUP)
    {
      //Crystal package call
      if(packages.find(sym.str.c_str()) != packages.end())
      {
        if(packages[sym.str.c_str()].pt == PGK_EXE)
        {
          sym.type = DAT_FUNCTION;
          sym.i32 = packages[sym.str.c_str()].info.arguments;
        }
      }
      else if(built_in.find(sym.str.c_str()) != built_in.end())
      {
        if(built_in[sym.str.c_str()].pt == PGK_EXE)
        {
          sym.type = DAT_BIFUNCTION;
          sym.i32 = built_in[sym.str.c_str()].info.arguments;
          sym.external = built_in[sym.str.c_str()].function;
        }
      }
      else if(local_map.find(sym.str.c_str()) != local_map.end())
      {
        sym.type = DAT_LOCAL;
        sym.i32 = local_map[sym.str.c_str()];
      }
      else
      {
        sym.type = DAT_LOCAL;
        sym.i32 = local_map.size();
        local_map[sym.str.c_str()] = sym.i32;
      }
    }
    //Creating the node
    Syntax_Node* new_node = syntax.Acquire_Node();
    *new_node->Acquire() = sym;

    //Precedence
    if(sym.type == DAT_OP)
      new_node->priority = Get_Precedence(sym.str.c_str()) + Get_Precedence(NULL) * precedence;
    else if(sym.type == DAT_STATEMENT)
      new_node->priority = Get_Precedence("k") + Get_Precedence(NULL) * precedence;
    else
      new_node->priority = Get_Precedence(NULL) * (precedence + 1);
    //Process node
    syntax.Process(new_node);
  }

  //Start the encoding
  comp->Start_Encode(entry.str.c_str(), local_map.size(), syntax.Get_Depth(), arguments);
  std::vector<Bytecode>* codes = syntax.Get_Bytecodes();
  for(unsigned i = 0; i < codes->size(); i++)
  {
    (*codes)[i].Execute(comp);
  }
  comp->End_Encode();
  syntax.Reset();
}
unsigned Crystal_Interpreter::Get_Precedence(const char* sym)
{
  if(!sym)
  {
    return 13;
  }
  //assignments
  switch(sym[0])
  {
  case '+':
  case '-':
  case '*':
  case '/':
  case '%':
  case '|':
  case '&':
    if(sym[1] == '=')
      return 2;
  }
  //general
  switch(sym[0])
  {
  case '(':
    return -1;
  case ')':
    return -2;
  case 'f':
  case '[':
    return 11;
  case ':':
    return 9;
  case '^':
    return 8;
  case '*':
  case '/':
    return 7;
  case '+':
  case '-':
    return 6;
  case '%':
    return 5;
  case '.':
    if(sym[1] == '.' && sym[2] == '.')
    {
      return 5;
    }
    else if(sym[1] == '\0')
    {
      return 10;
    }
  case '|':
  case '&':
    return 3;
  case '<':
  case '>':
    return 4;
  case '=':
    if(sym[1] == '=')
      return 4;
    return 2;
  case 'k':
  case ',':
    return 1;
  default:
    return 13;
  }
}