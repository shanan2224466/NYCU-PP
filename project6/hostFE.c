#include <stdio.h>
#include <stdlib.h>
#include "hostFE.h"
#include "helper.h"

void hostFE(int filterWidth, float *filter, int imageHeight, int imageWidth,
            float *inputImage, float *outputImage, cl_device_id *device,
            cl_context *context, cl_program *program)
{
    cl_int status;
    int filterSize = filterWidth * filterWidth * sizeof(float);
    int inputSize = imageHeight * imageWidth * sizeof(float);

    // Create the kernel.
    cl_kernel kernel = clCreateKernel(*program, "convolution", &status);
    CHECK(status, "clCreateKernel");

    // Allocate the device memory and copy data to the device.
    cl_mem inputBuffer = clCreateBuffer(*context, CL_MEM_USE_HOST_PTR, inputSize, inputImage, &status);
    CHECK(status, "clCreateinputBuffer");
    cl_mem filterBuffer = clCreateBuffer(*context, CL_MEM_USE_HOST_PTR, filterSize, filter, &status);
    CHECK(status, "clCreatefilterBuffer");
    cl_mem outputBuffer = clCreateBuffer(*context, CL_MEM_WRITE_ONLY, inputSize, NULL, &status);
    CHECK(status, "clCreateoutputBuffer");

    // Create the command queue.
    cl_command_queue queue = clCreateCommandQueue(*context, *device, 0, NULL);
    CHECK(status, "clCreateCommandQueue");

    // Set the kernel arguments.
    status = clSetKernelArg(kernel, 0, sizeof(int), (void*)&imageWidth);
    CHECK(status, "clSetKernelArg0");
    status = clSetKernelArg(kernel, 1, sizeof(int), (void*)&imageHeight);
    CHECK(status, "clSetKernelArg1");
    status = clSetKernelArg(kernel, 2, sizeof(int), (void*)&filterWidth);
    CHECK(status, "clSetKernelArg2");
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void*)&inputBuffer);
    CHECK(status, "clSetKernelArg3");
    status = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void*)&filterBuffer);
    CHECK(status, "clSetKernelArg4");
    status = clSetKernelArg(kernel, 5, sizeof(cl_mem), (void*)&outputBuffer);
    CHECK(status, "clSetKernelArg5");

    int numVector = (imageWidth * imageHeight) / 4;
    size_t globalWorkgroupSize[2] = {numVector, 1};

    // Execute the kernel.
    status = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, globalWorkgroupSize, NULL, 0, NULL, NULL);
    CHECK(status, "clEnqueueNDRangeKernel");

    // Copy the data from device to host memeory.
    status = clEnqueueReadBuffer(queue, outputBuffer, CL_TRUE, 0, inputSize, (void*)outputImage, 0, NULL, NULL);
    CHECK(status, "clEnqueueReadBuffer");

    status = clReleaseKernel(kernel);
    CHECK(status, "clReleaseKernel");
    status = clReleaseCommandQueue(queue);
    CHECK(status, "clReleaseCommandQueue");
    status = clReleaseMemObject(inputBuffer);
    CHECK(status, "clReleaseinputBuffer");
    status = clReleaseMemObject(filterBuffer);
    CHECK(status, "clReleasefilterBuffer");
    status = clReleaseMemObject(outputBuffer);
    CHECK(status, "clReleaseoutputBuffer");

    return;
}