#include "Library.h"
#include "Lexicon.h"

void Crystal_Print(Crystal_Symbol* sym)
{
  if(sym->type == CRY_STRING || sym->type == CRY_TEXT)
  {
    if(sym->type == CRY_STRING)
      sym->ptr.str[sym->size - 1] = 0;
    printf("%s\n", sym->ptr.str);
    return;
  }
  std::string val;
  if(sym->type == CRY_INT)
  {
    i_to_str(sym->i32, &val);
  }
  else if(sym->type == CRY_INT64)
  {
    l_to_str(sym->i64, &val);
  }
  else if(sym->type == CRY_DOUBLE)
  {
    d_to_str(sym->d, &val);
  }
  else if(sym->type == CRY_BOOL)
  {
    b_to_str(sym->b, &val);
  }
  else if(sym->type == CRY_NIL)
  {
    val.assign("nil");
  }
  printf("%s\n", val.c_str());
}

void Crystal_Text_Append(Crystal_Symbol* symd, Crystal_Symbol* syms)
{
  std::string val_left;
  if(symd->type == CRY_TEXT || symd->type == CRY_STRING)
  {
    val_left.assign(symd->ptr.str);
  }
  else if(symd->type == CRY_INT)
  {
    i_to_str(symd->i32, &val_left);
  }
  else if(symd->type == CRY_INT64)
  {
    l_to_str(symd->i64, &val_left);
  }
  else if(symd->type == CRY_DOUBLE)
  {
    d_to_str(symd->d, &val_left);
  }
  else if(symd->type == CRY_BOOL)
  {
    b_to_str(symd->b, &val_left);
  }
  else
  {
    val_left.assign("nil");
  }

  std::string val_right;
  if(syms->type == CRY_TEXT || syms->type == CRY_STRING)
  {
    val_right.assign(syms->ptr.str);
  }
  else if(syms->type == CRY_INT)
  {
    i_to_str(syms->i32, &val_right);
  }
  else if(syms->type == CRY_INT64)
  {
    l_to_str(syms->i64, &val_right);
  }
  else if(syms->type == CRY_DOUBLE)
  {
    d_to_str(syms->d, &val_right);
  }
  else if(syms->type == CRY_BOOL)
  {
    b_to_str(syms->b, &val_right);
  }
  else
  {
    val_right.assign("nil");
  }

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

void Crystal_Text_Append_C(Crystal_Symbol* symd, const char* str, unsigned length)
{
  char* new_buffer = static_cast<char*>(malloc(symd->size + length));
  strcpy(new_buffer, symd->ptr.str);
  strcpy(new_buffer + symd->size - 1, str);

  free(symd->ptr.str);

  symd->ptr.str = new_buffer;
  symd->size = symd->size + length;
}

void Crystal_Const_Append_T(Crystal_Symbol* symd, const char* str, unsigned length)
{  
  std::string val_left;
  if(symd->type == CRY_TEXT || symd->type == CRY_STRING)
  {
    val_left.assign(symd->ptr.str);
  }
  else if(symd->type == CRY_INT)
  {
    i_to_str(symd->i32, &val_left);
  }
  else if(symd->type == CRY_INT64)
  {
    l_to_str(symd->i64, &val_left);
  }
  else if(symd->type == CRY_DOUBLE)
  {
    d_to_str(symd->d, &val_left);
  }
  else if(symd->type == CRY_BOOL)
  {
    b_to_str(symd->b, &val_left);
  }
  else
  {
    val_left.assign("nil");
  }

  char* new_buffer = static_cast<char*>(malloc(val_left.size() + length + 1));
  strcpy(new_buffer, val_left.c_str());
  strcpy(new_buffer + val_left.size(), str);

  symd->ptr.str = new_buffer;
  symd->size = val_left.size() + length + 1;
}