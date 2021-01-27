//
//  opencl_kernel_info.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2019 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "HAWOpenCL.h"

/*************** Macro definitions ***************/
#define GETKERNELINFO(cl_kernel, param, ptr) do {                              \
        size_t __len = 0;                                                      \
        int __err;                                                             \
        __err = clGetKernelInfo((cl_kernel), (param),                          \
                        0, NULL, &__len);                                      \
        if (CL_SUCCESS != __err)                                               \
            FATAL_ERROR("clGetKernelInfo", __err);                             \
        (ptr) = NULL;                                                          \
        if (0 < __len) {                                                       \
            (ptr) = (char *) malloc(__len+1);                                  \
            if (NULL == ptr)                                                   \
                FATAL_ERROR("malloc", ENOMEM);                                 \
            __err = clGetKernelInfo((cl_kernel), (param),                      \
                                    __len, (ptr), NULL);                       \
            if (CL_SUCCESS != __err)                                           \
                FATAL_ERROR("clGetKernelInfo", __err);                         \
            (ptr)[__len+1] = '\0';                                             \
        }                                                                      \
        /* printf ("param:%d len:%lu val:%s\n", param, __len, ptr); */         \
    } while(0)

#define GETARGINFO(cl_kernel, idx, param, ptr) do {                            \
        size_t __len = 0;                                                      \
        int __err;                                                             \
        __err = clGetKernelArgInfo((cl_kernel), (idx), (param),                \
                        0, NULL, &__len);                                      \
        if (CL_SUCCESS != __err)                                               \
            FATAL_ERROR("clGetKernelArgInfo", __err);                          \
        (ptr) = NULL;                                                          \
        if (0 < __len) {                                                       \
            (ptr) = (char *) malloc(__len+1);                                  \
            if (NULL == ptr)                                                   \
                FATAL_ERROR("malloc", ENOMEM);                                 \
            __err = clGetKernelArgInfo((cl_kernel), (idx), (param),            \
                                       __len, (ptr), NULL);                    \
            if (CL_SUCCESS != __err)                                           \
                FATAL_ERROR("clGetKernelArgInfo", __err);                      \
            (ptr)[__len+1] = '\0';                                             \
        }                                                                      \
        /* printf ("idx:%d len:%lu val:%s\n", (idx), __len, (ptr)); */         \
    } while(0)

int opencl_kernel_info(const cl_kernel kernel,
        const cl_device_id device_id,
        hawopencl_kernel * kernel_info) {
    int err;
    int idx;

    memset(kernel_info, 0, sizeof (hawopencl_kernel));

    GETKERNELINFO(kernel, CL_KERNEL_FUNCTION_NAME, kernel_info->kernel_function_name);
    GETKERNELINFO(kernel, CL_KERNEL_ATTRIBUTES, kernel_info->kernel_attributes);

    err = clGetKernelInfo(kernel, CL_KERNEL_NUM_ARGS, sizeof (kernel_info->kernel_num_args), &kernel_info->kernel_num_args, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelInfo", err);

    // FIRST: All the Work group sizes
#if defined(CL_VERSION_1_0)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE,
            sizeof (kernel_info->work_group_size), &kernel_info->work_group_size,
            NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelWorkGroupInfo", err);
#endif
    
#if defined(CL_VERSION_1_0)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
            sizeof (kernel_info->compile_work_group_size), &kernel_info->compile_work_group_size,
            NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelWorkGroupInfo", err);
#endif

#if defined(CL_VERSION_1_2)
    /* glGetKernelWorkGroupInfo does not return proper information for
     * CL_KERNEL_GLOBAL_WORK_SIZE, unless this is a built-in kernel or this is 
     * custom device.
     * Therefore initialize to something "sane".
     */
    cl_device_type type;
    err = clGetDeviceInfo(device_id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetDeviceInfo", err);
    kernel_info->global_work_size[0] = 0;
    kernel_info->global_work_size[1] = 0;
    kernel_info->global_work_size[2] = 0;
    
    if (type == CL_DEVICE_TYPE_CUSTOM) {
        err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_GLOBAL_WORK_SIZE,
                sizeof (kernel_info->global_work_size), &kernel_info->global_work_size,
                NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetKernelWorkGroupInfo", err);
    }
#endif
  
#if defined(CL_VERSION_1_1)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
            sizeof (kernel_info->preferred_work_group_size_multiple), &kernel_info->preferred_work_group_size_multiple,
            NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelWorkGroupInfo", err);
#endif
    
    // NEXT: All the memory sizes
#if defined(CL_VERSION_1_0)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_LOCAL_MEM_SIZE,
            sizeof (kernel_info->local_mem_size), &kernel_info->local_mem_size,
            NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelWorkGroupInfo", err);
#endif

    
#if defined(CL_VERSION_1_1)
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_PRIVATE_MEM_SIZE,
            sizeof (kernel_info->private_mem_size), &kernel_info->private_mem_size,
            NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelWorkGroupInfo", err);
#endif


    kernel_info->args = (hawopencl_kernelarg*) malloc(sizeof (hawopencl_kernelarg) * kernel_info->kernel_num_args);
    if (NULL == kernel_info->args)
        FATAL_ERROR("malloc", ENOMEM);

    for (idx = 0; idx < kernel_info->kernel_num_args; idx++) {
        GETARGINFO(kernel, idx, CL_KERNEL_ARG_TYPE_NAME, kernel_info->args[idx].arg_type_name);
        GETARGINFO(kernel, idx, CL_KERNEL_ARG_NAME, kernel_info->args[idx].arg_name);

        err = clGetKernelArgInfo(kernel, idx, CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                sizeof (kernel_info->args[idx].address_qualifier), &kernel_info->args[idx].address_qualifier, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetKernelArgInfo", err);

        err = clGetKernelArgInfo(kernel, idx, CL_KERNEL_ARG_ACCESS_QUALIFIER,
                sizeof (kernel_info->args[idx].access_qualifier), &kernel_info->args[idx].access_qualifier, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetKernelArgInfo", err);

        err = clGetKernelArgInfo(kernel, idx, CL_KERNEL_ARG_TYPE_QUALIFIER,
                sizeof (kernel_info->args[idx].type_qualifier), &kernel_info->args[idx].type_qualifier, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetKernelArgInfo", err);
    }

    return CL_SUCCESS;
}
