//Created by: William Jonathan  Kusnomo
#include <wx/wx.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include "OpenCVFrame2.h"
#include "VideoThread.h"
#include "heatmap.h"
#include "convertmattowxbmp.h"
#include "bmpfromocvpanel.h"

enum
{
    ID_Filter_Blur = 1001,
    ID_Filter_Grayscale = 69
};
OpenCVFrame2::OpenCVFrame2()
    : wxFrame(nullptr, wxID_ANY, ""), m_timer(this) {

    // Create a main horizontal sizer to hold the sliders on the left and video panel on the right
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxHORIZONTAL); // Horizontal sizer for left and right layout

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL); // Video slider control

    wxButton* button = nullptr;

    // Create sliders and add to leftSizer
    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Canny Low Threshold")), 0, wxALL, 5);
    cannyLowSlider = new wxSlider(mainPanel, wxID_ANY, canny_low_threshold, 1, 300, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(cannyLowSlider, 0, wxALL, 5);
    cannyLowSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnCannyLowSlider, this);

    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Canny High Threshold")), 0, wxALL, 5);
    cannyHighSlider = new wxSlider(mainPanel, wxID_ANY, canny_high_threshold, 1, 300, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(cannyHighSlider, 0, wxALL, 5);
    cannyHighSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnCannyHighSlider, this);

    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Blur Kernel Size")), 0, wxALL, 5);
    blurKernelSlider = new wxSlider(mainPanel, wxID_ANY, blur_kernel_size, 1, 20, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(blurKernelSlider, 0, wxALL, 5);
    blurKernelSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnBlurKernelSlider, this);

    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Opacity Value")), 0, wxALL, 5);
    opacitySlider = new wxSlider(mainPanel, wxID_ANY, opacity_value, 0, 100, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(opacitySlider, 0, wxALL, 5);
    opacitySlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnOpacitySlider, this);

    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Decay Rate")), 0, wxALL, 5);
    decayRateSlider = new wxSlider(mainPanel, wxID_ANY, decay_rate, 1, 20, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(decayRateSlider, 0, wxALL, 5);
    decayRateSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnDecayRateSlider, this);

    leftSizer->Add(new wxStaticText(mainPanel, wxID_ANY, wxT("Increase Value")), 0, wxALL, 5);
    increaseValueSlider = new wxSlider(mainPanel, wxID_ANY, increase_value, 1, 100, wxDefaultPosition, wxSize(250, -1));
    leftSizer->Add(increaseValueSlider, 0, wxALL, 5);
    increaseValueSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnIncreaseValueSlider, this);

    // Add buttons to the leftSizer below sliders
    button = new wxButton(mainPanel, wxID_ANY, "&Video...");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame2::OnStartVideo, this);
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());

    button = new wxButton(mainPanel, wxID_ANY, "&Clear");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame2::OnClear, this);
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());

    finishButton = new wxButton(mainPanel, wxID_ANY, "&Finish Road");
    finishButton->Bind(wxEVT_BUTTON, &OpenCVFrame2::OnFinish, this);
    buttonSizer->Add(finishButton, wxSizerFlags().Proportion(1).Expand().Border());
    finishButton->Disable();
    
    undoButton = new wxButton(mainPanel, wxID_ANY, "&Undo");
    undoButton->Bind(wxEVT_BUTTON, &OpenCVFrame2::OnUndo, this);
    buttonSizer->Add(undoButton, wxSizerFlags().Proportion(1).Expand().Border());
    undoButton->Disable();

    leftSizer->Add(buttonSizer, wxSizerFlags().Expand().Border());

    // Add video display panel to the rightSizer
    m_bitmapPanel = new wxBitmapFromOpenCVPanel(mainPanel);
    rightSizer->Add(m_bitmapPanel, wxSizerFlags().Proportion(1).Expand());

    // Add video slider to the rightSizer
    m_videoSlider = new wxSlider(mainPanel, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
    m_videoSlider->Bind(wxEVT_SLIDER, &OpenCVFrame2::OnSliderChanged, this);
    bottomSizer->Add(m_videoSlider, wxSizerFlags().Proportion(1).Expand().Border().ReserveSpaceEvenIfHidden());
    m_videoSlider->Hide();

    rightSizer->Add(bottomSizer, wxSizerFlags().Expand().Border());

    // Add left and right sizers to mainPanelSizer
    mainPanelSizer->Add(leftSizer, wxSizerFlags().Expand().Border());
    mainPanelSizer->Add(rightSizer, wxSizerFlags().Proportion(1).Expand());

    // Set sizer and fit layout
    SetMinClientSize(FromDIP(wxSize(600, 400)));
    SetSize(FromDIP(wxSize(800, 600)));

    mainPanel->SetSizerAndFit(mainPanelSizer);

    // Set up background subtraction and face detection
    pBackSub = cv::createBackgroundSubtractorMOG2();
    if (!faceCascade.load("Resources/haarcascade_frontalface_default.xml")) {
        wxLogError("Could not load Haar Cascade for face detection.");
    }
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());

    // Bind events
    Bind(wxEVT_VIDEO_FRAME, &OpenCVFrame2::OnVideoFrame, this);
    Bind(wxEVT_VIDEO_EMPTY, &OpenCVFrame2::OnVideoEmpty, this);
    Bind(wxEVT_VIDEO_EXCEPTION, &OpenCVFrame2::OnVideoException, this);
}


OpenCVFrame2::~OpenCVFrame2() {

}
wxBitmap OpenCVFrame2::ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert)
{
    wxCHECK(!matBitmap.empty(), wxBitmap());

    wxBitmap    bitmap(matBitmap.cols, matBitmap.rows, 24);
    bool        converted = false;
    wxStopWatch stopWatch;
    long        time = 0;

    stopWatch.Start();
    converted = ConvertMatBitmapTowxBitmap(matBitmap, bitmap);
    time = stopWatch.Time();

    if (!converted)
    {
        wxLogError("Could not convert Mat to wxBitmap.");
        return wxBitmap();
    }

    timeConvert = time;
    return bitmap;
}
void OpenCVFrame2::clear() {
    if (m_heatmapThread) {
        m_heatmapThread->Delete(nullptr, wxTHREAD_WAIT_BLOCK);
        wxDELETE(m_heatmapThread);
    }

    m_videoSlider->Disable();
    m_mode = Empty;
    m_sourceName.clear();
    m_currentVideoFrameNumber = 0;
    m_isPlaying = false;
    m_bitmapPanel->SetBitmap(wxBitmap(), 0, 0);
    m_bitmapPanel->vectorClear();
}
void OpenCVFrame2::OnClear(wxCommandEvent&) {
    clear();
}
void OpenCVFrame2::OnStartVideo(wxCommandEvent&) {
    static wxString fileName;

    fileName = wxFileSelector("Select Video", "", fileName, "",
        "Video files (*.avi;*.mp4;*.mkv)|*.avi;*.mp4;*.mkv",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (fileName.empty())
        return;

    // Start the video processing thread
    if (m_heatmapThread) {
        m_heatmapThread->Delete();
    }

    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    m_heatmapThread = new heatmapThread(this, std::string(fileName.mb_str()));
    if (m_heatmapThread->Run() != wxTHREAD_NO_ERROR) {
        wxLogError("salah");
    }
    wxMilliSleep(1000);
    m_videoSlider->SetRange(0, m_heatmapThread->GetTotalFrames()-1);
    m_bitmapPanel->drawMode = true;
    finishButton->Enable();
    undoButton->Enable();
}
void OpenCVFrame2::OnStopVideo(wxCommandEvent& event) {
    if (m_heatmapThread) {
        m_heatmapThread->Delete();
        m_heatmapThread = nullptr;
    }
    Close(true);
}
void OpenCVFrame2::OnVideoFrame(wxThreadEvent& event) {

    VideoProcessingThread::VideoFrame* frame = event.GetPayload<VideoProcessingThread::VideoFrame*>();
    
    if (frame) {
        DisplayFrame(frame->matBitmap);
        delete frame;
    }
    else {
        wxLogError("salahe");
    }
}
void OpenCVFrame2::OnVideoEmpty(wxThreadEvent& event) {

    SetTitle(wxString::Format("wxOpenCVTest: %s", "kont"));
    wxMessageBox("Video has ended or cannot retrieve frames!", "Info", wxICON_INFORMATION);
}
void OpenCVFrame2::OnVideoException(wxThreadEvent& event) {
    wxMessageBox(event.GetString(), "Video Thread Exception", wxICON_ERROR);
}
void OpenCVFrame2::DisplayFrame(const cv::Mat mat) {
    wxBitmap bitmap;
    long     timeConvert = 0;
    bitmap = ConvertMatToBitmap(mat, timeConvert);
    if (!bitmap.IsOk()) {
        m_bitmapPanel->SetBitmap(wxBitmap(), 0, 0);
        wxLogError("Could not convert frame %d to wxBitmap.");
        return;
    }
    m_bitmapPanel->SetBitmap(bitmap, 0, timeConvert);
}
void OpenCVFrame2::OnSliderChanged(wxCommandEvent& event) {
    if (m_heatmapThread) {
        m_heatmapThread->Seek(m_videoSlider->GetValue());  // Seek to the target frame
    }
}
void OpenCVFrame2::OnFinish(wxCommandEvent&) {
    if (m_heatmapThread) {
        m_heatmapThread->Play();
    }
    auto road_line = m_bitmapPanel->getHeatmap();
    m_heatmapThread->SetRoadMask(road_line);
    m_bitmapPanel->vectorClear();
    m_bitmapPanel->drawMode = false;
    finishButton->Disable();
    undoButton->Disable();
}
void OpenCVFrame2::OnCannyLowSlider(wxCommandEvent& event){
    m_heatmapThread->SetCannyLow(cannyLowSlider->GetValue());
}
void OpenCVFrame2::OnCannyHighSlider(wxCommandEvent& event){
    m_heatmapThread->SetCannyHigh(cannyHighSlider->GetValue());
}
void OpenCVFrame2::OnBlurKernelSlider(wxCommandEvent& event){
    int blur_kernel = blurKernelSlider->GetValue();
    if (blur_kernel % 2 == 0)
        blur_kernel += 1;  // Ensure it's an odd value
    m_heatmapThread->SetBlur(blur_kernel);
}
void OpenCVFrame2::OnOpacitySlider(wxCommandEvent& event){
    m_heatmapThread->SetOpacity(opacitySlider->GetValue());
}
void OpenCVFrame2::OnDecayRateSlider(wxCommandEvent& event){
    m_heatmapThread->SetDecay(decayRateSlider->GetValue());
}
void OpenCVFrame2::OnIncreaseValueSlider(wxCommandEvent& evt) {
    m_heatmapThread->SetIncrease(increaseValueSlider->GetValue());
}
void OpenCVFrame2::OnUndo(wxCommandEvent& event){
    m_bitmapPanel->undo();
}