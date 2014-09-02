#include "Lexicon.h"

#pragma warning(disable : 4244)

bool is_symbol(char object)
{
  switch(object)
  {
  case '+':
  case '.':
  case '-':
  case '*':
  case '/':
  case '^':
  case '%':
  case '(':
  case ')':
  case '[':
  case ']':
  case '=':
  case '"':
  case '<':
  case '>':
  case ':':
  case ',':
  case '|':
  case '&':
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '\n':
  case ' ':
    return true;
  default:
    return false;
  }
}
bool is_number(char object)
{
  if(object >= '0' && object <= '9')
    return true;
  else
    return false;
}

void Create_Symbol(const char** stream, Crystal_Data* sym)
{
  sym->str.clear();
  while(**stream == ' ')
  {
    (*stream)++;
  }
  if(**stream != '\"')
  {
    while(**stream != ' ' && **stream != 0)
    {
      sym->str.push_back(**stream);
      (*stream)++;
    }
  }
  else
  {
    (*stream)++;
    while(**stream != '\"' && **stream != 0)
    {
      sym->str.push_back(**stream);
      (*stream)++;
    }
    (*stream)++;
    sym->type = DAT_STRING;
    return;
  }
  Resolve_Type(sym);
}

void Resolve_Type(Crystal_Data* sym)
{
  if(!sym->str.size())
  {
    return;
  }
  if(sym->str[0] == '\"')
  {
    sym->type = DAT_STRING;
  }
  else if(sym->str.find('.', 1) != std::string::npos)
  {
    sym->type = DAT_DOUBLE;
  }
  else if(!sym->str.compare("true") || !sym->str.compare("false"))
  {
    sym->type = DAT_BOOL;
  }
  else if(!sym->str.compare("nil"))
  {
    sym->type = DAT_NIL;
  }
  else if(is_symbol(sym->str[0]))
  {
    if(is_number(sym->str[0]))
    {
      sym->type = DAT_INT;
    }
    else
    {
      sym->type = DAT_OP;
    }
  }
  sym->type = DAT_LOOKUP;
}

void i_to_str(int object, std::string* value)
{
  if(object > 0)
  {
    while(object > 0)
    {
      char number = object % 10 + 48;
      value->push_back(number);
      object /= 10;
    }
    Reverse_Str(value);
  }
  else if(object == 0)
  {
    value->assign("0");
  }
  else
  {
    object *= -1;
    while(object > 0)
    {
      char number = object % 10 + 48;
      value->push_back(number);
      object /= 10;
    }
    value->push_back('-');
    Reverse_Str(value);
  }
}
void l_to_str(__int64 object, std::string* value)
{
  if(object > 0)
  {
    while(object > 0)
    {
      char number = object % 10 + 48;
      value->push_back(number);
      object /= 10;
    }
    Reverse_Str(value);
  }
  else if(object == 0)
  {
    value->assign("0");
  }
  else
  {
    object *= -1;
    while(object > 0)
    {
      char number = object % 10 + 48;
      value->push_back(number);
      object /= 10;
    }
    value->push_back('-');
    Reverse_Str(value);
  }
}
void d_to_str(double object, std::string* value)
{
  value->clear();  
  int integer_val = object;
  if(object > 0)
  {    
    if(integer_val == 0)
    {
      value->assign("0");
    }
    else
    {
      while(integer_val > 0)
      {
        char number = integer_val % 10 + 48;
        value->push_back(number);
        integer_val /= 10;
      }
      Reverse_Str(value);
    }
  }
  else
  {
    object *= -1;
    integer_val *= -1;
    if(integer_val == 0)
    {
      value->assign("0");
    }
    else
    {
      while(integer_val > 0)
      {
        char number = integer_val % 10 + 48;
        value->push_back(number);
        integer_val /= 10;
      }
    }
    value->push_back('-');
    Reverse_Str(value);
  }
  if(object < 0)
  {
    object *= -1;
  }
  value->append(".");
  int con_data = object;
  object -= con_data;
  if(object < 0.000001)
  {
    value->append("0");
  }
  else
  {
    int percision = 1;
    while(object > 0.00001 && percision < 8)
    {
      object *= 10;
      int temp = object;
      object -= temp;
      if(object > 0.99999)
      {
        temp += 1;
        object -= 1;
      }
      percision += 1;
      value->push_back((temp + 48));
    }
  }
}
void b_to_str(bool object, std::string* value)
{
  if(object)
  {
    value->assign("true");
  }
  else
  {
    value->assign("false");
  }
}

void Reverse_Str(std::string* value)
{
  for(unsigned i = 0; i < value->size() / 2; i++)
  {
    char copy = (*value)[i];
    (*value)[i] = (*value)[value->size() - (i + 1)];
    (*value)[value->size() - (i + 1)] = copy;
  }
}
