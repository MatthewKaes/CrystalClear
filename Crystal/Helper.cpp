#include "Helper.h"
#include "Lexicon.h"

extern std::vector<Class_Info*> Class_Listing;

int Parse_Int(Crystal_Symbol* sym)
{
  switch(sym->type)
  {
  case CRY_STRING:
    {
      std::string str(sym->str);
      return str_to_i(&str);
    }
  case CRY_TEXT:
    {
      std::string str(sym->text);
      return str_to_i(&str);
    }
  case CRY_INT:
    return sym->i32;
  case CRY_BOOL:
    return sym->b;
  case CRY_DOUBLE:
    return static_cast<int>(sym->d);
  case CRY_POINTER:
  case CRY_REFERENCE:
    return Parse_Int(sym->sym);
  default:
    return 0;
  }
}

int Parse_Bool(Crystal_Symbol* sym)
{
  if(sym->type == CRY_REFERENCE || sym->type == CRY_POINTER)
    return Parse_Bool(sym->sym);
  if(sym->type == CRY_NIL)
    return 0;
  return sym->i32 != 0;
}

double Parse_Double(Crystal_Symbol* sym)
{
  switch(sym->type)
  {
  case CRY_STRING:
    {
      std::string str(sym->str);
      return str_to_d(&str);
    }
  case CRY_TEXT:
    {
      std::string str(sym->text);
      return str_to_d(&str);
    }
  case CRY_BOOL:
    return sym->b;
  case CRY_DOUBLE:
    return sym->d;
  case CRY_INT:
    return sym->i32;
  case CRY_POINTER:
  case CRY_REFERENCE:
    return Parse_Double(sym->sym);
  default:
    return 0.0;
  }
}

void Parse_String(Crystal_Symbol* sym, std::string* str)
{
  str->clear();
  switch(sym->type)
  {
  case CRY_STRING:
    str->assign(sym->str);
    return;
  case CRY_ARRAY:
    // Break the array down.
    str->assign("[");
    {
      std::string sub_str;
      for(unsigned i = 0; i < sym->size; i++)
      {
        Parse_String(sym->sym + i, &sub_str);
        str->append(sub_str);

        if(i != sym->size - 1)
        {
          str->append(", ");
        }
      }
    }
    str->append("]");
    return;
  case CRY_CLASS_OBJ:
    // Break the array down.
    str->assign(sym->klass->name);
    str->append(" :{");
    {
      std::string sub_str;
      for(unsigned i = 0; i < sym->size; i++)
      {
        str->append(sym->klass->attributes[i]);
        str->append(": ");
        Parse_String(sym->sym + i, &sub_str);
        str->append(sub_str);

        if(i != sym->size - 1)
        {
          str->append(", ");
        }
      }
    }
    str->append("}");
    return;
  case CRY_TEXT:
    str->assign(sym->text);
    return;
  case CRY_INT:
    i_to_str(sym->i32, str);
    return;
  case CRY_INT64:
    l_to_str(sym->i64, str);
    return;
  case CRY_DOUBLE:
    d_to_str(sym->d, str);
    return;
  case CRY_BOOL:
    b_to_str(sym->b, str);
    return;
  case CRY_REFERENCE:
  case CRY_POINTER:
    Parse_String(sym->sym, str);
    return;
  case CRY_NIL:
    str->assign("nil");
    return;
  }
}

bool Fast_strcmp(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  int i = 0;
  const char* l = syml->type == CRY_TEXT ? syml->text : syml->str;
  const char* r = symr->type == CRY_TEXT ? symr->text : symr->str;
  while(*(l + i) || *(r + i))
  {
    if(*(l + i) != *(r + i))
      return false;
    i++;
  }
  return true;
}

int Fast_pointercmp(Crystal_Symbol* aryl, Crystal_Symbol* aryr)
{
  if(aryl->type != CRY_POINTER || aryr->type != CRY_POINTER)
  {
    return false;
  }
  return Fast_arraycmp(aryl->sym, aryr->sym);
}

int Fast_arraycmp(Crystal_Symbol* aryl, Crystal_Symbol* aryr)
{
  if(aryl->size != aryr->size)
  {
    return false;
  }

  for(unsigned i = 0; i < aryl->size; i++)
  {
    if(aryl->sym[i].type == CRY_POINTER)
    {
      switch(aryl->sym[i].sym->type)
      {
      case CRY_STRING:
        if(aryr->sym[i].type == CRY_TEXT)
        {
          if(!Fast_strcmp(aryl->sym[i].sym, aryr->sym + i))
          {
            return 0;
          }
        }
        else if(aryr->sym[i].type != CRY_STRING)
        {
          if(!Fast_strcmp(aryl->sym[i].sym, aryr->sym[i].sym))
          {
            return 0;
          }
        }
        else
        {
          return 0;
        }

        break;
      case CRY_ARRAY:
        if(aryl->sym[i].type != aryr->sym[i].type)
          return 0;

        if(!Fast_arraycmp(aryl->sym[i].sym, aryr->sym[i].sym))
          return 0;

        break;
      }
      continue;
    }

    if(aryl->sym[i].type == CRY_TEXT)
    {
      if(aryr->sym[i].type == CRY_TEXT)
      {
        if(!Fast_strcmp(aryl->sym + i, aryr->sym + i))
        {
          return 0;
        }
      }
      else if(aryr->sym[i].type != CRY_STRING)
      {
        if(!Fast_strcmp(aryl->sym + i, aryr->sym[i].sym))
        {
          return 0;
        }
      }
      else
      {
        return 0;
      }
    }

    if(aryl->sym[i].type != aryr->sym[i].type)
      return 0;
    
    if(aryl->sym[i].LOWWER != aryr->sym[i].LOWWER)
      return 0;

    if((aryl->sym[i].type == CRY_INT64 || aryl->sym[i].type == CRY_DOUBLE) 
      && aryl->sym[i].UPPER != aryr->sym[i].UPPER)
      return 0;
  }

  return 1;
}

void Stack_Copy(Crystal_Symbol* sym_stack, Crystal_Symbol* sym_from)
{
  sym_stack->i64 = sym_from->i64;
  sym_stack->sym = sym_from->sym;
  sym_stack->type = sym_from->type;
}

void Power_Syms(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  double l = syml->type == CRY_DOUBLE ? syml->d : syml->i32;
  double r = symr->type == CRY_DOUBLE ? symr->d : symr->i32;
  syml->d = pow(l, r);
}

void Power_SymsR(Crystal_Symbol* syml, Crystal_Symbol* symr)
{
  double l = syml->type == CRY_DOUBLE ? syml->d : syml->i32;
  double r = symr->type == CRY_DOUBLE ? symr->d : symr->i32;
  syml->d = pow(r, l);
}
void Cry_Derefrence(Crystal_Symbol** sym)
{
  if((*sym)->type == CRY_REFERENCE)
  {
    *sym = (*sym)->sym;
  }

  if((*sym)->type == CRY_POINTER)
  {
    *sym = (*sym)->sym;
  }
}

//Array functions
void Array_Add_Nil(int index, Crystal_Symbol* ary)
{
  ary[index].type = CRY_NIL;
}

void Array_Add_Bool(int num, int index, Crystal_Symbol* ary)
{
  ary[index].type = CRY_BOOL;
  ary[index].i32 = num;
}

void Array_Add_Int(int num, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_INT;
  ary[index].i32 = num;
}

void Array_Add_Double(double dec, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_DOUBLE;
  ary[index].d = dec;
}

void Array_Add_Text(const char* text, int index,  Crystal_Symbol* ary)
{
  ary[index].type = CRY_TEXT;
  ary[index].text = text;
}

void Array_Add_Var(Crystal_Symbol* sym, int index, Crystal_Symbol* ary)
{
  Stack_Copy(ary->sym + index, sym);
}

void Array_Add_Stack(Crystal_Symbol* sym_stack, int index, Crystal_Symbol* ary)
{
  if(sym_stack->type == CRY_STRING)
  {
    ary[index].size = sym_stack->size;
    ary[index].text = sym_stack->str;
    ary[index].type = CRY_TEXT;
    return;
  }
  else
  {
    ary[index].i64 = sym_stack->i64;
    ary[index].sym = sym_stack->sym;
    ary[index].type = sym_stack->type;
  }
}

void Push_Nil(Crystal_Symbol* sym)
{
  sym->sym->type = CRY_NIL;
}

void Push_Bool(int num, Crystal_Symbol* sym)
{
  sym->sym->type = CRY_BOOL;
  sym->sym->i32 = num;
}

void Push_Int(int num, Crystal_Symbol* sym)
{
  sym->sym->type = CRY_INT;
  sym->sym->i32 = num;
}

void Push_Double(double dec, Crystal_Symbol* sym)
{
  sym->sym->type = CRY_DOUBLE;
  sym->sym->d = dec;
}

void Push_Text(const char* text, Crystal_Symbol* sym)
{
  sym->sym->type = CRY_TEXT;
  sym->sym->text = text;
}

void Cry_Assignment(Crystal_Symbol* src, Crystal_Symbol* dest)
{
  *dest = *src;
}

void Val_Binding(Crystal_Symbol* ret, Crystal_Symbol* src, int index)
{
  if(src->type == CRY_REFERENCE)
  {
    src = src->sym;
  }

  if(index < 0)
  {
    index += src->sym->size;
  }

  if(index >= static_cast<int>(src->sym->size))
  {
    ret->type = CRY_NIL;
    return;
  }

  ret->type = CRY_REFERENCE;
  ret->sym = src->sym->sym + index;
}

void* Late_Func_Binding(int id, Crystal_Symbol* symd)
{
  if(symd->type != CRY_POINTER)
    return Class_Listing[symd->type]->lookup[id].function;
  else
    return symd->sym->klass->lookup[id].function;
}

void* Late_Func_Binding_Ref(int id, Crystal_Symbol* symd)
{
  return symd->sym->sym->klass->lookup[id].function;
}

void Late_Attr_Binding(int id, Crystal_Symbol* symd, Crystal_Symbol* ret)
{
  ret->type = CRY_REFERENCE;
  ret->sym = symd->sym->sym + symd->sym->klass->attributes_loc[id];
}

void Late_Attr_Binding_Ref(int id, Crystal_Symbol* symd, Crystal_Symbol* ret)
{
  ret->type = CRY_REFERENCE;
  ret->sym = symd->sym->sym->sym + symd->sym->sym->klass->attributes_loc[id];
}

void Copy_Ref(Crystal_Symbol* dest, Crystal_Symbol* src)
{
  if(dest->type == CRY_REFERENCE)
    dest = dest->sym;
  
  if(src->type == CRY_REFERENCE)
    src = src->sym;

  *dest = *src;
}
