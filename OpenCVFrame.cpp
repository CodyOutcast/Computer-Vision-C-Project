//Created by: William Jonathan Kusnomo

#include <wx/wx.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include <regex>

#include "convertmattowxbmp.h"
#include "bmpfromocvpanel.h"
#include "OpenCVFrame.h"
#include "CameraThread.h"
#include "face_filters.h"
#include "VideoThread.h"
#include "OpenCVFrame2.h"

enum
{
    ID_Filter_Blur = 68,
    ID_Filter_Grayscale = 69,
    ID_Human_Detect = 70,
    ID_Face_Detect = 71,
    ID_Blob_detect = 72,
    ID_Counter=73
};
OpenCVFrame::OpenCVFrame()
    : wxFrame(nullptr, wxID_ANY, ""), m_timer(this){
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainPanelSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL); // Properties button and wxSlider
    wxButton* button = nullptr;

    filterMenu = new wxMenu;
    filterMenu->Append(ID_Filter_Blur, "&Blur");
    filterMenu->Append(ID_Filter_Grayscale, "&Grayscale");
    detectionMenu = new wxMenu;
    detectionMenu->Append(ID_Face_Detect, "&Face Detection");
    detectionMenu->Append(ID_Human_Detect, "&Human Detection");
    detectionMenu->Append(ID_Blob_detect, "&Blob Detection");

    button = new wxButton(mainPanel, wxID_ANY, "&Image...");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame::OnImage, this);
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());

    button = new wxButton(mainPanel, wxID_ANY, "&Video...");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame::OnStartVideo, this);
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());

    button = new wxButton(mainPanel, wxID_ANY, "&WebCam...");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame::OnWebCam, this);
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());

    buttonSizer->AddSpacer(FromDIP(20));

    button = new wxButton(mainPanel, wxID_ANY, "&Clear");
    button->Bind(wxEVT_BUTTON, [this](wxCommandEvent& event) {Clear();});
    buttonSizer->Add(button, wxSizerFlags().Proportion(1).Expand().Border());


    wxButton* secondFrameButton = new wxButton(mainPanel, wxID_ANY, "&Open Heatmap Frame");
    secondFrameButton->Bind(wxEVT_BUTTON, &OpenCVFrame::OnOpenSecondFrame, this);
    buttonSizer->Add(secondFrameButton, wxSizerFlags().Proportion(1).Expand().Border());

    filterButton = new wxButton(mainPanel, wxID_ANY, "&Filter");
    buttonSizer->Add(filterButton, wxSizerFlags().Proportion(1).Expand().Border());
    filterButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxPoint position = filterButton->GetScreenPosition();
        wxSize buttonSize = filterButton->GetSize();
        wxPoint menuPosition = wxPoint(position.x, position.y + buttonSize.GetHeight());

        filterMenu->SetInvokingWindow(this);
        PopupMenu(filterMenu, menuPosition);
        });
    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if (m_cameraThread)m_cameraThread->filters["blurred"] = !m_cameraThread->filters["blurred"];
        else if (m_videoThread)m_videoThread->filters["blurred"] = !m_videoThread->filters["blurred"];
        },ID_Filter_Blur);
    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if(m_cameraThread)m_cameraThread->filters["grayscale"] = !m_cameraThread->filters["grayscale"];
        else if (m_videoThread)m_videoThread->filters["grayscale"] = !m_videoThread->filters["grayscale"];
        }, ID_Filter_Grayscale);
    filterButton->Disable();

    detectionButton= new wxButton(mainPanel, wxID_ANY, "&Detect");
    buttonSizer->Add(detectionButton, wxSizerFlags().Proportion(1).Expand().Border());
    detectionButton->Bind(wxEVT_BUTTON, [&](wxCommandEvent&) {
        wxPoint position = detectionButton->GetScreenPosition();
        wxSize buttonSize = detectionButton->GetSize();
        wxPoint menuPosition = wxPoint(position.x, position.y + buttonSize.GetHeight());

        detectionMenu->SetInvokingWindow(this);
        PopupMenu(detectionMenu, menuPosition);
        });
    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if (m_cameraThread) m_cameraThread->filters["faceDetect"] = !m_cameraThread->filters["faceDetect"];
        else if (m_videoThread)m_videoThread->filters["faceDetect"] = !m_videoThread->filters["faceDetect"];
        }, ID_Face_Detect);
    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if(m_cameraThread)m_cameraThread->filters["humanDetect"] = !m_cameraThread->filters["humanDetect"];
        else if (m_videoThread)m_videoThread->filters["humanDetect"] = !m_videoThread->filters["humanDetect"];
        }, ID_Human_Detect);
    Bind(wxEVT_MENU, [this](wxCommandEvent& event) {
        if (m_cameraThread)m_cameraThread->filters["blobDetect"] = !m_cameraThread->filters["blobDetect"];
        else if (m_videoThread)m_videoThread->filters["blobDetect"] = !m_videoThread->filters["blobDetect"];
        }, ID_Blob_detect);
    detectionButton->Enable();

    m_bitmapPanel = new wxBitmapFromOpenCVPanel(mainPanel);

    m_propertiesButton = new wxButton(mainPanel, wxID_ANY, "&Properties...");
    m_propertiesButton->Bind(wxEVT_BUTTON, &OpenCVFrame::OnProperties, this);
    bottomSizer->Add(m_propertiesButton, wxSizerFlags().Expand().Border());

    button = new wxButton(mainPanel, wxID_ANY, "&Play");
    button->Bind(wxEVT_BUTTON, &OpenCVFrame::OnPlayPause, this);
    bottomSizer->Add(button, wxSizerFlags().Expand().Border());

    m_videoSlider = new wxSlider(mainPanel, wxID_ANY, 0, 0, 100, wxDefaultPosition, wxDefaultSize, wxSL_LABELS);
    m_videoSlider->Bind(wxEVT_SLIDER, &OpenCVFrame::OnSliderChanged, this);
    bottomSizer->Add(m_videoSlider, wxSizerFlags().Proportion(1).Expand().Border().ReserveSpaceEvenIfHidden());

    mainPanelSizer->Add(buttonSizer, wxSizerFlags().Expand().Border());
    mainPanelSizer->Add(m_bitmapPanel, wxSizerFlags().Proportion(1).Expand());
    mainPanelSizer->Add(bottomSizer, wxSizerFlags().Expand().Border());

    SetMinClientSize(FromDIP(wxSize(600, 400)));
    SetSize(FromDIP(wxSize(800, 600)));

    mainPanel->SetSizerAndFit(mainPanelSizer);

    Clear();

    Bind(wxEVT_VIDEO_FRAME, &OpenCVFrame::OnVideoFrame, this);
    Bind(wxEVT_VIDEO_EMPTY, &OpenCVFrame::OnVideoEmpty, this);
    Bind(wxEVT_VIDEO_EXCEPTION, &OpenCVFrame::OnVideoException, this);
    Bind(wxEVT_CAMERA_FRAME, &OpenCVFrame::OnCameraFrame, this);
    Bind(wxEVT_CAMERA_EMPTY, &OpenCVFrame::OnCameraEmpty, this);
    Bind(wxEVT_CAMERA_EXCEPTION, &OpenCVFrame::OnCameraException, this);
   
    m_bitmapPanel->Bind(wxEVT_LEFT_DOWN, &OpenCVFrame::OnMouseClick, this);

    if (!faceCascade.load("Resources/haarcascade_frontalface_default.xml")) {
        wxLogError("Could not load Haar Cascade for face detection.");
    }
    if (!plateCascade.load("Resources/haarcascade_russian_plate_number.xml")) {
        wxLogError("Could not load russian plate");
    }
}

OpenCVFrame::~OpenCVFrame() {
    DeleteCameraThread();
}

wxBitmap OpenCVFrame::ConvertMatToBitmap(const cv::Mat& matBitmap, long& timeConvert)
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
void OpenCVFrame::OnOpenSecondFrame(wxCommandEvent&) {
    OpenCVFrame2* secondFrame = new OpenCVFrame2();
    secondFrame->Show();
}
void OpenCVFrame::Clear() {
    DeleteCameraThread();
    if (m_videoCapture)
        wxDELETE(m_videoCapture);
    if (m_videoThread) {
        m_videoThread->Delete(nullptr, wxTHREAD_WAIT_BLOCK);
        wxDELETE(m_videoThread);
    }

    m_mode = Empty;
    m_sourceName.clear();
    m_currentVideoFrameNumber = 0;
    m_isPlaying = false;

    m_bitmapPanel->SetBitmap(wxBitmap(), 0, 0);
    m_bitmapPanel->vectorClear();
    m_propertiesButton->Disable();
    UpdateFrameTitle();
    m_videoSlider->Disable();
    filterButton->Disable();
}
void OpenCVFrame::UpdateFrameTitle() {
    wxString modeStr;
    switch (m_mode) {
    case Empty:
        modeStr = "Empty";
        break;
    case Video:
        modeStr = "Video";
    case Image:
        modeStr = "Image";
    case WebCam:
        modeStr = "WebCam";
        break;
    }
    SetTitle(wxString::Format("wxOpenCVTest: %s", modeStr));
}
bool OpenCVFrame::StartCameraCapture(const wxString& address, const wxSize& resolution, bool useMJPEG) {
    const bool        isDefaultWebCam = address.empty();
    cv::VideoCapture* cap = nullptr;

    Clear();

    {
        wxWindowDisabler disabler;
        wxBusyCursor     busyCursor;

        if (isDefaultWebCam)
            cap = new cv::VideoCapture(0);
        else
            cap = new cv::VideoCapture(address.ToStdString());
    }

    if (!cap->isOpened()) {
        delete cap;
        wxLogError("Could not connect to the camera.");
        return false;
    }

    m_videoCapture = cap;

    if (isDefaultWebCam) {
        m_videoCapture->set(cv::CAP_PROP_FRAME_WIDTH, resolution.GetWidth());
        m_videoCapture->set(cv::CAP_PROP_FRAME_HEIGHT, resolution.GetHeight());

        if (useMJPEG)
            m_videoCapture->set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
    }

    if (!StartCameraThread()) {
        Clear();
        return false;
    }

    return true;
}
bool OpenCVFrame::StartCameraThread() {
    DeleteCameraThread();
    m_cameraThread = new CameraThread(this, m_videoCapture, &faceCascade);
    if (m_cameraThread->Run() != wxTHREAD_NO_ERROR) {
        wxDELETE(m_cameraThread);
        wxLogError("Could not create the thread needed to retrieve the images from a camera.");
        return false;
    }

    return true;
}
void OpenCVFrame::DeleteCameraThread() {
    if (m_cameraThread) {
        m_cameraThread->Delete(nullptr, wxTHREAD_WAIT_BLOCK);
        wxDELETE(m_cameraThread);
    }
}
void OpenCVFrame::OnWebCam(wxCommandEvent&) {
    static const wxSize resolutions[] = { { 320,  240},
                                          { 640,  480},
                                          { 800,  600},
                                          {1024,  576},
                                          {1280,  720},
                                          {1920, 1080} };
    static int    resolutionIndex = 1;
    wxArrayString resolutionStrings;
    bool          useMJPEG = false;

    for (const auto& r : resolutions)
        resolutionStrings.push_back(wxString::Format("%d x %d", r.GetWidth(), r.GetHeight()));

    resolutionIndex = wxGetSingleChoiceIndex("Select resolution", "WebCam", resolutionStrings, resolutionIndex, this);
    if (resolutionIndex == -1)
        return;

    useMJPEG = wxMessageBox("Press Yes to use MJPEG or No to use the default FourCC.\nMJPEG may be much faster, particularly at higher resolutions.", "WebCamera", wxYES_NO, this) == wxYES;
   

    if (StartCameraCapture(wxEmptyString, resolutions[resolutionIndex], useMJPEG)) {
        m_mode = WebCam;
        m_sourceName = "Default WebCam";
        UpdateFrameTitle();

        filterButton->Enable();
        detectionButton->Enable();
        m_propertiesButton->Enable();
    }
}
void OpenCVFrame::OnClear(wxCommandEvent&) {
    Clear();
}
void OpenCVFrame::OnProperties(wxCommandEvent&) {
    wxArrayString properties;

    properties.push_back(wxString::Format("Source: %s", m_sourceName));

    if (m_videoCapture) {
        const int  fourCCInt = static_cast<int>(m_videoCapture->get(cv::CAP_PROP_FOURCC));
        const char fourCCStr[] = { (char)(fourCCInt & 0XFF),
                                  (char)((fourCCInt & 0XFF00) >> 8),
                                  (char)((fourCCInt & 0XFF0000) >> 16),
                                  (char)((fourCCInt & 0XFF000000) >> 24), 0 };

        properties.push_back(wxString::Format("Backend: %s", wxString(m_videoCapture->getBackendName())));

        properties.push_back(wxString::Format("Width: %.0f", m_videoCapture->get(cv::CAP_PROP_FRAME_WIDTH)));
        properties.push_back(wxString::Format("Height: %0.f", m_videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT)));
        properties.push_back(wxString::Format("FPS: %.1f", m_videoCapture->get(cv::CAP_PROP_FPS)));
    }

    wxGetSingleChoice("Name: value", "Properties", properties, this);
}
void OpenCVFrame::OnCameraFrame(wxThreadEvent& evt) {
    CameraThread::CameraFrame* frame = evt.GetPayload<CameraThread::CameraFrame*>();

    if (m_mode != WebCam) {
        delete frame;
        return;
    }
    long timeConvert = 0;
    wxBitmap bitmap = ConvertMatToBitmap(frame->matBitmap, timeConvert);
    //cv::imshow("", frame->matBitmap);
    if (bitmap.IsOk())
        m_bitmapPanel->SetBitmap(bitmap, frame->timeGet, timeConvert);
    else
        m_bitmapPanel->SetBitmap(wxBitmap(), 0, 0);

    delete frame;
}
void OpenCVFrame::OnCameraEmpty(wxThreadEvent&) {
    wxLogError("Connection to the camera lost.");
    Clear();
}
void OpenCVFrame::OnCameraException(wxThreadEvent& evt) {
    wxLogError("Exception in the camera thread: %s", evt.GetString());
    Clear();
}
void OpenCVFrame::OnImage(wxCommandEvent&) {
    static wxString fileName;

    fileName = wxFileSelector("Select Bitmap Image", "", fileName, "",
        "Image files (*.jpg;*.png;*.tga;*.bmp)|*.jpg;*.png;*.tga;*.bmp",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);

    if (fileName.empty())
        return;

    cv::Mat matBitmap;
    wxStopWatch stopWatch;
    long timeGet = 0, timeConvert = 0;

    stopWatch.Start();
    matBitmap = cv::imread(fileName.ToStdString(), cv::IMREAD_COLOR);
    timeGet = stopWatch.Time();

    if (matBitmap.empty()) {
        wxLogError("Could not read image '%s'.", fileName);
        return;
    }

    Clear();
    std::vector<cv::Rect> plates;
    FaceFilters::ApplyPlateDetection(matBitmap,plateCascade,plates);
    detectRect = plates;
    currFrame = matBitmap;
    wxBitmap bitmap = ConvertMatToBitmap(matBitmap, timeConvert);

    if (!bitmap.IsOk()) {
        wxLogError("Could not convert Mat to wxBitmap.", fileName);
        Clear();
        return;
    }

    m_bitmapPanel->SetBitmap(bitmap, timeGet, timeConvert);
    m_propertiesButton->Enable();
    m_mode = Image;
    m_sourceName = fileName;
    UpdateFrameTitle();
}

void OpenCVFrame::OnStartVideo(wxCommandEvent&) {
    static wxString fileName;

    fileName = wxFileSelector("Select Video", "", fileName, "",
        "Video files (*.avi;*.mp4;*.mkv)|*.avi;*.mp4;*.mkv",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST, this);


    if (fileName.empty())
        return;
    // Start the video processing thread
    if (m_videoThread) {
        m_videoThread->Delete();
    }

    m_videoThread = new VideoProcessingThread(this, std::string(fileName.mb_str()), &faceCascade);

    if (m_videoThread->Run() != wxTHREAD_NO_ERROR) {
        wxLogError("salah");
    }
    UpdateFrameTitle();
    m_videoSlider->Enable();
    m_videoSlider->Show();
    m_videoSlider->SetFocus();
    wxMilliSleep(1000);
    m_videoSlider->SetRange(0, m_videoThread->GetTotalFrames());
    m_propertiesButton->Enable();
}

void OpenCVFrame::OnStopVideo(wxCommandEvent& event) {
    if (m_videoThread) {
        m_videoThread->Delete(); 
        m_videoThread = nullptr;
    }
    Close(true);
}
void OpenCVFrame::OnVideoFrame(wxThreadEvent& event) {
 
    VideoProcessingThread::VideoFrame* frame = event.GetPayload<VideoProcessingThread::VideoFrame*>();
    if (frame) {
        DisplayFrame(frame->matBitmap);  
        m_videoSlider->SetValue(m_videoThread->GetCurrentFrame());
        delete frame;
    }
    else {
        wxLogError("salahe");
    }
}
void OpenCVFrame::OnVideoEmpty(wxThreadEvent& event) {

    SetTitle(wxString::Format("wxOpenCVTest: %s", "kont"));
    wxMessageBox("Video has ended or cannot retrieve frames!", "Info", wxICON_INFORMATION);
}
void OpenCVFrame::OnVideoException(wxThreadEvent& event) {
    wxMessageBox(event.GetString(), "Video Thread Exception", wxICON_ERROR);
}
void OpenCVFrame::DisplayFrame(const cv::Mat mat) {
    wxBitmap bitmap;
    long     timeConvert = 0;
    bitmap = ConvertMatToBitmap(mat, timeConvert);

    if (!bitmap.IsOk()) {
        m_bitmapPanel->SetBitmap(wxBitmap(), 0, 0);
        wxLogError("Could not convert frame %d to wxBitmap.");
        return;
    }
    m_bitmapPanel->m_texts.clear();
    m_bitmapPanel->SetBitmap(bitmap, 0, timeConvert);
    std::vector<std::pair<cv::Rect, std::string> > tmp;
    if (m_videoThread) {
        tmp = m_videoThread->speedRectangles;
        for (auto i : tmp) {
            cv::Point bottomLeft(i.first.x, i.first.y + i.first.height);
            m_bitmapPanel->DrawTextAtPoint(i.second, wxPoint(bottomLeft.x, bottomLeft.y));
        }
        
    }
}

void OpenCVFrame::OnSliderChanged(wxCommandEvent& event) {
    if (m_videoThread) {
        m_videoThread->Seek(m_videoSlider->GetValue()); 
    }
}
void OpenCVFrame::UpdateSliderPosition() {
    if (m_videoThread) {
        int currentFrame = m_videoThread->GetCurrentFrame();  
        m_videoSlider->SetValue(currentFrame);
    }
}
void OpenCVFrame::OnPlayPause(wxCommandEvent&) {
    if (m_videoThread) {
        if (m_videoThread->isPlaying())m_videoThread->Pause();
        else m_videoThread->Play();
    }
}
void OpenCVFrame::OnMouseClick(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    int x = pos.x, y = pos.y;
    std::string path = "C:/Users/WilliamJonathan/Documents/Coding/c++/GUIPLLS/GUIPLLS/Resources/SavedImage/"; 
    std::regex pattern(R"((\d+)\.png)"); // Regex to match files like "1.png", "2.png", etc.
    int curr = 0; 
    for (const auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_regular_file()) { // Check if it's a file
            std::string filename = entry.path().filename().string();
            std::smatch match;
            if (std::regex_match(filename, match, pattern)) {
                int number = std::stoi(match[1].str()); // Extract the number
                curr = std::max(curr, number); // Update max number if larger
            }
        }
    }
    curr++;

    if (m_cameraThread) {
        detectRect = m_cameraThread->detectionRectangles;
        currFrame = m_cameraThread->curFrame.clone();
        for (size_t i = 0; i < detectRect.size(); i++) {
            if (detectRect[i].contains(cv::Point(x, y))) {
                
                cv::Mat roi = currFrame(detectRect[i]);
                wxMessageBox("Saved");
                cv::imwrite(path + std::to_string(curr) + ".png", roi);
                std::cout << "Saved Image " << curr << std::endl;
                return;
            }
        }
    }
    else if(m_videoThread){
        detectRect = m_videoThread->detectionRectangles;
        currFrame = m_videoThread->curFrame.clone();
        for (size_t i = 0; i < detectRect.size(); i++) {
            if (detectRect[i].contains(cv::Point(x, y))) {
                wxMessageBox("Saved");
                cv::Mat roi = currFrame(detectRect[i]);
                cv::imwrite(path + std::to_string(curr) + ".png", roi);
                std::cout << "Saved Image " << curr << std::endl;
                return;
            }
        }
    }
    else if (m_mode==Image) {
        for (size_t i = 0; i < detectRect.size(); i++) {
            if (detectRect[i].contains(cv::Point(x, y))) {
                wxMessageBox("Saved");
                cv::Mat roi = currFrame(detectRect[i]);
                cv::imwrite(path + std::to_string(curr) + ".png", roi);
                std::cout << "Saved Image " << curr << std::endl;
                return;
            }
        }
    }
}

/* https://ideone.com/m7vNGM */