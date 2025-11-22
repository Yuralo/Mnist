# MNIST Neural Network in C++ (Accelerate + NEON + Terminal Plotting)

A compact, high-performance MNIST classification pipeline written entirely in C++17.
It includes:
-	A custom tensor class (DLArray) with aligned memory, slicing, views, BLAS-accelerated matmul, vectorized operations, and initialization helpers.
-	A fully manual neural network (forward + backward) with He-initialization, ReLU, and softmax.
-	A terminal-based plotting library for visualizing MNIST images directly in the terminal.
-	A raw MNIST loader (no external dependencies).
-	Optimized math using Apple Accelerate and room for ARM NEON intrinsics.

<hr/>

## Features

## 🧮 DLArray Tensor Library

A lightweight but powerful multidimensional array implementation supporting:
-	Arbitrary ndim tensors
-	Dynamic shape + stride tracking
-	Views and slicing without copying
-	Aligned allocations for SIMD/BLAS
-	vDSP-accelerated:
-	Matrix multiplication
-	Elementwise addition
-	High-level ops:
-	relu()
-	softmax()
-	transpose()
-	he_init()

This library works as a tiny NumPy-like backend in pure C++.

###  🔢 MNIST Loader

Reads the official IDX MNIST image + label files:
-	Loads t10k-images.idx3-ubyte and t10k-labels.idx1-ubyte
-	Supports:
-	Magic number parsing
-	Dimension parsing
-	Image + label buffers
-	Converts images to floats and normalizes to [0,1]

No external library required.

<hr>

### 🎨 Terminal Image Plotting

Using terminal_plot.h, this project can render MNIST digits directly in your terminal with:
-	ANSI color support
-	Customizable color themes
-	2D image heatmaps

This helps inspect data quickly while staying within the terminal.

<hr>

### 🤖 Neural Network

A small 3-layer MLP implemented from scratch:
```bash
Input → Dense(784→128) → Dense(128→128) + ReLU → Dense(128→10) + Softmax
```

Training loop features:
-	Manual forward pass
-	Manual backward pass
-	Cross-entropy loss for one-hot labels
-	Weight updates via gradient descent
-	He initialization

This demonstrates full control over every step of training a classifier—no frameworks needed.

<hr/>

### Directory Structure
```
.
├── dlarray.h           → Tensor library
├── terminal_plot.h     → Terminal plotting utilities
├── mnist.cpp           → Main program
├── data/
│   ├── t10k-images.idx3-ubyte
│   └── t10k-labels.idx1-ubyte
└── README.md
```

Dependencies
- C++17
- Apple Accelerate framework (for matrix ops)
- ARM NEON (optional, for SIMD ReLU/Softmax)
- macOS with clang++
(Linux is possible but requires BLAS and NEON modifications)

⸻
### Usage

Place MNIST files in the data/ directory:

Compile using:
```
./build.sh
```     
Then run
``` 
./build/mnist
```
<hr/>

### License

MIT License.
