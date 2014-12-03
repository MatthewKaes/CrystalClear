#include "Library.h"
#include "Helper.h"
#include "Lexicon.h"
#include "Function.h"
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
    char* value = (char*)malloc(str.size() + 1);
    strcpy(value, str.c_str());
    Construct_String(ret_sym, value, str.size());
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
      int size = strlen(sym->text);
      char* str = static_cast<char*>(malloc(size + 1));
      ret_sym->size = size;
      strcpy(str, sym->text);
      Construct_String(ret_sym, str, size);
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
    char* str = static_cast<char*>(malloc(conv.size() + 1));
    int size = conv.size();
    strcpy(str, conv.c_str());
    Construct_String(ret_sym, str, size);
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
  Garbage_Collection(ret_sym);
  if(ret_sym->type == CRY_STRING && ret_sym == sym)
    return;
  ret_sym->type = CRY_STRING;
  if(sym->type == CRY_TEXT)
  {
    int size = strlen(sym->text);
    char* str = static_cast<char*>(malloc(size + 1));
    strcpy(str, sym->text);
    Construct_String(ret_sym, str, size);
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
  char* str = static_cast<char*>(malloc(conv.size() + 1));
  int size = conv.size();
  strcpy(str, conv.c_str());
  Construct_String(ret_sym, str, size);
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
  case CRY_POINTER:
    if(sym->sym->type == CRY_STRING)
      ret_sym->text = "STRING";
    else if(sym->sym->type == CRY_ARRAY)
      ret_sym->text = "ARRAY";
    else
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
  ret_sym->i32 = Printer(sym);
  printf("\n");
  ret_sym->type = CRY_INT;
}

void Crystal_PrintColor(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* color)
{  
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Parse_Int(color));
  Crystal_Print(ret_sym, sym);
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}
void Crystal_Size(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  if(sym->type == CRY_POINTER)
  {
    ret_sym->i32 = sym->sym->size;
    ret_sym->type = CRY_INT;
  }
  else
  {
    ret_sym->type = CRY_NIL;
  }
}

#if INCLUDE_PYTHON
#include <boost\python.hpp>
void Crystal_Python(Crystal_Symbol* ret_sym, Crystal_Symbol* sym, Crystal_Symbol* func)
{
  try{
    //Get realitive path of the project, not where Crystal is
    //running.
    std::string val;
    Parse_String(sym, &val);
    val = CRY_ROOT + ("\\" + val);

    std::string py_func;
    Parse_String(func, &py_func);
    if(!py_func.size())
      py_func.assign("nil");

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
  
    // Retrieve the main namespace
    boost::python::object main = boost::python::import("__main__");
    // Retrieve the main module's dictionary
    boost::python::object main_namespace = main.attr("__dict__");
  
    // Execute the code
	  boost::python::object result;
    result = boost::python::exec(code, main_namespace);

    // Now call the python function runPyProg with an argument
    if(py_func.compare("nil"))
    {
      boost::python::object PyFunc = main.attr(py_func.c_str());
      result = PyFunc();
    }

    // Here we try to pull out something useful for Crystal
    // Integer Conversion
    boost::python::extract<int> conv_int(result);
    if (conv_int.check())
    {
      ret_sym->type = CRY_INT;
      ret_sym->i32 = conv_int();
      return;
    }
  
    // Double Conversion
    boost::python::extract<double> conv_double(result);
    if (conv_double.check())
    {
      ret_sym->type = CRY_DOUBLE;
      ret_sym->d = conv_double();
      return;
    }

    // String Conversion
    boost::python::extract<std::string> conv_string(result);
    if (conv_string.check())
    {
      std::string val = conv_string();
      char* str = static_cast<char*>(malloc(val.size() + 1));
      strcpy(str, val.c_str());
      Construct_String(ret_sym, str, val.size());
      return;
    }
  }
  catch(const boost::python::error_already_set&)
  {
    PyErr_Print();
  }

  //Failed to do any known conversions
  ret_sym->type = CRY_NIL;
}
#else
void Crystal_Python(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  printf("PYTHON NOT SUPPORTED\n");
  ret_sym->type = CRY_NIL;
}
#endif