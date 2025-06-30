# Image Processing Basis

This repository provides **basic image processing practice** focusing on:

- 🚩 Image **Rotation**
- 🚩 Image **Scaling**

implemented in **C++** for foundational understanding.

---
## 📂 Directory Structure

```bash
.
├── image_rotation/ # Rotation implementation and test
│ ├── build.sh
│ └── run.sh
│
├── image_scaling/ # Scaling implementation and test
│ ├── build.sh
│ └── run.sh
│
└── README.md
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
