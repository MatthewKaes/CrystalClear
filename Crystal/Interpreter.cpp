#include "Interpreter.h"
#include <stdio.h>

#define BUFFER_SIZE 0x8000

Crystal_Interpreter::Crystal_Interpreter(Crystal_Compiler* compiler) : stree(compiler)
{
  comp = compiler;
  code_cache.assign(" ");
}
Crystal_Interpreter::~Crystal_Interpreter()
{

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
      if(code_out[code_out.size() - 1] == '\n')
        code_ptr++;
      else
        code_out.push_back(*code_ptr++);
     continue;
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
    case '-':
    case '+':
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
        while(!is_symbol(*code_ptr) && *code_ptr != '\0' && *code_ptr != '#')
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
  unsigned offset = 0;
  offset = code_out.find("def ", offset);
  while(offset != std::string::npos)
  {
    offset += strlen("def ");
    Crystal_Data pkg;
    const char* ptr = code_out.c_str() + offset;
    Create_Symbol(&ptr, &pkg);
    if(packages.find(pkg.str) != packages.end())
    {
      printf("CRYSTAL ERROR: package %s is defined multiple times.", pkg.str.c_str());
    }
    else
    {
      packages[pkg.str] = PGK_EXE;
    }
    offset = code_out.find("def ", offset);
  }
}
void Crystal_Interpreter::Process_Code()
{

}
void Crystal_Interpreter::Process_Package()
{

}