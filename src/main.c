#include "mylloc.h"
#include <stddef.h>
#include <stdio.h>

int main() {
  int a;
  scanf("%d", &a);

  void *something_nice = MyMalloc(a);
  return 0;
}
