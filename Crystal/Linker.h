#ifndef CRYSTAL_LINKER
#define CRYSTAL_LINKER

#include "Machines\Machine.h"
#include "Crystal.h"

class Crystal_Linker {
public:
  Crystal_Linker();
  ~Crystal_Linker();
  void Add_Double(double value, unsigned offset);
  void Set_Doubles(const std::unordered_map<double, std::vector<unsigned>>* values);
  void Add_String(const char* str, unsigned offset);
  void Set_Strings(const std::unordered_map<std::string, std::vector<unsigned>>* values);
  void Add_Function(unsigned func, unsigned offset);
  void Set_Functions(const std::unordered_map<std::string, PackageLinks>* values);
  void Add_Internal(unsigned address, unsigned offset);
  BYTE* Link(BYTE* code);

private:
  std::unordered_map<double, std::vector<unsigned>> doubles;
  std::unordered_map<std::string, std::vector<unsigned>> strings;
  std::unordered_map<unsigned, std::vector<unsigned>> functions;
  std::unordered_map<unsigned, std::vector<unsigned>> internals;
  BYTE* read_only_memory;
  unsigned code_size;

};

#endif