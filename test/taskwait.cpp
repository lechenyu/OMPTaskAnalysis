#include <iostream>
#include <string>

int main(int argc, char *argv[]) {

  #pragma omp parallel num_threads(3)
  {
    #pragma omp parallel
    {
      printf("hello \n");
    }
    // The single construct specifies that the associated structured block is executed by only one of the threads in the team (not necessarily the master thread), 
    // in the context of its implicit task. 
    // The other threads in the team, which do not execute the block, 
    // wait at an implicit barrier at the end of the single construct unless a nowait clause is specified
    #pragma omp single
    {
        #pragma omp task
        printf("A \n");

        #pragma omp task
        printf("car \n");

        #pragma omp task
        printf("race \n");

        #pragma omp taskwait
        printf("is fun to watch \n");
    }
  }
  return 0;
}