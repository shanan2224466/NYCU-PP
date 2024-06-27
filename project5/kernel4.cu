#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>

__global__ void mandelKernel(float stepX, float stepY, float lowerX, float lowerY, int* d_img, int maxIterations) {
    // To avoid error caused by the floating number, use the following pseudo code
    //
    // float x = lowerX + thisX * stepX;
    // float y = lowerY + thisY * stepY;

    int thisX = blockIdx.x * blockDim.x + threadIdx.x;
    int thisY = blockIdx.y * blockDim.y + threadIdx.y;
    float c_re = lowerX + thisX * stepX;
    float c_im = lowerY + thisY * stepY;

    float z_re = c_re, z_im = c_im;
    int i;
    for (i = 0; i < maxIterations; ++i)
    {
        if (z_re * z_re + z_im * z_im > 4.f)
            break;

        float new_re = z_re * z_re - z_im * z_im;
        float new_im = 2.f * z_re * z_im;
        z_re = c_re + new_re;
        z_im = c_im + new_im;
    }

    d_img[thisX + thisY * gridDim.x * blockDim.x] = i;
}


// Host front-end function that allocates the memory and launches the GPU kernel
void hostFE (float upperX, float upperY, float lowerX, float lowerY, int* img, int resX, int resY, int maxIterations)
{
    float stepX = (upperX - lowerX) / resX;
    float stepY = (upperY - lowerY) / resY;

    int *d_img;

    // Allocate memory space for host and device.
    cudaMalloc((void **)&d_img, resX * resY * sizeof(int));

    // Based on the website "the CUDA programming guide" plus the CUDA version is V11.5, we have at most 1024 threads per block.
    dim3 blockSize(16, 12);
    dim3 numBlock(resX / blockSize.x, resY / blockSize.y);
    mandelKernel <<<numBlock, blockSize>>> (stepX, stepY, lowerX, lowerY, d_img, maxIterations);

    // Copy the result back to host.
    cudaMemcpy(img, d_img, sizeof(int) * resX * resY, cudaMemcpyDeviceToHost);

    cudaFree(d_img);
}
