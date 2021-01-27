/*
 *  Library to ease OpenCL Programming
 *
 *  Copyright (c) 2015-2016 Rainer Keller, HFT Stuttgart. All rights reserved.
 *  Copyright (c) 2018-2021 Rainer Keller, HS Esslingen. All rights reserved.
 *
 */

#ifndef HAWOPENCL_H
#define HAWOPENCL_H

#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <errno.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#define HAWOPENCL_LIBRARY_VERSION "libHAWOpenCL " # HAWOpenCL_VERSION_MAJOR # "." # HAWOpenCL_VERSION_MINOR

#ifndef FATAL_ERROR
#define FATAL_ERROR(func,errno) do { \
    fprintf(stderr, "(FILE:%s:%d) func:%s ; errno:%d\n", \
            __FILE__, __LINE__, (func), (errno)); \
    exit(errno); \
} while(0)
#endif

// Usage: OPENCL_CHECK(clFinish, (command_queue));
#define OPENCL_CHECK(func, args) do { \
    int __err = func args; \
    if (CL_SUCCESS != __err) \
        FATAL_ERROR(#func, __err); \
} while(0)

BEGIN_C_DECLS

/*********************** DATA STRUCTURES ***************************/

typedef struct {
    cl_device_id device_id;
    char * device_name;
} hawopencl_device;

typedef struct {
    cl_kernel_arg_address_qualifier address_qualifier;
    cl_kernel_arg_access_qualifier access_qualifier;
    char * arg_type_name;
    cl_kernel_arg_type_qualifier type_qualifier;
    char * arg_name;
} hawopencl_kernelarg;

typedef struct {
    // Information provided by clGetKernelInfo()
    char * kernel_function_name;
    cl_uint kernel_num_args;
    char * kernel_attributes;
    
    // cl_uint CL_KERNEL_REFERENCE_COUNT is not viable; since this is "stale" info, always
    // cl_context CL_KERNEL_CONTEXT returns the current context; not really helpful?
    // cl_program CL_KERNEL_PROGRAM returns the source program; not really helpful? Already freed?

    // Information provided by clGetKernelWorkGroupInfo
    // global_work_size is valid only for CL_DEVICE_TYPE_CUSTOM or built-in-kernels
    size_t global_work_size[3];
    size_t work_group_size;
    size_t compile_work_group_size[3];
    size_t preferred_work_group_size_multiple;
    cl_ulong local_mem_size;
    cl_ulong private_mem_size;

    // Information provided by clGetKernelArgInfo()
    // Array of size kernel_num_args (as above)
    hawopencl_kernelarg * args;
} hawopencl_kernel;

typedef struct {
    char ** event_names;
    cl_event * events;
    size_t * sizes;
    int idx;
    int max;
} hawopencl_profile_events;

/*********************** FUNCTION DEFINITIONS ***************************/

/**
 * Print information to stdout about a specific platform.
 * PLEASE NOTE: This is part of opencl_print_info().
 *
 * @param[in] cl_platform  The platform opened prior to this.
 *
 * @return 0 in case of success
 */
int opencl_print_platform(cl_platform_id cl_platform);

/**
 * Print information to stdout about a specific device.
 * PLEASE NOTE: This is part of opencl_print_info().
 *
 * @param[in] cl_device  The device opened prior to this.
 *
 * @return 0 in case of success
 */
int opencl_print_device(cl_device_id cl_device);

/**
 * Print complete information to stdout about attached OpenCL.
 * This includes platforms and each device.
 *
 * @return 0 in case of success
 */
int opencl_print_info(void);

/**
 * Get all the available device IDs.
 * This may be used in conjunction with opencl_print_device() or with
 * opencl_init() to select a preferred device.
 * 
 * @param[in] on_device_type Either CL_DEVICE_TYPE_CPU / GPU / ACCELERATOR,
 *                           a combination of the previous, or
 *                           CL_DEVICE_TYPE_CUSTOM
 * @param[out] num_devices   The number of matching devices in the array
 * @param[out] devices       The array of matching devices.
 * 
 * @return CL_SUCCESS in case of no error
 * @warn The User is responsible for freeing the devices array!
 */
int opencl_get_devices(const cl_device_type on_device_type, 
        cl_uint * num_devices,
        hawopencl_device ** devices) __HAW_OPENCL_ATTR_NONNULL__(2,3);


/**
 * Initialize OpenCL, searching for the first of the requested target
 *
 * @param[in] on_device_type Either CL_DEVICE_TYPE_CPU / GPU / ACCELERATOR,
 *                           a combination of the previous, or
 *                           CL_DEVICE_TYPE_CUSTOM
 * @param[in] preferred_device_id By default, the 1. matching device of given
 *                           type is selected, aka passing 0. This may be used
 *                           to select another device than the first device.
 * @param[out] device_id     The device id opened
 * @param[out] context       The context opened with the device
 * @param[out] command_queue The command queue to which further commands are send.
 * 
 * @return CL_SUCCESS in case of no error
 */
int opencl_init(const cl_device_type on_device_type,
        cl_device_id preferred_device_id,
        cl_device_id * device_id,
        cl_context * context,
        cl_command_queue * command_queue) __HAW_OPENCL_ATTR_NONNULL__(3,4,5);

/**
 * Print the provided error-status into the print-buffer of length len.
 *
 * @param[in]    status       The error status returned by any OpenCL function
 * @param[in]    len          The length of the print_buffer
 * @param[inout] print_buffer This buffer of length len is overwritten with the
 *                            clear-text description of the error (NUL-terminated) 
 *  
 * @return the error-status as passed in;
 *         CL_SUCCESS in case of no error
 */
int opencl_printf_error(cl_int status, unsigned int len, char * print_buffer) __HAW_OPENCL_ATTR_NONNULL__(3);

/**
 * Macro to exit in case of error status (!= CL_SUCCESS) with provided message.
 *
 * @param[in]   status        The status variable which is checked against CL_SUCCESS
 * @param[in]   msg           The error message to print in case of error
 */
#define opencl_check_error(status,msg) do { \
            if (CL_SUCCESS != status) { \
                char buf[256]; \
                opencl_printf_error(status, sizeof(buf)-1, buf); \
                fprintf(stderr, "ERROR (%s:%d): %s. %s\n", \
                        __FILE__, __LINE__, (msg), buf); \
                exit(-1); \
            } \
        } while(0)

/**
 * Load the file named kernel_file_name as String.
 *
 * @return Kernel as String in case of success
 *         NULL otherwise
 */
char * opencl_kernel_load(const char * kernel_file_name) __HAW_OPENCL_ATTR_NONNULL__(1) __HAW_OPENCL_ATTR_WARN_UNUSED_RESULT__;


/**
 * Builds a kernel of name from the specified kernel string
 * for a specific device.
 * 
 * PLEASE NOTE: This needs to be called after OpenGL Initialization
 *
 * @param kernel_source[in]  The kernels source code
 * @param kernel_name[in]    The kernel name within the source
 * @param device_id[in]      The previously initialized device
 * @param context[in]        The previously initialized device's context
 * @param kernel[out]        The generated kernel for this device
 *
 * @return CL_SUCCESS in case of success
 */
int opencl_kernel_build(const char * kernel_source,
        const char * kernel_name,
        const cl_device_id device_id,
        const cl_context context,
        cl_kernel * kernel) __HAW_OPENCL_ATTR_NONNULL__(1,2,5);

/**
 * Gets the kernel information for a specific device.
 *
 * @param kernel[in]         The kernel to get information
 * @param device_id[in]      The previously initialized device
 * @param kernel_info[out]   The info of the generated kernel
 *
 * @return CL_SUCCESS in case of success
 */
int opencl_kernel_info(const cl_kernel kernel,
        const cl_device_id device_id,
        hawopencl_kernel * kernel_info) __HAW_OPENCL_ATTR_NONNULL__(3);

/**
 * Print the kernel information just gathered.
 *
 * @param kernel_info[in]   The info of the generated kernel
 *
 * @return CL_SUCCESS in case of success
 */
int opencl_kernel_print_info(const hawopencl_kernel ki);

/**
 * Initialize the Profile event mechanism, setting all values to NULL.
 *
 * @param[out] events List of events
 *
 * @return 0 in case of success
 */
int opencl_profile_event_init(hawopencl_profile_events * events) __HAW_OPENCL_ATTR_NONNULL__(1);

/**
 * Add a event to the list of profiled events, resizing internal arrays.
 *
 * @param[inout] events     List of events
 * @param[in]    event      Event to be added
 * @param[in]    size       Size of data, e.g. in terms of Bytes for WRITE/READ
 * @param[in]    event_name String to correspond to this event
 *
 * @return 0 in case of success
 */
int opencl_profile_event_add(hawopencl_profile_events * events,
        cl_event event,
        size_t size,
        const char * event_name) __HAW_OPENCL_ATTR_NONNULL__(1,4);

/**
 * Clear all information for profiled events.
 *
 * @param[inout] events List of events
 *
 * @return 0 in case of success
 */
int opencl_profile_event_clear(const hawopencl_profile_events * events) __HAW_OPENCL_ATTR_NONNULL__(1);


/**
 * Print all the relevant information for the profiled events.
 *
 * @param[in] events List of events
 *
 * @return 0 in case of success
 */
int opencl_profile_event_print(const hawopencl_profile_events * events) __HAW_OPENCL_ATTR_NONNULL__(1);

END_C_DECLS

#endif /* HAWOPENCL_H */
