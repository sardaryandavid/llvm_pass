//=============================================================================
// FILE:
//      input.c
//
// DESCRIPTION:
//      Sample input file
//
// License: MIT
//=============================================================================
#include <stdlib.h>

int 
bar(int c) 
{
  return c * c;
}

int
foo(int a) 
{
  if (a < 0)
  {
    a = -a;
  }

  else 
  {
    exit(1);
  }

  int c = 3;
  int val = bar(c); 
  for (int i = 0; i < a; ++i)
    val += a;

  return val;
}

int
fact(int a)
{
  if (a == 1)
    return 1;

  int val = foo(a) * fact(a - 1);
  if (a == 10) 
  {
    val *= fact(val);
  }

  else if (a == 15)
  {
    exit (15);
  }

  return val;
}