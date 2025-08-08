#include "my_malloc.h"
#include <stddef.h>
#include <stdio.h>

int main(void) {
  int a;
  scanf("%d", &a);

  void *something_nice = MyMalloc(a);
  void *more_nice = MyMalloc(2 * a);
  return 0;
}
