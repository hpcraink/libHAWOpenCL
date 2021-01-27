//
//  opencl_profile_events.c : Part of libHAWOpenCL.a
//
//  Copyright (c) 2015 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2019 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenCL/cl.h>
#else
#  include <CL/cl.h>
#endif

#include "HAWOpenCL.h"


int opencl_profile_event_init(hawopencl_profile_events * events) {
    events->event_names = NULL;
    events->events = NULL;
    events->sizes = NULL;
    events->idx = 0;
    events->max = 0 ;
    return 0;
}

int opencl_profile_event_add(hawopencl_profile_events * events, cl_event event, size_t size, const char * event_name) {
    if (events->event_names == NULL) {
        events->idx = 0;
        events->max = 16;
        events->event_names = (char **) malloc(sizeof(char *) * events->max);
        events->events = (cl_event*) malloc(sizeof(cl_event) * events->max);
        events->sizes = (size_t*) malloc(sizeof(size_t) * events->max);
        
        if (events->event_names == NULL || events->events == NULL || events->sizes == NULL)
            return errno;
    }
    if (events->idx == events->max) {
        int i;
        events->event_names = (char **) realloc(events->event_names, sizeof(char*) * events->max * 2);
        events->events = (cl_event*) realloc(events->events, sizeof(cl_event) * events->max * 2);
        events->sizes = (size_t*) realloc(events->sizes, sizeof(size_t) * events->max * 2);
        
        if (events->event_names == NULL || events->events == NULL || events->sizes == NULL)
            return errno;
        
        events->max = events->max * 2;
        for (i=events->idx; i < events->max; i++) {
            events->event_names[i] = NULL;
            events->events[i] = 0;
            events->sizes[i] = 0;
        }
    }

    events->event_names[events->idx] = strdup (event_name);
    events->events[events->idx] = event;
    events->sizes[events->idx] = size;
    events->idx++;
    return 0;
}

int opencl_profile_event_clear(const hawopencl_profile_events * events) {
    int i;
    for (i = 0; i < events->max; i++) {
        free(events->event_names[i]);
        events->events[i] = 0;
        events->sizes[i] = 0;
    }
    return 0;
}


int opencl_profile_event_print(const hawopencl_profile_events * events) {
    int i;
    for (i = 0; i < events->idx; i++) {
        char buffer[1024];
        char * out = buffer;
        cl_ulong start_time;
        cl_ulong end_time;
        unsigned long diff;
        cl_command_type command_type;
        clGetEventProfilingInfo(events->events[i], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start_time, NULL);
        clGetEventProfilingInfo(events->events[i], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end_time, NULL);
        
        clGetEventInfo (events->events[i], CL_EVENT_COMMAND_TYPE,
                              sizeof(cl_command_type), &command_type, NULL);
        diff = end_time - start_time;
        
        out = memset(buffer,0, sizeof(buffer));
        out += snprintf(out, sizeof(buffer), "OpenCL Event %d '%s' ", i, events->event_names[i]);
        out += snprintf(out, sizeof(buffer), "took %luns ", diff);
        if (command_type == CL_COMMAND_READ_BUFFER || command_type == CL_COMMAND_WRITE_BUFFER) {
            if (command_type == CL_COMMAND_WRITE_BUFFER) {
                out+=snprintf(out, sizeof(buffer), "WRITE: ");
            } else {
                out+=snprintf(out, sizeof(buffer), "READ: ");
            }
            out+=snprintf(out, sizeof(buffer), "%fMB/s",
                          (1000.0*1000.0*1000.0/1024.0)*events->sizes[i] / diff / 1024.0);

        }
        printf ("%s\n", buffer);
    }
    return 0;
}

