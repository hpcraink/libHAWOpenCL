//
//  opencl_kernel_load.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015-2017 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2019 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenCL/opencl.h>
#else
#  include <CL/cl.h>
#endif

#include "HAWOpenCL.h"

char * opencl_kernel_load(const char * kernel_file_name)
{
    char kernel_path[1024];
    char * kernel_pwd_env;
    char * buffer;
    char * tmp;
    FILE * file;
    int len;
    size_t remain;
    size_t num_read;
    int ret;
    struct stat file_stat_buf;
    
    assert (NULL != kernel_file_name);
    
    kernel_pwd_env = getenv("OPENCL_KERNEL_PATH");
    len = snprintf(kernel_path, sizeof(kernel_path), "%s/%s",
                   kernel_pwd_env != NULL ? kernel_pwd_env : ".", kernel_file_name);
    kernel_path[len] = '\0';
    
    // First check the existence of this file; and some further tests
    ret = stat (kernel_path, &file_stat_buf);
    if (-1 == ret) {
        fprintf (stderr, "ERROR in %s(): Cannot find OpenCL-Kernel file %s; please set env.-var. OPENCL_KERNEL_PATH\n",
                 __func__, kernel_file_name);
        FATAL_ERROR("stat", errno);
    }
    if (!S_ISREG(file_stat_buf.st_mode)) {
        fprintf (stderr, "ERROR in %s(): OpenCL-Kernel file %s is not a regular file.\n",
                 __func__, kernel_file_name);
        FATAL_ERROR("stat", errno);
    }
    if (!(file_stat_buf.st_mode & S_IRUSR)) {
        fprintf (stderr, "ERROR in %s(): Cannot open OpenCL-Kernel file %s in read-only mode.\n",
                 __func__, kernel_file_name);
        FATAL_ERROR("stat", errno);
    }
    
    // Finally open it
    file = fopen (kernel_path, "r");
    if (NULL == file) {
        fprintf (stderr, "ERROR in %s(): Cannot find OpenCL-Kernel file %s; please set env.-var. OPENCL_KERNEL_PATH\n",
                 __func__, kernel_file_name);
        FATAL_ERROR("fopen", errno);
    }

    buffer = malloc (file_stat_buf.st_size+1);
    if (NULL == buffer)
        FATAL_ERROR("malloc", errno);
    tmp = buffer;
    num_read = 0;
    remain = file_stat_buf.st_size;
    while (!feof(file)) {
        size_t ret;
        
        /*
         * fread() may return fewer bytes than requested, therefore loop over
         * the file to fetch any remaining characters. If it is zero, just break
         */ 
        ret = fread (tmp, sizeof(char), remain, file);
        if (0 == ret)
            break;
        num_read += ret;
        tmp += ret;
        remain -= ret;
    }
    fclose(file);
    
    assert (num_read == file_stat_buf.st_size);
    
    // Just to make sure, that the last byte is NUL/'\0'
    buffer[num_read] = '\0';
    
    return buffer;
}
