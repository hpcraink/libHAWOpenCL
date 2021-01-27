/*
 * Small test to show the usage of the HAWOpenCL API.
 * This test has KERNEL_SOURCE defined inline -- not very good to code...
 * 
 * 
 */
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif
#include <stdlib.h>

#include "HAWOpenCL.h"

#define LEN (1024*1024)
#define USE_DEVICE_TYPE (CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_CPU)
#define USE_DEVICE_NUM  0

const char KERNEL_SOURCE[] = "\n" \
    "__kernel void vector_add(__global int* a, __global int* b, const unsigned int count)\n"
    "{\n"
    "  const size_t i = get_global_id(0);\n"
    "  if (i < count) {\n"
    "    a[i] += b[i];\n"
    "  }\n"
    "}\n";

int main(int argc, char * argv[]) {
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_kernel kernel;
    cl_mem cl_a;
    cl_mem cl_b;
    unsigned int count = LEN;
    int i;
    int * a;
    int * b;
    size_t global;
    cl_uint haw_devices_num;
    hawopencl_device * haw_devices;
    hawopencl_kernel kernel_info;
    
    opencl_get_devices(USE_DEVICE_TYPE, &haw_devices_num, &haw_devices);
    if (0 == haw_devices_num)
        FATAL_ERROR("No devices of device type available; please change USE_DEVICE_TYPE", EINVAL);
    for (i = 0; i < haw_devices_num; i++) {
        printf ("%d. OpenCL device:%s\n",
                i, haw_devices[i].device_name);
    }
    // Select any of the above devices, OR select one specific preferred device.
    // opencl_init(USE_DEVICE_TYPE, 0, &device_id, &context, &command_queue);
    opencl_init(USE_DEVICE_TYPE, haw_devices[USE_DEVICE_NUM].device_id, &device_id, &context, &command_queue);
    // opencl_print_device(device_id);
    opencl_kernel_build(KERNEL_SOURCE, "vector_add",
                        device_id, context, &kernel);
    
    a = (int*) malloc(sizeof(int) * count);
    b = (int*) malloc(sizeof(int) * count);
    if (!a || !b)
        FATAL_ERROR("Failed to allocate host memory", ENOMEM);
    
    for (i = 0; i < count; i++) {
        a[i] = 1;
        b[i] = 1;
    }

    // Now we could execute on the device
    cl_a = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int) * count, a, NULL);
    cl_b = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(int) * count, b, NULL);
    if (!cl_a || !cl_b)
        FATAL_ERROR("Failed to allocate device memory", ENOMEM);

    // Set the arguments to our compute kernel
    OPENCL_CHECK(clSetKernelArg, (kernel, 0, sizeof(cl_mem), &cl_a));
    OPENCL_CHECK(clSetKernelArg, (kernel, 1, sizeof(cl_mem), &cl_b));
    OPENCL_CHECK(clSetKernelArg, (kernel, 2, sizeof(unsigned int), &count));

    opencl_kernel_info (kernel, device_id, &kernel_info);
    // opencl_kernel_print_info (kernel_info);
    
    global = count;
    OPENCL_CHECK(clEnqueueNDRangeKernel, (command_queue, kernel, 1, NULL,
            &global, &kernel_info.work_group_size, 0, NULL, NULL));

    OPENCL_CHECK(clFinish, (command_queue));

    OPENCL_CHECK(clEnqueueReadBuffer, (command_queue, cl_a, CL_TRUE, 0, sizeof(int) * count, a, 0, NULL, NULL));
    
    for (i=0 ; i < count; i++) {
        if (a[i] != 2)
            break;
    }
    if (i != count) {
        FATAL_ERROR("Check error at position", i);
    }
    printf("Test vector_add finished successfully.\n");
#if 0 // Just for debugging
    for (i=0 ; i < count; i++) {
        printf("a[%d]:%d\n", i, a[i]);
    }
#endif
    return 0;
}
