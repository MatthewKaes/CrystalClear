#ifndef CRYSTAL_LINKER
#define CRYSTAL_LINKER

#include "Machines\Machine.h"
#include "Crystal.h"
#include "Library.h"
#include <stdio.h>

class Crystal_Linker {
public:
  Crystal_Linker();
  ~Crystal_Linker();
  void Set_Doubles(const std::unordered_map<double, std::vector<unsigned>>* values);
  void Set_Strings(const std::unordered_map<std::string, std::vector<unsigned>>* values);
  void Set_Functions(const std::unordered_map<std::string, PackageLinks>* values);
  void Set_Internal(const std::unordered_map<std::string, std::vector<unsigned>>* values);
  BYTE* Link(BYTE* code, unsigned code_length);
  BYTE* Entry();

  void Write(std::string output, BYTE* code, unsigned code_length);
  void Write_Header(FILE* output);
  void Write_Internals(FILE* output);
  void Write_Doubles(FILE* output);
  void Write_Strings(FILE* output);
  void Write_Functions(FILE* output);
  void Write_Code(FILE* output, BYTE* code, unsigned code_length);

  void Read(std::string input);
  void Read_Header(FILE* input);
  void Read_Internals(FILE* input);
  void Read_Doubles(FILE* input);
  void Read_Strings(FILE* input);
  void Read_Functions(FILE* input);
  void Read_Code(FILE* input);

  static Crystal_Library library;
private:
  std::unordered_map<double, std::vector<unsigned>> doubles;
  std::unordered_map<std::string, std::vector<unsigned>> strings;
  std::unordered_map<unsigned, std::vector<unsigned>> functions;
  std::unordered_map<unsigned, std::vector<unsigned>> internals;
  std::unordered_map<std::string, std::vector<unsigned>> raw_internals;
  std::unordered_map<std::string, PackageLinks> raw_functions;
  BYTE* executable;
  BYTE* read_only_memory;
  unsigned code_size;
  unsigned entry_point;
};

#endif