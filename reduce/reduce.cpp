#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<chrono>
#include <unistd.h>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

  void generateReduceData (int* arr, size_t n);

#ifdef __cplusplus
}
#endif



int main (int argc, char* argv[]) {
  //forces openmp to create the threads beforehand
#pragma omp parallel
  {
    int fd = open (argv[0], O_RDONLY);
    if (fd != -1) {
      close (fd);
    }
    else {
      std::cerr<<"something is amiss"<<std::endl;
    }
  }

  if (argc < 3) {
    std::cerr<<"usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }


  int n = atoi(argv[1]);
  int nbthreads= atoi(argv[2]);
  int granularity = n/nbthreads;
  int * arr = new int [n];
  int sum=0,i,threadid;

  generateReduceData (arr, atoi(argv[1]));
  //insert reduction code here
  auto start_time = std::chrono::system_clock::now();

 omp_set_num_threads(nbthreads);
  #pragma omp parallel default(shared) private(threadid,i) 
		{
	   int start, end, usum=0;
           threadid= omp_get_thread_num();
           start= ((threadid)*granularity);
           end= ((threadid+1)*granularity);
	   if((end+(1)*granularity)>n)
	   {
end=n;
	   }
		#pragma omp task firstprivate(usum)
	   {
//		 cout<<(start)<<":end:"<<(end);
                 for(i=start; i<end; i++){
			usum+= arr[i] ;
		}
	    
#pragma omp taskwait 
	    {
    		   sum+=usum;
	    }
	}
		}
  auto end_time = std::chrono::system_clock::now();
  std::chrono::duration<double> executiontime = end_time-start_time;
  std::cout<<sum<<std::endl;

  std::cerr<<executiontime.count()<<std::endl;


  delete[] arr;

  return 0;
}
