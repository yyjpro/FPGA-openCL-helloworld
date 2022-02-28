#include <stdio.h>
#include <stdlib.h>
#include "CL/cl.h"
unsigned char *load_aocx(const char *aocx_filename, size_t *aocx_len)
{
    FILE *binary_file = NULL;
    if (NULL == (binary_file = fopen(aocx_filename, "rb")))
    {
        printf("Cannot open fpga binary file\n");
        return NULL;
    }
    fseek(binary_file, 0, SEEK_END);
    size_t binary_lenth = ftell(binary_file);
    unsigned char *binary_context = NULL;
    if (NULL == (binary_context = (unsigned char *)malloc(sizeof(unsigned char) * binary_lenth + 1)))
    {
        printf("Cannot allocate more memory for binary context\n");
        fclose(binary_file);
        return NULL;
    }
    rewind(binary_file);
    fread(binary_context, sizeof(unsigned char), binary_lenth, binary_file);
    binary_context[binary_lenth] = '\0';
    fclose(binary_file);
    *aocx_len = binary_lenth;
    return binary_context;
}

int main(int argc, char *argv[])
{
    size_t binary_lenth = 0;
    unsigned char *binary_context = load_aocx("helloworld.aocx", &binary_lenth);
    if (NULL == binary_context)
    {
        return -1;
    }
    cl_int err = 0;
    cl_platform_id *platform = NULL;
    cl_uint platform_num = 0;
    err = clGetPlatformIDs(0, NULL, &platform_num);
    if (CL_SUCCESS != err)
    {
        printf("Cannot found any OpenCL Platform on this system.\n");
        return -1;
    }
    platform = (cl_platform_id *)malloc(sizeof(cl_platform_id) * platform_num);
    if (NULL == platform)
    {
        printf("Cannot allocate more memory for OpenCL Platform\n");
        return -1;
    }
    err = clGetPlatformIDs(1, platform, 0);
    if (CL_SUCCESS != err)
    {
        printf("Cannot init the OpenCL Platform entry\n");
        return -1;
    }
    cl_device_id *device = NULL;
    cl_uint device_num = 0;
    err = clGetDeviceIDs(*platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &device_num);
    if (CL_SUCCESS != err)
    {
        printf("Cannot found any OpenCL Accelerator Device on this Platform\n");
        return -1;
    }
    device = (cl_device_id *)malloc(sizeof(cl_device_id) * device_num);
    if (NULL == device)
    {
        printf("Cannot allocate more memory for OpenCL Device\n");
        return -1;
    }
    err = clGetDeviceIDs(*platform, CL_DEVICE_TYPE_ACCELERATOR, 1, device, NULL);
    if (CL_SUCCESS != err)
    {
        printf("Cannot init the OpenCL Device\n");
        return -1;
    }
    cl_context context = clCreateContext(NULL, 1, &(device[0]), NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueue(context, device[0], 0, &err);

    cl_program program = clCreateProgramWithBinary(context, 1, &(device[0]), &binary_lenth,
                                                   (const unsigned char **)(&binary_context), NULL, &err);
    err = clBuildProgram(program, 1, &(device[0]), "", NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "helloworld", &err);
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int b[10] = {11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
    int c[10];

    cl_mem a_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * 10, a, &err);
    cl_mem b_buffer = clCreateBuffer(
        context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
        sizeof(int) * 10, b, &err);
    cl_mem c_buffer = clCreateBuffer(
        context, CL_MEM_WRITE_ONLY,
        sizeof(int) * 10, NULL, &err);
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &a_buffer);
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &b_buffer);
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &c_buffer);

    err = clEnqueueTask(queue, kernel, 0, NULL, NULL);
    err = clFinish(queue);

    err = clEnqueueReadBuffer(queue, c_buffer, CL_TRUE, 0,
                              sizeof(int) * 10, c, 0, NULL, NULL);
    int i = 0;
    printf("The output is :\n");
    for (i = 0; i < 10; i++)
    {
        printf("%d\t", c[i]);
    }
    printf("\n");

    clReleaseMemObject(a_buffer);
    clReleaseMemObject(b_buffer);
    clReleaseMemObject(c_buffer);

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    if (NULL != binary_context)
    {
        free(binary_context);
        binary_context = NULL;
    }

    return 0;
}
