//Created by: William Jonathan Kusnomo
#pragma once
#include <wx/wx.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "face_filters.h"
#include <vector>


wxDECLARE_EVENT(wxEVT_VIDEO_FRAME, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EMPTY, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EXCEPTION, wxThreadEvent);
class WXDLLIMPEXP_FWD_CORE wxSlider;
class wxBitmapFromOpenCVPanel;

namespace cv{
    class Mat;
    class VideoCapture;
}

class CameraThread;
class VideoProcessingThread;


class OpenCVFrame : public wxFrame{
public:
    OpenCVFrame();
    ~OpenCVFrame();
private:
    enum Mode{
        Empty,
        Image,
        Video,
        WebCam,
        IPCamera,
    };

    Mode                     m_mode{ Empty };
    wxString                 m_sourceName;
    int                      m_currentVideoFrameNumber{ 0 };

    cv::VideoCapture* m_videoCapture{ nullptr };
    CameraThread* m_cameraThread{ nullptr };
    VideoProcessingThread* m_videoThread { nullptr };

    cv::CascadeClassifier faceCascade, plateCascade;
    cv::HOGDescriptor hog;

    wxBitmapFromOpenCVPanel* m_bitmapPanel;
    wxSlider* m_videoSlider;
    wxButton* m_propertiesButton,* m_faceDetectionButton;
    wxButton* filterButton, *detectionButton;
    wxMenu* filterMenu,*detectionMenu;
    wxTimer m_timer;                    // Timer for video playback
    bool m_isPlaying = false;           // Playback state flag

    std::string modelConfiguration;
    std::string modelWeights;
    std::vector<cv::Rect > detectRect;
    cv::Mat currFrame;
    static wxBitmap ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert);

    void Clear();
    void UpdateFrameTitle();
    void OnClear(wxCommandEvent&);
    void OnProperties(wxCommandEvent&);


    void OnWebCam(wxCommandEvent&);
    bool StartCameraCapture(const wxString& address,const wxSize& resolution = wxSize(),
        bool useMJPEG = false);
    bool  StartCameraThread();
    void DeleteCameraThread();
    void OnCameraFrame(wxThreadEvent& evt);
    void OnCameraEmpty(wxThreadEvent&);
    void OnCameraException(wxThreadEvent& evt);

    void OnImage(wxCommandEvent&);

    void OnStartVideo(wxCommandEvent& evt);
    void OnStopVideo(wxCommandEvent& event);
    void OnVideoFrame(wxThreadEvent& event);
    void OnVideoEmpty(wxThreadEvent& event);
    void OnVideoException(wxThreadEvent& event);
    void OnSliderChanged(wxCommandEvent& event);
    void UpdateSliderPosition();
    void DisplayFrame(const cv::Mat);
    void OnPlayPause(wxCommandEvent& evt);   // Handler for Play button
    void OnMouseClick(wxMouseEvent& evt);
    void OnOpenSecondFrame(wxCommandEvent&);
};
