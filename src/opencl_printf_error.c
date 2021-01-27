//
//  opencl_get_error_string.c : Part of libHAWOpenCL.a
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

int opencl_printf_error(cl_int status, unsigned int len, char * print_buffer)
{
    int ret;
    const char * error_string;
    switch (status)
    {
        case CL_SUCCESS:
            error_string = "CL_SUCCESS";
            break;
        case CL_BUILD_PROGRAM_FAILURE:
            error_string = "CL_BUILD_PROGRAM_FAILURE";
            break;
        case CL_COMPILE_PROGRAM_FAILURE:
            error_string = "CL_COMPILE_PROGRAM_FAILURE";
            break;
        case CL_COMPILER_NOT_AVAILABLE:
            error_string = "CL_COMPILER_NOT_AVAILABLE";
            break;
        case CL_DEVICE_NOT_AVAILABLE:
            error_string = "CL_DEVICE_NOT_AVAILABLE";
            break;
        case CL_DEVICE_NOT_FOUND:
            error_string = "CL_DEVICE_NOT_FOUND";
            break;
        case CL_DEVICE_PARTITION_FAILED:
            error_string = "CL_DEVICE_PARTITION_FAILED";
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            error_string = "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
            break;
        case CL_IMAGE_FORMAT_MISMATCH:
            error_string = "CL_IMAGE_FORMAT_MISMATCH";
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            error_string = "CL_IMAGE_FORMAT_NOT_SUPPORTED";
            break;
        case CL_INVALID_ARG_INDEX:
            error_string = "CL_INVALID_ARG_INDEX";
            break;
        case CL_INVALID_ARG_SIZE:
            error_string = "CL_INVALID_ARG_SIZE";
            break;
        case CL_INVALID_ARG_VALUE:
            error_string = "CL_INVALID_ARG_VALUE";
            break;
        case CL_INVALID_BINARY:
            error_string = "CL_INVALID_BINARY";
            break;
        case CL_INVALID_BUFFER_SIZE:
            error_string = "CL_INVALID_BUFFER_SIZE";
            break;
        case CL_INVALID_BUILD_OPTIONS:
            error_string = "CL_INVALID_BUILD_OPTIONS";
            break;
        case CL_INVALID_COMMAND_QUEUE:
            error_string = "CL_INVALID_COMMAND_QUEUE";
            break;
        case CL_INVALID_COMPILER_OPTIONS:
            error_string = "CL_INVALID_COMPILER_OPTIONS";
            break;
        case CL_INVALID_CONTEXT:
            error_string = "CL_INVALID_CONTEXT";
            break;
        case CL_INVALID_DEVICE:
            error_string = "CL_INVALID_DEVICE";
            break;
        case CL_INVALID_DEVICE_PARTITION_COUNT:
            error_string = "CL_INVALID_DEVICE_PARTITION_COUNT";
            break;
        case CL_INVALID_DEVICE_TYPE:
            error_string = "CL_INVALID_DEVICE_TYPE";
            break;
        case CL_INVALID_EVENT:
            error_string = "CL_INVALID_EVENT";
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            error_string = "CL_INVALID_EVENT_WAIT_LIST";
            break;
        case CL_INVALID_GL_OBJECT:
            error_string = "CL_INVALID_GL_OBJECT";
            break;
        case CL_INVALID_GLOBAL_OFFSET:
            error_string = "CL_INVALID_GLOBAL_OFFSET";
            break;
        case CL_INVALID_GLOBAL_WORK_SIZE:
            error_string = "CL_INVALID_GLOBAL_WORK_SIZE";
            break;
        case CL_INVALID_HOST_PTR:
            error_string = "CL_INVALID_HOST_PTR";
            break;
        case CL_INVALID_IMAGE_DESCRIPTOR:
            error_string = "CL_INVALID_IMAGE_DESCRIPTOR";
            break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            error_string = "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
            break;
        case CL_INVALID_IMAGE_SIZE:
            error_string = "CL_INVALID_IMAGE_SIZE";
            break;
        case CL_INVALID_KERNEL_NAME:
            error_string = "CL_INVALID_KERNEL_NAME";
            break;
        case CL_INVALID_KERNEL:
            error_string = "CL_INVALID_KERNEL";
            break;
        case CL_INVALID_KERNEL_ARGS:
            error_string = "CL_INVALID_KERNEL_ARGS";
            break;
        case CL_INVALID_KERNEL_DEFINITION:
            error_string = "CL_INVALID_KERNEL_DEFINITION";
            break;
        case CL_INVALID_LINKER_OPTIONS:
            error_string = "CL_INVALID_LINKER_OPTIONS";
            break;
        case CL_INVALID_MEM_OBJECT:
            error_string = "CL_INVALID_MEM_OBJECT";
            break;
        case CL_INVALID_MIP_LEVEL:
            error_string = "CL_INVALID_MIP_LEVEL";
            break;
        case CL_INVALID_OPERATION:
            error_string = "CL_INVALID_OPERATION";
            break;
        case CL_INVALID_PLATFORM:
            error_string = "CL_INVALID_PLATFORM";
            break;
        case CL_INVALID_PROGRAM:
            error_string = "CL_INVALID_PROGRAM";
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            error_string = "CL_INVALID_PROGRAM_EXECUTABLE";
            break;
        case CL_INVALID_PROPERTY:
            error_string = "CL_INVALID_PROPERTY";
            break;
        case CL_INVALID_QUEUE_PROPERTIES:
            error_string = "CL_INVALID_QUEUE_PROPERTIES";
            break;
        case CL_INVALID_SAMPLER:
            error_string = "CL_INVALID_SAMPLER";
            break;
        case CL_INVALID_VALUE:
            error_string = "CL_INVALID_VALUE";
            break;
        case CL_INVALID_WORK_DIMENSION:
            error_string = "CL_INVALID_WORK_DIMENSION";
            break;
        case CL_INVALID_WORK_GROUP_SIZE:
            error_string = "CL_INVALID_WORK_GROUP_SIZE";
            break;
        case CL_INVALID_WORK_ITEM_SIZE:
            error_string = "CL_INVALID_WORK_ITEM_SIZE";
            break;
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
            error_string = "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
            break;
        case CL_LINKER_NOT_AVAILABLE:
            error_string = "CL_LINKER_NOT_AVAILABLE";
            break;
        case CL_LINK_PROGRAM_FAILURE:
            error_string = "CL_LINK_PROGRAM_FAILURE";
            break;
        case CL_MAP_FAILURE:
            error_string = "CL_MAP_FAILURE";
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            error_string = "CL_MEM_OBJECT_ALLOCATION_FAILURE";
            break;
        case CL_MEM_COPY_OVERLAP:
            error_string = "CL_MEM_COPY_OVERLAP";
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            error_string = "CL_MISALIGNED_SUB_BUFFER_OFFSET";
            break;
        case CL_OUT_OF_HOST_MEMORY:
            error_string = "CL_OUT_OF_HOST_MEMORY";
            break;
        case CL_OUT_OF_RESOURCES:
            error_string = "CL_OUT_OF_RESOURCES";
            break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            error_string = "CL_PROFILING_INFO_NOT_AVAILABLE";
            break;
    }
    ret = snprintf(print_buffer, len-1, "OpenCL error:%s", error_string);
    print_buffer[ret] = '\0';
    return status;
}
