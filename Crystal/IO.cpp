#include "IO.h"
#include <stdio.h>
#include "Helper.h"
#include "Function.h"

static Marshaler global_marshal;
extern const char* CRY_ROOT;

void Crystal_MarshalOpen(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string path;
  Parse_String(sym, &path);

  if(path.c_str()[0] == '\0' || path.c_str()[1] != ':')
  {
    path.insert(0, CRY_ROOT);
  }

  global_marshal.Open(path.c_str());
  
  ret_sym->type = CRY_NIL;
}

void Crystal_MarshalDump(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  global_marshal.Dump(sym);

  ret_sym->type = CRY_NIL;
}

void Crystal_MarshalLoad(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string path;
  Parse_String(sym, &path);
  
  if(path.c_str()[0] == '\0' || path.c_str()[1] != ':')
  {
    path.insert(0, CRY_ROOT);
  }

  global_marshal.Load(path.c_str(), ret_sym);
}

void Crystal_MarshalClose(Crystal_Symbol* ret_sym)
{
  global_marshal.Close();

  ret_sym->type = CRY_NIL;
}

void Crystal_WriteText(Crystal_Symbol* ret_sym, Crystal_Symbol* file, Crystal_Symbol* sym)
{  
  std::string text;
  Parse_String(file, &text);
  
  if(text.c_str()[0] == '\0' || text.c_str()[1] != ':')
  {
    text.insert(0, CRY_ROOT);
  }

  FILE* output = fopen(text.c_str(), "a");
  if(!output)
  {
    ret_sym->text = "ERROR - UNABLE TO OPEN FILE";
    ret_sym->type = CRY_TEXT;
    return;
  }
  
  Parse_String(sym, &text);
  fwrite(text.c_str(), sizeof(text[0]), text.size(), output);
  fclose(output);

  ret_sym->type = CRY_NIL;
}

void Crystal_ReadText(Crystal_Symbol* ret_sym, Crystal_Symbol* file)
{
  std::string text;
  char buffer[1000];
  Parse_String(file, &text);
  
  if(text.c_str()[0] == '\0' || text.c_str()[1] != ':')
  {
    text.insert(0, CRY_ROOT);
  }

  FILE* output = fopen(text.c_str(), "r");
  if(!output)
  {
    ret_sym->text = "ERROR - UNABLE TO OPEN FILE";
    ret_sym->type = CRY_TEXT;
    return;
  }
  
  text.clear();
  while(!feof(output))
  {
    size_t size = fread(buffer, sizeof(char), 1000, output);
    buffer[size] = '\0';
    text.append(buffer);
  }

  fclose(output);


  Construct_String(ret_sym, reinterpret_cast<char*>(memcpy(new char[text.size() + 1], text.c_str(), text.size() + 1)), text.size());
}