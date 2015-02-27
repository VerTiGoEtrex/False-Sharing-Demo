#include <thread>
#include <vector>
#include <cmath>
#include <cassert>
#include <limits>
#include <omp.h>
#include <iostream>
#include <sys/time.h>

using std::vector;
using std::numeric_limits;
using std::cout; using std::endl;

// OUTDATED: Get a massive vector whose sum adds up to unsigned int max value
static_assert(sizeof(unsigned long) > sizeof(unsigned int), "long is not larger than int (why?)");
//const size_t SIZE = ( sqrt(8ul * numeric_limits<unsigned int>::max() + 1) - 1) / 2;

// Get array
//const size_t SIZE = pow(2, 33) / sizeof(unsigned int); // 8GB
const size_t SIZE = pow(2, 30) / sizeof(unsigned int); // 1GB
//const size_t SIZE = pow(2, 27) / sizeof(unsigned int); // 128MB
//const size_t SIZE = pow(2, 22) / sizeof(unsigned int); // 4MB

double getWallTime()
{
  timeval timeofday;
  gettimeofday( &timeofday, NULL );
  return timeofday.tv_sec + timeofday.tv_usec / 1000000.0;
}

unsigned long __attribute__((optimize("O1"))) serialSum(vector<unsigned int> &vec){
   __attribute__((__aligned__((64)))) unsigned long result = 0;
  for (size_t i = 0; i < vec.size(); ++i)
    result += vec[i];
  return result;
}

unsigned long __attribute__((optimize("O1"))) parallelSumOneAccumulate(vector<unsigned int> &vec){
  __attribute__((__aligned__((64)))) unsigned long result = 0;
  #pragma omp parallel for // Start a team of threads to calculate the sum
  for (size_t i = 0; i < vec.size(); ++i)
    #pragma omp atomic
    result += vec[i];
  return result;
}

unsigned long __attribute__((optimize("O1"))) parallelSumReduce(vector<unsigned int> &vec){
  unsigned long result = 0;
  #pragma omp parallel // Start a team of threads to calculate the sum
  {
    __attribute__((__aligned__((64)))) unsigned long privateResult = 0; //Ensure private results are on their own cache lines
    #pragma omp for
    for (size_t i = 0; i < vec.size(); ++i)
      privateResult += vec[i];
    #pragma omp atomic
    result += privateResult;
  }
  return result;
}

/*unsigned long __attribute__((optimize("O0"))) parallelSumReduceFalseSharing(vector<unsigned int> &vec){
  unsigned long result = 0;
  unsigned long *privateResult = 0;
  void *alignedMem;
  #pragma omp parallel// Start a team of threads to calculate the sum
  {
    #pragma omp master
    {
      posix_memalign(&alignedMem, 64 , omp_get_num_threads() * sizeof(unsigned long) * 2);
      privateResult = (unsigned long *)alignedMem;
    }
    int threadId = omp_get_thread_num();
    #pragma omp barrier
    #pragma omp for
    for (size_t i = 0; i < vec.size(); ++i)
      privateResult[threadId] += vec[i];
    #pragma omp atomic
    result += privateResult[threadId];
  }
  return result;
}*/

int main(int argc, char *argv[]) {
  // Initialize base array
  cout << "Initializing array -- size: " << SIZE << endl;
  auto arr = std::vector<unsigned int/*, boost::alignment::aligned_allocator<unsigned int>*/ >(SIZE, 1);

  // Timekeeping
  double start;

  cout << "--- REDUCTION DEMO ---\n";
  // Normal summation
  cout << "Summing with normal summation\n";
  start = getWallTime();
  auto serialSumResult = serialSum(arr);
  cout << "\tresult: " << serialSumResult << endl;
  cout << "\ttime elapsed" << " : " << getWallTime()-start << " sec" << endl;

  // Parallel summation (one increment)
  cout << "Summing with one shared summation\n";
  start = getWallTime();
  auto parallelSumSharedResult = parallelSumOneAccumulate(arr);
  cout << "\tresult: " << parallelSumSharedResult << endl;
  cout << "\ttime elapsed" << " : " << getWallTime()-start << " sec" << endl;
  assert(parallelSumSharedResult == serialSumResult);

  /*// Parallel summation (false sharing)
  cout << "Summing with reduction (false sharing)\n";
  start = getWallTime();
  auto parallelSumPrivateFalseResult = parallelSumReduceFalseSharing(arr);
  cout << "\tresult: " << parallelSumPrivateFalseResult << endl;
  cout << "\ttime elapsed" << " : " << getWallTime()-start << " sec" << endl;
  assert(parallelSumPrivateFalseResult == serialSumResult);*/

  // Parallel summation (map and reduce)
  cout << "Summing with reduction\n";
  start = getWallTime();
  auto parallelSumPrivateResult = parallelSumReduce(arr);
  cout << "\tresult: " << parallelSumPrivateResult << endl;
  cout << "\ttime elapsed" << " : " << getWallTime()-start << " sec" << endl;
  assert(parallelSumPrivateResult == serialSumResult);

  return 0;
}
