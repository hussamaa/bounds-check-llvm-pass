#include<stdlib.h>

int main(){
  int * a = (int*) malloc(sizeof(int) * 2);
  *(a + 4) = 3; 
  return 0;
}
