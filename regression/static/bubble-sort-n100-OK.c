#define N 100
int list[N] = {9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1,9,8,7,6,5,4,3,2,1};

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
