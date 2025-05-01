//Created by: William Jonathan Kusnomo
#include <wx/wx.h>
#include <wx/rawbmp.h>
#include <opencv2/opencv.hpp>
#include "convertmattowxbmp.h"

bool ConvertMatBitmapTowxBitmap(const cv::Mat& matBitmap, wxBitmap& bitmap){
    wxCHECK(!matBitmap.empty(), false);
    wxCHECK(matBitmap.type() == CV_8UC3, false);
    wxCHECK(matBitmap.dims == 2, false);
    wxCHECK(bitmap.IsOk(), false);
    wxCHECK(bitmap.GetWidth() == matBitmap.cols && bitmap.GetHeight() == matBitmap.rows, false);
    wxCHECK(bitmap.GetDepth() == 24, false);

    wxNativePixelData           pixelData(bitmap);
    wxNativePixelData::Iterator pixelDataIt(pixelData);

    if ( matBitmap.isContinuous() )
    {
        const uchar* bgr = matBitmap.data;

        for ( int row = 0; row < pixelData.GetHeight(); ++row )
        {
            pixelDataIt.MoveTo(pixelData, 0, row);

            for ( int col = 0;
                  col < pixelData.GetWidth();
                  ++col, ++pixelDataIt )
            {
                pixelDataIt.Blue()  = *bgr++;
                pixelDataIt.Green() = *bgr++;
                pixelDataIt.Red()   = *bgr++;
            }
        }
    }
    
    return bitmap.IsOk();
}
