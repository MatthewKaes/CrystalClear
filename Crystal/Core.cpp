#include "Core.h"
#include "Helper.h"
#include "Function.h"
#include <algorithm>
#include <Windows.h>

extern std::unordered_map<std::string, Package_Info> built_in;

void Crystal_CrystalCopyright(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_COPYRIGHT;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalVersion(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_VERSION;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalLinker(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_LINKER;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalCompiler(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_COMPILER;
  ret_sym->type = CRY_TEXT;
}

void Crystal_CrystalTarget(Crystal_Symbol* ret_sym)
{
  ret_sym->text = CRY_MACHINE;
  ret_sym->type = CRY_TEXT;
}

void Crystal_Help(Crystal_Symbol* ret_sym, Crystal_Symbol* command)
{
  std::string command_str;
  Parse_String(command, &command_str);


  if (built_in.find(command_str) == built_in.end())
  {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
    printf("\nCrystal v%s ", CRY_VERSION);
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    printf("Functionality:\n");

    std::vector<std::string> functions;
    for (auto itr = built_in.begin(); itr != built_in.end(); itr++)
    {
      functions.push_back(itr->first.c_str());
    }

    std::sort(functions.begin(), functions.end());

    unsigned index = 0;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    for (auto itr = functions.begin(); itr != functions.end(); itr++)
    {
      printf("%-20s", itr->c_str());
      index++;
      if (index % 4 == 0)
        printf("\n");
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);

    if (index % 4 != 0)
      printf("\n");
    printf("Use 'help(\"command\")' for more information.\n\n");

    Crystal_Symbol* syms = new Crystal_Symbol[functions.size()];
    for (unsigned i = 0; i < functions.size(); i++)
    {
      syms[i].type = CRY_TEXT;
      syms[i].text = built_in[functions[i].c_str()].lookup;
    }

    Construct_Array(ret_sym, functions.size(), functions.size(), syms);
  }
  else
  {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
    printf("%s\n", command_str.c_str());
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    printf("Information:\n");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    printf("%s\n", built_in[command_str].discript.c_str());
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    printf("Arguments: ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    printf("%d\n", built_in[command_str].info.arguments);
    for (unsigned i = 0; i < built_in[command_str].argtext.size(); i++)
    {
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
      printf("Arg %-5d: ", i + 1);
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
      size_t offset = built_in[command_str].argtext[i].find(' ');
      printf("%s", built_in[command_str].argtext[i].substr(0, offset).c_str());
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
      printf("%s\n", built_in[command_str].argtext[i].substr(offset).c_str());
    }
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 3);
    printf("Returns  : ");
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 11);
    size_t offset = built_in[command_str].returndis.find(' ');
    printf("%s\n", built_in[command_str].returndis.substr(0, offset).c_str());
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
    printf("%s\n", built_in[command_str].returndis.substr(offset + 1).c_str());
    printf("\n");

    ret_sym->type = CRY_NIL;
  }
}