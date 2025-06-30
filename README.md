# Image Processing Basis

This repository provides **basic image processing practice** focusing on:

- 🚩 Image **Rotation**
- 🚩 Image **Scaling**

implemented in **C++** for foundational understanding.

---
## 📂 Directory Structure

```bash
.
├── image_rotation
│   ├── build.sh
│   ├── image_rotator
│   ├── image_rotator.cpp
│   ├── lenna_30_opencv_verify.png
│   ├── lenna_30.png
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

2 directories, 16 files
```

---

## 🚀 Quick Start

### 1️⃣ Clone Repository

```bash
git clone https://github.com/yourname/image-processing-basis.git
cd image-processing-basis
```

### 2️⃣ Image Rotation
```bash
cd image_rotation
bash build.sh       # Build the rotation executable
bash run.sh         # Run the rotation demo
```
### 3️⃣ Image Scaling
```bash
cd image_scaling
bash build.sh       # Build the scaling executable
bash run.sh         # Run the scaling demo
```

## 🛠 Requirements
Linux with g++

OpenCV

## 🖼️ Description
Image Rotation
Rotates input images by a specified angle (in degrees).

Uses bilinear interpolation.

Image Scaling
Resizes input images to specified scales.

Supports upscaling and downscaling.

## 📈 Future Plans

Add a demo of FFT.
