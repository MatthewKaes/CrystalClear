#include "Filesystems.h"
#include "Helper.h"

// Boost functionalities
#include <boost\filesystem.hpp>

extern const char* CRY_ROOT;

void Crystal_MakeDirectory(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);

  if(str[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  boost::filesystem::path dir(str.c_str());
  ret_sym->i32 = boost::filesystem::create_directories(dir);
  ret_sym->type = CRY_BOOL;
}

void Crystal_RemovePath(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  boost::filesystem::path dir(str.c_str());
  if(boost::filesystem::exists(dir))
  {
    ret_sym->i32 = static_cast<int>(boost::filesystem::remove_all(dir));
  }
  else
  {
    ret_sym->i32 = false;
  }
  ret_sym->type = CRY_BOOL;
}
