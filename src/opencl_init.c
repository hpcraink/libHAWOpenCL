//
//  opencl_init.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015-2016 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2022 Rainer Keller, HS Esslingen. All rights reserved.
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
#include <GL/glx.h>
#include <CL/cl_gl.h>
#endif
#endif

#include "HAWOpenCL.h"

int opencl_init(const cl_device_type on_device_type,
        cl_device_id preferred_device_id,
        cl_device_id * device_id,
        cl_context * context,
        cl_command_queue * command_queue) {
    unsigned int platform;
    int err;
    cl_uint num_platform;
    cl_uint num_device;
    cl_platform_id * cl_platform;
    cl_context_properties cl_properties[7] = {0,};

    // Get all the Platforms first!
    err = clGetPlatformIDs(0, NULL, &num_platform);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformIDs", err);

    if (num_platform == 0)
        FATAL_ERROR("No OpenCL Platform detected.", ENODEV);
    if (num_platform > 1)
        fprintf(stderr, "ATTENTION: opencl_init() detected %d OpenCL Platforms. Will select 1st matching\n", num_platform);

    cl_platform = (cl_platform_id*) malloc(sizeof (cl_platform_id) * num_platform);
    if (NULL == cl_platform)
        FATAL_ERROR("malloc", ENOMEM);

    cl_uint actual_platform;
    err = clGetPlatformIDs(num_platform, cl_platform, &actual_platform);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformIDs", err);
    assert (actual_platform == num_platform);

    static char * cl_platform_name = NULL;
    static size_t cl_platform_name_len = 0;

    for (platform = 0; platform < num_platform; platform++) {
        // First get the numbers of devices for this specific type on this platform
        err = clGetDeviceIDs(cl_platform[platform], on_device_type, 0, NULL, &num_device);
        // Only error out, if there's an error and it's not CL_DEVICE_NOT_FOUND
        if (CL_SUCCESS != err && CL_DEVICE_NOT_FOUND != err)
            FATAL_ERROR("clGetDeviceIDs", err);
        // First print do some error printing (if any)
        if (CL_DEVICE_NOT_FOUND == err) {
            size_t len;
            char * tmp;
            char cl_device_type_name[512];
            
            err = clGetPlatformInfo(cl_platform[platform], CL_PLATFORM_NAME,
                    0, NULL, &len);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);
            if (NULL == cl_platform_name) {
                cl_platform_name = malloc(len);
                if (NULL == cl_platform_name)
                    FATAL_ERROR("malloc", ENOMEM);
                cl_platform_name_len = len;
            } else if (len > cl_platform_name_len) {
                cl_platform_name = realloc(cl_platform_name, len);
                if (NULL == cl_platform_name)
                    FATAL_ERROR("realloc", ENOMEM);
                cl_platform_name_len = len;
            }
            err = clGetPlatformInfo(cl_platform[platform], CL_PLATFORM_NAME,
                    len, cl_platform_name, &len);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);

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

            fprintf(stderr, "INFO: Platform %s does not offer device of type %s\n",
                    cl_platform_name, cl_device_type_name);
            continue;
        }
        // Warn the user, there's more than one fitting device
        if (num_device > 1) {
#if defined (HAWOPENCL_WANT_OPENGL)
            const unsigned char * renderer = (unsigned char*) "(none)";
#if defined(__APPLE__) && defined(__MACH__)
            if (NULL != CGLGetCurrentContext())
                renderer = glGetString(GL_RENDERER);
#elif defined(__linux__)
            if (NULL != glXGetCurrentContext())
                renderer = glGetString(GL_RENDERER);
#elif defined(_WIN32) || defined(_WIN64)
            if (NULL != wglGetCurrentContext())
                renderer = glGetString(GL_RENDERER);
#endif
#endif /* HAWOPENCL_WANT_OPENGL */

            // The warning is more elaborate if we want OpenGL and have more devices.
            fprintf(stderr, "ATTENTION: opencl_init() detected %d OpenCL devices. "
#if defined (HAWOPENCL_WANT_OPENGL)
                    "Will try to select OpenCL device matching OpenGL %s.\n"
#else
                    "Will try to select %s OpenCL device.\n"
#endif
                    , num_device
#if defined (HAWOPENCL_WANT_OPENGL)
                    , renderer
#else
                    , ((preferred_device_id == 0) ? "1." : "preferred")
#endif                                  
                    );
        }
        if (num_device >= 1)
            break;
    }
    // Free the cl_platform_name again, if it was allocated
    if (NULL != cl_platform_name) {
        free (cl_platform_name);
        cl_platform_name = NULL;
        cl_platform_name_len = 0;
    }
    // By NOW, we should have at least one fitting device within this platform
    if (platform == num_platform) {
        fprintf(stderr, "ERROR in %s(): Have checked all %d OpenCL platforms, but no matching devices found.\n",
                __func__, num_platform);
        FATAL_ERROR("opencl_init", ENODEV);
    }
    assert(num_device >= 1);

    // Now run the code to initialize the OS-dependent OpenCL<->OpenGL interaction
#if !defined(HAWOPENCL_WANT_OPENGL)
    // If no device ID is preferred, select the first one, otherwise check for the available
    if (preferred_device_id == 0) {
        err = clGetDeviceIDs(cl_platform[platform], on_device_type, 1, device_id, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);
    } else {
        int i;
	cl_device_id * all_device_ids = malloc(sizeof(cl_device_id) * num_device);
        if (NULL == all_device_ids)
            FATAL_ERROR("malloc", ENOMEM);
        err = clGetDeviceIDs(cl_platform[platform], on_device_type, num_device, all_device_ids, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);
        for (i = 0; i < num_device; i++) {
            if (preferred_device_id == all_device_ids[i])
                break;
        }
        if (i == num_device) {
            fprintf(stderr, "ERROR in %s(): Have checked all %d OpenCL devices, but none matched the preferred device:%lu.\n",
                    __func__, num_device, (long unsigned int)preferred_device_id);
            FATAL_ERROR("opencl_init", ENODEV);
        }
        free (all_device_ids);
        *device_id = preferred_device_id;
    }
    // Create a context with this one device; for apple enable logging to stdout.
#if defined(__APPLE__) && defined(__MACH__)
    *context = clCreateContext(NULL, 1, device_id, clLogMessagesToStdoutAPPLE, NULL, &err);
#else
    cl_properties[0]=CL_CONTEXT_PLATFORM;
    cl_properties[1]=(long int) cl_platform[platform];
    cl_properties[2]=0;
    *context = clCreateContext(cl_properties, 1, device_id, NULL, NULL, &err);
#endif
    if (NULL == *context || CL_SUCCESS != err)
        FATAL_ERROR("clCreateContext", err);
#else
    /*
     * Now, we do want to share OpenGL/OpenCL context, so select the device
     * that does the visualization on the current Virtual Screen.
     *
     * On Mac OSX it get's complicated -- it "has a long history of supporting multiple GPUs"
     * See:
     *  https://developer.apple.com/library/content/technotes/tn2335/_index.html
     *
     * But then again, on other systems, we need to use a Khronos extension to figure out the same.
     */
#if defined(__APPLE__) && defined(__MACH__)
    CGLContextObj cglContext = CGLGetCurrentContext();

    /*
     * Only if we have an initialized GL-context, we may use it.
     * 
     * First create a context with the cl_properties; THEN we may get the device_id
     */
    if (NULL != cglContext) {
        CGLShareGroupObj shareGroup = CGLGetShareGroup(cglContext);

        cl_properties[0] = CL_CONTEXT_PROPERTY_USE_CGL_SHAREGROUP_APPLE;
        cl_properties[1] = (cl_context_properties) shareGroup;
        cl_properties[2] = 0;

        // The context needs to be created automatically without specifying the device
        *context = clCreateContext(cl_properties, 0, NULL, clLogMessagesToStdoutAPPLE, NULL, &err);
        if (NULL == *context || CL_SUCCESS != err)
            FATAL_ERROR("clCreateContext", err);
        
        err = clGetGLContextInfoAPPLE(*context, cglContext,
                CL_CGL_DEVICE_FOR_CURRENT_VIRTUAL_SCREEN_APPLE,
                sizeof (cl_device_id), device_id, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetGLContextInfoAPPLE", err);
    } else {
        err = clGetDeviceIDs(cl_platform[platform], on_device_type, 1, device_id, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);
        *context = clCreateContext(NULL, 1, device_id, clLogMessagesToStdoutAPPLE, NULL, &err);
        if (NULL == *context || CL_SUCCESS != err)
            FATAL_ERROR("clCreateContext", err);
    }
    // NOW, we should have a context for a device according the on_device_type and OpenGL
#elif defined(__linux__)
    // If any of the two functions fails, don't provie any cl_properties
    if (NULL == glXGetCurrentContext() || NULL == glXGetCurrentDisplay())
        cl_properties[0] =  0;
    else {
        cl_properties[0] = CL_GL_CONTEXT_KHR;
        cl_properties[1] = (cl_context_properties) glXGetCurrentContext();
        cl_properties[2] = CL_GLX_DISPLAY_KHR;
        cl_properties[3] = (cl_context_properties) glXGetCurrentDisplay();
        cl_properties[4] = CL_CONTEXT_PLATFORM;
        cl_properties[5] = (cl_context_properties) cl_platform[platform];
        cl_properties[6] = 0;

        /* For Linux we are dependent on the Khronos extension to get the OpenGL context info */
#if defined (CL_VERSION_1_2)
        clGetGLContextInfoKHR_fn func = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddressForPlatform(cl_platform[platform], "clGetGLContextInfoKHR");
#else
        clGetGLContextInfoKHR_fn func = (clGetGLContextInfoKHR_fn) clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
#endif /* CL_VERSION_1_2 */

        if (NULL == func) {
            FATAL_ERROR("clGetGLContextInfoKHR not supported", EINVAL);
        }

        func(cl_properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof (cl_device_id), device_id, NULL);
    }
    if (preferred_device_id == 0) {
        err = clGetDeviceIDs(cl_platform[platform], on_device_type, 1, &preferred_device_id, NULL);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);
    }

    // Create a context with this one device
    cl_properties[0]=CL_CONTEXT_PLATFORM;
    cl_properties[1]=(long int) cl_platform[platform];
    cl_properties[2]=0;

    *context = clCreateContext(cl_properties, 1, &preferred_device_id, NULL, NULL, &err);
    if (NULL == *context || CL_SUCCESS != err)
        FATAL_ERROR("clCreateContext", err);
    *device_id = preferred_device_id;

#elif defined(_WIN32) || defined(_WIN64)
    // Windows:
    cl_properties[0] = CL_GET_CONTEXT_KHR;
    cl_properties[1] = (cl_context_properties) wglGetCurrentContext();
    cl_properties[2] = CL_WGL_HDC_KHR;
    cl_properties[3] = (cl_context_properties) wglGetCurrentDC();
    cl_properties[4] = CL_CONTEXT_PLATFORM;
    cl_properties[5] = (cl_context_properties) cl_platform;
    cl_properties[6] = 0;
#if defined (CL_VERSION_1_2)
    clGetGLContextInfoKHR_fn func = clGetExtensionFunctionAddressForPlatform(cl_platform, "clGetGLContextInfoKHR");
#else
    clGetGLContextInfoKHR_fn func = clGetExtensionFunctionAddress("clGetGLContextInfoKHR");
#endif /* CL_VERSION_1_2 */
    func(cl_properties, CL_CURRENT_DEVICE_FOR_GL_CONTEXT_KHR, sizeof (cl_device_id), device_id, NULL);
    // Create a context with this one device
    *context = clCreateContext(NULL, 1, device_id, NULL, NULL, &err);
    if (NULL == *context || CL_SUCCESS != err)
        FATAL_ERROR("clCreateContext", err);
#else
#error "Architecture currently not supported"
#endif

#endif /* HAWOPENCL_WANT_OPENGL */

    // Create a command queue to issue commands to this device;
    // Might want to check, if CL_QUEUE_PROFILING_ENABLE is reducing performance...
    // This property is mandatory, anyhow!
    cl_command_queue_properties properties = CL_QUEUE_PROFILING_ENABLE;
#if defined(CL_VERSION_2_0)
    char * platform_extensions;
    size_t len;
    err = clGetPlatformInfo(cl_platform[platform], CL_PLATFORM_EXTENSIONS,
                            0, NULL, &len);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);
    platform_extensions = (char*) malloc(len);
    if (NULL == platform_extensions)
        FATAL_ERROR("malloc", ENOMEM);    
    err = clGetPlatformInfo(cl_platform[platform], CL_PLATFORM_EXTENSIONS,
                            len, platform_extensions, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);
    // Don't trust anyone, let's set the last character to NUL character.
    platform_extensions[len-1] = '\0';

    // Let's call the (deprecated) function, if we're on OpenCL < 2.0
    if (NULL == strstr(platform_extensions, "cl_khr_create_command_queue")) {
        *command_queue = clCreateCommandQueue(*context, *device_id, properties, &err);
    } else {
        cl_queue_properties  qp[] = {CL_QUEUE_PROPERTIES, properties, 0};
        *command_queue = clCreateCommandQueueWithProperties(*context, *device_id, qp, &err);
    }
#else
    *command_queue = clCreateCommandQueue(*context, *device_id, properties, &err);
#endif
    if (!*command_queue || err != CL_SUCCESS)
        FATAL_ERROR("clCreateCommandQueue*", err);

    free(cl_platform);

    return CL_SUCCESS;
}

