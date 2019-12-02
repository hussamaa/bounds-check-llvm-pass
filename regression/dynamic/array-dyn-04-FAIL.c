void assign (int * a){
  *(a+2) = 3;
}

int main(){
  int * a = (int *) malloc (sizeof(int) * 2);
  assign(a);
  return 0;
}
