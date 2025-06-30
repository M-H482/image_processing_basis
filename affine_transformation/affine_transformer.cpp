#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm> // for std::max
#include <opencv2/opencv.hpp>
#include <cassert> 

// 为了在 Windows (MSVC) 下也能使用 M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 使用 cv 命名空间和 std 命名空间
using namespace cv;
using namespace std;

struct Matrix2d3x3 {
	typedef double T;
	Matrix2d3x3(){
		memset(data, 0, sizeof(data));
	}
	#define SET_VALUE(i) data[i] = x##i;
	Matrix2d3x3(T x0, T x1, T x2, T x3, T x4, T x5, T x6, T x7, T x8){
		SET_VALUE(0); SET_VALUE(1); SET_VALUE(2);  
        SET_VALUE(3); SET_VALUE(4); SET_VALUE(5);
		SET_VALUE(6); SET_VALUE(7); SET_VALUE(8);
	}
	Matrix2d3x3 operator*(const Matrix2d3x3& m){
		Matrix2d3x3 m_out;
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				T sum(0);
				for (int k = 0; k < 3; k++) {
					sum += this->data[i * 3 + k] * m.data[j + k * 3];
				}
				m_out.data[i * 3 + j] = sum;
			}
		}
		return m_out;
	}
	inline Matrix2d3x3& operator=(const Matrix2d3x3& m) throw() {
		memcpy(data, m.data, sizeof(data));
		return *this;
	}
	T data[9];
};

struct Vector1dx3 {
	typedef double T;
	Vector1dx3(){
		memset(data, 0, sizeof(data));
	}
	#define SET_VALUE(i) data[i] = x##i;
	Vector1dx3(T x0, T x1, T x2){
		SET_VALUE(0); SET_VALUE(1); SET_VALUE(2);  
	}
	Vector1dx3 operator*(const Matrix2d3x3& m) const {
		Vector1dx3 m_out(0, 0, 1);
        for (int j = 0; j < 2; j++) {
            T sum(0);
            for (int k = 0; k < 3; k++) {
                sum += this->data[k] * m.data[k * 3 + j];
            }
            m_out.data[j] = sum;
        }
		return m_out;
	}
	inline Vector1dx3& operator=(const Vector1dx3& m) throw() {
		memcpy(data, m.data, sizeof(data));
		return *this;
	}
	T data[3];
};

void print_Vector1dx3(string text, Vector1dx3 v){
    cout << text << " " << v.data[0] << " " << v.data[1] << " " << v.data[2] << endl;
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

double max_distance(double a, double b, double c, double d) {
    double max_val = std::max({a, b, c, d});
    double min_val = std::min({a, b, c, d});
    printf("%f %f\n", max_val, min_val);
    return round(max_val - min_val);
}

double min_distance(double a, double b, double c, double d) {
    double max_val = std::max({a, b, c, d});
    double min_val = std::min({a, b, c, d});
    printf("%f %f\n", max_val, min_val);
    return round(max_val - min_val);
}

/**
 * @brief 手动实现图像的任意中心旋转（整理优化版）。
 *
 * 该函数通过以下三个核心步骤实现旋转：
 * 1.  **计算输出尺寸**：通过正向变换旋转源图像的四个角点，计算出能完整容纳旋转后图像的边界框（Bounding Box），以此确定输出图像的尺寸。
 * 2.  **构建逆向变换矩阵**：构建一个从目标图像坐标映射回源图像坐标的3x3仿射变换矩阵。
 * 3.  **像素填充**：遍历目标图像的每一个像素，使用逆向变换矩阵找到其在源图像中的对应坐标，并通过双线性插值获取像素值进行填充。
 *
 * @param src_image      要旋转的源图像 (const Mat&)。
 * @param center_x       旋转中心的X坐标 (double)。
 * @param center_y       旋转中心的Y坐标 (double)。
 * @param angle_degrees  旋转角度（度），正值表示逆时针。
 * @return Mat           旋转后的新图像。
 */
Mat rotateImageManually(const Mat& src_image, double center_x, double center_y, double angle_degrees) {
    // --- 准备工作 ---
    const double angle_radians = angle_degrees * M_PI / 180.0;
    const double fcos = cos(angle_radians);
    const double fsin = sin(angle_radians);
    const int src_w = src_image.cols;
    const int src_h = src_image.rows;

    // --- 1. 计算输出图像尺寸 (通过正向变换) ---
    // 定义将旋转中心平移到原点(0,0)的矩阵
    Matrix2d3x3 translate_to_origin_mat(
        1, 0, 0,
        0, 1, 0,
        -center_x, -center_y, 1
    );

    // 定义围绕原点(0,0)旋转的矩阵
    Matrix2d3x3 rotation_mat(
        fcos, fsin, 0,
        -fsin, fcos, 0,
        0, 0, 1
    );

    // 将两个变换合并为一个矩阵，效率更高
    Matrix2d3x3 forward_transform_mat = translate_to_origin_mat * rotation_mat;

    // 定义源图像的四个角点（使用齐次坐标）
    std::vector<Vector1dx3> src_corners = {
        Vector1dx3(0,     0,     1), // 左上
        Vector1dx3(src_w, 0,     1), // 右上
        Vector1dx3(0,     src_h, 1), // 左下
        Vector1dx3(src_w, src_h, 1)  // 右下
    };

    // 使用正向变换计算旋转后角点的新坐标
    std::vector<Vector1dx3> dst_corners(4);
    std::transform(src_corners.begin(), src_corners.end(), dst_corners.begin(),
                   [&](const Vector1dx3& corner) { return corner * forward_transform_mat; });

    // 从旋转后的四个角点中找到X和Y坐标的最小值和最大值
    double min_x = dst_corners[0].data[0], max_x = dst_corners[0].data[0];
    double min_y = dst_corners[0].data[1], max_y = dst_corners[0].data[1];

    for (int i = 1; i < 4; ++i) {
        min_x = std::min(min_x, dst_corners[i].data[0]);
        max_x = std::max(max_x, dst_corners[i].data[0]);
        min_y = std::min(min_y, dst_corners[i].data[1]);
        max_y = std::max(max_y, dst_corners[i].data[1]);
    }

    // 新图像的宽高就是旋转后坐标的范围
    const int dst_w = static_cast<int>(round(max_x - min_x));
    const int dst_h = static_cast<int>(round(max_y - min_y));

    // 创建一个黑色的目标图像
    Mat dest_image = Mat::zeros(dst_h, dst_w, src_image.type());


    // --- 2. 构建逆向变换矩阵 ---
    // 逆向变换是将目标图像的像素坐标映射回源图像。
    // 过程与正向变换完全相反：
    // a. 将目标像素坐标平移，使其参考系与旋转后的坐标系对齐。
    // b. 应用逆旋转（角度取反）。
    // c. 将坐标平移回原始的旋转中心。

    // a. 平移矩阵：将目标画布坐标(0, dst_w)映射到旋转后坐标(min_x, max_x)
    Matrix2d3x3 translate_to_rotated_space_mat(
        1, 0, 0,
        0, 1, 0,
        min_x, min_y, 1 // 目标图像的(0,0)对应于旋转后空间的(min_x, min_y)
    );

    // b. 逆旋转矩阵 (sin符号取反)
    Matrix2d3x3 inverse_rotation_mat(
        fcos, -fsin, 0,
        fsin, fcos,  0,
        0,    0,     1
    );

    // c. 从原点平移回旋转中心的矩阵
    Matrix2d3x3 translate_from_origin_mat(
        1, 0, 0,
        0, 1, 0,
        center_x, center_y, 1
    );

    // 合并所有逆向变换步骤为一个最终的变换矩阵
    Matrix2d3x3 inverse_transform_mat = translate_to_rotated_space_mat * inverse_rotation_mat * translate_from_origin_mat;


    // --- 3. 像素填充 (遍历目标图像) ---
    for (int dst_y = 0; dst_y < dst_h; ++dst_y) {
        for (int dst_x = 0; dst_x < dst_w; ++dst_x) {
            // 对目标像素应用逆向变换，找到源坐标
            Vector1dx3 dst_point(dst_x, dst_y, 1);
            Vector1dx3 src_point = dst_point * inverse_transform_mat;
            
            double src_x = src_point.data[0];
            double src_y = src_point.data[1];

            // 检查计算出的源坐标是否在源图像边界内
            if (src_x >= 0 && src_x < src_w - 1 && src_y >= 0 && src_y < src_h - 1) {
                // 使用双线性插值获得更平滑的像素值
                dest_image.at<Vec3b>(dst_y, dst_x) = bilinear_interpolate(src_image, src_x, src_y);
            }
        }
    }
    
    return dest_image;
}

/**
 * @brief 使用OpenCV内置函数实现图像旋转，确保旋转后内容完整且居中 (已修正)。
 * @param src_image      源图像。
 * @param angle_degrees  旋转角度（度数）。正值表示逆时针旋转。
 * @param center         在源图像坐标系中的旋转中心点。
 * @return Mat           旋转后的图像，其尺寸会调整以容纳所有旋转后的内容。
 */
Mat rotateImageWithOpenCV(const Mat& src_image, double angle_degrees, const Point2f& center) {
    // 1. 获取围绕指定中心旋转的2x3仿射变换矩阵
    //    这一步是正确的，它定义了旋转的核心操作。
    Mat rot_mat = getRotationMatrix2D(center, angle_degrees, 1.0);

    // 2. 计算旋转后图像的边界框 (修正后的正确方法)
    //    我们需要手动计算源图像四个角点旋转后的位置，以确定新画布的大小。
    Rect2f src_rect(Point2f(0, 0), src_image.size());
    std::vector<Point2f> corners;
    corners.push_back(src_rect.tl());
    corners.push_back(Point2f(src_rect.br().x, src_rect.tl().y));
    corners.push_back(src_rect.br());
    corners.push_back(Point2f(src_rect.tl().x, src_rect.br().y));

    // 使用仿射变换矩阵来变换四个角点的坐标
    std::vector<Point2f> transformed_corners;
    transform(corners, transformed_corners, rot_mat);

    // 使用cv::boundingRect获取能包裹所有变换后角点的最小正立矩形
    Rect2f bbox = boundingRect(transformed_corners);

    // 3. 调整仿射矩阵的平移分量
    //    这一步的逻辑是正确的。我们需要将旋转后的图像平移，
    //    使其左上角对齐到新画布的(0,0)点。
    //    平移量 = -bbox.tl()
    rot_mat.at<double>(0, 2) -= bbox.tl().x;
    rot_mat.at<double>(1, 2) -= bbox.tl().y;

    // 4. 应用经过修正的仿射变换
    //    目标图像的大小使用新计算出的bbox的尺寸。
    Mat dest_image;
    warpAffine(src_image, dest_image, rot_mat, bbox.size());
    
    return dest_image;
}

// main函数 
int main(int argc, char* argv[]) {
    if (argc != 7) {
        cerr << "用法: " << argv[0] << " <输入图像路径> <输出图像路径> <旋转角度> <旋转中心X(百分比)> <旋转中心Y(百分比)> <是否生成校验图(true/false)>" << endl;
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

    double center_x_ratio = atof(argv[4]);
    double center_y_ratio = atof(argv[5]);
    if (center_x_ratio < 0 || center_x_ratio > 1 || center_y_ratio < 0 || center_y_ratio > 1){
        cerr << "错误：旋转中心位置需要在0到1之间。" << endl;
        return -1;
    }

    string verify_str = argv[6];
    if (verify_str == "true" || verify_str == "1" || verify_str == "yes") {
        generate_verify_image = true;
    }

    Mat src_image = imread(input_path, IMREAD_COLOR);
    if (src_image.empty()) {
        cerr << "错误: 无法加载图片: " << input_path << endl;
        return -1;
    }

    cout << "正在执行手动实现的图像旋转..." << endl;
    Mat manual_rotated_image = rotateImageManually(src_image, src_image.cols * center_x_ratio, src_image.rows * center_y_ratio, angle);
    imwrite(output_path, manual_rotated_image);
    cout << "手动旋转的图像已保存到: " << output_path << endl;

    if (generate_verify_image) {
        cout << "正在使用OpenCV内置函数生成校验图像..." << endl;
        Point2f image_center(src_image.cols * center_x_ratio, src_image.rows * center_y_ratio);
        Mat opencv_rotated_image = rotateImageWithOpenCV(src_image, -angle, image_center);
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