//
//  opencl_kernel_load.c : Part of libHAWOpenCL
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

#define BUILD_LOG(cl_program, device_id, build_param, log, len) do {           \
        size_t __len = 0;                                                      \
        clGetProgramBuildInfo((cl_program), (device_id), (build_param),        \
                              0, NULL, &__len);                                \
        if (len >= 0) {                                                        \
            log = (char *) realloc(log, __len);                                \
        } else {                                                               \
            log = (char *) malloc(__len);                                      \
        }                                                                      \
        if (NULL == log)                                                       \
            FATAL_ERROR("malloc", ENOMEM);                                     \
        len = __len;                                                           \
        clGetProgramBuildInfo((cl_program), (device_id), (build_param),        \
                              len, log, NULL);                                 \
    } while(0)

int opencl_kernel_build(const char * kernel_source,
        const char * kernel_name,
        const cl_device_id device_id,
        const cl_context context,
        cl_kernel * kernel) {
    int err;
    cl_program cl_program;
    char * pwd_env = NULL;
    char * compile_option = NULL;
    int compile_option_len;

    compile_option_len = strlen("-cl-kernel-arg-info");
    compile_option = strdup("-cl-kernel-arg-info");

    // If this env. var. is set, pass this directory as -I option 
    // twice to the compiler, aka the length has to be twice plus some overhead.
    pwd_env = getenv("OPENCL_KERNEL_PATH");
    if (NULL != pwd_env) {
        compile_option_len += 2 * strlen(pwd_env) + 256;

        compile_option = (char*) realloc(compile_option, compile_option_len);
        if (NULL == compile_option)
            FATAL_ERROR("realloc", ENOMEM);
        compile_option_len = snprintf(compile_option, compile_option_len,
                "-cl-kernel-arg-info -I %s -I %s/../include/",
                pwd_env, pwd_env);
    }

    cl_program = clCreateProgramWithSource(context, 1, (const char**) &kernel_source, NULL, &err);
    if (!cl_program || err != CL_SUCCESS)
        FATAL_ERROR("clCreateProgramWithSource", err);

    // Build Program -- only in case of error report the build-log.
    err = clBuildProgram(cl_program, 0, NULL, compile_option, NULL, NULL);
    if (CL_SUCCESS != err) {
        char * build_log = NULL;
        size_t len = 0;

        printf("--------------------------------------\n");
        printf("Building of Kernel '%s' failed:\n", kernel_name);

        BUILD_LOG(cl_program, device_id, CL_PROGRAM_BUILD_OPTIONS, build_log, len);
        printf("Build Options (len:%zd):\n%s\n", len, build_log);

        BUILD_LOG(cl_program, device_id, CL_PROGRAM_BUILD_LOG, build_log, len);
        printf("Build Info Log (len:%zd):\n%s\n", len, build_log);
        printf("--------------------------------------\n");

        free(build_log);
        free(compile_option);

        FATAL_ERROR("clBuildProgram", err);
    }

    // Create the kernel
    *kernel = clCreateKernel(cl_program, kernel_name, &err);
    if (!*kernel || CL_SUCCESS != err)
        FATAL_ERROR("clCreateKernel", err);
#if defined (CL_KERNEL_BINARY_PROGRAM_INTEL)
    err = clGetKernelInfo(*kernel, CL_KERNEL_BINARY_PROGRAM_INTEL, 0, NULL, &len);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetKernelInfo", err);
    fprintf(stderr, "INFO: CL_KERNEL_BINARY_PROGRAM_INTEL available len:%d\n", len);
#endif

    err = clReleaseProgram(cl_program);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clReleaseProgram", err);
    free(compile_option);
    return CL_SUCCESS;
}
