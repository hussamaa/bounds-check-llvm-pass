#include<stdlib.h>

int main(){
  int * a = (int*) malloc(sizeof(int) * 3);
  a[4] = 3; 
  return 0;
}
