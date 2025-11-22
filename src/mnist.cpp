#include "dlarray.cpp"
#include "terminal_plot.h"
#include <arm_neon.h> // for NEON intrinsics on Apple Silicon (M1/M2)
#include <iostream>
#include <sstream>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMAGES_PATH "data/t10k-images.idx3-ubyte"
#define LABELS_PATH "data/t10k-labels.idx1-ubyte"

struct MNIST_DATA {
  unsigned int magic_number;
  unsigned int size;
  unsigned int rows;
  unsigned int columns;
  unsigned char *data;
  unsigned char *labels;
} __attribute__((packed)); // Pack the structure to avoid padding to make it
                           // easier to read from the file

// read images and labels from the file
// should move those to header file
#define MAGIC_NUMBER_IMAGES 2051
#define MAGIC_NUMBER_LABELS 2049

void read_images(const char *images_path, const char *labels_path,
                 struct MNIST_DATA *mnist_data) {
  FILE *fptr_images = fopen(images_path, "rb");
  if (fptr_images == NULL) {
    printf("Error opening file\n");
    exit(1);
  }

  // Read the magic number
  unsigned char magic[4];
  fread(magic, sizeof(unsigned char), 4, fptr_images);
  int magic_number = ((int)magic[0] << 24) + ((int)magic[1] << 16) +
                     ((int)magic[2] << 8) + (int)magic[3];
  mnist_data->magic_number = magic_number;
  unsigned char size[4];
  fread(size, sizeof(unsigned char), 4, fptr_images);
  int size_number = ((int)size[0] << 24) + ((int)size[1] << 16) +
                    ((int)size[2] << 8) + (int)size[3];
  mnist_data->size = size_number;

  unsigned char rows[4];
  fread(rows, sizeof(unsigned char), 4, fptr_images);
  int rows_number = ((int)rows[0] << 24) + ((int)rows[1] << 16) +
                    ((int)rows[2] << 8) + (int)rows[3];
  mnist_data->rows = rows_number;

  unsigned char columns[4];
  fread(columns, sizeof(unsigned char), 4, fptr_images);
  int columns_number = ((int)columns[0] << 24) + ((int)columns[1] << 16) +
                       ((int)columns[2] << 8) + (int)columns[3];
  mnist_data->columns = columns_number;

  mnist_data->data =
      (unsigned char *)malloc(mnist_data->size * mnist_data->rows *
                              mnist_data->columns * sizeof(unsigned char));
  fread(mnist_data->data, sizeof(unsigned char),
        mnist_data->size * mnist_data->rows * mnist_data->columns, fptr_images);

  fclose(fptr_images);

  FILE *fptr_labels = fopen(labels_path, "rb");
  if (fptr_labels == NULL) {
    printf("Error opening file\n");
    exit(1);
  }

  // Read the magic number
  unsigned char magic_labels[4];
  fread(magic_labels, sizeof(unsigned char), 4, fptr_labels);
  int magic_number_labels = ((int)magic_labels[0] << 24) +
                            ((int)magic_labels[1] << 16) +
                            ((int)magic_labels[2] << 8) + (int)magic_labels[3];
  if (magic_number_labels != 2049) {
    printf("\n\nMagic number is not 2049\n");
    exit(1);
  }

  unsigned char size_labels[4];
  fread(size_labels, sizeof(unsigned char), 4, fptr_labels);
  int size_number_labels = ((int)size_labels[0] << 24) +
                           ((int)size_labels[1] << 16) +
                           ((int)size_labels[2] << 8) + (int)size_labels[3];
  mnist_data->size = size_number_labels;

  mnist_data->labels =
      (unsigned char *)malloc(size_number_labels * sizeof(unsigned char));
  fread(mnist_data->labels, sizeof(unsigned char), size_number_labels,
        fptr_labels);

  fclose(fptr_labels);
}

void print_images(struct MNIST_DATA *mnist_data, int index) {
  printf("Image %d:\n", index);
  for (int i = 0; i < mnist_data->rows; i++) {
    for (int j = 0; j < mnist_data->columns; j++) {
      unsigned char pixel =
          mnist_data->data[index * mnist_data->rows * mnist_data->columns +
                           i * mnist_data->columns + j];
      if (pixel == 0) {
        printf(" .");
      } else {
        printf(" *");
      }
    }
    printf("\n");
  }
  printf("Label: %d\n", mnist_data->labels[index]);
}

int main() {
  struct MNIST_DATA mnist_data;
  read_images(IMAGES_PATH, LABELS_PATH, &mnist_data);
  DLArray Images;
  DLArray Labels;
  size_t img_idx = 0;

  // Copy data from raw pointers (unsigned char arrays)
  Images.copy_from<unsigned char>(
      mnist_data.data, {mnist_data.size, mnist_data.rows, mnist_data.columns});
  Labels.copy_from<unsigned char>(mnist_data.labels, {mnist_data.size});

  // Now normalize the Images (which are already floats)
  for (size_t i = 0; i < Images.size; ++i) {
      Images.data[i] = Images.data[i] / 255.0f;
  }
  // Print dataset info
  std::cout << "Images shape: ";
  Images.print_shape();
  std::cout << "Labels shape: ";
  Labels.print_shape();
  std::cout << "\nDataset loaded: " << mnist_data.size << " images"
            << std::endl;
  std::cout << "Image size: " << mnist_data.rows << "x" << mnist_data.columns
            << std::endl;

  Labels.slice(img_idx).print();

  TerminalPlot plot(40, 30);
  plot.set_color_theme("bright");
  plot.set_num_ticks_x(28);
  plot.set_num_ticks_y(28);

  DLArray current_image = Images.slice(img_idx);
  std::vector<float> image_data = current_image.to_vector();
  std::ostringstream title;
  title << "MNIST Image #" << img_idx
        << " | Label: " << (int)Labels.data[img_idx];
  plot.set_title(title.str());
  plot.show_image(image_data, mnist_data.rows, mnist_data.columns, 1);
  plot.show();

  DLArray w1;
  w1.he_init({28*28, 128}, 784);  // fan_in = 784
  
  DLArray b1;
  b1.zeros({1, 128});  // Initialize biases to zero
  
  DLArray w2;
  w2.he_init({128, 128}, 128);  // fan_in = 128
  
  DLArray b2;
  b2.zeros({1, 128});
  
  DLArray w3;
  w3.he_init({128, 10}, 128);  // fan_in = 128
  
  DLArray b3;
  b3.zeros({1, 10});

  DLArray out_one_hot;
  out_one_hot.zeros({10, 10});
  for (size_t i = 0; i < 10; ++i) {
    out_one_hot.data[i * 10 + (int)i] = 1.0f;
  }

  float loss = 0.0f;
  float learning_rate = 0.01f;

  for (size_t i = 0; i < 200; ++i) {
    DLArray x = Images.slice(i);
    DLArray y = Labels.slice(i);
    x.reshape({1, 784});
    DLArray f1 = x.matmul(w1).add(b1);
    DLArray f2 = f1.matmul(w2).add(b2).relu();
    DLArray output = f2.matmul(w3).add(b3).softmax();
    DLArray exp = out_one_hot.slice(y.data[0]);

    // L = -\sum_i y_i \log p_i
    // Since exp is one-hot, this simplifies to: -log(p_correct_class)
    float p_correct = ((float *)output.data)[(int)y.data[0]];
    float loss = -std::log(p_correct + 1e-8f); // epsilon to prevent log(0)

    // backward
    DLArray grad;
    grad.from(output);
    ((float *)grad.data)[(int)y.data[0]] -= 1.0f;
    DLArray dl_df2 = grad.matmul(w3.transpose());
    DLArray dl_dw3 = f2.transpose().matmul(grad);
    DLArray dl_db3 = grad;

    // Backprop through ReLU - ADD THIS HERE
    DLArray dl_df2_relu = dl_df2;
    for (size_t j = 0; j < dl_df2_relu.size; ++j) {
      if (f2.data[j] <= 0) {
        dl_df2_relu.data[j] = 0;
      }
    }
    // print shapes of all
    DLArray dl_df1 = dl_df2_relu.matmul(w2.transpose());
    DLArray dl_dw2 = f1.transpose().matmul(dl_df2_relu);
    DLArray dl_db2 = dl_df2_relu;

    DLArray dl_dw1 = x.transpose().matmul(dl_df1); 
    DLArray dl_db1 = dl_df1;

    // Update weights and biases using gradient descent

    // Update layer 1
    for (size_t j = 0; j < w1.size; ++j) {
      w1.data[j] -= learning_rate * dl_dw1.data[j];
    }
    for (size_t j = 0; j < b1.size; ++j) {
      b1.data[j] -= learning_rate * dl_db1.data[j];
    }

    // Update layer 2
    for (size_t j = 0; j < w2.size; ++j) {
      w2.data[j] -= learning_rate * dl_dw2.data[j];
    }
    for (size_t j = 0; j < b2.size; ++j) {
      b2.data[j] -= learning_rate * dl_db2.data[j];
    }

    // Update layer 3
    for (size_t j = 0; j < w3.size; ++j) {
      w3.data[j] -= learning_rate * dl_dw3.data[j];
    }
    for (size_t j = 0; j < b3.size; ++j) {
      b3.data[j] -= learning_rate * dl_db3.data[j];
    }

    if (i % 100 == 0) {
      printf("Iteration %zu, Loss: %.4f\n", i, loss);
    }
  }
}
