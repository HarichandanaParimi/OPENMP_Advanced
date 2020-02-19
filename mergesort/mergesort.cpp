#include <omp.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

int chunk;
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
  void generateMergeSortData (int* arr, size_t n);
  void checkMergeSortResult (int* arr, size_t n);
#ifdef __cplusplus
}
#endif

void merge(int a[], int low, int mid, int high,int b[]) 
{ 
    int i, j, k; 
    int n1 = mid - low + 1; 
    int n2 =  high - mid; 
    for(i=low,j=mid+1,k=low;i<=mid &&j<=high;)
     {
        if(a[i] <a[j]){
           b[k] = a[i];
           i++; k++;
   	}
        else
        {
           b[k] = a[j];
           j++; k++;
         }           
     }
    if(i <= mid)
    {
     for(;i<=mid;i++,k++)
      b[k] = a[i];
    }
    if(j <= high)
    {
     for(;j<=high;j++,k++)
      b[k] = a[j];
    }
    for(i = low;i<=high;i++)
        a[i] = b[i];
	
}

void mergeSort(int arr[], int low, int high,int b[]) 
{
   if( low >= high)
       return;
   int mid = (low+high)/2; 
    if((high-low) <= chunk)
    {
      mergeSort(arr,low,mid,b);
      mergeSort(arr,mid+1,high,b);
      merge(arr,low,mid,high,b);
      return;
     }
    
    #pragma omp task untied firstprivate(arr,b,low,mid,chunk)
          mergeSort(arr,low,mid,b);
    #pragma omp task untied firstprivate(arr,b,high,mid,chunk)
          mergeSort(arr,mid+1,high,b);
    #pragma omp taskwait
    merge(arr,low,mid,high,b);
}


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
  
  if (argc < 3) { std::cerr<<"Usage: "<<argv[0]<<" <n> <nbthreads>"<<std::endl;
    return -1;
  }

  int n = atoi(argv[1]);
  
  // get arr data
  int *arr = new int [n];
  int *b = new int[n];
  generateMergeSortData (arr, n);
  int nbthreads = atoi(argv[2]);
  chunk = n/nbthreads;
  omp_set_num_threads(nbthreads);
  //insert sorting code here.

  double start = omp_get_wtime();
  #pragma omp parallel 
  {
   #pragma omp single
     mergeSort(arr,0,n-1,b);

   }
  double end = omp_get_wtime();
  
  cerr<<(end-start)<<endl;

  
  checkMergeSortResult (arr, n);
  
  delete[] arr;
  delete[] b;
  return 0;
}