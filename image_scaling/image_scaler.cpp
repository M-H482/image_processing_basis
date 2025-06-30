#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <opencv2/opencv.hpp>

// 使用 cv 命名空间和 std 命名空间
using namespace cv;
using namespace std;

/**
 * @brief 在源图像上进行双线性插值采样 (与旋转示例中完全相同，可复用)
 * @param src_image 源图像
 * @param src_x 采样的浮点x坐标
 * @param src_y 采样的浮点y坐标
 * @return 插值后的像素颜色 (Vec3b)
 */
Vec3b bilinear_interpolate(const Mat& src_image, double src_x, double src_y) {
    int x_floor = static_cast<int>(src_x);
    int y_floor = static_cast<int>(src_y);

    double dx = src_x - x_floor;
    double dy = src_y - y_floor;

    Vec3b p1 = src_image.at<Vec3b>(y_floor,     x_floor);
    Vec3b p2 = src_image.at<Vec3b>(y_floor,     x_floor + 1);
    Vec3b p3 = src_image.at<Vec3b>(y_floor + 1, x_floor);
    Vec3b p4 = src_image.at<Vec3b>(y_floor + 1, x_floor + 1);
    
    Vec3b interpolated_pixel;
    for (int c = 0; c < 3; ++c) {
        double top_inter = p1[c] * (1 - dx) + p2[c] * dx;
        double bottom_inter = p3[c] * (1 - dx) + p4[c] * dx;
        interpolated_pixel[c] = static_cast<uchar>(top_inter * (1 - dy) + bottom_inter * dy);
    }
    return interpolated_pixel;
}

/**
 * @brief 【新接口】手动实现图像缩放
 * @param src_image 源图像
 * @param scale_x 水平缩放比例 (例如, 2.0代表放大一倍, 0.5代表缩小一半)
 * @param scale_y 垂直缩放比例
 * @return 缩放后的图像
 */
Mat scaleImageManually(const Mat& src_image, double scale_x, double scale_y) {
    int src_w = src_image.cols;
    int src_h = src_image.rows;

    // 计算目标图像的尺寸
    int dest_w = static_cast<int>(round(src_w * scale_x));
    int dest_h = static_cast<int>(round(src_h * scale_y));

    // 创建目标图像
    Mat dest_image = Mat::zeros(dest_h, dest_w, src_image.type());

    // 遍历目标图像的每一个像素 (反向映射)
    for (int y_dest = 0; y_dest < dest_h; ++y_dest) {
        for (int x_dest = 0; x_dest < dest_w; ++x_dest) {
            
            // 计算在源图像中对应的坐标
            double x_src = x_dest / scale_x;
            double y_src = y_dest / scale_y;

            // 检查边界，防止计算出的坐标超出源图像范围
            if (x_src >= 0 && x_src < src_w - 1 && y_src >= 0 && y_src < src_h - 1) {
                // 使用双线性插值获取像素颜色
                dest_image.at<Vec3b>(y_dest, x_dest) = bilinear_interpolate(src_image, x_src, y_src);
            }
        }
    }
    return dest_image;
}

/**
 * @brief 【新接口】使用OpenCV内置函数实现图像缩放
 * @param src_image 源图像
 * @param scale_x 水平缩放比例
 * @param scale_y 垂直缩放比例
 * @return 缩放后的图像
 */
Mat scaleImageWithOpenCV(const Mat& src_image, double scale_x, double scale_y) {
    Mat dest_image;
    // 使用 cv::resize 函数，dsize设为Size(0,0)时，会通过fx和fy来计算目标尺寸
    // INTER_LINEAR 表示使用双线性插值，与我们手动实现的方法一致
    resize(src_image, dest_image, Size(0, 0), scale_x, scale_y, INTER_LINEAR);
    return dest_image;
}


int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "用法: " << argv[0] << " <输入路径> <缩放x> <缩放y> <手动输出路径> <OpenCV输出路径>" << endl;
        return -1;
    }

    string input_path = argv[1];
    double scale_x = stod(argv[2]);
    double scale_y = stod(argv[3]);
    string manual_output_path = argv[4];
    string opencv_output_path = argv[5];
    
    Mat src_image = imread(input_path, IMREAD_COLOR);
    if (src_image.empty()) {
        cerr << "错误: 无法加载图片: " << input_path << endl;
        return -1;
    }

    // --- 手动实现 ---
    cout << "正在执行手动实现的图像缩放..." << endl;
    Mat manual_scaled_image = scaleImageManually(src_image, scale_x, scale_y);
    imwrite(manual_output_path, manual_scaled_image);
    cout << "手动缩放的图像已保存到: " << manual_output_path << endl;

    // --- OpenCV实现 ---
    cout << "正在使用OpenCV内置函数进行缩放..." << endl;
    Mat opencv_scaled_image = scaleImageWithOpenCV(src_image, scale_x, scale_y);
    imwrite(opencv_output_path, opencv_scaled_image);
    cout << "OpenCV缩放的图像已保存到: " << opencv_output_path << endl;

    cout << "处理完成。" << endl;
    return 0;
}