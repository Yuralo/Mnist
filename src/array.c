// dlpack_matrix_mul.c
// Minimal DLPack-like example: CPU + OpenCL backend matrix multiply (C only).
// Build: clang -std=c11 dlpack_matrix_mul.c -framework OpenCL -O2 -o dlpack_mm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#if defined(__APPLE__)
  #include <OpenCL/opencl.h>
#else
  #include <CL/cl.h>
#endif

/* --- Minimal DLPack-like definitions --- */
typedef enum {
    kDLCPU = 1,
    kDLCUDA = 2,
    kDLOpenCL = 31, // custom value for this example
} DLDeviceType;

typedef struct {
    int device_type;   // DLDeviceType
    int device_id;
} DLDevice;

typedef struct {
    uint8_t code;   // e.g., 0=int,1=uint,2=float
    uint8_t bits;   // bits per element
    uint16_t lanes; // lanes (vectorization) usually 1
} DLDataType;

typedef struct {
    void* data;
    DLDevice device;
    int ndim;
    int64_t* shape;
    int64_t* strides; // bytes between elements (optional)
    DLDataType dtype;
    uint64_t byte_offset;
} DLTensor;

typedef void (*deleter_fn)(void* manager_ctx);

typedef struct DLManagedTensor {
    DLTensor dl_tensor;
    void* manager_ctx;
    deleter_fn deleter;
} DLManagedTensor;

/* --- Utility helpers --- */

static DLDataType dtype_float32() {
    DLDataType dt;
    dt.code = 2; dt.bits = 32; dt.lanes = 1;
    return dt;
}

/* allocate shape array copy */
static int64_t* shape_copy(const int64_t* s, int ndim) {
    int64_t* out = (int64_t*)malloc(sizeof(int64_t)*ndim);
    memcpy(out, s, sizeof(int64_t)*ndim);
    return out;
}

/* proper deleter for CPU-managed tensor */
static void cpu_deleter(void* manager_ctx) {
        float* data = (float*)manager_ctx;
        free(data);
}
    
/* create a CPU DLManagedTensor that owns its data */
static DLManagedTensor* create_cpu_managed_tensor(float* data, int ndim, const int64_t* shape) {
    DLManagedTensor* m = (DLManagedTensor*)malloc(sizeof(DLManagedTensor));
    m->dl_tensor.data = (void*)data;
    m->dl_tensor.device.device_type = kDLCPU;
    m->dl_tensor.device.device_id = 0;
    m->dl_tensor.ndim = ndim;
    m->dl_tensor.shape = shape_copy(shape, ndim);
    m->dl_tensor.strides = NULL;
    m->dl_tensor.dtype = dtype_float32();
    m->dl_tensor.byte_offset = 0;
    m->manager_ctx = data; // we store pointer so deleter can free
    m->deleter = cpu_deleter;
    return m;
}


/* create CPU managed tensor with allocation */
static DLManagedTensor* alloc_cpu_tensor(int ndim, const int64_t* shape) {
    int64_t N = 1;
    for (int i=0;i<ndim;i++) N *= shape[i];
    float* data = (float*)malloc(sizeof(float)*N);
    DLManagedTensor* m = (DLManagedTensor*)malloc(sizeof(DLManagedTensor));
    m->dl_tensor.data = data;
    m->dl_tensor.device.device_type = kDLCPU;
    m->dl_tensor.device.device_id = 0;
    m->dl_tensor.ndim = ndim;
    m->dl_tensor.shape = shape_copy(shape, ndim);
    m->dl_tensor.strides = NULL;
    m->dl_tensor.dtype = dtype_float32();
    m->dl_tensor.byte_offset = 0;
    m->manager_ctx = data;
    m->deleter = cpu_deleter;
    return m;
}

/* call deleter */
static void free_managed_tensor(DLManagedTensor* m) {
    if (!m) return;
    if (m->deleter) m->deleter(m->manager_ctx);
    free(m->dl_tensor.shape);
    free(m);
}

/* --- CPU matrix multiply C = A * B
   All tensors are row-major, shapes:
   A: (M, K), B: (K, N), C: (M, N)
*/
static void matmul_cpu(const DLManagedTensor* A, const DLManagedTensor* B, DLManagedTensor* C) {
    if (A->dl_tensor.dtype.code != 2 || B->dl_tensor.dtype.code != 2) {
        fprintf(stderr, "Expected float32 tensors\n"); return;
    }
    int64_t M = A->dl_tensor.shape[0];
    int64_t K = A->dl_tensor.shape[1];
    int64_t Kb = B->dl_tensor.shape[0];
    int64_t N = B->dl_tensor.shape[1];
    if (K != Kb) { fprintf(stderr, "Shape mismatch K\n"); return; }
    if (C->dl_tensor.shape[0] != M || C->dl_tensor.shape[1] != N) { fprintf(stderr, "Output shape mismatch\n"); return; }

    float* a = (float*)A->dl_tensor.data;
    float* b = (float*)B->dl_tensor.data;
    float* c = (float*)C->dl_tensor.data;

    for (int64_t i=0;i<M;i++) {
        for (int64_t j=0;j<N;j++) {
            double sum = 0.0;
            for (int64_t k=0;k<K;k++) {
                sum += (double)a[i*K + k] * (double)b[k*N + j];
            }
            c[i*N + j] = (float)sum;
        }
    }
}

/* --- OpenCL fallback: compile kernel and launch --- */

/* simple matmul kernel (row-major) */
const char* cl_kernel_src =
"__kernel void matmul(const int M, const int N, const int K, __global float* A, __global float* B, __global float* C) {\n"
"    int row = get_global_id(0);\n"
"    int col = get_global_id(1);\n"
"    if (row >= M || col >= N) return;\n"
"    float sum = 0.0f;\n"
"    for (int k=0;k<K;k++) {\n"
"        sum += A[row*K + k] * B[k*N + col];\n"
"    }\n"
"    C[row*N + col] = sum;\n"
"}\n";

/* Try to run OpenCL matmul. Returns 0 on success, nonzero on failure or if OpenCL not available. */
static int try_opencl_matmul(const DLManagedTensor* A, const DLManagedTensor* B, DLManagedTensor* C) {
    cl_int err;
    cl_uint num_platforms = 0;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if (err != CL_SUCCESS || num_platforms == 0) {
        // OpenCL not available
        return 1;
    }
    cl_platform_id* platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id)*num_platforms);
    clGetPlatformIDs(num_platforms, platforms, NULL);

    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    int found_device = 0;

    for (cl_uint pi=0; pi<num_platforms && !found_device; ++pi) {
        cl_uint num_devices = 0;
        // try GPU first, then CPU
        if (clGetDeviceIDs(platforms[pi], CL_DEVICE_TYPE_GPU, 0, NULL, &num_devices) == CL_SUCCESS && num_devices > 0) {
            cl_device_id* devs = malloc(sizeof(cl_device_id)*num_devices);
            clGetDeviceIDs(platforms[pi], CL_DEVICE_TYPE_GPU, num_devices, devs, NULL);
            device = devs[0];
            free(devs);
            found_device = 1;
            break;
        } else if (clGetDeviceIDs(platforms[pi], CL_DEVICE_TYPE_CPU, 0, NULL, &num_devices) == CL_SUCCESS && num_devices > 0) {
            cl_device_id* devs = malloc(sizeof(cl_device_id)*num_devices);
            clGetDeviceIDs(platforms[pi], CL_DEVICE_TYPE_CPU, num_devices, devs, NULL);
            device = devs[0];
            free(devs);
            found_device = 1;
            break;
        }
    }
    free(platforms);
    if (!found_device) return 1;

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) return 1;
    queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
    if (err != CL_SUCCESS) { clReleaseContext(context); return 1; }

    // compile program
    const char* src = cl_kernel_src;
    cl_program program = clCreateProgramWithSource(context, 1, &src, NULL, &err);
    if (err != CL_SUCCESS) { clReleaseCommandQueue(queue); clReleaseContext(context); return 1; }
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        // print build log
        size_t log_size = 0;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char* log = (char*)malloc(log_size+1);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        log[log_size] = '\0';
        fprintf(stderr, "OpenCL build log:\n%s\n", log);
        free(log);
        clReleaseProgram(program);
        clReleaseCommandQueue(queue);
        clReleaseContext(context);
        return 1;
    }

    cl_kernel kernel = clCreateKernel(program, "matmul", &err);
    if (err != CL_SUCCESS) { clReleaseProgram(program); clReleaseCommandQueue(queue); clReleaseContext(context); return 1; }

    // Create buffers
    int64_t M = A->dl_tensor.shape[0];
    int64_t K = A->dl_tensor.shape[1];
    int64_t Kb = B->dl_tensor.shape[0];
    int64_t N = B->dl_tensor.shape[1];
    if (K != Kb) { fprintf(stderr,"Shape mismatch\n"); clReleaseKernel(kernel); clReleaseProgram(program); clReleaseCommandQueue(queue); clReleaseContext(context); return 1; }

    size_t sizeA = (size_t)(M*K)*sizeof(float);
    size_t sizeB = (size_t)(K*N)*sizeof(float);
    size_t sizeC = (size_t)(M*N)*sizeof(float);

    // host pointers
    float* hostA = (float*)A->dl_tensor.data;
    float* hostB = (float*)B->dl_tensor.data;
    float* hostC = (float*)C->dl_tensor.data;

    cl_mem bufA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeA, hostA, &err);
    if (err != CL_SUCCESS) goto cleanup_opencl;
    cl_mem bufB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeB, hostB, &err);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufA); goto cleanup_opencl; }
    cl_mem bufC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeC, NULL, &err);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufA); clReleaseMemObject(bufB); goto cleanup_opencl; }

    // set args
    err  = clSetKernelArg(kernel, 0, sizeof(int), (void*)&(int){(int)M});
    err |= clSetKernelArg(kernel, 1, sizeof(int), (void*)&(int){(int)N});
    err |= clSetKernelArg(kernel, 2, sizeof(int), (void*)&(int){(int)K});
    err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &bufA);
    err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &bufB);
    err |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &bufC);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufA); clReleaseMemObject(bufB); clReleaseMemObject(bufC); goto cleanup_opencl; }

    size_t global[2] = {(size_t)M, (size_t)N};
    size_t local[2] = {8, 8}; // simple local size; driver will adjust if needed

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, local, 0, NULL, NULL);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufA); clReleaseMemObject(bufB); clReleaseMemObject(bufC); goto cleanup_opencl; }

    clFinish(queue);

    // read result
    err = clEnqueueReadBuffer(queue, bufC, CL_TRUE, 0, sizeC, hostC, 0, NULL, NULL);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufA); clReleaseMemObject(bufB); clReleaseMemObject(bufC); goto cleanup_opencl; }

    // success -> release buffers and objects
    clReleaseMemObject(bufA);
    clReleaseMemObject(bufB);
    clReleaseMemObject(bufC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;

cleanup_opencl:
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 1;
}

/* --- Example run: create matrices, run matmul on GPU if possible, otherwise CPU --- */
int main(int argc, char** argv) {
    (void)argc; (void)argv;
    // small example sizes
    const int64_t M = 128;
    const int64_t K = 64;
    const int64_t N = 96;

    int64_t shapeA[2] = {M, K};
    int64_t shapeB[2] = {K, N};
    int64_t shapeC[2] = {M, N};

    DLManagedTensor* A = alloc_cpu_tensor(2, shapeA);
    DLManagedTensor* B = alloc_cpu_tensor(2, shapeB);
    DLManagedTensor* C = alloc_cpu_tensor(2, shapeC);

    // fill A and B with some values
    int64_t a_elems = M*K;
    int64_t b_elems = K*N;
    for (int64_t i=0;i<a_elems;i++) ((float*)A->dl_tensor.data)[i] = (float)(i % 7) * 0.01f + 1.0f;
    for (int64_t i=0;i<b_elems;i++) ((float*)B->dl_tensor.data)[i] = (float)(i % 5) * 0.02f + 0.5f;

    // try OpenCL
    fprintf(stdout, "Attempting OpenCL matmul (falls back to CPU if not available)...\n");
    int cl_ok = try_opencl_matmul(A, B, C);
    if (cl_ok == 0) {
        fprintf(stdout, "OpenCL matmul succeeded.\n");
    } else {
        fprintf(stdout, "OpenCL not available or failed -> using CPU matmul.\n");
        matmul_cpu(A, B, C);
    }

    // Print first 5 elements of C row 0 to verify
    float* cref = (float*)C->dl_tensor.data;
    fprintf(stdout, "C[0,0..4] = ");
    for (int i=0;i<5;i++) {
        printf("%f ", cref[i]);
    }
    printf("\n");

    // small correctness check against CPU reference (recompute first row via CPU and compare)
    double max_diff = 0.0;
    for (int j=0;j<N;j++) {
        double sum = 0.0;
        for (int k=0;k<K;k++) sum += ((float*)A->dl_tensor.data)[0*K + k] * ((float*)B->dl_tensor.data)[k*N + j];
        double diff = fabs(sum - (double)cref[j]);
        if (diff > max_diff) max_diff = diff;
    }
    fprintf(stdout, "Max absolute diff (row 0) vs CPU reference: %g\n", max_diff);

    // cleanup
    free_managed_tensor(A);
    free_managed_tensor(B);
    free_managed_tensor(C);

    return 0;
}