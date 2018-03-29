#include "op.h"

using namespace std;

//bool operator< (const Op& a, const Op& b) {
//  return (a.obj_id < b.obj_id);
//}

Op::~Op()
{
}

ostream& operator<< (ostream& os, const Op::Type& opt) {
  static const char* s[] = {"NA", "W", "R"};
  os << s[opt];
  return os;
}
