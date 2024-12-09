//
//  opencl_kernel_print_info.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015-2016 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2019 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <string.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "HAWOpenCL.h"

// Macro evaluates to two arguments: the parameter and it's String value.
#define OPENCL_NAME(x) (x), #x

struct address_qualifier {
    cl_kernel_arg_access_qualifier adq;
    char * str;
    char * address_qualifier;
    char * desription;
} address_qualifier[] = {
    {OPENCL_NAME(CL_KERNEL_ARG_ADDRESS_GLOBAL), "__global", "Global address space"},
    {OPENCL_NAME(CL_KERNEL_ARG_ADDRESS_LOCAL), "__local", "Local address space"},
    {OPENCL_NAME(CL_KERNEL_ARG_ADDRESS_CONSTANT), "__constant", "Constant address space"},
    {OPENCL_NAME(CL_KERNEL_ARG_ADDRESS_PRIVATE), "__private", "Private address space (default)‚"}
};


struct type_qualifier {
    cl_kernel_arg_type_qualifier tq;
    char * str;
    int combination_allowed;
    char * type_qualifier;
    char * desription;
} type_qualifier[] = {
    {OPENCL_NAME(CL_KERNEL_ARG_TYPE_CONST), 1, "const", "Constant pointer type"},
    {OPENCL_NAME(CL_KERNEL_ARG_TYPE_RESTRICT), 1, "restrict", "Restricted pointer type"},
    {OPENCL_NAME(CL_KERNEL_ARG_TYPE_VOLATILE), 1, "volatile", "Volatile pointer type"},
#if defined (CL_KERNEL_ARG_TYPE_PIPE)
    {OPENCL_NAME(CL_KERNEL_ARG_TYPE_PIPE), 0, "pipe", "Pipe type"},
#endif
    {OPENCL_NAME(CL_KERNEL_ARG_TYPE_NONE), 0, "none", "No type qualifier specified"}
};

struct access_qualifier {
    cl_kernel_arg_access_qualifier aq;
    char * str;
    char * access_qualifier;
    char * desription;
} access_qualifier[] = {
    {OPENCL_NAME(CL_KERNEL_ARG_ACCESS_READ_ONLY), "__read_only", "Read only access"},
    {OPENCL_NAME(CL_KERNEL_ARG_ACCESS_WRITE_ONLY), "__write_only", "Write only access"},
    {OPENCL_NAME(CL_KERNEL_ARG_ACCESS_READ_WRITE), "__read_write", "Read and write access"},
    {OPENCL_NAME(CL_KERNEL_ARG_ACCESS_NONE), "none", "No access qualifier specified"},
};

int opencl_kernel_print_info(const hawopencl_kernel ki) {
    unsigned int idx;
    printf("---------- Kernel Information: %s ----------\n",
            ki.kernel_function_name);
    printf("CL_KERNEL_FUNCTION_NAME: %s\n",
            (ki.kernel_function_name == NULL || !strcmp(ki.kernel_function_name, "")) ?
                " (none)" : ki.kernel_function_name);
    printf("CL_KERNEL_ATTRIBUTES: %s\n",
            (ki.kernel_attributes == NULL || !strcmp(ki.kernel_attributes, "")) ?
                " (none)" : ki.kernel_attributes);
    printf("CL_KERNEL_NUM_ARGS: %u\n",
            ki.kernel_num_args);
    for (idx = 0; idx < ki.kernel_num_args; idx++) {
        unsigned int i;
        unsigned int printed;
        printf("---------- %d. Argument ----------\n", idx + 1);
        printf("  CL_KERNEL_ARG_TYPE_NAME:%s\n", 
                (ki.args[idx].arg_type_name == NULL || !strcmp(ki.args[idx].arg_type_name, "")) ?
                    " (none)" : ki.args[idx].arg_type_name);
        printf("  CL_KERNEL_ARG_NAME:%s\n",
                (ki.args[idx].arg_name == NULL || !strcmp(ki.args[idx].arg_name, "")) ?
                    " (none)" : ki.args[idx].arg_name);

        printf("  CL_KERNEL_ARG_ADDRESS_QUALIFIER:");
        for (printed = i = 0; i < (sizeof (address_qualifier) / sizeof (address_qualifier[0])); i++) {
            if (ki.args[idx].address_qualifier == address_qualifier[i].adq) {
                printf(" %s", address_qualifier[i].address_qualifier);
                printed = 1;
            }
        }
        if (!printed)
            printf(" (none)");
        printf("\n");

        printf("  CL_KERNEL_ARG_TYPE_QUALIFIER:");
        for (printed = i = 0; i < (sizeof (type_qualifier) / sizeof (type_qualifier[0])); i++) {
            if (type_qualifier[i].combination_allowed) {
                if (ki.args[idx].type_qualifier & type_qualifier[i].tq) {
                    printf(" %s", type_qualifier[i].type_qualifier);
                    printed = 1;
                }
            } else {
                if (ki.args[idx].type_qualifier == type_qualifier[i].tq) {
                    printf(" %s", type_qualifier[i].type_qualifier);
                    printed = 1;
                }
            }
        }
        if (!printed)
            printf(" (none)");
        printf("\n");

        printf("  CL_KERNEL_ARG_ACCESS_QUALIFIER:");
        for (printed = i = 0; i < sizeof (access_qualifier) / sizeof (access_qualifier[0]); i++) {
            if (ki.args[idx].access_qualifier == access_qualifier[i].aq) {
                printf(" %s", access_qualifier[i].access_qualifier);
                printed = 1;
            }
        }
        if (!printed)
            printf(" (none)");
        printf("\n");

    }

    printf("CL_KERNEL_GLOBAL_WORK_SIZE: (%lu, %lu, %lu)\n",
            (long unsigned int) ki.global_work_size[0],
            (long unsigned int) ki.global_work_size[1],
            (long unsigned int) ki.global_work_size[2]);
    printf("CL_KERNEL_COMPILE_WORK_GROUP_SIZE: (%lu, %lu, %lu)\n",
            (long unsigned int) ki.compile_work_group_size[0],
            (long unsigned int) ki.compile_work_group_size[1],
            (long unsigned int) ki.compile_work_group_size[2]);
    printf("CL_KERNEL_WORK_GROUP_SIZE: %lu\n",
            (long unsigned int) ki.work_group_size);
    printf("CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE: %lu\n",
            (long unsigned int) ki.preferred_work_group_size_multiple);
    printf("CL_KERNEL_LOCAL_MEM_SIZE: %llu\n",
            (unsigned long long) ki.local_mem_size);
    printf("CL_KERNEL_PRIVATE_MEM_SIZE: %llu\n",
            (unsigned long long) ki.private_mem_size);
    return CL_SUCCESS;
}
