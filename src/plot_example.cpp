#include "terminal_plot.h"
#include "dlarray.cpp"
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Terminal Plotting Library - Examples  " << std::endl;
    std::cout << "========================================\n" << std::endl;
    
    TerminalPlot plot(120, 35);
    
    // Configure plot settings
    plot.set_color_theme("bright");  // Use bright color theme
    plot.set_num_ticks_x(8);  // More ticks on X axis
    plot.set_num_ticks_y(6);  // More ticks on Y axis
    
    // Example 1: Colored Scatter Plot
    std::cout << "=== Example 1: Colored Scatter Plot ===" << std::endl;
    std::vector<float> x_scatter, y_scatter;
    for (int i = 0; i < 50; ++i) {
        x_scatter.push_back(i * 0.2f);
        y_scatter.push_back(std::sin(i * 0.2f) * 10 + 20 + (i % 3));
    }
    plot.set_title("Colored Scatter Plot");
    plot.set_xlabel("X values");
    plot.set_ylabel("Y values");
    plot.scatter(x_scatter, y_scatter, "*");
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 2: Colored Line Plot with Labels
    std::cout << "\n=== Example 2: Colored Line Plot ===" << std::endl;
    std::vector<float> x_line, y_line;
    for (int i = 0; i < 100; ++i) {
        x_line.push_back(i * 0.1f);
        y_line.push_back(std::sin(i * 0.1f) * 10 + 15);
    }
    plot.set_title("Sine Wave with Axis Labels");
    plot.set_xlabel("Time (units)");
    plot.set_ylabel("Amplitude");
    plot.line_plot(x_line, y_line);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 3: Multiple Series with Legend
    std::cout << "\n=== Example 3: Multiple Line Plots with Legend ===" << std::endl;
    std::vector<std::vector<float>> x_multi, y_multi;
    std::vector<std::string> series_labels;
    
    for (int series = 0; series < 3; ++series) {
        std::vector<float> x_ser, y_ser;
        for (int i = 0; i < 80; ++i) {
            x_ser.push_back(i * 0.1f);
            y_ser.push_back(std::sin((i * 0.1f) + series) * (5 + series * 2) + 15);
        }
        x_multi.push_back(x_ser);
        y_multi.push_back(y_ser);
        series_labels.push_back("Series " + std::to_string(series + 1));
    }
    
    plot.set_title("Multiple Series with Legend");
    plot.set_xlabel("X Axis");
    plot.set_ylabel("Y Axis");
    plot.line_plot_multi(x_multi, y_multi, series_labels);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 4: Colored Bar Chart
    std::cout << "\n=== Example 4: Colored Bar Chart ===" << std::endl;
    std::vector<float> bars = {23.5, 45.2, 67.8, 12.3, 89.1, 34.6, 56.7, 78.9};
    std::vector<std::string> bar_labels = {"Q1", "Q2", "Q3", "Q4", "Q5", "Q6", "Q7", "Q8"};
    plot.set_title("Quarterly Sales Data");
    plot.set_xlabel("Quarter");
    plot.set_ylabel("Sales (thousands)");
    plot.bar_chart(bars, bar_labels);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 5: Colored Histogram
    std::cout << "\n=== Example 5: Colored Histogram ===" << std::endl;
    std::vector<float> hist_data;
    for (int i = 0; i < 1000; ++i) {
        hist_data.push_back(std::rand() % 100);
    }
    plot.set_title("Data Distribution Histogram");
    plot.set_xlabel("Value");
    plot.set_ylabel("Frequency");
    plot.histogram(hist_data, 20);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 6: Colored Pie Chart
    std::cout << "\n=== Example 6: Colored Pie Chart ===" << std::endl;
    std::vector<float> pie_values = {30, 25, 20, 15, 10};
    std::vector<std::string> pie_labels = {"Category A", "Category B", "Category C", "Category D", "Category E"};
    plot.set_title("Pie Chart - Category Distribution");
    plot.pie_chart(pie_values, pie_labels);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 7: Colored Box Plot
    std::cout << "\n=== Example 7: Colored Box Plot ===" << std::endl;
    std::vector<float> box_data;
    for (int i = 0; i < 200; ++i) {
        box_data.push_back(std::rand() % 50 + 10);
    }
    plot.set_title("Statistical Box Plot");
    plot.set_ylabel("Values");
    plot.box_plot(box_data);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 8: Colored Area Chart
    std::cout << "\n=== Example 8: Colored Area Chart ===" << std::endl;
    std::vector<float> x_area, y_area;
    for (int i = 0; i < 60; ++i) {
        x_area.push_back(i);
        y_area.push_back(std::abs(std::sin(i * 0.2f)) * 20);
    }
    plot.set_title("Area Under Curve");
    plot.set_xlabel("X");
    plot.set_ylabel("Y");
    plot.area_chart(x_area, y_area);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 9: Colored Heatmap
    std::cout << "\n=== Example 9: Colored Heatmap ===" << std::endl;
    std::vector<float> heatmap_data(12 * 20);
    for (size_t i = 0; i < heatmap_data.size(); ++i) {
        heatmap_data[i] = std::sin(i * 0.1f) * 50 + 50;
    }
    plot.set_title("Heatmap - Color Gradient");
    plot.set_xlabel("Columns");
    plot.set_ylabel("Rows");
    plot.heatmap(heatmap_data, 12, 20);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 10: Multiple Scatter Series
    std::cout << "\n=== Example 10: Multiple Scatter Plots ===" << std::endl;
    std::vector<std::vector<float>> x_scatter_multi, y_scatter_multi;
    std::vector<std::string> scatter_labels;
    
    for (int s = 0; s < 3; ++s) {
        std::vector<float> xs, ys;
        for (int i = 0; i < 40; ++i) {
            xs.push_back(i * 0.3f + s * 2);
            ys.push_back(std::cos(i * 0.1f + s) * 8 + 15 + s * 3);
        }
        x_scatter_multi.push_back(xs);
        y_scatter_multi.push_back(ys);
        scatter_labels.push_back("Dataset " + std::to_string(s + 1));
    }
    
    plot.set_title("Multiple Scatter Series");
    plot.set_xlabel("X Coordinate");
    plot.set_ylabel("Y Coordinate");
    plot.scatter_multi(x_scatter_multi, y_scatter_multi, scatter_labels);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 11: 3D Plot with Color
    std::cout << "\n=== Example 11: Colored 3D Plot ===" << std::endl;
    std::vector<float> x3d, y3d, z3d;
    for (int i = 0; i < 120; ++i) {
        float t = i * 0.1f;
        x3d.push_back(std::cos(t) * 10);
        y3d.push_back(std::sin(t) * 10);
        z3d.push_back(t * 0.5f);
    }
    plot.set_title("3D Spiral Projection");
    
    // Convert to DLArray for 3D plot
    DLArray x_arr({static_cast<size_t>(x3d.size())});
    DLArray y_arr({static_cast<size_t>(y3d.size())});
    DLArray z_arr({static_cast<size_t>(z3d.size())});
    for (size_t i = 0; i < x3d.size(); ++i) {
        x_arr.data[i] = x3d[i];
        y_arr.data[i] = y3d[i];
        z_arr.data[i] = z3d[i];
    }
    plot.set_xlabel("X");
    plot.set_ylabel("Y");
    plot.plot3d(x_arr, y_arr, z_arr);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 12: Image Visualization (Grayscale)
    std::cout << "\n=== Example 12: Image Visualization (Grayscale) ===" << std::endl;
    std::vector<float> image_data(28 * 28);
    for (size_t i = 0; i < 28 * 28; ++i) {
        int row = i / 28;
        int col = i % 28;
        float dist = std::sqrt((row - 14.0f) * (row - 14.0f) + (col - 14.0f) * (col - 14.0f));
        image_data[i] = std::max(0.0f, 255.0f - dist * 8.0f);
    }
    plot.set_title("Grayscale Image (28x28)");
    plot.show_image(image_data, 28, 28, 1);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 13: Image with RGB (3 channels) - Full Color Display
    std::cout << "\n=== Example 13: RGB Image with True Colors ===" << std::endl;
    std::cout << "Displaying RGB image with actual terminal colors..." << std::endl;
    std::vector<float> rgb_image(30 * 30 * 3);
    for (size_t i = 0; i < 30 * 30; ++i) {
        int row = i / 30;
        int col = i % 30;
        
        // Create a colorful pattern with gradients
        float center_x = 15.0f;
        float center_y = 15.0f;
        float dist = std::sqrt((row - center_y) * (row - center_y) + (col - center_x) * (col - center_x));
        
        // Create RGB gradients
        rgb_image[i * 3] = std::max(0.0f, 255.0f - dist * 8.0f);  // Red - radial gradient
        rgb_image[i * 3 + 1] = (col / 30.0f) * 255.0f;           // Green - horizontal gradient
        rgb_image[i * 3 + 2] = (row / 30.0f) * 255.0f;           // Blue - vertical gradient
    }
    plot.set_title("True Color RGB Image (30x30) - ANSI 24-bit");
    plot.show_image(rgb_image, 30, 30, 3);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 13b: Colorful RGB Pattern
    std::cout << "\n=== Example 13b: Colorful RGB Pattern ===" << std::endl;
    std::vector<float> colorful_image(28 * 28 * 3);
    for (size_t i = 0; i < 28 * 28; ++i) {
        int row = i / 28;
        int col = i % 28;
        
        // Create rainbow-like pattern
        float angle = std::atan2(row - 14.0f, col - 14.0f);
        float dist = std::sqrt((row - 14.0f) * (row - 14.0f) + (col - 14.0f) * (col - 14.0f));
        
        // Rainbow colors based on angle
        float hue = (angle + M_PI) / (2.0f * M_PI);
        float saturation = 1.0f - (dist / 20.0f);
        saturation = std::max(0.0f, std::min(1.0f, saturation));
        
        // Convert HSV to RGB (simplified)
        float r, g, b;
        int hue_i = static_cast<int>(hue * 6.0f) % 6;
        float f = hue * 6.0f - hue_i;
        float p = 0.0f;
        float q = saturation * f;
        float t = saturation * (1.0f - f);
        
        switch (hue_i) {
            case 0: r = saturation; g = t; b = p; break;
            case 1: r = q; g = saturation; b = p; break;
            case 2: r = p; g = saturation; b = t; break;
            case 3: r = p; g = q; b = saturation; break;
            case 4: r = t; g = p; b = saturation; break;
            default: r = saturation; g = p; b = q; break;
        }
        
        // Adjust brightness based on distance
        float brightness = std::max(0.3f, 1.0f - dist / 15.0f);
        r *= brightness;
        g *= brightness;
        b *= brightness;
        
        colorful_image[i * 3] = r * 255.0f;
        colorful_image[i * 3 + 1] = g * 255.0f;
        colorful_image[i * 3 + 2] = b * 255.0f;
    }
    plot.set_title("Rainbow Color Pattern (28x28)");
    plot.show_image(colorful_image, 28, 28, 3);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Example 14: Color Theme Demo
    std::cout << "\n=== Example 14: Different Color Themes ===" << std::endl;
    std::vector<std::string> themes = {"default", "bright", "pastel", "monochrome"};
    
    for (const auto& theme : themes) {
        plot.set_color_theme(theme);
        plot.clear_legend();
        plot.set_title("Theme: " + theme);
        plot.set_xlabel("X");
        plot.set_ylabel("Y");
        
        std::vector<float> x_theme, y_theme;
        for (int i = 0; i < 60; ++i) {
            x_theme.push_back(i);
            y_theme.push_back(std::sin(i * 0.15f) * 10 + 15);
        }
        plot.line_plot(x_theme, y_theme);
        plot.show();
        std::cout << "\nTheme: " << theme << " - Press Enter for next theme...";
        std::cin.get();
    }
    
    // Example 15: Advanced Legend
    std::cout << "\n=== Example 15: Advanced Legend Usage ===" << std::endl;
    plot.set_color_theme("bright");
    plot.clear_legend();
    
    std::vector<float> x1, y1, x2, y2;
    for (int i = 0; i < 50; ++i) {
        x1.push_back(i);
        y1.push_back(std::sin(i * 0.2f) * 5 + 10);
        x2.push_back(i);
        y2.push_back(std::cos(i * 0.2f) * 5 + 10);
    }
    
    std::vector<std::vector<float>> x_legend = {x1, x2};
    std::vector<std::vector<float>> y_legend = {y1, y2};
    std::vector<std::string> legend_labels = {"Sine Wave", "Cosine Wave"};
    
    plot.set_title("Multiple Series with Custom Legend");
    plot.set_xlabel("Time");
    plot.set_ylabel("Amplitude");
    plot.line_plot_multi(x_legend, y_legend, legend_labels);
    plot.show();
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
    
    // Final summary
    std::cout << "\n========================================" << std::endl;
    std::cout << "  All Examples Completed Successfully!" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "\nLibrary Features Demonstrated:" << std::endl;
    std::cout << "  ✓ Scatter plots (single & multi-series)" << std::endl;
    std::cout << "  ✓ Line plots (single & multi-series)" << std::endl;
    std::cout << "  ✓ Bar charts with colored bars" << std::endl;
    std::cout << "  ✓ Histograms with color gradients" << std::endl;
    std::cout << "  ✓ Pie charts with colored slices" << std::endl;
    std::cout << "  ✓ Box plots with colored elements" << std::endl;
    std::cout << "  ✓ Area charts" << std::endl;
    std::cout << "  ✓ Heatmaps with color gradients" << std::endl;
    std::cout << "  ✓ 3D plots" << std::endl;
    std::cout << "  ✓ Image visualization (grayscale & RGB with true colors)" << std::endl;
    std::cout << "  ✓ Multiple color themes" << std::endl;
    std::cout << "  ✓ Legends with colors" << std::endl;
    std::cout << "  ✓ Axis labels and tick marks" << std::endl;
    std::cout << "  ✓ Titles and axis labels" << std::endl;
    
    return 0;
}

