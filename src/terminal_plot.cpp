#include "terminal_plot.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <limits>
#include <cstring>
#include <cstdlib>

// Gradient characters for different visualizations
const char* GRAYSCALE_CHARS = " .:-=+*#%@";
const char* HEATMAP_CHARS = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";

TerminalPlot::TerminalPlot(size_t width, size_t height) 
    : width_(width), height_(height), 
      plot_left_(5), plot_right_(width - 5), 
      plot_top_(3), plot_bottom_(height - 3),
      subplot_mode_(false), subplot_rows_(1), subplot_cols_(1), subplot_current_(0),
      show_axis_labels_(true), num_ticks_x_(5), num_ticks_y_(5),
      use_colors_(true), color_theme_("default") {
    initialize_canvas();
    initialize_color_palette();
}

void TerminalPlot::initialize_canvas() {
    canvas_.clear();
    canvas_.resize(height_);
    plot_area_.clear();
    plot_area_.resize(height_);
    for (size_t i = 0; i < height_; ++i) {
        canvas_[i].resize(width_, " ");
        plot_area_[i].resize(width_, false);
    }
}

void TerminalPlot::clear() {
    initialize_canvas();
    title_.clear();
    xlabel_.clear();
    ylabel_.clear();
    legend_labels_.clear();
    legend_symbols_.clear();
    subplot_mode_ = false;
}

void TerminalPlot::set_size(size_t width, size_t height) {
    width_ = width;
    height_ = height;
    plot_left_ = 5;
    plot_right_ = width - 5;
    plot_top_ = 3;
    plot_bottom_ = height - 3;
    initialize_canvas();
}

void TerminalPlot::set_title(const std::string& title) {
    title_ = title;
}

void TerminalPlot::set_xlabel(const std::string& label) {
    xlabel_ = label;
}

void TerminalPlot::set_ylabel(const std::string& label) {
    ylabel_ = label;
}

void TerminalPlot::add_legend(const std::vector<std::string>& labels, const std::vector<std::string>& symbols) {
    legend_labels_ = labels;
    if (!symbols.empty() && symbols.size() == labels.size()) {
        legend_symbols_ = symbols;
    } else {
        // Default symbols
        legend_symbols_.clear();
        const std::string default_symbols = "*+xo.^v<>s";
        for (size_t i = 0; i < labels.size(); ++i) {
            legend_symbols_.push_back(std::string(1, default_symbols[i % default_symbols.length()]));
        }
    }
}

void TerminalPlot::clear_legend() {
    legend_labels_.clear();
    legend_symbols_.clear();
}

void TerminalPlot::set_show_axis_labels(bool show) {
    show_axis_labels_ = show;
}

void TerminalPlot::set_num_ticks_x(size_t num) {
    num_ticks_x_ = num;
}

void TerminalPlot::set_num_ticks_y(size_t num) {
    num_ticks_y_ = num;
}

void TerminalPlot::set_pixel(size_t x, size_t y, const std::string& ch, int color_code, bool bold) {
    if (x < width_ && y < height_) {
        if (use_colors_ && color_code >= 0) {
            canvas_[y][x] = colorize(ch, color_code, bold);
        } else {
            canvas_[y][x] = ch;
        }
        plot_area_[y][x] = true;
    }
}

void TerminalPlot::enable_colors(bool enable) {
    use_colors_ = enable;
}

void TerminalPlot::set_color_theme(const std::string& theme) {
    color_theme_ = theme;
    initialize_color_palette();
}

std::string TerminalPlot::colorize(const std::string& text, int color_code, bool bold) const {
    if (!use_colors_ || color_code < 0) return text;
    
    std::ostringstream oss;
    if (bold) {
        oss << "\033[1;" << color_code << "m" << text << "\033[0m";
    } else {
        oss << "\033[" << color_code << "m" << text << "\033[0m";
    }
    return oss.str();
}

std::string TerminalPlot::get_color_code(int color) const {
    // ANSI color codes: 30-37 (foreground), 90-97 (bright foreground)
    std::ostringstream oss;
    oss << "\033[" << color << "m";
    return oss.str();
}

std::string TerminalPlot::rgb_colorize(const std::string& text, int r, int g, int b) const {
    if (!use_colors_) return text;
    
    // Clamp RGB values to 0-255
    r = std::max(0, std::min(255, r));
    g = std::max(0, std::min(255, g));
    b = std::max(0, std::min(255, b));
    
    // ANSI 24-bit color code: \033[38;2;R;G;Bm (foreground) or \033[48;2;R;G;Bm (background)
    std::ostringstream oss;
    oss << "\033[38;2;" << r << ";" << g << ";" << b << "m" << text << "\033[0m";
    return oss.str();
}

void TerminalPlot::set_pixel_rgb(size_t x, size_t y, const std::string& ch, int r, int g, int b) {
    if (x < width_ && y < height_) {
        if (use_colors_) {
            canvas_[y][x] = rgb_colorize(ch, r, g, b);
        } else {
            canvas_[y][x] = ch;
        }
        plot_area_[y][x] = true;
    }
}

void TerminalPlot::initialize_color_palette() {
    color_palette_.clear();
    
    if (color_theme_ == "bright" || color_theme_ == "default") {
        // Bright, vibrant colors
        color_palette_ = {31, 32, 33, 34, 35, 36, 91, 92, 93, 94, 95, 96};  // Red, Green, Yellow, Blue, Magenta, Cyan, etc.
    } else if (color_theme_ == "pastel") {
        // Softer colors (using bright but lighter tones)
        color_palette_ = {91, 92, 93, 94, 95, 96, 97, 33, 36, 35, 32, 31};
    } else if (color_theme_ == "monochrome") {
        // Grayscale
        color_palette_ = {37, 90, 37, 90, 37, 90, 37, 90, 37, 90, 37, 90};
    } else {
        // Default to bright
        color_palette_ = {31, 32, 33, 34, 35, 36, 91, 92, 93, 94, 95, 96};
    }
}

int TerminalPlot::get_series_color(size_t index) const {
    if (color_palette_.empty()) return 37;  // Default white
    return color_palette_[index % color_palette_.size()];
}

std::string TerminalPlot::get_pixel(size_t x, size_t y) const {
    if (x < width_ && y < height_) {
        return canvas_[y][x];
    }
    return " ";
}

void TerminalPlot::draw_border() {
    // Unicode box-drawing characters with color
    const std::string h_line = "\u2500";  // ─
    const std::string v_line = "\u2502";  // │
    const std::string tl_corner = "\u250C"; // ┌
    const std::string tr_corner = "\u2510"; // ┐
    const std::string bl_corner = "\u2514"; // └
    const std::string br_corner = "\u2518"; // ┘
    
    int border_color = use_colors_ ? 37 : -1;  // White/gray border
    
    // Top border
    for (size_t x = plot_left_; x < plot_right_; ++x) {
        set_pixel(x, plot_top_, h_line, border_color);
    }
    // Bottom border
    for (size_t x = plot_left_; x < plot_right_; ++x) {
        set_pixel(x, plot_bottom_, h_line, border_color);
    }
    // Left border
    for (size_t y = plot_top_; y <= plot_bottom_; ++y) {
        set_pixel(plot_left_, y, v_line, border_color);
    }
    // Right border
    for (size_t y = plot_top_; y <= plot_bottom_; ++y) {
        set_pixel(plot_right_, y, v_line, border_color);
    }
    // Corners
    set_pixel(plot_left_, plot_top_, tl_corner, border_color);
    set_pixel(plot_right_, plot_top_, tr_corner, border_color);
    set_pixel(plot_left_, plot_bottom_, bl_corner, border_color);
    set_pixel(plot_right_, plot_bottom_, br_corner, border_color);
}

void TerminalPlot::draw_axes(float x_min, float x_max, float y_min, float y_max) {
    // Unicode box-drawing characters with color
    const std::string h_line = "\u2500";  // ─
    const std::string v_line = "\u2502";  // │
    const std::string cross = "\u253C";    // ┼
    
    int axis_color = use_colors_ ? 90 : -1;  // Dark gray for axes
    
    // Draw X axis (at y = 0 if in range, otherwise at bottom)
    float x_axis_y = (y_min <= 0 && y_max >= 0) ? 0.0f : y_min;
    size_t screen_y = to_screen_y(normalize_y(x_axis_y, y_min, y_max));
    
    for (size_t x = plot_left_ + 1; x < plot_right_; ++x) {
        set_pixel(x, screen_y, h_line, axis_color);
    }
    
    // Draw Y axis (at x = 0 if in range, otherwise at left)
    float y_axis_x = (x_min <= 0 && x_max >= 0) ? 0.0f : x_min;
    size_t screen_x = to_screen_x(normalize_x(y_axis_x, x_min, x_max));
    
    for (size_t y = plot_top_ + 1; y < plot_bottom_; ++y) {
        set_pixel(screen_x, y, v_line, axis_color);
    }
    
    // Axis intersection
    set_pixel(screen_x, screen_y, cross, axis_color);
    
    // Draw axis ticks and labels
    if (show_axis_labels_) {
        draw_axis_ticks(x_min, x_max, y_min, y_max);
    }
}

void TerminalPlot::draw_labels() {
    int title_color = use_colors_ ? 1 : -1;  // Bold for title
    int label_color = use_colors_ ? 37 : -1;  // White for labels
    
    // Title (bold and colored)
    if (!title_.empty()) {
        size_t title_x = (width_ - title_.length()) / 2;
        for (size_t i = 0; i < title_.length() && title_x + i < width_; ++i) {
            std::string char_str = std::string(1, title_[i]);
            if (use_colors_) {
                char_str = colorize(char_str, 1, true);  // Bold white
            }
            set_pixel(title_x + i, 0, char_str);
        }
    }
    
    // X label
    if (!xlabel_.empty() && plot_bottom_ + 2 < height_) {
        size_t xlabel_x = plot_left_ + (plot_right_ - plot_left_ - xlabel_.length()) / 2;
        for (size_t i = 0; i < xlabel_.length() && xlabel_x + i < width_; ++i) {
            std::string char_str = std::string(1, xlabel_[i]);
            if (use_colors_) {
                char_str = colorize(char_str, label_color);
            }
            set_pixel(xlabel_x + i, plot_bottom_ + 2, char_str);
        }
    }
    
    // Y label (vertical, rotated 90 degrees conceptually - shown horizontally for simplicity)
    if (!ylabel_.empty() && plot_left_ >= ylabel_.length()) {
        for (size_t i = 0; i < ylabel_.length(); ++i) {
            std::string char_str = std::string(1, ylabel_[i]);
            if (use_colors_) {
                char_str = colorize(char_str, label_color);
            }
            set_pixel(i, plot_top_ + (plot_bottom_ - plot_top_) / 2, char_str);
        }
    }
    
    // Draw legend
    draw_legend();
}

void TerminalPlot::draw_legend() {
    if (legend_labels_.empty()) return;
    
    size_t legend_x = plot_right_ + 2;
    size_t legend_y = plot_top_ + 1;
    
    // Check if there's enough space
    if (legend_x + 20 >= width_) return;
    
    size_t max_label_len = 0;
    for (const auto& label : legend_labels_) {
        if (label.length() > max_label_len) max_label_len = label.length();
    }
    
    if (legend_x + max_label_len + 5 >= width_) return;
    
    for (size_t i = 0; i < legend_labels_.size(); ++i) {
        if (legend_y + i >= plot_bottom_ || legend_y + i >= height_ - 2) break;
        
        // Draw symbol with color
        std::string symbol = (i < legend_symbols_.size()) ? legend_symbols_[i] : "*";
        int symbol_color = get_series_color(i);
        set_pixel(legend_x, legend_y + i, symbol, symbol_color, true);
        
        // Draw label with same color
        size_t label_start = legend_x + 2;
        int label_color = use_colors_ ? 37 : -1;
        for (size_t j = 0; j < legend_labels_[i].length() && label_start + j < width_; ++j) {
            std::string char_str = std::string(1, legend_labels_[i][j]);
            if (use_colors_) {
                char_str = colorize(char_str, label_color);
            }
            set_pixel(label_start + j, legend_y + i, char_str);
        }
    }
}

void TerminalPlot::draw_axis_ticks(float x_min, float x_max, float y_min, float y_max) {
    // Draw X axis ticks
    float x_axis_y = (y_min <= 0 && y_max >= 0) ? 0.0f : y_min;
    size_t screen_y = to_screen_y(normalize_y(x_axis_y, y_min, y_max));
    
    if (num_ticks_x_ > 0) {
        for (size_t i = 0; i <= num_ticks_x_; ++i) {
            float tick_value = x_min + (x_max - x_min) * static_cast<float>(i) / static_cast<float>(num_ticks_x_);
            size_t tick_x = to_screen_x(normalize_x(tick_value, x_min, x_max));
            
            // Draw tick mark
            if (screen_y > 0 && screen_y < height_) {
                int tick_color = use_colors_ ? 90 : -1;
                set_pixel(tick_x, screen_y, "+", tick_color);
            }
            
            // Draw tick label below axis
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << tick_value;
            std::string label_str = oss.str();
            size_t label_y = screen_y + 1;
            
            if (label_y < height_ && tick_x + label_str.length() < width_) {
                size_t label_start = tick_x - label_str.length() / 2;
                int tick_label_color = use_colors_ ? 37 : -1;
                for (size_t j = 0; j < label_str.length() && label_start + j < width_; ++j) {
                    if (label_start + j >= plot_left_ && label_start + j <= plot_right_ + 5) {
                        std::string char_str = std::string(1, label_str[j]);
                        if (use_colors_) {
                            char_str = colorize(char_str, tick_label_color);
                        }
                        set_pixel(label_start + j, label_y, char_str);
                    }
                }
            }
        }
    }
    
    // Draw Y axis ticks
    float y_axis_x = (x_min <= 0 && x_max >= 0) ? 0.0f : x_min;
    size_t screen_x = to_screen_x(normalize_x(y_axis_x, x_min, x_max));
    
    if (num_ticks_y_ > 0) {
        for (size_t i = 0; i <= num_ticks_y_; ++i) {
            float tick_value = y_min + (y_max - y_min) * static_cast<float>(i) / static_cast<float>(num_ticks_y_);
            size_t tick_y = to_screen_y(normalize_y(tick_value, y_min, y_max));
            
            // Draw tick mark
            if (screen_x > 0 && screen_x < width_) {
                int tick_color = use_colors_ ? 90 : -1;
                set_pixel(screen_x, tick_y, "+", tick_color);
            }
            
            // Draw tick label to the left of axis
            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << tick_value;
            std::string label_str = oss.str();
            
            if (screen_x >= label_str.length() + 1) {
                size_t label_start = screen_x - label_str.length() - 1;
                int tick_label_color = use_colors_ ? 37 : -1;
                for (size_t j = 0; j < label_str.length(); ++j) {
                    if (label_start + j < plot_left_) {
                        std::string char_str = std::string(1, label_str[j]);
                        if (use_colors_) {
                            char_str = colorize(char_str, tick_label_color);
                        }
                        set_pixel(label_start + j, tick_y, char_str);
                    }
                }
            }
        }
    }
}

float TerminalPlot::normalize_x(float x, float x_min, float x_max) const {
    if (x_max == x_min) return 0.5f;
    return (x - x_min) / (x_max - x_min);
}

float TerminalPlot::normalize_y(float y, float y_min, float y_max) const {
    if (y_max == y_min) return 0.5f;
    return (y - y_min) / (y_max - y_min);
}

size_t TerminalPlot::to_screen_x(float x_norm) const {
    size_t plot_width = plot_right_ - plot_left_ - 1;
    return plot_left_ + 1 + static_cast<size_t>(x_norm * plot_width);
}

size_t TerminalPlot::to_screen_y(float y_norm) const {
    size_t plot_height = plot_bottom_ - plot_top_ - 1;
    // Invert Y because screen coordinates start at top
    return plot_top_ + 1 + static_cast<size_t>((1.0f - y_norm) * plot_height);
}

std::pair<float, float> TerminalPlot::get_minmax(const DLArray& arr) const {
    float min_val = std::numeric_limits<float>::max();
    float max_val = std::numeric_limits<float>::lowest();
    
    for (size_t i = 0; i < arr.size; ++i) {
        if (arr.data[i] < min_val) min_val = arr.data[i];
        if (arr.data[i] > max_val) max_val = arr.data[i];
    }
    
    return {min_val, max_val};
}

std::pair<float, float> TerminalPlot::get_minmax(const std::vector<float>& arr) const {
    if (arr.empty()) return {0.0f, 0.0f};
    auto [min_it, max_it] = std::minmax_element(arr.begin(), arr.end());
    return {*min_it, *max_it};
}

std::vector<float> TerminalPlot::to_vector(const DLArray& arr) const {
    std::vector<float> result(arr.size);
    for (size_t i = 0; i < arr.size; ++i) {
        result[i] = arr.data[i];
    }
    return result;
}

float TerminalPlot::get_quantile(const std::vector<float>& sorted_data, float quantile) const {
    if (sorted_data.empty()) return 0.0f;
    float index = quantile * (sorted_data.size() - 1);
    size_t lower = static_cast<size_t>(std::floor(index));
    size_t upper = static_cast<size_t>(std::ceil(index));
    
    if (lower == upper) return sorted_data[lower];
    float weight = index - lower;
    return sorted_data[lower] * (1.0f - weight) + sorted_data[upper] * weight;
}

void TerminalPlot::scatter(const DLArray& x, const DLArray& y, const std::string& symbol) {
    scatter(to_vector(x), to_vector(y), symbol);
}

void TerminalPlot::scatter(const std::vector<float>& x, const std::vector<float>& y, const std::string& symbol) {
    if (x.size() != y.size() || x.empty()) return;
    
    clear();
    auto [x_min, x_max] = get_minmax(x);
    auto [y_min, y_max] = get_minmax(y);
    
    // Add padding
    float x_range = x_max - x_min;
    float y_range = y_max - y_min;
    x_min -= x_range * 0.1f;
    x_max += x_range * 0.1f;
    y_min -= y_range * 0.1f;
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(x_min, x_max, y_min, y_max);
    
    // Use colored symbols for scatter plot
    int plot_color = get_series_color(0);
    for (size_t i = 0; i < x.size(); ++i) {
        size_t sx = to_screen_x(normalize_x(x[i], x_min, x_max));
        size_t sy = to_screen_y(normalize_y(y[i], y_min, y_max));
        set_pixel(sx, sy, symbol, plot_color, true);
    }
    
    draw_labels();
}

void TerminalPlot::line_plot(const DLArray& x, const DLArray& y, const std::string& line_char) {
    line_plot(to_vector(x), to_vector(y), line_char);
}

void TerminalPlot::line_plot(const std::vector<float>& x, const std::vector<float>& y, const std::string& line_char) {
    if (x.size() != y.size() || x.empty()) return;
    
    clear();
    auto [x_min, x_max] = get_minmax(x);
    auto [y_min, y_max] = get_minmax(y);
    
    float x_range = x_max - x_min;
    float y_range = y_max - y_min;
    x_min -= x_range * 0.1f;
    x_max += x_range * 0.1f;
    y_min -= y_range * 0.1f;
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(x_min, x_max, y_min, y_max);
    
    for (size_t i = 0; i < x.size() - 1; ++i) {
        size_t x1 = to_screen_x(normalize_x(x[i], x_min, x_max));
        size_t y1 = to_screen_y(normalize_y(y[i], y_min, y_max));
        size_t x2 = to_screen_x(normalize_x(x[i+1], x_min, x_max));
        size_t y2 = to_screen_y(normalize_y(y[i+1], y_min, y_max));
        
        // Bresenham's line algorithm
        int dx = static_cast<int>(x2) - static_cast<int>(x1);
        int dy = static_cast<int>(y2) - static_cast<int>(y1);
        int steps = std::max(std::abs(dx), std::abs(dy));
        
        if (steps > 0) {
            float x_step = static_cast<float>(dx) / steps;
            float y_step = static_cast<float>(dy) / steps;
            
            // Use colored line
            int line_color = get_series_color(0);
            for (int j = 0; j <= steps; ++j) {
                size_t px = static_cast<size_t>(x1 + j * x_step);
                size_t py = static_cast<size_t>(y1 + j * y_step);
                set_pixel(px, py, line_char, line_color);
            }
        }
    }
    
    draw_labels();
}

void TerminalPlot::bar_chart(const DLArray& values, const std::vector<std::string>& labels) {
    bar_chart(to_vector(values), labels);
}

void TerminalPlot::bar_chart(const std::vector<float>& values, const std::vector<std::string>& labels) {
    if (values.empty()) return;
    
    clear();
    auto [y_min, y_max] = get_minmax(values);
    y_min = std::min(0.0f, y_min);
    float y_range = y_max - y_min;
    y_min -= y_range * 0.1f;
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(0.0f, static_cast<float>(values.size()), y_min, y_max);
    
    size_t bar_width = (plot_right_ - plot_left_ - 1) / (values.size() * 2);
    if (bar_width < 1) bar_width = 1;
    
    for (size_t i = 0; i < values.size(); ++i) {
        size_t bar_x = to_screen_x(normalize_x(static_cast<float>(i) + 0.5f, 0.0f, static_cast<float>(values.size())));
        size_t bar_y_top = to_screen_y(normalize_y(0.0f, y_min, y_max));
        size_t bar_y_bottom = to_screen_y(normalize_y(values[i], y_min, y_max));
        
        if (bar_y_top > bar_y_bottom) std::swap(bar_y_top, bar_y_bottom);
        
        // Use gradient colors for bars
        int bar_color = get_series_color(i % 12);
        for (size_t j = 0; j < bar_width && bar_x + j < plot_right_; ++j) {
            for (size_t y = bar_y_top; y <= bar_y_bottom && y < plot_bottom_; ++y) {
                set_pixel(bar_x + j, y, "\u2588", bar_color);
            }
        }
        
        // Label
        if (i < labels.size() && plot_bottom_ + 1 < height_) {
            size_t label_x = bar_x;
            for (size_t k = 0; k < labels[i].length() && label_x + k < width_; ++k) {
                set_pixel(label_x + k, plot_bottom_ + 1, std::string(1, labels[i][k]));
            }
        }
    }
    
    draw_labels();
}

void TerminalPlot::histogram(const DLArray& data, size_t bins) {
    histogram(to_vector(data), bins);
}

void TerminalPlot::histogram(const std::vector<float>& data, size_t bins) {
    if (data.empty() || bins == 0) return;
    
    clear();
    auto [min_val, max_val] = get_minmax(data);
    float bin_width = (max_val - min_val) / bins;
    if (bin_width == 0.0f) bin_width = 1.0f;
    
    std::vector<size_t> bin_counts(bins, 0);
    
    for (float val : data) {
        size_t bin_idx = static_cast<size_t>((val - min_val) / bin_width);
        if (bin_idx >= bins) bin_idx = bins - 1;
        bin_counts[bin_idx]++;
    }
    
    size_t max_count = *std::max_element(bin_counts.begin(), bin_counts.end());
    
    draw_border();
    draw_axes(min_val, max_val, 0.0f, static_cast<float>(max_count));
    
    float x_range = max_val - min_val;
    float x_min = min_val - x_range * 0.1f;
    float x_max = max_val + x_range * 0.1f;
    float y_max_count = static_cast<float>(max_count) * 1.1f;
    
    for (size_t i = 0; i < bins; ++i) {
        float bin_start = min_val + i * bin_width;
        float bin_center = bin_start + bin_width / 2.0f;
        
        size_t x_left = to_screen_x(normalize_x(bin_start, x_min, x_max));
        size_t x_right = to_screen_x(normalize_x(bin_start + bin_width, x_min, x_max));
        size_t y_top = to_screen_y(normalize_y(0.0f, 0.0f, y_max_count));
        size_t y_bottom = to_screen_y(normalize_y(static_cast<float>(bin_counts[i]), 0.0f, y_max_count));
        
        if (y_top > y_bottom) std::swap(y_top, y_bottom);
        
        // Use gradient colors based on bin value
        int hist_color = get_series_color(i % color_palette_.size());
        for (size_t x = x_left; x < x_right && x < plot_right_; ++x) {
            for (size_t y = y_top; y <= y_bottom && y < plot_bottom_; ++y) {
                set_pixel(x, y, "\u2588", hist_color);
            }
        }
    }
    
    draw_labels();
}

void TerminalPlot::pie_chart(const DLArray& values, const std::vector<std::string>& labels) {
    pie_chart(to_vector(values), labels);
}

void TerminalPlot::pie_chart(const std::vector<float>& values, const std::vector<std::string>& labels) {
    if (values.empty()) return;
    
    clear();
    
    float total = std::accumulate(values.begin(), values.end(), 0.0f);
    if (total == 0.0f) return;
    
    size_t center_x = (plot_left_ + plot_right_) / 2;
    size_t center_y = (plot_top_ + plot_bottom_) / 2;
    size_t radius = std::min(plot_right_ - plot_left_, plot_bottom_ - plot_top_) / 2 - 2;
    
    std::vector<float> angles;
    float current_angle = 0.0f;
    
    for (float val : values) {
        angles.push_back(current_angle);
        current_angle += (val / total) * 2.0f * M_PI;
    }
    angles.push_back(2.0f * M_PI);
    
    // Draw pie slices
    for (size_t i = 0; i < values.size(); ++i) {
        float start_angle = angles[i];
        float end_angle = angles[i + 1];
        
        for (size_t r = 0; r < radius; ++r) {
            for (float angle = start_angle; angle <= end_angle; angle += 0.05f) {
                int x = static_cast<int>(center_x + r * std::cos(angle));
                int y = static_cast<int>(center_y + r * std::sin(angle));
                
                if (x >= static_cast<int>(plot_left_) && x < static_cast<int>(plot_right_) &&
                    y >= static_cast<int>(plot_top_) && y < static_cast<int>(plot_bottom_)) {
                    char digit = '0' + (i % 10);
                    int pie_color = get_series_color(i);
                    set_pixel(static_cast<size_t>(x), static_cast<size_t>(y), std::string(1, digit), pie_color);
                }
            }
        }
    }
    
    draw_border();
    draw_labels();
    
    // Draw labels
    if (!labels.empty() && plot_bottom_ + 3 < height_) {
        for (size_t i = 0; i < labels.size() && i < values.size(); ++i) {
            std::ostringstream oss;
            oss << "[" << (i % 10) << "] " << labels[i] 
                << " (" << std::fixed << std::setprecision(1) 
                << (values[i] / total * 100.0f) << "%)";
            std::string label_text = oss.str();
            
            for (size_t j = 0; j < label_text.length() && plot_left_ + j < width_; ++j) {
                set_pixel(plot_left_ + j, plot_bottom_ + 3 + i, std::string(1, label_text[j]));
            }
        }
    }
}

void TerminalPlot::box_plot(const DLArray& data) {
    box_plot(to_vector(data));
}

void TerminalPlot::box_plot(const std::vector<float>& data) {
    if (data.empty()) return;
    
    clear();
    
    std::vector<float> sorted_data = data;
    std::sort(sorted_data.begin(), sorted_data.end());
    
    float q1 = get_quantile(sorted_data, 0.25f);
    float median = get_quantile(sorted_data, 0.5f);
    float q3 = get_quantile(sorted_data, 0.75f);
    float iqr = q3 - q1;
    float lower_whisker = std::max(sorted_data.front(), q1 - 1.5f * iqr);
    float upper_whisker = std::min(sorted_data.back(), q3 + 1.5f * iqr);
    
    float y_min = lower_whisker - iqr * 0.2f;
    float y_max = upper_whisker + iqr * 0.2f;
    
    draw_border();
    
    size_t box_x = (plot_left_ + plot_right_) / 2;
    
    // Draw whiskers
    size_t y_lower = to_screen_y(normalize_y(lower_whisker, y_min, y_max));
    size_t y_upper = to_screen_y(normalize_y(upper_whisker, y_min, y_max));
    
    const std::string h_line = "\u2500";  // ─
    const std::string v_line = "\u2502";  // │
    
    int box_color = use_colors_ ? 36 : -1;  // Cyan for box plot
    int outlier_color = use_colors_ ? 31 : -1;  // Red for outliers
    
    for (size_t i = 0; i < 5; ++i) {
        set_pixel(box_x - 2 + i, y_lower, h_line, box_color);
        set_pixel(box_x - 2 + i, y_upper, h_line, box_color);
    }
    set_pixel(box_x, y_lower, v_line, box_color);
    set_pixel(box_x, y_upper, v_line, box_color);
    
    // Draw box (Q1 to Q3)
    size_t y_q1 = to_screen_y(normalize_y(q1, y_min, y_max));
    size_t y_q3 = to_screen_y(normalize_y(q3, y_min, y_max));
    size_t y_median = to_screen_y(normalize_y(median, y_min, y_max));
    
    for (size_t x = box_x - 3; x <= box_x + 3 && x < plot_right_; ++x) {
        set_pixel(x, y_q1, h_line, box_color);
        set_pixel(x, y_q3, h_line, box_color);
    }
    
    for (size_t y = y_q1; y <= y_q3; ++y) {
        set_pixel(box_x - 3, y, v_line, box_color);
        set_pixel(box_x + 3, y, v_line, box_color);
    }
    
    // Draw median line (bold)
    for (size_t x = box_x - 3; x <= box_x + 3 && x < plot_right_; ++x) {
        set_pixel(x, y_median, h_line, box_color, true);
    }
    
    // Draw outliers
    for (float val : sorted_data) {
        if (val < lower_whisker || val > upper_whisker) {
            size_t y = to_screen_y(normalize_y(val, y_min, y_max));
            set_pixel(box_x, y, "*", outlier_color, true);
        }
    }
    
    draw_labels();
}

void TerminalPlot::area_chart(const DLArray& x, const DLArray& y, const std::string& fill_char) {
    area_chart(to_vector(x), to_vector(y), fill_char);
}

void TerminalPlot::area_chart(const std::vector<float>& x, const std::vector<float>& y, const std::string& fill_char) {
    if (x.size() != y.size() || x.empty()) return;
    
    clear();
    auto [x_min, x_max] = get_minmax(x);
    auto [y_min, y_max] = get_minmax(y);
    
    float x_range = x_max - x_min;
    float y_range = y_max - y_min;
    x_min -= x_range * 0.1f;
    x_max += x_range * 0.1f;
    y_min = std::min(0.0f, y_min);
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(x_min, x_max, y_min, y_max);
    
    size_t y_base = to_screen_y(normalize_y(0.0f, y_min, y_max));
    
    for (size_t i = 0; i < x.size() - 1; ++i) {
        size_t x1 = to_screen_x(normalize_x(x[i], x_min, x_max));
        size_t x2 = to_screen_x(normalize_x(x[i+1], x_min, x_max));
        
        size_t y1 = to_screen_y(normalize_y(y[i], y_min, y_max));
        size_t y2 = to_screen_y(normalize_y(y[i+1], y_min, y_max));
        
        for (size_t px = x1; px <= x2 && px < plot_right_; ++px) {
            float t = (x2 > x1) ? static_cast<float>(px - x1) / static_cast<float>(x2 - x1) : 0.0f;
            size_t py_curr = static_cast<size_t>(y1 * (1.0f - t) + y2 * t);
            
            size_t py_start = std::min(py_curr, y_base);
            size_t py_end = std::max(py_curr, y_base);
            
            int area_color = get_series_color(0);
            for (size_t py = py_start; py <= py_end && py < plot_bottom_; ++py) {
                set_pixel(px, py, fill_char, area_color);
            }
        }
    }
    
    draw_labels();
}

void TerminalPlot::heatmap(const DLArray& data, size_t rows, size_t cols) {
    heatmap(to_vector(data), rows, cols);
}

void TerminalPlot::heatmap(const std::vector<float>& data, size_t rows, size_t cols) {
    if (data.size() != rows * cols || data.empty()) return;
    
    clear();
    auto [min_val, max_val] = get_minmax(data);
    
    size_t cell_width = (plot_right_ - plot_left_ - 1) / cols;
    size_t cell_height = (plot_bottom_ - plot_top_ - 1) / rows;
    
    if (cell_width < 1) cell_width = 1;
    if (cell_height < 1) cell_height = 1;
    
    size_t heatmap_chars_len = strlen(HEATMAP_CHARS);
    
    for (size_t r = 0; r < rows; ++r) {
        for (size_t c = 0; c < cols; ++c) {
            float val = data[r * cols + c];
            float normalized = (max_val - min_val > 0.0f) ? 
                (val - min_val) / (max_val - min_val) : 0.0f;
            
            size_t char_idx = static_cast<size_t>(normalized * (heatmap_chars_len - 1));
            std::string cell_char = std::string(1, HEATMAP_CHARS[char_idx]);
            
            // Color gradient: red (low) -> yellow -> green (high)
            int heatmap_color;
            if (normalized < 0.33f) {
                heatmap_color = 31;  // Red
            } else if (normalized < 0.66f) {
                heatmap_color = 33;  // Yellow
            } else {
                heatmap_color = 32;  // Green
            }
            
            size_t start_x = plot_left_ + 1 + c * cell_width;
            size_t start_y = plot_top_ + 1 + r * cell_height;
            
            for (size_t i = 0; i < cell_height && start_y + i < plot_bottom_; ++i) {
                for (size_t j = 0; j < cell_width && start_x + j < plot_right_; ++j) {
                    set_pixel(start_x + j, start_y + i, cell_char, heatmap_color);
                }
            }
        }
    }
    
    draw_border();
    draw_labels();
}

void TerminalPlot::plot3d(const DLArray& x, const DLArray& y, const DLArray& z, const std::string& symbol) {
    if (x.size != y.size || x.size != z.size || x.size == 0) return;
    
    clear();
    auto [x_min, x_max] = get_minmax(x);
    auto [y_min, y_max] = get_minmax(y);
    auto [z_min, z_max] = get_minmax(z);
    
    // Project 3D to 2D using isometric projection
    std::vector<std::pair<size_t, size_t>> screen_coords;
    std::vector<float> depths;
    
    for (size_t i = 0; i < x.size; ++i) {
        float nx = normalize_x(x.data[i], x_min, x_max);
        float ny = normalize_y(y.data[i], y_min, y_max);
        float nz = normalize_x(z.data[i], z_min, z_max);
        
        // Simple isometric projection
        float screen_x = (nx - ny) * 0.707f + 0.5f;
        float screen_y = (nx + ny) * 0.5f * 0.707f - nz * 0.707f + 0.5f;
        
        screen_coords.push_back({
            to_screen_x(screen_x),
            to_screen_y(screen_y)
        });
        depths.push_back(nz);
    }
    
    // Sort by depth (back to front)
    std::vector<size_t> indices(screen_coords.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), 
              [&depths](size_t i, size_t j) { return depths[i] < depths[j]; });
    
    draw_border();
    
    int plot3d_color = get_series_color(0);
    for (size_t idx : indices) {
        set_pixel(screen_coords[idx].first, screen_coords[idx].second, symbol, plot3d_color, true);
    }
    
    draw_labels();
}

void TerminalPlot::show_image(const DLArray& image_data, size_t height, size_t width, size_t channels) {
    show_image(to_vector(image_data), height, width, channels);
}

void TerminalPlot::show_image(const std::vector<float>& image_data, size_t height, size_t width, size_t channels) {
    if (image_data.size() != height * width * channels || image_data.empty()) return;
    
    clear();
    
    size_t display_height = std::min(height, plot_bottom_ - plot_top_ - 1);
    size_t display_width = std::min(width, plot_right_ - plot_left_ - 1);
    
    size_t step_y = (height > display_height) ? height / display_height : 1;
    size_t step_x = (width > display_width) ? width / display_width : 1;
    
    size_t grayscale_len = strlen(GRAYSCALE_CHARS);
    
    for (size_t r = 0; r < display_height; ++r) {
        size_t src_r = r * step_y;
        if (src_r >= height) break;
        
        for (size_t c = 0; c < display_width; ++c) {
            size_t src_c = c * step_x;
            if (src_c >= width) break;
            
            if (channels == 1) {
                // Grayscale - display in actual grayscale colors using ANSI 24-bit color
                float pixel_value = image_data[src_r * width + src_c];
                
                // Normalize to 0-255 range
                if (pixel_value <= 1.0f) pixel_value *= 255.0f;
                
                // Clamp to 0-255
                int gray_int = static_cast<int>(std::max(0.0f, std::min(255.0f, pixel_value)));
                
                // Use a filled block character with grayscale color (R=G=B for true grayscale)
                std::string display_char = "\u2588";  // Full block
                set_pixel_rgb(plot_left_ + 1 + c, plot_top_ + 1 + r, display_char, gray_int, gray_int, gray_int);
                
            } else if (channels == 3) {
                // RGB - display in actual colors using ANSI 24-bit color
                size_t idx = (src_r * width + src_c) * 3;
                float r_val = image_data[idx];
                float g_val = image_data[idx + 1];
                float b_val = image_data[idx + 2];
                
                // Normalize to 0-255 range
                if (r_val <= 1.0f) r_val *= 255.0f;
                if (g_val <= 1.0f) g_val *= 255.0f;
                if (b_val <= 1.0f) b_val *= 255.0f;
                
                // Clamp to 0-255
                int r_int = static_cast<int>(std::max(0.0f, std::min(255.0f, r_val)));
                int g_int = static_cast<int>(std::max(0.0f, std::min(255.0f, g_val)));
                int b_int = static_cast<int>(std::max(0.0f, std::min(255.0f, b_val)));
                
                // Use a filled block character for RGB images to show color better
                std::string display_char = "\u2588";  // Full block
                set_pixel_rgb(plot_left_ + 1 + c, plot_top_ + 1 + r, display_char, r_int, g_int, b_int);
                
            } else {
                // Multi-channel: take first 3 channels as RGB, or convert to grayscale
                size_t idx = (src_r * width + src_c) * channels;
                
                if (channels >= 3) {
                    // Use first 3 channels as RGB
                    float r_val = image_data[idx];
                    float g_val = image_data[idx + 1];
                    float b_val = image_data[idx + 2];
                    
                    // Normalize to 0-255 range
                    if (r_val <= 1.0f) r_val *= 255.0f;
                    if (g_val <= 1.0f) g_val *= 255.0f;
                    if (b_val <= 1.0f) b_val *= 255.0f;
                    
                    int r_int = static_cast<int>(std::max(0.0f, std::min(255.0f, r_val)));
                    int g_int = static_cast<int>(std::max(0.0f, std::min(255.0f, g_val)));
                    int b_int = static_cast<int>(std::max(0.0f, std::min(255.0f, b_val)));
                    
                    std::string display_char = "\u2588";
                    set_pixel_rgb(plot_left_ + 1 + c, plot_top_ + 1 + r, display_char, r_int, g_int, b_int);
                } else {
                    // Average all channels for grayscale
                    float pixel_value = 0.0f;
                    for (size_t ch = 0; ch < channels; ++ch) {
                        pixel_value += image_data[idx + ch];
                    }
                    pixel_value /= channels;
                    
                    // Normalize to 0-255 range
                    if (pixel_value <= 1.0f) pixel_value *= 255.0f;
                    
                    int gray_int = static_cast<int>(std::max(0.0f, std::min(255.0f, pixel_value)));
                    
                    // Display in grayscale color
                    std::string display_char = "\u2588";
                    set_pixel_rgb(plot_left_ + 1 + c, plot_top_ + 1 + r, display_char, gray_int, gray_int, gray_int);
                }
            }
        }
    }
    
    draw_border();
    draw_labels();
}

void TerminalPlot::subplot(size_t rows, size_t cols, size_t index) {
    subplot_mode_ = true;
    subplot_rows_ = rows;
    subplot_cols_ = cols;
    subplot_current_ = index;
    
    size_t subplot_width = (plot_right_ - plot_left_ - 1) / cols;
    size_t subplot_height = (plot_bottom_ - plot_top_ - 1) / rows;
    
    size_t row = index / cols;
    size_t col = index % cols;
    
    plot_left_ = plot_left_ + col * subplot_width;
    plot_right_ = plot_left_ + subplot_width;
    plot_top_ = plot_top_ + row * subplot_height;
    plot_bottom_ = plot_top_ + subplot_height;
}

void TerminalPlot::subplot_clear() {
    subplot_mode_ = false;
    plot_left_ = 5;
    plot_right_ = width_ - 5;
    plot_top_ = 3;
    plot_bottom_ = height_ - 3;
}

void TerminalPlot::show() {
        // // Clear screen (ANSI escape code)
        // std::cout << "\033[2J\033[H";
    // Reset color at the start
    if (use_colors_) {
        std::cout << "\033[0m";
    }
    
    for (size_t y = 0; y < height_; ++y) {
        for (size_t x = 0; x < width_; ++x) {
            std::cout << canvas_[y][x];
        }
        std::cout << '\n';
    }
    
    // Reset color at the end
    if (use_colors_) {
        std::cout << "\033[0m";
    }
    
    std::cout << std::flush;
}

void TerminalPlot::scatter_multi(const std::vector<std::vector<float>>& x_data, 
                                  const std::vector<std::vector<float>>& y_data,
                                  const std::vector<std::string>& labels) {
    if (x_data.empty() || x_data.size() != y_data.size()) return;
    
    clear();
    
    // Find global min/max
    float x_min = std::numeric_limits<float>::max();
    float x_max = std::numeric_limits<float>::lowest();
    float y_min = std::numeric_limits<float>::max();
    float y_max = std::numeric_limits<float>::lowest();
    
    for (size_t series = 0; series < x_data.size(); ++series) {
        if (x_data[series].size() != y_data[series].size()) continue;
        for (size_t i = 0; i < x_data[series].size(); ++i) {
            if (x_data[series][i] < x_min) x_min = x_data[series][i];
            if (x_data[series][i] > x_max) x_max = x_data[series][i];
            if (y_data[series][i] < y_min) y_min = y_data[series][i];
            if (y_data[series][i] > y_max) y_max = y_data[series][i];
        }
    }
    
    float x_range = x_max - x_min;
    float y_range = y_max - y_min;
    x_min -= x_range * 0.1f;
    x_max += x_range * 0.1f;
    y_min -= y_range * 0.1f;
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(x_min, x_max, y_min, y_max);
    
    const std::string symbols = "*+xo.^v<>s";
    
    for (size_t series = 0; series < x_data.size(); ++series) {
        if (x_data[series].size() != y_data[series].size()) continue;
        
        int series_color = get_series_color(series);
        char symbol_char = symbols[series % symbols.length()];
        std::string symbol(1, symbol_char);
        
        for (size_t i = 0; i < x_data[series].size(); ++i) {
            size_t sx = to_screen_x(normalize_x(x_data[series][i], x_min, x_max));
            size_t sy = to_screen_y(normalize_y(y_data[series][i], y_min, y_max));
            set_pixel(sx, sy, symbol, series_color, true);
        }
    }
    
    // Add legend if labels provided
    if (!labels.empty()) {
        std::vector<std::string> legend_symbols;
        for (size_t i = 0; i < labels.size() && i < x_data.size(); ++i) {
            legend_symbols.push_back(std::string(1, symbols[i % symbols.length()]));
        }
        add_legend(labels, legend_symbols);
    }
    
    draw_labels();
}

void TerminalPlot::line_plot_multi(const std::vector<std::vector<float>>& x_data,
                                    const std::vector<std::vector<float>>& y_data,
                                    const std::vector<std::string>& labels) {
    if (x_data.empty() || x_data.size() != y_data.size()) return;
    
    clear();
    
    // Find global min/max
    float x_min = std::numeric_limits<float>::max();
    float x_max = std::numeric_limits<float>::lowest();
    float y_min = std::numeric_limits<float>::max();
    float y_max = std::numeric_limits<float>::lowest();
    
    for (size_t series = 0; series < x_data.size(); ++series) {
        if (x_data[series].size() != y_data[series].size()) continue;
        for (size_t i = 0; i < x_data[series].size(); ++i) {
            if (x_data[series][i] < x_min) x_min = x_data[series][i];
            if (x_data[series][i] > x_max) x_max = x_data[series][i];
            if (y_data[series][i] < y_min) y_min = y_data[series][i];
            if (y_data[series][i] > y_max) y_max = y_data[series][i];
        }
    }
    
    float x_range = x_max - x_min;
    float y_range = y_max - y_min;
    x_min -= x_range * 0.1f;
    x_max += x_range * 0.1f;
    y_min -= y_range * 0.1f;
    y_max += y_range * 0.1f;
    
    draw_border();
    draw_axes(x_min, x_max, y_min, y_max);
    
    for (size_t series = 0; series < x_data.size(); ++series) {
        if (x_data[series].size() != y_data[series].size()) continue;
        if (x_data[series].empty()) continue;
        
        int line_color = get_series_color(series);
        const std::string line_char = "-";
        
        for (size_t i = 0; i < x_data[series].size() - 1; ++i) {
            size_t x1 = to_screen_x(normalize_x(x_data[series][i], x_min, x_max));
            size_t y1 = to_screen_y(normalize_y(y_data[series][i], y_min, y_max));
            size_t x2 = to_screen_x(normalize_x(x_data[series][i+1], x_min, x_max));
            size_t y2 = to_screen_y(normalize_y(y_data[series][i+1], y_min, y_max));
            
            // Bresenham's line algorithm
            int dx = static_cast<int>(x2) - static_cast<int>(x1);
            int dy = static_cast<int>(y2) - static_cast<int>(y1);
            int steps = std::max(std::abs(dx), std::abs(dy));
            
            if (steps > 0) {
                float x_step = static_cast<float>(dx) / steps;
                float y_step = static_cast<float>(dy) / steps;
                
                for (int j = 0; j <= steps; ++j) {
                    size_t px = static_cast<size_t>(x1 + j * x_step);
                    size_t py = static_cast<size_t>(y1 + j * y_step);
                    set_pixel(px, py, line_char, line_color);
                }
            }
        }
    }
    
    // Add legend if labels provided
    if (!labels.empty()) {
        add_legend(labels);
    }
    
    draw_labels();
}

