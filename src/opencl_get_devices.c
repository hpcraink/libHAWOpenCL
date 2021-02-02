//
//  opencl_get_devices.c : Part of libHAWOpenCL
//
//  Copyright (c) 2019-2021 Rainer Keller, HS Esslingen. All rights reserved.
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

#if defined(HAWOPENCL_WANT_OPENGL)
#if defined(__APPLE__) && defined(__MACH__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "HAWOpenCL.h"

int opencl_get_devices(const cl_device_type on_device_type, 
        cl_uint * num_devices, hawopencl_device ** devices) {
    assert (num_devices != NULL);
    int i;
    int j;
    cl_int err;
    cl_platform_id * cl_platform;
    cl_device_id * cl_devices;
    cl_uint num_platform;
    
    // Get all the Platforms first!
    err = clGetPlatformIDs(0, NULL, &num_platform);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformIDs", err);
    if (num_platform == 0)
        FATAL_ERROR("No OpenCL Platform detected.", ENODEV);
    if (num_platform > 1)
        fprintf(stderr, "ATTENTION: opencl_get_devices() detected %d OpenCL Platforms. Will select 1st matching\n", num_platform);

    cl_platform = (cl_platform_id*) malloc(sizeof(cl_platform_id) * num_platform);
    if (NULL == cl_platform)
        FATAL_ERROR("malloc", ENOMEM);
    err = clGetPlatformIDs(num_platform, cl_platform, &num_platform);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformIDs", err);
    for (i = 0; i < num_platform; i++) {
        cl_uint num;
        err = clGetDeviceIDs(cl_platform[i], on_device_type, 0, NULL, &num);
        // Only error out, if there's an error and it's not CL_DEVICE_NOT_FOUND
        if (CL_SUCCESS != err && CL_DEVICE_NOT_FOUND != err)
            FATAL_ERROR("clGetDeviceIDs", err);
        // Continue the loop of platforms, if there are no matching devices
        if (CL_DEVICE_NOT_FOUND == err || num <= 0) {
            // printf ("CONTINUE Platform i:%d does not offer on_device_type:%d err:%d num:%u\n",
            //        i, on_device_type, err, num));
            continue;
        }
        *num_devices = num;
        *devices = (hawopencl_device*)malloc(num * sizeof(hawopencl_device));
        if (*devices == NULL)
            FATAL_ERROR("malloc", ENOMEM);
        cl_devices = (cl_device_id*)malloc(num * sizeof(cl_device_id));
        if (cl_devices == NULL)
            FATAL_ERROR("malloc", ENOMEM);
        err = clGetDeviceIDs(cl_platform[i], on_device_type, num, cl_devices, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);
        for (j = 0; j < num; j++) {
            size_t len;
            cl_device_id id = cl_devices[j];
            cl_device_type type;

            err = clGetDeviceInfo(id, CL_DEVICE_NAME, 0, NULL, &len);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetDeviceInfo", err);

            err = clGetDeviceInfo(id, CL_DEVICE_TYPE, sizeof(cl_device_type), &type, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetDeviceInfo", err);

            (*devices)[j].device_id = id;
            (*devices)[j].device_type = type;
            (*devices)[j].device_name = (char*)malloc(len * sizeof(char));

            err = clGetDeviceInfo(id, CL_DEVICE_NAME,
                    len, (*devices)[j].device_name, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetDeviceInfo", err);
        }
        free (cl_devices);
        // SUCCESS, WE FOUND AT LEAST ONE DEVICE FOR THIS PLATFORM
        break;
    }
    if (i == num_platform) {
        char * tmp;
        char cl_device_type_name[512];
        // The device type may be either CUSTOM or a combination...
        if (on_device_type & CL_DEVICE_TYPE_CUSTOM)
            sprintf(cl_device_type_name, "CL_DEVICE_TYPE_CUSTOM");
        else {
            tmp = cl_device_type_name;
            if (on_device_type & CL_DEVICE_TYPE_CPU)
                tmp += sprintf(tmp, "CL_DEVICE_TYPE_CPU ");
            if (on_device_type & CL_DEVICE_TYPE_GPU)
                tmp += sprintf(tmp, "CL_DEVICE_TYPE_GPU ");
            if (on_device_type & CL_DEVICE_TYPE_ACCELERATOR)
                tmp += sprintf(tmp, "CL_DEVICE_TYPE_ACCELERATOR ");
            if (on_device_type & CL_DEVICE_TYPE_DEFAULT)
                tmp += sprintf(tmp, "CL_DEVICE_TYPE_DEFAULT ");
        }
        fprintf(stderr, "ERROR: No platform offers device(s) of type %s\n",
                cl_device_type_name);
        exit (-1);
    }
    free (cl_platform);
    return CL_SUCCESS;
}