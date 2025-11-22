/**
 * TerminalPlot - A comprehensive terminal-based plotting library for C++
 * 
 * This library provides various types of plots that can be rendered in the terminal:
 * - Scatter plots
 * - Line plots
 * - Bar charts
 * - Histograms
 * - Pie charts
 * - Box plots
 * - Area charts
 * - Heatmaps
 * - 3D plots
 * - Image visualization (grayscale and RGB)
 * - Subplot support
 * 
 * Usage example:
 *   TerminalPlot plot(100, 30);
 *   plot.set_title("My Plot");
 *   plot.scatter(x_data, y_data);
 *   plot.show();
 * 
 * All plotting functions accept either DLArray or std::vector<float> for data input.
 */

#ifndef TERMINAL_PLOT_H
#define TERMINAL_PLOT_H

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <map>
#include <numeric>
#include "dlarray.cpp"

class TerminalPlot {
public:
    // Constructor
    TerminalPlot(size_t width = 80, size_t height = 24);
    
    // Basic plotting functions
    void scatter(const DLArray& x, const DLArray& y, const std::string& symbol = "*");
    void scatter(const std::vector<float>& x, const std::vector<float>& y, const std::string& symbol = "*");
    
    void line_plot(const DLArray& x, const DLArray& y, const std::string& line_char = "-");
    void line_plot(const std::vector<float>& x, const std::vector<float>& y, const std::string& line_char = "-");
    
    void bar_chart(const DLArray& values, const std::vector<std::string>& labels = {});
    void bar_chart(const std::vector<float>& values, const std::vector<std::string>& labels = {});
    
    void histogram(const DLArray& data, size_t bins = 20);
    void histogram(const std::vector<float>& data, size_t bins = 20);
    
    void pie_chart(const DLArray& values, const std::vector<std::string>& labels = {});
    void pie_chart(const std::vector<float>& values, const std::vector<std::string>& labels = {});
    
    void box_plot(const DLArray& data);
    void box_plot(const std::vector<float>& data);
    
    void area_chart(const DLArray& x, const DLArray& y, const std::string& fill_char = "#");
    void area_chart(const std::vector<float>& x, const std::vector<float>& y, const std::string& fill_char = "#");
    
    void heatmap(const DLArray& data, size_t rows, size_t cols);
    void heatmap(const std::vector<float>& data, size_t rows, size_t cols);
    
    // 3D plot
    void plot3d(const DLArray& x, const DLArray& y, const DLArray& z, const std::string& symbol = "*");
    
    // Image visualization
    void show_image(const DLArray& image_data, size_t height, size_t width, size_t channels = 1);
    void show_image(const std::vector<float>& image_data, size_t height, size_t width, size_t channels = 1);
    
    // Subplot functionality
    void subplot(size_t rows, size_t cols, size_t index);
    void subplot_clear();
    
    // Display the plot
    void show();
    void clear();
    
    // Get/Set dimensions
    void set_size(size_t width, size_t height);
    size_t get_width() const { return width_; }
    size_t get_height() const { return height_; }
    
    // Utilities
    void set_title(const std::string& title);
    void set_xlabel(const std::string& label);
    void set_ylabel(const std::string& label);
    
    // Legend and advanced labeling
    void add_legend(const std::vector<std::string>& labels, const std::vector<std::string>& symbols = {});
    void clear_legend();
    void set_show_axis_labels(bool show);  // Show numeric tick labels on axes
    void set_num_ticks_x(size_t num);  // Number of ticks on X axis
    void set_num_ticks_y(size_t num);  // Number of ticks on Y axis
    
    // Color and theme support
    void enable_colors(bool enable = true);
    void set_color_theme(const std::string& theme);  // "default", "bright", "pastel", "monochrome"
    std::string colorize(const std::string& text, int color_code, bool bold = false) const;
    std::string get_color_code(int color) const;
    
    // Multiple series support with colors
    void scatter_multi(const std::vector<std::vector<float>>& x_data, 
                       const std::vector<std::vector<float>>& y_data,
                       const std::vector<std::string>& labels = {});
    void line_plot_multi(const std::vector<std::vector<float>>& x_data,
                         const std::vector<std::vector<float>>& y_data,
                         const std::vector<std::string>& labels = {});

private:
    size_t width_;
    size_t height_;
    std::vector<std::vector<std::string>> canvas_;
    std::vector<std::vector<bool>> plot_area_;
    
    // Plot area boundaries
    size_t plot_left_;
    size_t plot_right_;
    size_t plot_top_;
    size_t plot_bottom_;
    
    // Labels
    std::string title_;
    std::string xlabel_;
    std::string ylabel_;
    
    // Legend
    std::vector<std::string> legend_labels_;
    std::vector<std::string> legend_symbols_;
    
    // Axis display options
    bool show_axis_labels_;
    size_t num_ticks_x_;
    size_t num_ticks_y_;
    
    // Color and theme options
    bool use_colors_;
    std::string color_theme_;
    std::vector<int> color_palette_;
    
    // Subplot state
    bool subplot_mode_;
    size_t subplot_rows_;
    size_t subplot_cols_;
    size_t subplot_current_;
    
    // Internal helper functions
    void initialize_canvas();
    void draw_axes(float x_min, float x_max, float y_min, float y_max);
    void draw_border();
    void draw_labels();
    void draw_legend();
    void draw_axis_ticks(float x_min, float x_max, float y_min, float y_max);
    void set_pixel(size_t x, size_t y, const std::string& ch, int color_code = -1, bool bold = false);
    void set_pixel_rgb(size_t x, size_t y, const std::string& ch, int r, int g, int b);
    std::string get_pixel(size_t x, size_t y) const;
    void initialize_color_palette();
    int get_series_color(size_t index) const;
    std::string rgb_colorize(const std::string& text, int r, int g, int b) const;
    
    // Data normalization
    float normalize_x(float x, float x_min, float x_max) const;
    float normalize_y(float y, float y_min, float y_max) const;
    size_t to_screen_x(float x_norm) const;
    size_t to_screen_y(float y_norm) const;
    
    // Statistics
    std::pair<float, float> get_minmax(const DLArray& arr) const;
    std::pair<float, float> get_minmax(const std::vector<float>& arr) const;
    float get_quantile(const std::vector<float>& sorted_data, float quantile) const;
    
    // Convert DLArray to vector (helper)
    std::vector<float> to_vector(const DLArray& arr) const;
};

#endif // TERMINAL_PLOT_H

