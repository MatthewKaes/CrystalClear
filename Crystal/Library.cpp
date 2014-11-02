#include "Library.h"
#include "Helper.h"
#include "Lexicon.h"
#include <windows.h>
#include <boost\date_time.hpp>

extern const char* CRY_ROOT;

void Crystal_Time(Crystal_Symbol* ret_sym)
{
  ret_sym->i32 = static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds());
  ret_sym->type = CRY_INT;
}
void Crystal_Input(Crystal_Symbol* ret_sym)
{
  std::string str;
  std::getline(std::cin, str);

  if(!str.size())
  {
    ret_sym->type = CRY_NIL;
  }
  else if(!str.compare("true") || !str.compare("false"))
  {
    ret_sym->type = CRY_BOOL;
    ret_sym->b = str_to_b(&str);
  }
  else if(!str.compare("nil"))
  {
    ret_sym->type = CRY_NIL;
  }
  else if(is_number(str[0]))
    if(str.find('.', 1) != std::string::npos)
    {
      ret_sym->type = CRY_DOUBLE;
      ret_sym->d = str_to_d(&str);
    }
    else
    {
      ret_sym->type = CRY_INT;
      ret_sym->i32 = str_to_i(&str);
    }
  else
  {
    if(ret_sym->ptr.str)
      free(ret_sym->ptr.str);
    ret_sym->type = CRY_STRING;
    char* value = (char*)malloc(str.size() + 1);
    strcpy(value, str.c_str());
    ret_sym->ptr.str = value;
    ret_sym->size = str.size() + 1;
  }
}
void Crystal_Convert(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* conversion)
{
  std::string conv;
  if(conversion->type != CRY_TEXT && conversion->type != CRY_STRING)
  {
    ret_sym->type = CRY_NIL;
  }
  Parse_String(conversion, &conv);
  if(!conv.compare("BOOLEAN"))
  {
    ret_sym->type = CRY_BOOL;
    ret_sym->i32 = Parse_Bool(sym);
    return;
  }
  else if(!conv.compare("INTEGER") || !conv.compare("INTEGER-64"))
  {
    ret_sym->type = CRY_INT;
    ret_sym->i32 = Parse_Int(sym);
    return;
  }
  else if(!conv.compare("DOUBLE"))
  {
    ret_sym->type = CRY_DOUBLE;
    ret_sym->d = Parse_Double(sym);
    return;
  }
  else if(!conv.compare("TEXT") || !conv.compare("STRING"))
  {
    //Save the expensive operation if we can!
    if(ret_sym->type == CRY_STRING && ret_sym == sym)
      return;
    ret_sym->type = CRY_STRING;
    if(sym->type == CRY_TEXT)
    {
      if(ret_sym->ptr.str != 0)
        free(ret_sym->ptr.str);
      int size = strlen(sym->text) + 1;
      ret_sym->ptr.str = static_cast<char*>(malloc(size));
      ret_sym->size = size;
      strcpy(ret_sym->ptr.str, sym->text);
      return;
    }
    if(sym->type == CRY_BOOL)
    {
      b_to_str(sym->b, &conv);
    }
    else if(sym->type == CRY_INT || sym->type == CRY_INT64)
    {
      i_to_str(sym->i32, &conv);
    }
    else if(sym->type == CRY_DOUBLE)
    {
      d_to_str(sym->d, &conv);
    }
    else if(sym->type == CRY_NIL)
    {
      conv.assign("nil");
    }
    if(ret_sym->ptr.str != 0)
      free(ret_sym->ptr.str);
    ret_sym->ptr.str = static_cast<char*>(malloc(conv.size() + 1));
    ret_sym->size = conv.size() + 1;
    strcpy(ret_sym->ptr.str, conv.c_str());
    return;
  }
  ret_sym->type = CRY_NIL;
}
void Crystal_Boolean(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_BOOL;
  ret_sym->i32 = Parse_Bool(sym);
}
void Crystal_Integer(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_INT;
  ret_sym->i32 = Parse_Int(sym);
}
void Crystal_Double(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  ret_sym->type = CRY_DOUBLE;
  ret_sym->d = Parse_Double(sym);
}
void Crystal_String(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string conv;
  //Save the expensive operation if we can!
  if(ret_sym->type == CRY_STRING && ret_sym == sym)
    return;
  ret_sym->type = CRY_STRING;
  if(sym->type == CRY_TEXT)
  {
    if(ret_sym->ptr.str != 0)
      free(ret_sym->ptr.str);
    int size = strlen(sym->text) + 1;
    ret_sym->ptr.str = static_cast<char*>(malloc(size));
    ret_sym->size = size;
    strcpy(ret_sym->ptr.str, sym->text);
    return;
  }
  if(sym->type == CRY_BOOL)
  {
    b_to_str(sym->b, &conv);
  }
  else if(sym->type == CRY_INT || sym->type == CRY_INT64)
  {
    i_to_str(sym->i32, &conv);
  }
  else if(sym->type == CRY_DOUBLE)
  {
    d_to_str(sym->d, &conv);
  }
  else if(sym->type == CRY_NIL)
  {
    conv.assign("nil");
  }
  if(ret_sym->ptr.str != 0)
    free(ret_sym->ptr.str);
  ret_sym->ptr.str = static_cast<char*>(malloc(conv.size() + 1));
  ret_sym->size = conv.size() + 1;
  strcpy(ret_sym->ptr.str, conv.c_str());
}
void Crystal_Type(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  switch(sym->type)
  {
  case CRY_BOOL:
    ret_sym->text = "BOOLEAN";
    break;
  case CRY_INT:
    ret_sym->text = "INTEGER";
    break;
  case CRY_INT64:
    ret_sym->text = "INTEGER-64";
    break;
  case CRY_DOUBLE:
    ret_sym->text = "DOUBLE";
    break;
  case CRY_ARRAY:
    ret_sym->text = "ARRAY";
    break;
  case CRY_POINTER:
    ret_sym->text = "OBJECT";
    break;
  case CRY_CLASS_OBJ:
    ret_sym->text = "CLASS";
    break;
  case CRY_NIL:
    ret_sym->text = "NIL";
    break;
  case CRY_TEXT:
    ret_sym->text = "TEXT";
    break;
  case CRY_STRING:
    ret_sym->text = "STRING";
    break;
  }
  ret_sym->size = strlen(ret_sym->text);
  ret_sym->type = CRY_TEXT;
}
void Crystal_Rand(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  Parse_Int(sym);
  ret_sym->i32 = rand() % Parse_Int(sym);
  ret_sym->type = CRY_INT;
}
void Crystal_Print(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string val;
  Parse_String(sym, &val);
  ret_sym->i32 = printf("%s\n", val.c_str()) - 1;
  ret_sym->type = CRY_INT;
}

void Crystal_PrintColor(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* color)
{  
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Parse_Int(color));
  Crystal_Print(ret_sym, sym);
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

#if INCLUDE_PYTHON
#include <boost\python.hpp>
void Crystal_Python(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  //Get realitive path of the project, not where Crystal is
  //running.
  std::string val;
  Parse_String(sym, &val);
  val = CRY_ROOT + ("\\" + val);

  //Load the code
  FILE* f = fopen(val.c_str(), "r");
  char code[PYTHON_SCRIPT_SIZE];
  unsigned code_size = fread(code, 1, PYTHON_SCRIPT_SIZE, f);
  //Check if code is to large.
  if(code_size > PYTHON_SCRIPT_SIZE)
  {
    printf("PYTHON FILE %s IS TO LARGE!\nIncrease Python side in Config.h", val.c_str());
    ret_sym->type = CRY_NIL;
    return;
  }
  code[code_size] = 0;
  
  // Retrieve the main module
  boost::python::object main = boost::python::import("__main__");
  // Retrieve the main module's namespace
  boost::python::object global(main.attr("__dict__"));
  
  //Execute the code
	boost::python::object result;
  result = boost::python::exec(code, global, global);

  //Currently no conversion from Python->Crystal so return nil
  ret_sym->type = CRY_NIL;
}
#else
void Crystal_Python(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  printf("PYTHON NOT SUPPORTED\n");
  ret_sym->type = CRY_NIL;
}
#endif


void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), val_right.c_str());
  if(symd->ptr.str)
    free(symd->ptr.str);
  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + val_right.size() + 1;
}
void Crystal_Text_AppendR(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_right.c_str());
  strcpy(new_buffer + val_right.size(), val_left.c_str());
  if(symd->ptr.str)
    free(symd->ptr.str);
  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + val_right.size() + 1;
}

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length + 1));
  strcpy(new_buffer, symd->ptr.str);
  strcpy(new_buffer + strlen(symd->ptr.str), str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, symd->ptr.str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}

void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), str);

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length;
}
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, val_left.c_str());

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length;
}