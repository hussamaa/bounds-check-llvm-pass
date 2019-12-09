#define N 50000

int main() {
   int list[N];
   int temp;
   int i,j;
   for(i = 0; i < N-1; i++) { 
      for(j = 0; j < N-1-i; j++) {	
         if(list[j] > list[j+1]) {
            temp = list[j];
            list[j] = list[j+1];
            list[j+1] = temp;
         } 
      }
   }
   return 0;
}
