//Created by: William Jonathan Kusnomo
#pragma once
#include <wx/wx.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "face_filters.h"
#include "bmpfromocvpanel.h"
#include "convertmattowxbmp.h"
#include <vector>


class WXDLLIMPEXP_FWD_CORE wxSlider;
class wxBitmapFromOpenCVPanel;

wxDECLARE_EVENT(wxEVT_VIDEO_FRAME, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EMPTY, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EXCEPTION, wxThreadEvent);
namespace cv {
    class Mat;
    class VideoCapture;
}


class heatmapThread;
class OpenCVFrame2 : public wxFrame {
public:
    OpenCVFrame2();
    ~OpenCVFrame2();
private:
    enum Mode {
        Empty,
        Image,
        Video,
        WebCam,
        IPCamera,
    };

    Mode                     m_mode{ Empty };
    wxString                 m_sourceName;
    int                      m_currentVideoFrameNumber{ 0 };

  

    bool firstFrame = false;
    bool userDraw = true;
    heatmapThread* m_heatmapThread{ nullptr };
    cv::CascadeClassifier faceCascade;
    std::vector<cv::Rect> faces;
    cv::Mat road_mask;
    cv::Ptr<cv::BackgroundSubtractor> pBackSub; 
    cv::Mat heatmap;

    // Sliders
    wxSlider* cannyLowSlider;
    wxSlider* cannyHighSlider;
    wxSlider* blurKernelSlider;
    wxSlider* opacitySlider;
    wxSlider* decayRateSlider;
    wxSlider* increaseValueSlider;

    // Event Handlers
    void OnCannyLowSlider(wxCommandEvent& event);
    void OnCannyHighSlider(wxCommandEvent& event);
    void OnBlurKernelSlider(wxCommandEvent& event);
    void OnOpacitySlider(wxCommandEvent& event);
    void OnDecayRateSlider(wxCommandEvent& event);
    void OnIncreaseValueSlider(wxCommandEvent& event);

    FaceFilters* m_filters;
    wxBitmapFromOpenCVPanel* m_bitmapPanel;
    wxSlider* m_videoSlider;
    wxButton* m_propertiesButton, * m_faceDetectionButton,* finishButton, * undoButton;
    bool m_isGrayscale, m_isFaceDetectionEnabled{ false }, m_isBlurred;
    wxTimer m_timer;                    
    bool m_isPlaying = false;           

    cv::HOGDescriptor hog;
    static wxBitmap ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert);



    void OnStartVideo(wxCommandEvent& evt);
    void OnStopVideo(wxCommandEvent& event);
    void OnVideoFrame(wxThreadEvent& event);
    void OnVideoEmpty(wxThreadEvent& event);
    void OnVideoException(wxThreadEvent& event);
    void OnSliderChanged(wxCommandEvent& event);
    void UpdateSliderPosition();
    void DisplayFrame(const cv::Mat);
    void OnPlay(wxCommandEvent& evt);   // Handler for Play button
    void OnPause(wxCommandEvent& evt);  // Handler for Pause button
    void OnFinish(wxCommandEvent& evt);
    void OnUndo(wxCommandEvent& evt);
    void clear();
    void OnClear(wxCommandEvent& evt);
    
    int canny_low_threshold = 10;
    int canny_high_threshold = 60;
    int blur_kernel_size = 5;  // Should be odd and >=1
    int opacity_value = 30;
    int decay_rate = 2;     // Heatmap decay rate (1-20)
    int increase_value = 60; // Heatmap increase value when motion is detected (1-100)
};
