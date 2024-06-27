#include <cuda.h>
#include <stdio.h>
#include <stdlib.h>

__global__ void mandelKernel(float stepX, float stepY, float lowerX, float lowerY, int* d_img, int maxIterations, int pitch) {
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

    d_img[thisX + thisY * pitch / 4] = i;
    return;
}

// Host front-end function that allocates the memory and launches the GPU kernel
void hostFE (float upperX, float upperY, float lowerX, float lowerY, int* img, int resX, int resY, int maxIterations)
{
    float stepX = (upperX - lowerX) / resX;
    float stepY = (upperY - lowerY) / resY;

    int *d_img;
    int *h_img;
    size_t pitch;

    // Allocate pinned memory space for host and pitched memory on the device.
    cudaHostAlloc(&h_img, resX * resY * sizeof(int), cudaHostAllocMapped);
    cudaMallocPitch((int **)&d_img, &pitch, sizeof(int) * resX, resY);
    
    // Based on the website "the CUDA programming guide" plus the CUDA version is V11.5, we have at most 1024 threads per block.
    dim3 blockSize(40, 25);
    dim3 numBlock(resX / blockSize.x, resY / blockSize.y);
    mandelKernel<<<numBlock, blockSize>>>(stepX, stepY, lowerX, lowerY, d_img, maxIterations, pitch);
    
    cudaDeviceSynchronize();

    // Copy the result back to host.
    cudaMemcpy2D(h_img, sizeof(int) * resX, d_img, pitch, sizeof(int) * resX, resY, cudaMemcpyDeviceToHost);
    memcpy(img, h_img, resX * resY * sizeof(int));
    
    cudaFree(d_img);
    cudaFreeHost(h_img);
    return;
}
