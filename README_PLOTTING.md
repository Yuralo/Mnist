# Terminal Plotting Library

A comprehensive, feature-rich terminal-based plotting library for C++ with full color support.

## Features

- **Multiple Plot Types**: Scatter, Line, Bar, Histogram, Pie, Box, Area, Heatmap, 3D, and Image visualization
- **Full Color Support**: ANSI color codes with multiple themes (default, bright, pastel, monochrome)
- **Multi-Series Support**: Plot multiple datasets on the same graph with different colors
- **Legends**: Automatic legend generation with matching colors
- **Axis Labels**: Customizable X/Y axis labels with numeric tick marks
- **Titles**: Plot titles with bold formatting
- **Subplots**: Support for creating subplot layouts
- **Image Display**: Display grayscale and RGB images from arrays

## Compilation

To compile the example program:

```bash
g++ -std=c++17 terminal_plot.cpp plot_example.cpp -o plot_example
```

## Running

To run the example program that demonstrates all features:

```bash
./plot_example
```

## Quick Start

```cpp
#include "terminal_plot.h"

TerminalPlot plot(100, 30);

// Set color theme
plot.set_color_theme("bright");

// Configure axes
plot.set_num_ticks_x(10);
plot.set_num_ticks_y(8);

// Create a plot
plot.set_title("My Plot");
plot.set_xlabel("X Axis");
plot.set_ylabel("Y Axis");
plot.scatter(x_data, y_data);
plot.show();
```

## Color Themes

- `"default"` or `"bright"` - Vibrant, bright colors (default)
- `"pastel"` - Softer, lighter colors
- `"monochrome"` - Grayscale only

## Example Usage

See `plot_example.cpp` for comprehensive examples of all features.

