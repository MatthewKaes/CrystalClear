#include "Interpreter.h"
#include <stdio.h>

#define BUFFER_SIZE 0x8000

Crystal_Interpreter::Crystal_Interpreter(Crystal_Compiler* compiler) : stree(compiler)
{
  comp = compiler;
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
  Process_Code();
}
void Crystal_Interpreter::Format_Code()
{

}
void Crystal_Interpreter::Process_Code()
{

}