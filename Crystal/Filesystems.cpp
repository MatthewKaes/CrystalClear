#include "Filesystems.h"
#include "Helper.h"
#include "Function.h"
#include <vector>

// Boost functionalities
#include <boost\filesystem.hpp>

extern const char* CRY_ROOT;

using namespace boost::filesystem;

void Crystal_MakeDirectory(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str.c_str()[0] == '\0' || str.c_str()[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  path dir(str.c_str());
  ret_sym->i32 = create_directories(dir);
  ret_sym->type = CRY_BOOL;
}

void Crystal_RemovePath(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str.c_str()[0] == '\0' || str.c_str()[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  path dir(str.c_str());
  if(exists(dir))
  {
    ret_sym->i32 = static_cast<int>(remove_all(dir));
  }
  else
  {
    ret_sym->i32 = false;
  }
  ret_sym->type = CRY_BOOL;
}

void Crystal_PathExists(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str.c_str()[0] == '\0' || str.c_str()[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  path dir(str.c_str());
  if(exists(dir))
  {
    ret_sym->i32 = true;
  }
  else
  {
    ret_sym->i32 = false;
  }
  ret_sym->type = CRY_BOOL;
}

void Crystal_FileSize(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str.c_str()[0] == '\0' || str.c_str()[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }

  path dir(str.c_str());
  if(exists(dir) && is_regular_file(dir))
  {
    ret_sym->i32 = static_cast<int>(file_size(dir));
    ret_sym->type = CRY_INT;
  }
  else
  {
    ret_sym->i32 = false;
    ret_sym->type = CRY_BOOL;
  }
}

void Crystal_FileList(Crystal_Symbol* ret_sym, Crystal_Symbol* sym)
{
  std::string str;
  Parse_String(sym, &str);
  
  if(str.c_str()[0] == '\0' || str.c_str()[1] != ':')
  {
    str.insert(0, CRY_ROOT);
  }
  
  path dir(str.c_str());
  directory_iterator end_itr;
  std::vector<Crystal_Symbol> files;

  for (directory_iterator itr(dir); itr != end_itr; ++itr)
  {
    Crystal_Symbol new_file;
    
    path root = canonical(itr->path());
    char* str = new char[root.string().size()];
    strcpy(str, root.string().c_str());

    Construct_String(&new_file, str, root.string().size());
    files.push_back(new_file);
  }

  Crystal_Symbol* syms = new Crystal_Symbol[files.size()];
  memcpy(syms, files.data(), sizeof(Crystal_Symbol) * files.size());

  Construct_Array(ret_sym, files.size(), files.size(), syms);
}

void Crystal_CryRoot(Crystal_Symbol* ret_sym)
{
  path root(CRY_ROOT);
  root = canonical(root);
  char* str = new char[root.string().size() + 1];
  strcpy(str, root.string().c_str());
  
  Construct_String(ret_sym, str, root.string().size());
}