#include "googleurl/src/url_canon.h"

namespace url_canon {

bool ReadUTFChar(const char* str, int* begin, int length,
                 unsigned* code_point_out)
{
  return false;
}

bool ReadUTFChar(const char16* str, int* begin, int length,
                 unsigned* code_point)
{
  return false;
}

bool IDNToASCII(const char16* src, int src_len, CanonOutputW* output)
{
    return false;
}

}   //url_canon
