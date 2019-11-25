#define N 10
int list[N] = {1,8,4,6,0,3,5,2,7,9};

void bubbleSort() {
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
}

int main() {
   bubbleSort();
   return 0;
}
