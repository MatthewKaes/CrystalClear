#ifndef CRYSTAL_LEXICON
#define CRYSTAL_LEXICON

#include "Crystal.h"
#include <stdio.h>
#include <string>

//void Process_Code(FILE* program_feed, DA_String* temp_line);
//
bool is_symbol(char object);
bool is_number(char object);
//unsigned get_symbol(const char* stream, DA_String* fill);
//bool arithmetic_operation(const char* object);
//int arithmetic_priority(const char* object);

void Create_Symbol(const char** stream, Crystal_Data* sym);
void Resolve_Type(Crystal_Data* sym);

//bool sym_to_b(const Crystal_Symbol* object);
//double sym_to_d(const Crystal_Symbol* object);
//__int64 sym_to_i(const Crystal_Symbol* object);

void i_to_str(int object, std::string* value);
void l_to_str(__int64 object, std::string* value);
void d_to_str(double object, std::string* value);
void b_to_str(bool object, std::string* value);

void Reverse_Str(std::string* value);

#endif
