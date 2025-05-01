//Created by: William Jonathan Kusnomo
#pragma once
namespace cv { class Mat; }
class wxBitmap;

bool ConvertMatBitmapTowxBitmap(const cv::Mat& matBitmap, wxBitmap& bitmap);