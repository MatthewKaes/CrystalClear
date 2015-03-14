#ifndef CRYSTAL_LEXICON
#define CRYSTAL_LEXICON

#include "Crystal.h"
#include <stdio.h>
#include <string>

bool is_symbol(char object);
bool is_op(char object);
bool is_number(char object);

bool Create_Symbol(const char** stream, Crystal_Data* sym);
void Resolve_Type(Crystal_Data* sym);

bool str_to_b(const std::string* object);
double str_to_d(const std::string* object);
int str_to_i(const std::string* object);

void i_to_str(int object, std::string* value);
void l_to_str(__int64 object, std::string* value);
void d_to_str(double object, std::string* value);
void b_to_str(bool object, std::string* value);

void Reverse_Str(std::string* value);

unsigned Get_Precedence(const char* sym);

#endif
