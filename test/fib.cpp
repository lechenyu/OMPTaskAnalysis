#include <iostream>
#include <string>

int fib(int n) {
  if (n < 2) {
    return n;
  }
  int partial_1, partial_2;
  #pragma omp task shared(partial_1)
  partial_1 = fib(n - 1);
  #pragma omp task shared(partial_2)
  partial_2 = fib(n - 2);
  #pragma omp taskwait 
  return partial_1 + partial_2;
}

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    std::cout << "fib <param...>" << std::endl;
    return 0;
  }
  #pragma omp parallel
  {
    #pragma omp single
    {
      for (int i = 1; i < argc; i++) {
        int val = std::stoi(argv[i]);
        int result = fib(val);
        std::cout << "fib(" << val << ") = " << result << std::endl;
      }
    }
  }
  return 0;
}
