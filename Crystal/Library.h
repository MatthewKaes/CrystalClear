#ifndef CRYSTAL_LIBRARY
#define CRYSTAL_LIBRARY

class Crystal_Library {
public:
  Crystal_Library();
  std::unordered_map<std::string, void*> supported_functions;
  std::unordered_map<std::string, Package_Info> built_in;
private:
  void Populate_Built_In();
};

#endif