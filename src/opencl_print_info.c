//
//  opencl_print_info.c : Part of libHAWOpenCL
//
//  Copyright (c) 2015 Rainer Keller, HFT Stuttgart. All rights reserved.
//  Copyright (c) 2018-2021 Rainer Keller, HS Esslingen. All rights reserved.
//
#include "HAWOpenCL_config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#  include <OpenCL/cl.h>
#else
#  include <CL/cl.h>
#endif

#include "HAWOpenCL.h"

// Macro evaluates to two arguments: the parameter and it's String value.
#define OPENCL_NAME(x) (x), #x

typedef enum {
    HAW_VERSION_1_0 = 1,
    HAW_VERSION_1_1,
    HAW_VERSION_1_2,
    HAW_VERSION_2_0,
    HAW_VERSION_2_1,
    HAW_VERSION_2_2,
    HAW_VERSION_3_0,
} haw_opencl_version_t;

static haw_opencl_version_t haw_platform_opencl_version = 0;
static haw_opencl_version_t haw_device_opencl_version = 0;
// Local functions
static void opencl_platform_supports_init(cl_platform_id platform) {
    cl_int err;
    size_t len;
    char * platform_version;
    err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 0, NULL, &len);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);
    platform_version = (char*)malloc(len);
    if (NULL == platform_version)
        FATAL_ERROR("malloc", ENOMEM);
    err = clGetPlatformInfo(platform, CL_PLATFORM_VERSION, len, platform_version, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);
    if (NULL != strstr(platform_version, "OpenCL 1.0"))
        haw_platform_opencl_version = HAW_VERSION_1_0;
    if (NULL != strstr(platform_version, "OpenCL 1.1"))
        haw_platform_opencl_version = HAW_VERSION_1_1;
    if (NULL != strstr(platform_version, "OpenCL 1.2"))
        haw_platform_opencl_version = HAW_VERSION_1_2;

    if (NULL != strstr(platform_version, "OpenCL 2.0"))
        haw_platform_opencl_version = HAW_VERSION_2_0;
    if (NULL != strstr(platform_version, "OpenCL 2.1"))
        haw_platform_opencl_version = HAW_VERSION_2_1;
    if (NULL != strstr(platform_version, "OpenCL 2.2"))
        haw_platform_opencl_version = HAW_VERSION_2_2;

    if (NULL != strstr(platform_version, "OpenCL 3.0"))
        haw_platform_opencl_version = HAW_VERSION_3_0;

    free (platform_version);
}
static int opencl_platform_supports_version(haw_opencl_version_t wants) {
    return (wants > haw_platform_opencl_version) ? 0 : 1;
}

static void opencl_device_supports_init(cl_device_id device) {
    cl_int err;
    size_t len;     
    char * device_version;
    err = clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &len);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetDeviceInfo", err);
    device_version = (char*)malloc(len);
    if (NULL == device_version)
        FATAL_ERROR("malloc", ENOMEM);
    err = clGetDeviceInfo(device, CL_DEVICE_VERSION, len, device_version, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetDeviceInfo", err);
    if (NULL != strstr(device_version, "OpenCL 1.0"))
        haw_device_opencl_version = HAW_VERSION_1_0;
    if (NULL != strstr(device_version, "OpenCL 1.1"))
        haw_device_opencl_version = HAW_VERSION_1_1;
    if (NULL != strstr(device_version, "OpenCL 1.2"))
        haw_device_opencl_version = HAW_VERSION_1_2;
        
    if (NULL != strstr(device_version, "OpenCL 2.0"))
        haw_device_opencl_version = HAW_VERSION_2_0;
    if (NULL != strstr(device_version, "OpenCL 2.1"))
        haw_device_opencl_version = HAW_VERSION_2_1;
    if (NULL != strstr(device_version, "OpenCL 2.2"))
        haw_device_opencl_version = HAW_VERSION_2_2;

    if (NULL != strstr(device_version, "OpenCL 3.0"))
        haw_device_opencl_version = HAW_VERSION_3_0;

    free (device_version);
}   
static int opencl_device_supports_version(haw_opencl_version_t wants) {
    return (wants > haw_device_opencl_version) ? 0 : 1;
}   
    

static void opencl_print_device_type(cl_device_id device, void * val) {
    cl_device_type my_type = *((cl_device_type*)val);
    struct my_types {
        cl_device_type type;
        char * str;
        char * description;
    } my_types[] = {
        {OPENCL_NAME(CL_DEVICE_TYPE_CUSTOM),      "Custom device"},
        {OPENCL_NAME(CL_DEVICE_TYPE_CPU),         "CPU device"},
        {OPENCL_NAME(CL_DEVICE_TYPE_GPU),         "GPU device"},
        {OPENCL_NAME(CL_DEVICE_TYPE_ACCELERATOR), "Accelerator"},
        {OPENCL_NAME(CL_DEVICE_TYPE_DEFAULT),     "Default device"},
    };
    const int my_types_max = sizeof(my_types)/sizeof(my_types[0]);
    int printed = 0;

    // Either the device is CL_DEVICE_TYPE_CUSTOM or a combination of the other
    if (my_type & my_types[0].type) {
        printf ("%s",
                my_types[0].str);
    } else {
        int i;
        // Print a comma-separated list of the types, so first [1], then [2]
        for (i=1; i < my_types_max; i++) {
            if (my_type & my_types[i].type)
                printf ("%s%s",
                        printed++ ? "," : "", my_types[i].str);
        }
    }
    printf("\n");
}

static void opencl_print_device_max_work_item_sizes(cl_device_id device, void * val) {
    size_t * arr = (size_t*)val;
    printf ("(%zd, %zd, %zd)\n",
            arr[0], arr[1], arr[2]);
}

static void opencl_print_device_mem_base_addr_align(cl_device_id device, void * val) {
    cl_uint bits = *((cl_uint*)val);
    printf ("%u Bits\n",
            bits);
}

static void opencl_print_device_single_fp_config(cl_device_id device, void * val) {
    cl_device_fp_config fp = *((cl_device_fp_config*)val);
    int i;
    int printed = 0;
    struct fp_modes {
        cl_device_fp_config fp;
        char * str;
        char * description;
    } fp_mode[] = {
        {OPENCL_NAME(CL_FP_DENORM),           "Denormals are supported."},
        {OPENCL_NAME(CL_FP_INF_NAN),          "INF and quiet NaNs are supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_NEAREST), "Round to nearest even rounding mode supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_ZERO),    "Round to zero rounding mode is supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_INF),     "Round to positive and negative infinity rounding mode is supported."},
        {OPENCL_NAME(CL_FP_FMA),              "IEEE754-2008 fused multiply-add is supported."},
        {OPENCL_NAME(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT), "Divide and sqrt are correctly rounded as defined by the IEEE754 specification."},
        {OPENCL_NAME(CL_FP_SOFT_FLOAT),       "Basic floating-point operations (such as addition, subtraction, multiplication) are implemented in software"}
    };
    const int fp_modes_max = sizeof(fp_mode)/sizeof(fp_mode[0]);

    // Print a comma-separated list of the modes, so first [0], then the loop from [1]
    for (i=0; i < fp_modes_max; i++) {
        if (fp_mode[i].fp & fp)
            printf ("%s%s",
                    printed++ ? ", " : "", fp_mode[i].str);

    }
    if (0 == printed)
        printf("none");
    printf ("\n");

}

static void opencl_print_device_double_fp_config(cl_device_id device, void * val) {
    cl_device_fp_config fp = *((cl_device_fp_config*)val);
    int i;
    int printed = 0;
    struct fp_modes {
        cl_device_fp_config fp;
        char * str;
        char * description;
    } fp_mode[] = {
        {OPENCL_NAME(CL_FP_DENORM), "Denormals are supported."},
        {OPENCL_NAME(CL_FP_INF_NAN), "INF and quiet NaNs are supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_NEAREST), "Round to nearest even rounding mode supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_ZERO), "Round to zero rounding mode is supported."},
        {OPENCL_NAME(CL_FP_ROUND_TO_INF), "Round to positive and negative infinity rounding mode is supported."},
        {OPENCL_NAME(CL_FP_FMA), "IEEE754-2008 fused multiply-add is supported."},
        {OPENCL_NAME(CL_FP_SOFT_FLOAT), "Basic floating-point operations (such as addition, subtraction, multiplication) are implemented in software"},
        /* Supposedly the following is not supported for DP */
        {OPENCL_NAME(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT), "Divide and sqrt are correctly rounded as defined by the IEEE754 specification."},
    };
    const int fp_modes_max = sizeof(fp_mode)/sizeof(fp_mode[0]);

    // Print a comma-separated list of the modes, so first [0], then the loop from [1]
    for (i=0; i < fp_modes_max; i++) {
        if (fp_mode[i].fp & fp)
            printf ("%s%s",
                    printed++ ? ", " : "", fp_mode[i].str);

    }
    if (0 == printed)
        printf("none");
    printf ("\n");
}


static void opencl_print_device_global_mem_cache_type(cl_device_id device, void * val) {
    cl_device_mem_cache_type c_type = *((cl_device_mem_cache_type*)val);

    if (c_type == CL_NONE)
        printf("CL_NONE\n");
    else if (c_type == CL_READ_ONLY_CACHE)
        printf ("CL_READ_ONLY_CACHE\n");
    else if (c_type == CL_READ_WRITE_CACHE)
        printf ("CL_READ_WRITE_CACHE\n");
    else
        FATAL_ERROR("opencl_print_device_global_mem_cache_type unexpected value", c_type);
}


static void opencl_print_device_local_mem_type(cl_device_id device, void * val) {
    cl_device_local_mem_type c_type = *((cl_device_local_mem_type*)val);
    cl_device_type device_type;
    int err;

    err = clGetDeviceInfo(device,
                CL_DEVICE_TYPE,
                sizeof(device_type),
                &device_type,
                NULL);
    if (err != CL_SUCCESS)
        FATAL_ERROR("opencl_print_device_local_mem_type: error figuring out device_type", err);

    if (c_type == CL_LOCAL)
        printf("CL_LOCAL\n");
    else if (c_type == CL_GLOBAL)
        printf ("CL_GLOBAL\n");
    else if (device_type == CL_DEVICE_TYPE_CUSTOM && c_type == CL_NONE)
        printf ("For CL_DEVICE_TYPE_CUSTOM no local memory is supported.\n");
    else
        FATAL_ERROR("opencl_print_device_local_mem_type unexpected value", c_type);
}

static void opencl_print_device_profiling_timer_resolution(cl_device_id device, void * val) {
    size_t resolution = *((size_t*)val);
    printf ("%zd ns = %f usec\n", resolution, 1.0 / 1000.0 * resolution);
}


static void opencl_print_device_execution_capabilities(cl_device_id device, void * val) {
    cl_device_exec_capabilities my_val = *((cl_device_exec_capabilities*)val);
    int i;
    int printed = 0;
    struct modes {
        cl_device_exec_capabilities val;
        char * str;
        char * description;
    } modes[] = {
        {OPENCL_NAME(CL_EXEC_KERNEL), "The OpenCL device can execute OpenCL kernels."},
        {OPENCL_NAME(CL_EXEC_NATIVE_KERNEL), "The OpenCL device can execute native kernels."},
    };
    const int modes_max = sizeof(modes)/sizeof(modes[0]);

    // Print a comma-separated list of the modes, so first [0], then the loop from [1]
    for (i=0; i < modes_max; i++) {
        if (modes[i].val & my_val)
            printf ("%s%s",
                    printed++ ? ", " : "", modes[i].str);

    }
    if (0 == printed)
        printf("none");
    printf ("\n");
}


static void opencl_print_device_queue_properties(cl_device_id device, void * val) {
    cl_command_queue_properties my_val = *((cl_command_queue_properties*)val);
    int i;
    int printed = 0;
    struct modes {
        cl_command_queue_properties val;
        char * str;
        char * description;
    } modes[] = {
        {OPENCL_NAME(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE), "Out-of-Order Queue enabled"},
        {OPENCL_NAME(CL_QUEUE_PROFILING_ENABLE),              "Profiling Queue enabled"},
    };
    const int modes_max = sizeof(modes)/sizeof(modes[0]);

    // Print a comma-separated list of the modes, so first [0], then the loop from [1]
    for (i=0; i < modes_max; i++) {
        if (modes[i].val & my_val)
            printf ("%s%s",
                    printed++ ? ", " : "", modes[i].str);

    }
    if (0 == printed)
        printf("none");
    printf ("\n");
}

static void opencl_print_device_partition_properties(cl_device_id device, void * val) {
    cl_device_partition_property * my_val = (cl_device_partition_property*)val;
    int i;
    int printed = 0;
    struct modes {
        cl_command_queue_properties val;
        char * str;
        char * description;
    } modes[] = {
        {OPENCL_NAME(CL_DEVICE_PARTITION_EQUALLY),            "Aggregate devices are split equally into as many smaller aggregate devices"},
        {OPENCL_NAME(CL_DEVICE_PARTITION_BY_COUNTS),          "Aggregate devices are split according to provided counts"},
        {OPENCL_NAME(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN), "Aggregate devices are split according to cache-hierarchy"},
    };
    const int modes_max = sizeof(modes)/sizeof(modes[0]);

    // Print a comma-separated list of the partition properties, the specification is SILLY
    // XXX No length for the array is given!(?)
    for ( ; *my_val != 0; my_val++)
        for (i=0; i < modes_max; i++) {
            if (modes[i].val == *my_val)
                printf ("%s%s",
                        printed++ ? ", " : "", modes[i].str);

        }
    if (0 == printed)
        printf("none");
    printf ("\n");
}


static void opencl_print_device_partition_affinity_domain(cl_device_id device, void * val) {
    cl_device_affinity_domain my_val = *((cl_device_affinity_domain*)val);
    int i;
    int printed = 0;
    struct modes {
        cl_command_queue_properties val;
        char * str;
        char * description;
    } modes[] = {
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_NUMA),               "ompute units that share a NUMA node"},
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE),           "XXX"},
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE),           "XXX"},
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE),           "XXX"},
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE),           "XXX"},
        {OPENCL_NAME(CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE), "XXX"},
    };
    const int modes_max = sizeof(modes)/sizeof(modes[0]);

    // Print a comma-separated list of the modes, so first [0], then the loop from [1]
    for (i=0; i < modes_max; i++) {
        if (modes[i].val & my_val)
            printf ("%s%s",
                    printed++ ? ", " : "", modes[i].str);

    }
    if (0 == printed)
        printf("none");
    printf ("\n");
}

#if defined CL_VERSION_2_0
static void opencl_print_device_svm_capabilities(cl_device_id device, void * val) {
    int printed = 0;
    cl_device_svm_capabilities my_val = *((cl_device_svm_capabilities*)val);
    if (my_val & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) {
        printf("COARSE_GRAIN_BUFFER");
        printed++;
    }
    if (my_val & CL_DEVICE_SVM_FINE_GRAIN_BUFFER)
        printf("%sFINE_GRAIN_BUFFER", printed++ ? ", ": "");
    if (my_val & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM)
        printf("%sFINE_GRAIN_SYSTEM", printed++ ? ", ": "");
    if (my_val & CL_DEVICE_SVM_ATOMICS)
        printf("%sATOMICS", printed++ ? ", ": "");
    printf("\n"); 
}
#endif



enum cl_print_t {
    CL_PRINT_SPECIFIC=0,
    CL_PRINT_BOOL,
    CL_PRINT_UINT,
    CL_PRINT_INT,
    CL_PRINT_ULONG,
    CL_PRINT_LONG,
    CL_PRINT_SIZE_T,
    CL_PRINT_STRING
};

typedef struct {
    cl_uint cl_devinfo;                 // The id to query clGetDeviceInfo
    char cl_devinfo_str[64];            // The equivalent name to print
    char cl_devinfo_long[512];          // Long explanation
    size_t cl_devinfo_size;             // The length of the value returned (or -1 if not yet known)
    enum cl_print_t cl_devinfo_print_t; // Choosing the printf-modifier
    void (*cl_devinfo_printf)(cl_device_id device, void * val);  // If printf-modifier is CL_PRINT_SPECIFIC, call function-ptr, otherwise NULL
} opencl_devinfo_t;

opencl_devinfo_t opencl_devinfo[] = {
    {OPENCL_NAME(CL_DEVICE_NAME),
        "Device name string",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_TYPE),
        "The OpenCL device type. Currently supported values are: CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR, CL_DEVICE_TYPE_DEFAULT, a combination of the above types or CL_DEVICE_TYPE_CUSTOM.",
        sizeof (cl_device_type),
        CL_PRINT_SPECIFIC,
        opencl_print_device_type
    },
    {OPENCL_NAME(CL_DEVICE_VENDOR_ID),
        "A unique device vendor identifier. An example of a unique device identifiert could be the PCIe ID.",
        sizeof (cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_COMPUTE_UNITS),
        "The number of parallel compute units on the OpenCL device. A work-group executes on a single compute unit. The minimum value is 1.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS),
        "Maxim  um dimensions that specify the global and local work-item IDs used by the data parallel execution model (Refer to clEnqueueNDRangeKernel). The minimum value is 3 for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_WORK_ITEM_SIZES),
        "Maximum number of work-items that can be specified in each dimension of the work-group to clEnqueueNDRangeKernel. Returns n size_t entries, where n is the value returned by the query for CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS. The minimum value is (1, 1, 1) for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        0, // This should be calculated from the previous!
        CL_PRINT_SPECIFIC,
        &opencl_print_device_max_work_item_sizes
    },
    {OPENCL_NAME(CL_DEVICE_MAX_WORK_GROUP_SIZE),
        "Maximum number of work-items in a work-group that a device is capable of executing on a single compute unit, for any given kernel-instance running on the device. (Refer to clEnqueueNDRangeKernel and CL_KERNEL_WORK_GROUP_SIZE). The minimum value is 1.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector. If double precision is not supported, CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF),
        "Preferred native vector width size for built-in scalar types that can be put into vectors. The vector width is defined as the number of scalar elements that can be stored in the ￼vector. If the cl_khr_fp16 extension is not supported, CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF),
        "Returns the native ISA vector width. The vector width is defined as the number of scalar elements that can be stored in the vector. If double precision is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_DOU BLE must return 0. If the cl_khr_fp16 extension is not supported, CL_DEVICE_NATIVE_VECTOR_WIDTH_HAL F must return 0.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_CLOCK_FREQUENCY),
        "Maximum configured clock frequency of the device in MHz.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_ADDRESS_BITS),
        "The default compute device address space size specified as an unsigned integer value in bits. Currently supported values are 32 or 64 bits.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_MEM_ALLOC_SIZE),
        "Maximum size of memory object allocation in bytes. The minimum value is max(1/4th of the CL_DEVICE_GLOBAL_MEM_SIZE, 128*1024*1024) for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE_SUPPORT),
        "Is CL_TRUE if images are supported by the OpenCL device and CL_FALSE otherwise.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_READ_IMAGE_ARGS),
        "Max number of simultaneous image objects that can be read by a kernel. The minimum value is 128 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_WRITE_IMAGE_ARGS),
        "Max number of simultaneous image objects that can be written by a kernel. The minimum value is 8 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE2D_MAX_WIDTH),
        "Max width of 2D image or 1D image not created from a buffer object in pixels. The minimum value is 8192 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE2D_MAX_HEIGHT),
        "Max height of 2D image or 1D image not created from a buffer object in pixels. The minimum value is 8192 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE3D_MAX_WIDTH),
        "Max width of 3D image in pixels. The minimum value is 2048 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE3D_MAX_HEIGHT),
        "Max height of 3D image in pixels. The minimum value is 2048 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE3D_MAX_DEPTH),
        "Max depth of 3D image in pixels. The minimum value is 2048 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE),
        "Max number of pixels for a 1D image created from a buffer object. The minimum value is 65536 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE),
        "Max number of pixels for a 1D or 2D image array. The minimum value is 2048 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_SAMPLERS),
        "Maximum number of samplers that can be used in a kernel. Refer to section 6.12.14 for a detailed description on samplers. The minimum value is 16 if CL_DEVICE_IMAGE_SUPPORT is CL_TRUE.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_PARAMETER_SIZE),
        "Max size in bytes of the arguments that can be passed to a kernel. The minimum value is 1024 for devices that are not of type CL_DEVICE_TYPE_CUSTOM. For this minimum value only a maximum of 128 arguments can be passed to a kernel.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MEM_BASE_ADDR_ALIGN),
        "The minimum value is the size (in bits) of the largest OpenCL built-in data type supported by the device (long16 in FULL profile, long16 or int16 in EMBEDDED profile) for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_uint),
        CL_PRINT_SPECIFIC,
        opencl_print_device_mem_base_addr_align
    },
    {OPENCL_NAME(CL_DEVICE_SINGLE_FP_CONFIG),
        "Describes single precision floating-point capability of the device. The mandated minimum floating-point capability for devices that are not of type CL_DEVICE_TYPE_CUSTOM is: CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN.",
        0,
        CL_PRINT_SPECIFIC,
        opencl_print_device_single_fp_config
    },
    {OPENCL_NAME(CL_DEVICE_DOUBLE_FP_CONFIG),
        "Describes double precision floating-point capability of the device. Double precision is an optional feature. If double precision is supported, then the minimum double precision floating-point capability must be: CL_FP_FMA | CL_FP_ROUND_TO_NEAREST | CL_FP_ROUND_TO_ZERO | CL_FP_ROUND_TO_INF | CL_FP_INF_NAN | CL_FP_DENORM.",
        0,
        CL_PRINT_SPECIFIC,
        opencl_print_device_double_fp_config
    },
    {OPENCL_NAME(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE),
        "Type of global memory cache supported. Valid values are: CL_NONE, CL_READ_ONLY_CACHE and CL_READ_WRITE_CACHE.",
        0,
        CL_PRINT_SPECIFIC,
        opencl_print_device_global_mem_cache_type
    },
    {OPENCL_NAME(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE),
        "Size of global memory cache line in bytes.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE),
        "Size of global memory cache in bytes.",
        sizeof(cl_ulong),
        CL_PRINT_ULONG,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_GLOBAL_MEM_SIZE),
        "Size of global device memory in bytes.",
        sizeof(cl_ulong),
        CL_PRINT_ULONG,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE),
        "Max size in bytes of a constant buffer allocation. The minimum value is 64 KB for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_ulong),
        CL_PRINT_ULONG,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_MAX_CONSTANT_ARGS),
        "Max number of arguments declared with the __constant qualifier in a kernel. The minimum value is 8 for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_LOCAL_MEM_TYPE),
        "Type of local memory supported. This can be set to CL_LOCAL implying dedicated local memory storage such as SRAM, or CL_GLOBAL. For custom devices, CL_NONE can also be returned indicating no local memory support.",
        0,
        CL_PRINT_SPECIFIC,
        opencl_print_device_local_mem_type
    },
    {OPENCL_NAME(CL_DEVICE_LOCAL_MEM_SIZE),
        "Size of local memory arena in bytes. The minimum value is 32 KB for devices that are not of type CL_DEVICE_TYPE_CUSTOM.",
        sizeof(cl_ulong),
        CL_PRINT_ULONG,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_ERROR_CORRECTION_SUPPORT),
        "Is CL_TRUE if the device implements error correction for all accesses to compute device memory (global and constant). Is CL_FALSE if the device does not implement such error correction.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_HOST_UNIFIED_MEMORY),
        "Is CL_TRUE if the device and the host have a unified memory subsystem and is CL_FALSE otherwise.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PROFILING_TIMER_RESOLUTION),
        "Describes the resolution of device timer. This is measured in nanoseconds.",
        sizeof(size_t),
        CL_PRINT_SPECIFIC,
        opencl_print_device_profiling_timer_resolution
    },
    {OPENCL_NAME(CL_DEVICE_ENDIAN_LITTLE),
        "Is CL_TRUE if the OpenCL device is a little endian device and CL_FALSE otherwise.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_COMPILER_AVAILABLE),
        "Is CL_FALSE if the implementation does not have a compiler available to compile the program source. Is CL_TRUE if the compiler is available. This can be CL_FALSE for the embedded platform profile only.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_LINKER_AVAILABLE),
        "Is CL_FALSE if the implementation does not have a linker. Is CL_TRUE if the compiler is available. This can be CL_FALSE for the embedded platform profile only. This must be CL_TRUE if CL_DEVICE_COMPILER_AVAILABLE is CL_TRUE.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_EXECUTION_CAPABILITIES),
        "Describes the execution capabilities of the device. This is a bit-field that describes one or more of the following values: CL_EXEC_KERNEL – The OpenCL device can execute OpenCL kernels. CL_EXEC_NATIVE_KERNEL – The OpenCL device can execute native kernels. The mandated minimum capability is: CL_EXEC_KERNEL.",
        sizeof(cl_device_exec_capabilities),
        CL_PRINT_SPECIFIC,
        opencl_print_device_execution_capabilities
    },
    {OPENCL_NAME(CL_DEVICE_QUEUE_PROPERTIES),
        "Describes the command-queue properties supported by the device. This is a bit-field that describes one or more of the following values: CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE and/or CL_QUEUE_PROFILING_ENABLE These properties are described in table 5.1. The mandated minimum capability is: CL_QUEUE_PROFILING_ENABLE.￼",
        sizeof(cl_command_queue_properties),
        CL_PRINT_SPECIFIC,
        opencl_print_device_queue_properties
    },
    {OPENCL_NAME(CL_DEVICE_BUILT_IN_KERNELS),
        "A semi-colon separated list of built-in kernels supported by the device. An empty string is returned if no built-in kernels are supported by the device.",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PLATFORM),
        "The platform associated with this device",
        sizeof(cl_platform_id),
        CL_PRINT_ULONG, // XXX
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_VENDOR),
        "Device vendor string",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DRIVER_VERSION),
        "OpenCL software driver version string in the form major_number.minor_number",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PROFILE),
        "OpenCL profile string. Returns the profile name supported by the device. The profile name returned can be one of the following strings: FULL_PROFILE – if the device supports the OpenCL specification (functionality defined as part of the core specification and does not require any extensions to be supported). EMBEDDED_PROFILE - if the device supports the OpenCL embedded profile.",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_VERSION),
        "OpenCL version string. Returns the OpenCL version supported by the device. This version string has the following format: OpenCL<space><major_version.minor_v ersion><space><vendor-specific information>",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_OPENCL_C_VERSION),
        "￼OpenCL C version string. Returns the highest OpenCL C version supported by the compiler for this device that is not of type CL_DEVICE_TYPE_CUSTOM. This version string has the following format: OpenCL<space>C<space><major_versio n.minor_version><space><vendor- specific information>.",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_EXTENSIONS),
        "Returns a space separated list of extension names (the extension names themselves do not contain any spaces) supported by the device. The list of extension names returned can be vendor supported extension names and one or more of the following Khronos approved extension names:",
        0,
        CL_PRINT_STRING,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PRINTF_BUFFER_SIZE),
        "Maximum size of the internal buffer that holds the output of printf calls from a kernel. The minimum value for the FULL profile is 1 MB.",
        sizeof(size_t),
        CL_PRINT_SIZE_T,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC),
        "Is CL_TRUE if the device’s preference is for the user to be responsible for synchronization, when sharing memory objects between OpenCL and other APIs such as DirectX, CL_FALSE if the device / implementation has a performant path for performing synchronization of memory object shared between OpenCL and other APIs such as DirectX.",
        sizeof(cl_bool),
        CL_PRINT_BOOL,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PARENT_DEVICE),
        "￼Returns the cl_device_id of the parent device to which this sub-device belongs. If device is a root-level device, a NULL value is returned.",
        sizeof(cl_device_id),
        CL_PRINT_ULONG, // XXX
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PARTITION_MAX_SUB_DEVICES),
        "Returns the maximum number of sub- devices that can be created when a device is partitioned. The value returned cannot exceed CL_DEVICE_MAX_COMPUTE_UNITS.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },
    {OPENCL_NAME(CL_DEVICE_PARTITION_PROPERTIES),
        "Returns the list of partition types supported by device. The is an array of cl_device_partition_property values drawn from the following list: ￼￼ CL_DEVICE_PARTITION_EQUALLY, CL_DEVICE_PARTITION_BY_COUNTS, CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN. If the device cannot be partitioned (i.e. there is no partitioning scheme supported by the device that will return at least two subdevices), a value of 0 will be returned.",
        0, // XXX
        CL_PRINT_SPECIFIC,
        opencl_print_device_partition_properties
    },
    {OPENCL_NAME(CL_DEVICE_PARTITION_AFFINITY_DOMAIN),
        "Returns the list of supported affinity domains for partitioning the device using CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN. This is a bit-field that describes one or more of the following values: CL_DEVICE_AFFINITY_DOMAIN_NUMA, CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE, CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE, CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE, CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE, CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE. If the device does not support any affinity domains, a value of 0 will be returned.",
        sizeof(cl_device_affinity_domain),
        CL_PRINT_SPECIFIC,
        opencl_print_device_partition_affinity_domain
    },
    // XXX CL_DEVICE_PARTITION_TYPE also only valid on partitionable devices / subdevices?
    // Either way, the print function would be similar to opencl_print_device_partition_properties, due to []


// This should actually be called only for devices > CL_VERSION_2_0
#if defined (CL_VERSION_2_0)
    {OPENCL_NAME(CL_DEVICE_SVM_CAPABILITIES),
        "Describes the vairous shared virtual memory (SVM) memory allocation types the device supports. This is a bit-field that describes a combination of the following values: CL_DEVICE_SVM_COARSE_GRAIN_BUFFER, CL_DEVICE_SVM_FINE_GRAIN_BUFFER, CL_DEVICE_SVM_FINE_GRAIN_SYSTEM, CL_DEVICE_SVM_ATOMICS",
        0,
        CL_PRINT_SPECIFIC,
        opencl_print_device_svm_capabilities
    },
#endif

    {OPENCL_NAME(CL_DEVICE_REFERENCE_COUNT),
        "Returns the device reference count. If the device is a root-level device, a reference count of one is returned.",
        sizeof(cl_uint),
        CL_PRINT_UINT,
        NULL
    },

};


int opencl_print_platform(cl_platform_id cl_platform)
{
    int err;
    char cl_platform_profile[512];
    char cl_platform_name[512];
    char cl_platform_version[512];
    char cl_platform_vendor[512];
    char cl_platform_extensions[512];

    err = clGetPlatformInfo(cl_platform, CL_PLATFORM_PROFILE,
                            sizeof(cl_platform_profile), &cl_platform_profile, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);

    err = clGetPlatformInfo(cl_platform, CL_PLATFORM_NAME,
                            sizeof(cl_platform_name), &cl_platform_name, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);

    err = clGetPlatformInfo(cl_platform, CL_PLATFORM_VERSION,
                            sizeof(cl_platform_version), &cl_platform_version, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);

    err = clGetPlatformInfo(cl_platform, CL_PLATFORM_VENDOR,
                            sizeof(cl_platform_vendor), &cl_platform_vendor, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);

    err = clGetPlatformInfo(cl_platform, CL_PLATFORM_EXTENSIONS,
                            sizeof(cl_platform_extensions), &cl_platform_extensions, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformInfo", err);

    printf ("Platform:\n"
            "\tname:%s Vendor:%s\n"
            "\tVersion:%s\n"
            "\tProfile:%s\n"
            "\tExtensions:%s\n",
            cl_platform_name, cl_platform_vendor, cl_platform_version,
            cl_platform_profile, cl_platform_extensions);
#if defined(CL_VERSION_2_1)
    {
        // The host timer resolution information is available with OpenCL 2.1
        if (!opencl_platform_supports_version(HAW_VERSION_2_1)) {
            printf ("\tHost Timer Resolution: Not supported (requires OpenCL 2.1 and above)\n");
        } else {
            cl_ulong cl_platform_host_timer_resolution;
            
            err = clGetPlatformInfo(cl_platform, CL_PLATFORM_HOST_TIMER_RESOLUTION,
                                    sizeof(cl_ulong), &cl_platform_host_timer_resolution, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);
            printf ("\tHost Timer Resolution: %ld (ns)\n", cl_platform_host_timer_resolution);
        }
    }
#endif
#if defined(CL_VERSION_3_0)
    {
        // The platform numeric version is available with OpenCL 3.0
        if (!opencl_platform_supports_version(HAW_VERSION_3_0))) {
            printf ("\tNumeric Version: Not supported (requires OpenCL 3.0 and above)\n");
        } else {
            cl_version platform_version;
            
            err = clGetPlatformInfo(cl_platform, CL_PLATFORM_NUMERIC_VERSION,
                                    sizeof(cl_version), &cl_numeric_version, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);
            printf ("\tNumeric Version: %d.%d.%d\n",
                    CL_VERSION_MAJOR(platform_version),
                    CL_VERSION_MINOR(platform_version),
                    CL_VERSION_PATCH(platform_version));
        }
    }
    {
        if (!opencl_platform_supports_version(HAW_VERSION_3_0))) {
            printf ("\tExtensions with Versions: Not supported (requires OpenCL 3.0 and above)\n");
        } else {
            size_t num;
            cl_name_version * ext_version;
            err = clGetPlatformInfo(cl_platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION, 0, NULL, &num);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);
            ext_version = (cl_name_version*)malloc(sizeof(cl_name_version) * num);
            if (NULL == ext_version)
                FATAL_ERROR("malloc", ENOMEM);
            err = clGetPlatformInfo(cl_platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION,
                                    num, &ext_version, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetPlatformInfo", err);
            printf ("\tExtensions with Versions: ");
            for (i = 0; i < num; i++)
                printf("%s (%d.%d.%d)%s",
                        ext_version[i].name, 
                        CL_VERSION_MAJOR(ext_version[i].version),
                        CL_VERSION_MINOR(ext_version[i].version),
                        CL_VERSION_PATCH(ext_version[i].version),
                        (i != num-1) ? ", " : "");
            printf ("\n");
            free(ext_version);
        }

    }
#endif

    return CL_SUCCESS;
}


int opencl_print_device(cl_device_id cl_device) {
    char * val;
    size_t val_size = 1024;
    int i;
    cl_int err;
    char ocl_device_name[256];

    err = clGetDeviceInfo(cl_device, CL_DEVICE_NAME,
            sizeof(ocl_device_name), &ocl_device_name, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetDeviceInfo", err);

    val = malloc (val_size);
    if (NULL == val)
        FATAL_ERROR("malloc", ENOMEM);

    for (i = 0; i < sizeof(opencl_devinfo) / sizeof(opencl_devinfo[0]); i++) {
        size_t size = opencl_devinfo[i].cl_devinfo_size;
        size_t val_len;

        if (size > val_size) {
            val = realloc(val, size);
            if (NULL == val)
                FATAL_ERROR("malloc", ENOMEM);
            val_size = size;
        }

        printf ("%s:",
                opencl_devinfo[i].cl_devinfo_str);
        // XXX Should only call if the OpenCL Version is supported
        err = clGetDeviceInfo(cl_device, opencl_devinfo[i].cl_devinfo,
                              val_size, val, &val_len);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceInfo", err);
        switch (opencl_devinfo[i].cl_devinfo_print_t) {
            case CL_PRINT_SPECIFIC:
                if (NULL == opencl_devinfo[i].cl_devinfo_printf)
                    FATAL_ERROR("INTERNAL ERROR: specific printf() not implemented", -1);
                opencl_devinfo[i].cl_devinfo_printf(cl_device, val);
                break;
            case CL_PRINT_BOOL:
                printf ("%s\n", *((cl_bool*)val) == CL_TRUE ? "true" : "false");
                break;
            case CL_PRINT_UINT:
                printf ("%u\n", *((cl_uint*)val));
                break;
            case CL_PRINT_INT:
                printf ("%d\n", *((cl_int*)val));
                break;
            case CL_PRINT_ULONG:
                printf ("%lu\n", *((unsigned long*)val));
                break;
            case CL_PRINT_LONG:
                printf ("%ld\n", *((long*)val));
                break;
            case CL_PRINT_SIZE_T:
                printf ("%zd\n", *((size_t*)val));
                break;
            case CL_PRINT_STRING:
                printf ("%s\n", ((char*)val));
                break;
            default: break;
        }
    }

    free(val);
    return CL_SUCCESS;
}


int opencl_print_info(void)
{
    cl_int i;
    cl_int j;
    cl_int err;
    cl_uint ocl_numPlatforms;
    cl_uint ocl_numDevices;
    cl_platform_id * ocl_platforms;
    cl_device_id * ocl_devices;

    err = clGetPlatformIDs( 0, NULL, &ocl_numPlatforms );
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatformIDs", err);
    if (0 == ocl_numPlatforms)
        FATAL_ERROR("Function clGetPlatformIDs did not find any Platforms!", 0);


    ocl_platforms = (cl_platform_id*) malloc (ocl_numPlatforms * sizeof (cl_platform_id));
    if (NULL == ocl_platforms)
        FATAL_ERROR("malloc", ENOMEM);

    err = clGetPlatformIDs(ocl_numPlatforms, ocl_platforms, NULL);
    if (CL_SUCCESS != err)
        FATAL_ERROR("clGetPlatFormIDs", err);
    if (NULL == ocl_platforms)
        FATAL_ERROR("Function clGetPlatformIDs did not find any Platforms!", 0);

    printf("Overall %u OpenCL platforms available. Printing information:\n", ocl_numPlatforms);
    for (i = 0; i < ocl_numPlatforms; i++) {

        printf ("================= %d. platform =================\n", i);
        opencl_platform_supports_init(ocl_platforms[i]);
        err = opencl_print_platform(ocl_platforms[i]);
        if (0 != err)
            FATAL_ERROR("opencl_print_info", err);

        printf ("Printing Devices:\n");
        err = clGetDeviceIDs(ocl_platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &ocl_numDevices);
        if (CL_SUCCESS != err)
            FATAL_ERROR("clGetDeviceIDs", err);

        if (0 == ocl_numDevices) {
            printf ("No Devices for Platform:%d\n", i);
        } else {
            ocl_devices = (cl_device_id*) malloc (ocl_numDevices * sizeof(cl_device_id));
            if (NULL == ocl_devices)
                FATAL_ERROR("malloc", ENOMEM);

            err = clGetDeviceIDs (ocl_platforms[i], CL_DEVICE_TYPE_ALL, ocl_numDevices, ocl_devices, NULL);
            if (CL_SUCCESS != err)
                FATAL_ERROR("clGetDeviceIDs", err);

            for (j = 0; j < ocl_numDevices; j++) {
                printf ("----------------- %d. device -----------------\n", j);
                opencl_print_device(ocl_devices[j]);
            }
        }
    }

    return 0;
}

