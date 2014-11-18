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
  case '!':
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
  while(**stream == '\t' || **stream == ' ')
  {
    (*stream)++;
  }
  if(**stream != '\"')
  {
    while(**stream != '\t' && **stream != ' ' && **stream != 0)
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
      if(**stream == '\\')
      {
        (*stream)++;
      }
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
    sym->d = str_to_d(&sym->str);
  }
  else if(!sym->str.compare("true") || !sym->str.compare("false"))
  {
    sym->type = DAT_BOOL;
    sym->b = str_to_b(&sym->str);
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
      sym->i32 = str_to_i(&sym->str);
    }
    else
    {
      sym->type = DAT_OP;
    }
  }
  else
  {
    sym->type = DAT_LOOKUP;
  }
}

bool str_to_b(const std::string* object)
{
  if(!object->compare("true"))
    return true;
  return false;
}
double str_to_d(const std::string* object)
{  
  double convert = 0.0;
  unsigned i;
  if((*object)[0] == '-')
  {
    for(i = 1; i < object->size(); i++)
    {
      if((*object)[i] == '.')
        break;
      convert *= 10;
      convert += (*object)[i] - '0';
    }
    convert *= -1;
  }
  else
  {
    for(i = 0; i < object->size(); i++)
    {
      if((*object)[i] == '.')
        break;
      convert *= 10;
      convert += (*object)[i] - '0';
    }
  }
  double convert_dec = 0.0;
  unsigned j = 0;
  for(; i + j + 1 < object->size(); j++)
  {
    convert_dec *= 10;
    convert_dec += (*object)[i + j + 1] - '0';
  }
  convert_dec /= pow(10.0, static_cast<int>(j));

  return convert + convert_dec;
}
int str_to_i(const std::string* object)
{
  int convert = 0;
  if((*object)[0] == '-')
  {
    for(unsigned i = 1; i < object->size(); i++)
    {
      convert *= 10;
      convert += (*object)[i] - '0';
    }
    convert *= -1;
  }
  else
  {
    for(unsigned i = 0; i < object->size(); i++)
    {
      convert *= 10;
      convert += (*object)[i] - '0';
    }
  }
  return convert;
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
  if(object < 0.000000001 && object > -0.000000001)
  {
    value->assign("0.0");
    return;
  }
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

unsigned Get_Precedence(const char* sym)
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
  case '^':
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
    if(sym[1] == '>')
      return 2;
  case '>':
    return 4;
  case '!':
    if(sym[1] == '=')
      return 4;
    return 9;
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