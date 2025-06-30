# Image Processing Basis

This repository provides **basic image processing practice** focusing on:

- 🚩 Image **Rotation**
- 🚩 Image **Scaling**

implemented in **C++** for foundational understanding.

---
## 📂 Directory Structure

```bash
.
├── affine_transformation
│   ├── affine_transformer
│   ├── affine_transformer.cpp
│   ├── build.sh
│   ├── lenna_transformed_opencv_verify.png
│   ├── lenna_transformed.png
│   └── run.sh
├── image_rotation
│   ├── build.sh
│   ├── image_rotator
│   ├── image_rotator.cpp
│   ├── lenna_rotated_opencv_verify.png
│   ├── lenna_rotated.png
│   └── run.sh
├── image_scaling
│   ├── build.sh
│   ├── image_scaler
│   ├── image_scaler.cpp
│   ├── manual_scaled.png
│   ├── opencv_scaled.png
│   └── run.sh
├── lenna.png
└── README.md

3 directories, 20 files
```

---

## 🚀 Quick Start

### 1️⃣ Clone Repository

```bash
git clone https://github.com/M-H482/image-processing-basis.git
cd image-processing-basis
```

### 2️⃣ Image Rotation
*Rotates input images by a specified angle (in degrees), using the image center as the rotation center.*
```bash
cd image_rotation
bash build.sh       # Build the rotation executable
bash run.sh         # Run the rotation demo
```
### 3️⃣ Image Scaling
*Resizes input images to specified scales.*
```bash
cd image_scaling
bash build.sh       # Build the scaling executable
bash run.sh         # Run the scaling demo
```
### 4️⃣ Affine Transformation 
*Implementing rotation of any angle around any center using an affine matrix*
```bash
cd affine_transformation
bash build.sh       # Build the scaling executable
bash run.sh         # Run the scaling demo
```

## 🛠 Requirements
Linux with g++

OpenCV

## 📈 Future Plans

Add a demo of FFT.
