#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  #pragma omp single
  {
      printf("Always the first to show \n");

      #pragma omp taskgroup
      {
          #pragma omp task
          {
            printf("  task 1 begins \n");
            #pragma omp taskgroup
            {
              #pragma omp task
              printf("    task 3 \n");
            }

            printf("  task 1 ends \n");
          }
          
          #pragma omp task
          printf("  task 2 \n");
      }
          
      printf("Always the last to show \n");
  }

  return 0;
}