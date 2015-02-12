#include "Marshal.h"
#include "Helper.h"
#include "Function.h"
#include "Garbage_Collector.h"

extern GC global_gc;

void Marshaler::Open(const char* filename)
{
  Close();
  
  // Open handle
  handle = fopen(filename, "wb");

  // Write out the signiture for the marshaler
  fwrite(&MARSHAL_SIG, sizeof(MARSHAL_SIG), 1, handle);
  
  // Write out place holder marshal count
  fwrite(&count, sizeof(count), 1, handle);
}

void Marshaler::Dump(Crystal_Symbol* sym)
{
  // Check state
  if(handle == NULL)
    return;

  // Increment symbol count
  count++;

  // Get underlying type
  Cry_Derefrence(&sym);

  // Special case:
  // Text needs to be saved as a string
  Symbol_Type type = sym->type;
  if(type == CRY_TEXT)
    type = CRY_STRING;

  // Write the type of the symbol
  fwrite(&type, sizeof(type), 1, handle);

  switch(sym->type)
  {
  case CRY_BOOL:
  case CRY_INT:
    fwrite(&sym->i32, sizeof(sym->i32), 1, handle);
    return;
  case CRY_DOUBLE:
    fwrite(&sym->d, sizeof(sym->d), 1, handle);
    return;
  case CRY_TEXT:
    fwrite(&sym->size, sizeof(sym->size), 1, handle);
    fwrite(sym->text, sizeof(char), sym->size + 1, handle);
    return;
  case CRY_STRING:
    fwrite(&sym->size, sizeof(sym->size), 1, handle);
    fwrite(sym->str, sizeof(char), sym->size + 1, handle);
    return;
  }
}
void Marshaler::Load(const char* filename, Crystal_Symbol* dest)
{
  // Open handle
  FILE* loader = fopen(filename, "rb");
  unsigned sym_count;

  // Check for marshal signature
  fread(&sym_count, sizeof(MARSHAL_SIG), 1, loader);
  if(sym_count != MARSHAL_SIG)
    return;

  fread(&sym_count, sizeof(sym_count), 1, loader);
  
  Crystal_Symbol* unmarshaled = new Crystal_Symbol[sym_count];

  for(unsigned i = 0; i < sym_count; i++)
  {

    fread(&unmarshaled[i].type, sizeof(unmarshaled[i].type), 1, loader);
    switch(unmarshaled[i].type)
    {
    case CRY_BOOL:
    case CRY_INT:
      fread(&unmarshaled[i].i32, sizeof(unmarshaled[i].i32), 1, loader);
      continue;
    case CRY_DOUBLE:
      fread(&unmarshaled[i].d, sizeof(unmarshaled[i].d), 1, loader);
      continue;
    case CRY_STRING:
      unmarshaled[i].type = CRY_POINTER;

      // Process String
      {
        unsigned length;
        fread(&length, sizeof(length), 1, loader);

        char* str = new char[length + 1];
        fread(str, sizeof(char), length + 1, loader);

        Construct_String(&unmarshaled[i], str, length);
      }
      continue;
    }
  }

  // Write out place holder marshal count
  Construct_Array(dest, sym_count, sym_count, unmarshaled);

  fclose(loader);
}

void Marshaler::Close()
{
  if(handle != NULL)
  {
    // Write out the current object count
    fseek(handle, sizeof(MARSHAL_SIG), SEEK_SET);
    fwrite(&count, sizeof(count), 1, handle);

    // Close handle
    fclose(handle);
  }

  // Reset count
  count = 0;
}