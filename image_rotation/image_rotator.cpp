#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm> // for std::max
#include <opencv2/opencv.hpp>

// 为了在 Windows (MSVC) 下也能使用 M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 使用 cv 命名空间和 std 命名空间
using namespace cv;
using namespace std;

/**
 * @brief 对一个2D坐标点进行旋转 (围绕原点)
 * @param x_dst 旋转后x坐标 (引用传参)
 * @param y_dst 旋转后y坐标 (引用传参)
 * @param x_src 源x坐标
 * @param y_src 源y坐标
 * @param theta_rad 旋转角度 (弧度)
 */
void rotation_2D(double &x_dst, double &y_dst, double x_src, double y_src, double theta_rad){
    double cos_theta = cos(theta_rad);
    double sin_theta = sin(theta_rad);
    x_dst = x_src * cos_theta - y_src * sin_theta;
    y_dst = x_src * sin_theta + y_src * cos_theta;
}

//  图像处理默认的坐标系方式如下所示：
//   (0,0) ---- x (列, cols) ---->
//     |
//     |
//     y (行, rows)
//     |
//     |
//     V

/**
 * @brief 【新】将点平移到以center为原点的坐标系
 * @param x_centered 输出的中心化x坐标
 * @param y_centered 输出的中心化y坐标
 * @param x_in 输入的x坐标
 * @param y_in 输入的y坐标
 * @param center 作为新原点的中心点
 */
void center_point(double &x_centered, double &y_centered, double x_in, double y_in, const Point2f& center) {
    x_centered = x_in - center.x;
    y_centered = y_in - center.y;
}

/**
 * @brief 【新】将中心化的点平移回常规坐标系
 * @param x_decentered 输出的常规x坐标
 * @param y_decentered 输出的常规y坐标
 * @param x_in 输入的中心化x坐标
 * @param y_in 输入的中心化y坐标
 * @param center 作为参考的中心点
 */
void decenter_point(double &x_decentered, double &y_decentered, double x_in, double y_in, const Point2f& center) {
    x_decentered = x_in + center.x;
    y_decentered = y_in + center.y;
}


/**
 * @brief 在源图像上进行双线性插值采样
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

// 函数：手动实现图像旋转
Mat rotateImageManually(const Mat& src_image, double angle_degrees) {
    double angle_radians = angle_degrees * M_PI / 180.0;
    int src_w = src_image.cols;
    int src_h = src_image.rows;

    double x1 = -src_w / 2.0, y1 = -src_h / 2.0;
    double x2 =  src_w / 2.0, y2 = -src_h / 2.0;
    double x3 =  src_w / 2.0, y3 =  src_h / 2.0;
    double x4 = -src_w / 2.0, y4 =  src_h / 2.0;
    
    double rot_x1, rot_y1, rot_x2, rot_y2, rot_x3, rot_y3, rot_x4, rot_y4;
    
    rotation_2D(rot_x1, rot_y1, x1, y1, angle_radians);
    rotation_2D(rot_x2, rot_y2, x2, y2, angle_radians);
    rotation_2D(rot_x3, rot_y3, x3, y3, angle_radians);
    rotation_2D(rot_x4, rot_y4, x4, y4, angle_radians);

    int new_w = static_cast<int>(round(max({abs(rot_x1), abs(rot_x2), abs(rot_x3), abs(rot_x4)}) * 2));
    int new_h = static_cast<int>(round(max({abs(rot_y1), abs(rot_y2), abs(rot_y3), abs(rot_y4)}) * 2));

    Mat dest_image = Mat::zeros(new_h, new_w, src_image.type());

    Point2f src_center(src_w / 2.0f, src_h / 2.0f);
    Point2f dest_center(new_w / 2.0f, new_h / 2.0f);

    for (int y_prime = 0; y_prime < new_h; ++y_prime) {
        for (int x_prime = 0; x_prime < new_w; ++x_prime) {
            
            // 步骤1: 平移到中心
            double x_centered, y_centered;
            center_point(x_centered, y_centered, x_prime, y_prime, dest_center);

            // 步骤2: (反向)旋转
            double src_x_centered, src_y_centered;
            rotation_2D(src_x_centered, src_y_centered, x_centered, y_centered, -angle_radians);
            
            // 步骤3: 平移回去
            double src_x, src_y;
            decenter_point(src_x, src_y, src_x_centered, src_y_centered, src_center);

            if (src_x >= 0 && src_x < src_w - 1 && src_y >= 0 && src_y < src_h - 1) {
                dest_image.at<Vec3b>(y_prime, x_prime) = bilinear_interpolate(src_image, src_x, src_y);
            }
        }
    }
    return dest_image;
}

// OpenCV内置函数实现 
Mat rotateImageWithOpenCV(const Mat& src_image, double angle_degrees) {
    Point2f center(src_image.cols / 2.0, src_image.rows / 2.0);
    Mat rot_mat = getRotationMatrix2D(center, angle_degrees, 1.0);
    Rect2f bbox = RotatedRect(center, src_image.size(), angle_degrees).boundingRect2f();
    rot_mat.at<double>(0, 2) += bbox.width / 2.0 - center.x;
    rot_mat.at<double>(1, 2) += bbox.height / 2.0 - center.y;
    Mat dest_image;
    warpAffine(src_image, dest_image, rot_mat, bbox.size());
    return dest_image;
}

// main函数 
int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "用法: " << argv[0] << " <输入图像路径> <输出图像路径> <旋转角度> <是否生成校验图(true/false)>" << endl;
        return -1;
    }

    string input_path = argv[1];
    string output_path = argv[2];
    double angle = 0.0;
    bool generate_verify_image = false;

    try {
        angle = stod(argv[3]);
    } catch (const std::exception& e) {
        cerr << "错误：旋转角度必须是一个数字。" << endl;
        return -1;
    }

    string verify_str = argv[4];
    if (verify_str == "true" || verify_str == "1" || verify_str == "yes") {
        generate_verify_image = true;
    }

    Mat src_image = imread(input_path, IMREAD_COLOR);
    if (src_image.empty()) {
        cerr << "错误: 无法加载图片: " << input_path << endl;
        return -1;
    }

    cout << "正在执行手动实现的图像旋转..." << endl;
    Mat manual_rotated_image = rotateImageManually(src_image, -angle);
    imwrite(output_path, manual_rotated_image);
    cout << "手动旋转的图像已保存到: " << output_path << endl;

    if (generate_verify_image) {
        cout << "正在使用OpenCV内置函数生成校验图像..." << endl;
        Mat opencv_rotated_image = rotateImageWithOpenCV(src_image, angle);
        string verify_output_path;
        size_t dot_pos = output_path.find_last_of(".");
        if (dot_pos != string::npos) {
            verify_output_path = output_path.substr(0, dot_pos) + "_opencv_verify" + output_path.substr(dot_pos);
        } else {
            verify_output_path = output_path + "_opencv_verify";
        }
        imwrite(verify_output_path, opencv_rotated_image);
        cout << "OpenCV校验图像已保存到: " << verify_output_path << endl;
    }
    
    cout << "处理完成。" << endl;
    return 0;
}