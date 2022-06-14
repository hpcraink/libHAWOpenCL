//
//  opencl_kernel_load.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015-2017 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2022 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#endif
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

/*
 * Local functions
 */
static int opencl_kernel_check_header(const char * path, const int want_fail);
static int opencl_kernel_skip_while(char * buffer, size_t buffer_len, int * from, const char * skip_characters);
static int opencl_kernel_skip_until(char * buffer, size_t buffer_len, int * from, const char * skip_characters);
static char * opencl_kernel_replace_include(char * kernel_paths[], const char * kernel_filename, char * buffer, size_t buffer_len);
static FILE * opencl_kernel_open_file(char * file_paths[],const char * file_name, size_t * file_length);
static size_t opencl_kernel_read_file(FILE * file, char * buffer, size_t len);

static int opencl_kernel_check_header(const char * path, const int want_fail) {
    int ret;
    struct stat file_stat_buf;
    // First check the existence of this file; and some further tests
    ret = stat (path, &file_stat_buf);
    if (-1 == ret) {
        fprintf (stderr, "ERROR in %s(): Cannot find OpenCL-Kernel header %s; please set env.-var. OPENCL_KERNEL_PATH and ../include\n",
                 __func__, path);
        if (want_fail)
            FATAL_ERROR("stat", errno);
        return -1;
    }
    if (!S_ISREG(file_stat_buf.st_mode)) {
        fprintf (stderr, "ERROR in %s(): OpenCL-Kernel header %s is not a regular file.\n",
                 __func__, path);
        if (want_fail)
            FATAL_ERROR("stat", errno);
        return -1;
    }
    if (!(file_stat_buf.st_mode & S_IRUSR)) {
        fprintf (stderr, "ERROR in %s(): Cannot open OpenCL-Kernel header %s in read-only mode.\n",
                 __func__, path);
        if (want_fail)
            FATAL_ERROR("stat", errno);
        return -1;
    }

    return ret;
}

// Skip while buffer contains chars in character set and fail if length is beyond limit
static int opencl_kernel_skip_while(char * buffer, size_t buffer_len, int * from, const char * skip_characters) {
    assert (NULL != buffer);
    assert (0 < buffer_len);
    assert (NULL != from);
    assert (NULL != skip_characters);
    int i = *from;
    int j;
    bool skip_character_found = true;

    // go over until the end of Skip-Characters
    while (i < buffer_len && skip_character_found) {
        skip_character_found = false;
        for (j = 0; j < strlen(skip_characters); j++)
            if (buffer[i] == skip_characters[j])
                skip_character_found = true;
        if (skip_character_found)
            i++;
    }
    if (i == buffer_len && !skip_character_found) {
        fprintf(stderr, "ERROR: Skipping %s searching for skip_characters:%s went over buffer",
            &buffer[i], skip_characters);
        FATAL_ERROR("opencl_kernel_skip_while", EINVAL);
    }
    *from = i;
    return skip_character_found;
}

// Skip until chars in the character set are foun and fail if length is beyond limit
static int opencl_kernel_skip_until(char * buffer, size_t buffer_len, int * from, const char * skip_characters) {
    assert (NULL != buffer);
    assert (0 < buffer_len);
    assert (NULL != from);
    assert (NULL != skip_characters);
    int i;
    int j;
    bool skip_character_found = false;

    // go over until the end of Skip-Characters
    for (i = *from; i < buffer_len; i++) {
        for (j = 0; j < strlen(skip_characters); j++)
            if (buffer[i] == skip_characters[j]) {
                skip_character_found = true;
                goto out;
            }
    }
out:
    if (i >= buffer_len && !skip_character_found) {
        fprintf(stderr, "ERROR: Skipping %s searching for skip_characters:%s went over buffer",
            &buffer[i], skip_characters);
        FATAL_ERROR("opencl_kernel_skip_until", EINVAL);
    }
    *from = i;
    return skip_character_found;
}

static void opencl_kernel_print(char * buffer, size_t len) {
    printf("buffer:%s\n", buffer);
}

// Parses buffer, searching for include and checks for any used header files.
// returns the extended buffer
static char * opencl_kernel_replace_include(char * kernel_paths[], const char * kernel_filename, char * buffer, size_t buffer_len) {
    int i = 0;
    #define MAX_INCLUDES 8
    char * include_filenames[MAX_INCLUDES];
    char * include_start[MAX_INCLUDES];
    char * include_end[MAX_INCLUDES];
    char * include_files[MAX_INCLUDES];
    size_t include_files_len[MAX_INCLUDES];
    int include_line[MAX_INCLUDES];
    int line_num = 1; // Line numbers start counting with 1!

    int includes = 0;

    // We need to check for lines containing the regular expression "[ \t]*#[ \t]*include[ \t] \"FILENAME\""
    // and copy the filename, as well as the position, whether to enter and replace 
    for (i = 0; i < buffer_len; i++) {
        // Check for single-line comments "//", which go until the end of line...
        if ('/' == buffer[i]) {
            if (i+1 < buffer_len && '/' == buffer[i+1])         // Single line comments, aka '//'
                opencl_kernel_skip_until (buffer, buffer_len, &i, "\n");
            else if (i+1 < buffer_len && '*' == buffer[i+1]) {  // Multi-line comments, aka '/*...*/, counting new-lines
                // Let's not get fooled by "/*  comments " such as
                //     /* we do not use #include "file.h" anymore */
                // Otherwise that would trick us....
                bool end_comment_found = false;
                i+=2; // Skip over the /*...
                while (i < buffer_len && !end_comment_found) {
                    // Skip until either end of comment (aka */) or there's an EOL
                    opencl_kernel_skip_until (buffer, buffer_len, &i, "*\n");
                    if ('\n' == buffer[i]) {
                        line_num++;
                        i++;
                    }
                    else if (i+1 < buffer_len && '/' == buffer[i+1]) {
                        end_comment_found = true;
                        i+=2; // Skip over the */...
                    } else if ('*' == buffer[i])
                        i++;  // Skip any '*' which is NOT part of the ending comment
                }
            }
        }
        if ('\n' == buffer[i])
            line_num++;

        // We found a preprocessor macro sign
        if ('#' == buffer[i]) {
            i++;
            // Skip over any white-space characters (space and tab), such as in "#  include"
            opencl_kernel_skip_while (buffer, buffer_len, &i, " \t");
            // Check whether we now have a "   #[ \t]*include"
            if (0 == strncmp(&buffer[i], "include", strlen("include"))) {
                // Yay, we found a filename (but -1, to also replace the '#')
                include_start[includes] = &buffer[i-1];
                include_line[includes] = line_num;

                i += strlen("include");
                // Skip over any white-space characters (space and tab), such as in '#include    "'
                opencl_kernel_skip_while (buffer, buffer_len, &i, " \t");

                // Check only for local include files #inlude "local.h" not global #include <stdio.h>
                if ('"' == buffer[i]) {
                    int len;
                    i++;                         // Do not include the '"' character
                    int j = i;                   // We start with the filename character array
                    while (j < buffer_len && '"' != buffer[j]) j++; /* Go over until next closing " */
                    if (j >= buffer_len)
                        FATAL_ERROR("opencl_kernel_replace_include: failed to find closing hyphen for file-name", EINVAL);

                    include_end[includes] = &buffer[j]+1;  // we replace until j (aka including the last '"')
                    len = j - i + 1;                 // but for copying we don't include '"', but (+1) to include the last '\0' aka NUL
                    include_filenames[includes] = malloc(sizeof(char) * len);
                    if (NULL == include_filenames[includes])
                        FATAL_ERROR("malloc", ENOMEM);
                    strncpy (include_filenames[includes], &buffer[i], len);
                    include_filenames[includes][len-1] = '\0'; // Be sure to have a NUL-terminated file name
                    if (++includes == MAX_INCLUDES)
                        FATAL_ERROR("opencl_kernel_replace_include: too many include files", EINVAL);
                }
            }
        }
    }

    if (0 == includes) {
        printf("opencl_kernel_replace_include: No includes good!\n");
        return buffer;
    }

    const size_t buffer_olen = buffer_len;

    // Now, that we figured all the include file names, try to search and read those
    // printf("opencl_kernel_replace_include:       includes:%d\n", includes);
    const int kernel_filename_len = strlen(kernel_filename);
    for (i = 0; i < includes; i++) {
        size_t file_length;
        size_t num_read;
        // printf("  %d. include:%s\n", i, include_filenames[i]);
        FILE * file =  opencl_kernel_open_file(kernel_paths, include_filenames[i], &include_files_len[i]);

        include_files[i] = malloc (include_files_len[i]);
        if (NULL == include_files[i])
            FATAL_ERROR("malloc", errno);
        num_read = opencl_kernel_read_file (file, include_files[i], include_files_len[i]);
        fclose(file);
        buffer_len += include_files_len[i]; // This disregards the space saved by overriding #include
        buffer_len += 1 + 1 + 5 + 1 + 1 + kernel_filename_len + 1; // Add space for new CPP-like position informatian: '# LINE-NUMBER(up to 5 digits) "KERNEL_FILE_NAME.c"'
    }

    char * new_buffer = malloc(buffer_len);
    if (NULL == new_buffer)
        FATAL_ERROR("malloc", ENOMEM);
    char * p = buffer;
    char * p_new = new_buffer;
    char * p_last_start = buffer;
    for (i = 0; i < includes; i++) {
        char cpp_buf[1+1+5+1+1+kernel_filename_len+1];

        // Copy from start of last include file (or the beginning of buffer from p to p_new)
        int len = include_start[i] - p_last_start;
        strncpy (p_new, p, len);
        p = include_end[i];
        p_last_start = include_end[i];
        p_new += len;
        strncpy (p_new, include_files[i], include_files_len[i]);
        p_new += include_files_len[i];
        len = snprintf(cpp_buf, sizeof(cpp_buf), "# %d \"%s\"", include_line[i]+1, kernel_filename);
        strncpy (p_new, cpp_buf, len);
        p_new += len;
    }
    // Copy last part...
    int len = buffer + buffer_olen - include_end[i-1];
    strncpy (p_new, p, len);
    free (buffer);

    // Just for DEBUGGING:
    // opencl_kernel_print(new_buffer, buffer_len);

    return new_buffer;
}

static FILE * opencl_kernel_open_file(char * file_paths[], const char * file_name, size_t * file_length) {
    int i;
    int ret;
    FILE * file;
    struct stat file_stat_buf;
    bool file_found = false;
    char path[2048];

    for (i = 0; NULL != file_paths[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s", file_paths[i], file_name);
        // First check the existence of this file; and some further tests
        ret = stat (path, &file_stat_buf);
        if (0 == ret) {
            file_found = true;
            break;
        }
    }
    if (!file_found) {
        fprintf (stderr, "ERROR in %s(): Cannot find OpenCL file %s; please set env.-var. OPENCL_KERNEL_PATH\n",
                 __func__, file_name);
        FATAL_ERROR("stat", errno);
    }

    if (!S_ISREG(file_stat_buf.st_mode)) {
        fprintf (stderr, "ERROR in %s(): OpenCL file %s is not a regular file.\n",
                __func__, file_name);
        FATAL_ERROR("stat", errno);
    }
    if (!(file_stat_buf.st_mode & S_IRUSR)) {
        fprintf (stderr, "ERROR in %s(): Cannot open OpenCL file %s in read-only mode.\n",
                __func__, file_name);
        FATAL_ERROR("stat", errno);
    }
    
    // Finally open it
    file = fopen (path, "r");
    if (NULL == file) {
        fprintf (stderr, "ERROR in %s(): Cannot find OpenCL file %s; please set env.-var. OPENCL_KERNEL_PATH\n",
                __func__, file_name);
        FATAL_ERROR("fopen", errno);
    }

    *file_length = file_stat_buf.st_size;

    return file;
}

static size_t opencl_kernel_read_file(FILE * file, char * buffer, size_t len) {
    size_t num_read = 0;
    char * tmp = buffer;
    size_t remain = len;
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
    return len;
}

char * opencl_kernel_load(const char * kernel_file_name)
{
    char * kernel_paths[5] = { NULL, };
    char * kernel_pwd_env;
    char * buffer;
    char * tmp;
    FILE * file;
    int len;
    size_t num_read;
    int ret;
    size_t file_length;

    assert (NULL != kernel_file_name);

    kernel_paths[0] = strdup(HAWOPENCL_SOURCE_DIR);
    kernel_paths[1] = strdup(HAWOPENCL_SOURCE_DIR "/src");
    kernel_paths[2] = strdup(HAWOPENCL_SOURCE_DIR "/include");
    
    kernel_pwd_env = getenv("OPENCL_KERNEL_PATH");
    if (NULL != kernel_pwd_env) {
        kernel_paths[3] = getenv("OPENCL_KERNEL_PATH");
        kernel_paths[4] = malloc(strlen(kernel_paths[3]) + strlen("../include/") +1);
        if (NULL == kernel_paths[4])
            FATAL_ERROR("malloc", ENOMEM);
        snprintf(kernel_paths[4], strlen(kernel_paths[3]) + strlen("../include/") +1, "%s/%s", kernel_paths[3], "../include/");
    }

    file = opencl_kernel_open_file(kernel_paths, kernel_file_name, &file_length);

    buffer = malloc (file_length+1);
    if (NULL == buffer)
        FATAL_ERROR("malloc", errno);
    num_read = opencl_kernel_read_file (file, buffer, file_length);
    fclose(file);

    assert (num_read == file_length);
    // Just to make sure, that the last byte is NUL/'\0'
    buffer[num_read] = '\0';

    return opencl_kernel_replace_include(kernel_paths, kernel_file_name, buffer, num_read);
}
