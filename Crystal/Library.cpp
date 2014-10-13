#include "Library.h"
#include "Helper.h"
#include <windows.h>
#include <boost\date_time.hpp>

void Crystal_Time(Crystal_Symbol* ret_sym)
{
  ret_sym->i32 = static_cast<unsigned>(boost::posix_time::microsec_clock::local_time().time_of_day().total_milliseconds());
  ret_sym->type = CRY_INT;
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

void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  Parse_String(symd, &val_left);

  std::string val_right;
  Parse_String(syms, &val_right);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + val_right.size() + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), val_right.c_str());
  if(symd->type == CRY_STRING)
  {
    free(symd->ptr.str);
  }
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
  if(symd->type == CRY_STRING)
  {
    free(symd->ptr.str);
  }
  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + val_right.size() + 1;
}

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length));
  strcpy(new_buffer, symd->ptr.str);
  strcpy(new_buffer + symd->size - 1, str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}
void Crystal_Text_Append_CR(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length));
  strcpy(new_buffer, str);
  strcpy(new_buffer + symd->size - 1, symd->ptr.str);

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
  symd->size = val_left.size() + length + 1;
}
void Crystal_Const_Append_TL(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  Parse_String(symd, &val_left);

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, str);
  strcpy(new_buffer + length, val_left.c_str());

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length + 1;
}