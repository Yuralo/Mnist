#ifndef DLARRAY_H
#define DLARRAY_H
// clang++ -O3 -ffast-math -framework Accelerate dlarray_accelerate.cpp -o dlarray

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <Accelerate/Accelerate.h>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>

class DLArray {
public:
    size_t ndim;
    std::vector<size_t> shape;
    std::vector<size_t> strides;
    size_t size;
    float* data;
    float* base_data;  // for views
    size_t offset;     // for views
    bool owns_data;    // whether this instance owns base_data

    // ------------------ Constructors ------------------
    DLArray() : ndim(0), size(0), data(nullptr), base_data(nullptr), offset(0), owns_data(false) {}

    explicit DLArray(const std::vector<size_t>& shape_)
        : ndim(0), size(0), data(nullptr), base_data(nullptr), offset(0), owns_data(false) {
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;

        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data; // offset == 0
            owns_data = true;
            std::memset(base_data, 0, size * sizeof(float));
        }

        // compute strides
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // ------------------ Copy Constructor ------------------
    DLArray(const DLArray& other)
        : ndim(other.ndim), shape(other.shape), strides(other.strides), size(other.size), data(nullptr), base_data(nullptr), offset(other.offset), owns_data(false) {
        if (other.size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, other.size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            std::memcpy(base_data, other.base_data, other.size * sizeof(float));
            data = base_data + other.offset;
            owns_data = true;
        }
    }

    // ------------------ Move Constructor ------------------
    DLArray(DLArray&& other)
        : ndim(other.ndim), shape(std::move(other.shape)), strides(std::move(other.strides)), size(other.size), data(other.data), base_data(other.base_data), offset(other.offset), owns_data(other.owns_data) {
        other.ndim = 0;
        other.size = 0;
        other.data = nullptr;
        other.base_data = nullptr;
        other.offset = 0;
        other.owns_data = false;
    }

    // Initialize from another DLArray
    void from(const DLArray& other) {
        // shallow view into other's storage
        ndim = other.ndim;
        shape = other.shape;
        strides = other.strides;
        size = other.size;
        offset = other.offset;
        base_data = other.base_data;
        data = base_data + offset;
        owns_data = false;
    }

    // Copy data from a void* pointer with given shape
    // template parameter T allows copying from different types (unsigned char, float, etc.)
    template<typename T>
    void copy_from(const void* src_ptr, const std::vector<size_t>& shape_) {
        // Free existing data if we own it
        if (owns_data && base_data) {
            free(base_data);
        }
        
        // Set shape and compute size
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        
        if (size > 0 && src_ptr != nullptr) {
            // Allocate aligned memory
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            
            // Copy and convert data from source type T to float
            const T* src = static_cast<const T*>(src_ptr);
            for (size_t i = 0; i < size; ++i) {
                base_data[i] = static_cast<float>(src[i]);
            }
        } else {
            base_data = nullptr;
            data = nullptr;
            owns_data = false;
        }
        
        // Compute strides
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // Overload for float* (direct copy, no conversion)
    void copy_from(const float* src_ptr, const std::vector<size_t>& shape_) {
        // Free existing data if we own it
        if (owns_data && base_data) {
            free(base_data);
        }
        
        // Set shape and compute size
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        
        if (size > 0 && src_ptr != nullptr) {
            // Allocate aligned memory
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            
            // Direct copy (float to float)
            std::memcpy(base_data, src_ptr, size * sizeof(float));
        } else {
            base_data = nullptr;
            data = nullptr;
            owns_data = false;
        }
        
        // Compute strides
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // Initilize with zeros of shape
    void zeros(const std::vector<size_t>& shape_) {
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            std::memset(base_data, 0, size * sizeof(float));
        }
        // compute strides
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // Initilize with ones of shape
    void ones(const std::vector<size_t>& shape_) {
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            for (size_t i = 0; i < size; ++i) base_data[i] = 1.0f;
        }
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // Initilize with random values of shape
    void random(const std::vector<size_t>& shape_) {
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1.0f);
            for (size_t i = 0; i < size; ++i) {
                base_data[i] = dis(gen);
            }
        }
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }

    // ------------------ Destructor ------------------
    ~DLArray() {
        if (owns_data && base_data) free(base_data);
    }


    // ------------------ Reshape ------------------
    void reshape(const std::vector<size_t>& new_shape) {
        size_t new_size = 1;
        for (size_t s : new_shape) new_size *= s;
        if (new_size != size) throw std::runtime_error("Invalid reshape: size mismatch");
        shape = new_shape;
        ndim = new_shape.size();

        // recompute strides
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = ndim - 1; i >= 0; --i) {
            strides[i] = stride;
            stride *= shape[i];
        }
    }

    // ------------------ Slice ------------------
    DLArray slice(size_t index) const {
        if (ndim < 1 || index >= shape[0])
            throw std::runtime_error("Invalid slice index");

        DLArray view;
        view.ndim = ndim - 1;
        view.shape.assign(shape.begin() + 1, shape.end());
        view.size = 1;
        for (auto s : view.shape) view.size *= s;

        view.strides.assign(strides.begin() + 1, strides.end());
        view.base_data = base_data;
        view.offset = offset + index * strides[0];
        view.data = base_data + view.offset;
        view.owns_data = false;
        return view;
    }

    // ------------------ Elementwise Add ------------------
    DLArray add(const DLArray& other) const {
        if (size != other.size)
            throw std::runtime_error("Shape mismatch in add");

        DLArray out(shape);
        vDSP_vadd(data, 1, other.data, 1, out.data, 1, size);
        return out;
    }

    // ------------------ Matrix Multiplication ------------------
    DLArray matmul(const DLArray& B) const {
        if (ndim != 2 || B.ndim != 2)
            throw std::runtime_error("Matmul requires 2D arrays");

        size_t M = shape[0];
        size_t K = shape[1];
        size_t N = B.shape[1];

        if (B.shape[0] != K)
            throw std::runtime_error("Shape mismatch in matmul");

        DLArray out({M, N});
        vDSP_mmul(data, 1, B.data, 1, out.data, 1, M, N, K);
        return out;
    }

    void print_shape() const {
        std::cout << "Shape: ";
        for (size_t i = 0; i < ndim; ++i) {
            std::cout << shape[i] << " ";
        }
        std::cout << "\n";
    }

    // ------------------ Convert to Vector ------------------
    std::vector<float> to_vector() const {
        std::vector<float> result;
        result.reserve(size);
        
        if (ndim == 0) {
            // 0D array (scalar)
            result.push_back(base_data[offset]);
        } else {
            // Multi-dimensional array - flatten using row-major order
            to_vector_recursive(0, offset, result);
        }
        
        return result;
    }

    // ------------------ Print ------------------
    void print() const {
        if (ndim == 0) {
            // Handle 0D array (scalar)
            std::cout << base_data[offset];
        } else {
            print_recursive(0, offset);
        }
        std::cout << "\n";
    }


    // ------------------ Copy Assignment Operator ------------------
    DLArray& operator=(const DLArray& other) { // deep copy
        if (this == &other) return *this;

        if (owns_data && base_data) {
            free(base_data);
        }

        ndim = other.ndim;
        shape = other.shape;
        strides = other.strides;
        size = other.size;
        offset = other.offset;
        base_data = nullptr;
        data = nullptr;
        owns_data = false;

        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            std::memcpy(base_data, other.base_data, size * sizeof(float));
            data = base_data + offset;
            owns_data = true;
        }
        return *this;
    }

     DLArray& operator=(DLArray&& other) {
        if (this == &other) return *this;

        if (owns_data && base_data) {
            free(base_data);
        }

        ndim = other.ndim;
        shape = std::move(other.shape);
        strides = std::move(other.strides);
        size = other.size;
        offset = other.offset;
        base_data = other.base_data;
        data = other.data;
        owns_data = other.owns_data;

        other.ndim = 0;
        other.size = 0;
        other.offset = 0;
        other.base_data = nullptr;
        other.data = nullptr;
        other.owns_data = false;
        return *this;
    }
        // ------------------ ReLU ------------------
    DLArray relu() const {
        DLArray out(shape);
        for (size_t i = 0; i < size; ++i) {
            out.data[i] = std::max(0.0f, data[i]);
        }
        return out;
    }

    // ------------------ Softmax ------------------
    DLArray softmax() const {
        DLArray out(shape);
        // handle overflow by subtracting the max value from each element
        float max_value = *std::max_element(data, data + size);
        for (size_t i = 0; i < size; ++i) {
            out.data[i] = std::exp(data[i] - max_value); // subtract the max value to prevent overflow
        }
        // normalize the sum of the elements to 1
        float sum = std::accumulate(out.data, out.data + size, 0.0f); // 
        for (size_t i = 0; i < size; ++i) {
            out.data[i] /= sum;
        }
        return out;
    }

    void he_init(const std::vector<size_t>& shape_, size_t fan_in) {
        shape = shape_;
        ndim = shape.size();
        size = 1;
        for (size_t s : shape) size *= s;
        offset = 0;
        
        if (size > 0) {
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, size * sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            base_data = static_cast<float*>(ptr);
            data = base_data;
            owns_data = true;
            
            // He initialization: scale = sqrt(2 / fan_in)
            std::random_device rd;
            std::mt19937 gen(rd());
            float std_dev = std::sqrt(2.0f / fan_in);
            std::normal_distribution<float> dis(0.0f, std_dev);
            
            for (size_t i = 0; i < size; ++i) {
                base_data[i] = dis(gen);
            }
        }
        
        strides.resize(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            strides[static_cast<size_t>(i)] = stride;
            stride *= shape[static_cast<size_t>(i)];
        }
    }
    // Optimized stuff
    // {

    //     // DLArray relu() const {
    //     //     DLArray out(shape);
    //     //     float32x4_t zero = vdupq_n_f32(0.0f);
    //     //     size_t i = 0;
            
    //     //     // Process 4 elements at a time with NEON
    //     //     for (; i + 4 <= size; i += 4) {
    //     //         float32x4_t vec = vld1q_f32(&data[i]);
    //     //         float32x4_t masked = vmaxq_f32(vec, zero);  // max(vec, 0)
    //     //         vst1q_f32(&out.data[i], masked);
    //     //     }
            
    //     //     // Handle remaining elements
    //     //     for (; i < size; ++i) {
    //     //         out.data[i] = std::max(0.0f, data[i]);
    //     //     }
            
    //     //     return out;
    //     // }

    //     // // ------------------ Softmax ------------------
    //     // DLArray softmax() const {
    //     //     DLArray out(shape);
            
    //     //     // Find max value for numerical stability
    //     //     float max_value = *std::max_element(data, data + size);
    //     //     float32x4_t max_vec = vdupq_n_f32(max_value);
            
    //     //     // Step 1: Subtract max and compute exp
    //     //     // Since NEON doesn't have vectorized exp, we compute it element-wise
    //     //     size_t i = 0;
    //     //     for (; i + 4 <= size; i += 4) {
    //     //         float32x4_t vec = vld1q_f32(&data[i]);
    //     //         float32x4_t shifted = vsubq_f32(vec, max_vec);
                
    //     //         // Store shifted values and compute exp
    //     //         float shifted_arr[4];
    //     //         vst1q_f32(shifted_arr, shifted);
    //     //         out.data[i]     = std::exp(shifted_arr[0]);
    //     //         out.data[i + 1] = std::exp(shifted_arr[1]);
    //     //         out.data[i + 2] = std::exp(shifted_arr[2]);
    //     //         out.data[i + 3] = std::exp(shifted_arr[3]);
    //     //     }
            
    //     //     // Handle remaining elements for exp
    //     //     for (; i < size; ++i) {
    //     //         out.data[i] = std::exp(data[i] - max_value);
    //     //     }
            
    //     //     // Step 2: Compute sum with NEON
    //     //     float32x4_t sum_vec = vdupq_n_f32(0.0f);
    //     //     i = 0;
    //     //     for (; i + 4 <= size; i += 4) {
    //     //         float32x4_t vec = vld1q_f32(&out.data[i]);
    //     //         sum_vec = vaddq_f32(sum_vec, vec);
    //     //     }
            
    //     //     // Reduce sum_vec to scalar
    //     //     float sum = vaddvq_f32(sum_vec);
    //     //     for (; i < size; ++i) {
    //     //         sum += out.data[i];
    //     //     }
            
    //     //     // Step 3: Normalize with NEON
    //     //     float32x4_t inv_sum = vdupq_n_f32(1.0f / sum);
    //     //     i = 0;
    //     //     for (; i + 4 <= size; i += 4) {
    //     //         float32x4_t vec = vld1q_f32(&out.data[i]);
    //     //         float32x4_t normalized = vmulq_f32(vec, inv_sum);
    //     //         vst1q_f32(&out.data[i], normalized);
    //     //     }
            
    //     //     // Handle remaining elements for normalization
    //     //     for (; i < size; ++i) {
    //     //         out.data[i] /= sum;
    //     //     }
            
    //     //     return out;
    //     // }
    // };
    DLArray transpose() const {
        if (ndim == 0) {
            // Scalar - return a copy
            DLArray out;
            out.ndim = 0;
            out.size = 1;
            void* ptr = nullptr;
            if (posix_memalign(&ptr, 64, sizeof(float)) != 0) {
                throw std::bad_alloc();
            }
            out.base_data = static_cast<float*>(ptr);
            out.data = out.base_data;
            out.offset = 0;
            out.owns_data = true;
            out.base_data[0] = base_data[offset];
            return out;
        }

        // Create new shape with reversed dimensions
        std::vector<size_t> new_shape(ndim);
        for (size_t i = 0; i < ndim; ++i) {
            new_shape[i] = shape[ndim - 1 - i];
        }

        DLArray out(new_shape);

        // Compute strides for transposed array
        std::vector<size_t> new_strides(ndim);
        size_t stride = 1;
        for (int i = static_cast<int>(ndim) - 1; i >= 0; --i) {
            new_strides[static_cast<size_t>(i)] = stride;
            stride *= new_shape[static_cast<size_t>(i)];
        }

        // Helper function to compute linear index from multi-dimensional indices
        auto compute_offset = [](const std::vector<size_t>& indices, 
                                 const std::vector<size_t>& strides_vec, 
                                 size_t ndims) -> size_t {
            size_t offset = 0;
            for (size_t i = 0; i < ndims; ++i) {
                offset += indices[i] * strides_vec[i];
            }
            return offset;
        };

        // Iterate through all elements in the original array
        std::vector<size_t> indices(ndim, 0);
        for (size_t i = 0; i < size; ++i) {
            // Compute offset in original array
            size_t orig_offset = compute_offset(indices, strides, ndim);
            
            // Compute transposed indices (reversed order)
            std::vector<size_t> transposed_indices(ndim);
            for (size_t j = 0; j < ndim; ++j) {
                transposed_indices[j] = indices[ndim - 1 - j];
            }
            
            // Compute offset in transposed array
            size_t trans_offset = compute_offset(transposed_indices, new_strides, ndim);
            
            // Copy element
            out.base_data[trans_offset] = base_data[offset + orig_offset];
            
            // Increment indices (like a multi-dimensional counter)
            for (int j = static_cast<int>(ndim) - 1; j >= 0; --j) {
                indices[static_cast<size_t>(j)]++;
                if (indices[static_cast<size_t>(j)] < shape[static_cast<size_t>(j)]) {
                    break;
                }
                indices[static_cast<size_t>(j)] = 0;
            }
        }

        return out;
    }
private:
    void print_recursive(size_t dim, size_t base_offset) const {
        if (dim == ndim - 1) {
            std::cout << "[";
            for (size_t i = 0; i < shape[dim]; ++i) {
                std::cout << base_data[base_offset + i * strides[dim]];
                if (i + 1 < shape[dim]) std::cout << ", ";
            }
            std::cout << "]";
            return;
        }

        std::cout << "[";
        for (size_t i = 0; i < shape[dim]; ++i) {
            print_recursive(dim + 1, base_offset + i * strides[dim]);
            if (i + 1 < shape[dim]) std::cout << ",\n";
        }
        std::cout << "]";
    }

    void to_vector_recursive(size_t dim, size_t base_offset, std::vector<float>& result) const {
        if (dim == ndim - 1) {
            // Last dimension - copy all elements
            for (size_t i = 0; i < shape[dim]; ++i) {
                result.push_back(base_data[base_offset + i * strides[dim]]);
            }
            return;
        }

        // Recurse into next dimension
        for (size_t i = 0; i < shape[dim]; ++i) {
            to_vector_recursive(dim + 1, base_offset + i * strides[dim], result);
        }
    }


};

#endif // DLARRAY_H