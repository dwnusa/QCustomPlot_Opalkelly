#include <stdio.h>
#include <cuda.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

__global__ void kernel()
{
	// ...
	int kk = 1;
}

extern "C" void launch_kernel()
{
	printf("RUN CUDA KERNEL\n");
	kernel << <1, 1 >> >();
}