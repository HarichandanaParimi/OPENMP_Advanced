#include <omp.h>
#include <stdio.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <math.h>
#include <stdlib.h>
#include <chrono>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

  void generateLCS(char* X, int m, char* Y, int n);
  void checkLCS(char* X, int m, char* Y, int n, int result);

#ifdef __cplusplus
}
#endif

int max(int p, int q)
{
	return (p > q)? p : q;
}

int lcs( char *X, char *Y, int m, int n, int nbthreads )
{
	int chunk = 500;
	if(n<=10)
		chunk = 50;
	else
		chunk = 5*n*0.01;

	int** K = new int*[m+1];
	for (int i=0; i<=m; ++i) {
		K[i] = new int[n+1];
	}

	int i, j;

	#pragma omp parallel for schedule(guided,chunk)
	for (i=0; i<=m; i++)
	{
		for (j=0; j<=n; j++)
		{
			if (i == 0 || j == 0)
				K[i][j] = 0;

			else if (X[i-1] == Y[j-1])
				K[i][j] = K[i-1][j-1] + 1;

			else
				K[i][j] = max(K[i-1][j], K[i][j-1]);
		}
	}

	int result = K[m][n];

	#pragma omp taskwait
	for (int i=0; i<=m; ++i) {
		delete[] K[i];
	}
	delete[] K;

	return result;

}



int main (int argc, char* argv[]) {

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

	if (argc < 4) { std::cerr<<"usage: "<<argv[0]<<" <m> <n> <nbthreads>"<<std::endl;
		return -1;
	}

	int nbthreads = atoi(argv[3]);
	omp_set_num_threads(nbthreads);

	int m = atoi(argv[1]);
	int n = atoi(argv[2]);

	// get string data
	char *X = new char[m];
	char *Y = new char[n];
	generateLCS(X, m, Y, n);


	std::chrono::time_point<std::chrono::system_clock> start_time = std::chrono::system_clock::now();

	int lcs_result = 0;

	#pragma omp parallel
	#pragma omp single nowait
	lcs_result = lcs( X, Y, m, n , nbthreads) ;

	std::chrono::time_point<std::chrono::system_clock> end_time = std::chrono::system_clock::now();
	std::chrono::duration<double> time = end_time-start_time;

	checkLCS(X, m, Y, n, lcs_result);
	std::cerr<<time.count()<<std::endl;

	delete[] X;
	delete[] Y;

	return 0;
}
