//Created by: William Jonathan Kusnomo
#pragma once
#include <wx/wx.h>
#include <opencv2/opencv.hpp>
#include "bmpfromocvpanel.h"    
#include "convertmattowxbmp.h"
#include "face_filters.h"
#include "TrackingData.h"
#include "blob.h"
#include "BlobDetection.h"
#include <iostream>
#include <atomic>

// Custom events for the video thread
wxDECLARE_EVENT(wxEVT_VIDEO_FRAME, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EMPTY, wxThreadEvent);
wxDECLARE_EVENT(wxEVT_VIDEO_EXCEPTION, wxThreadEvent);



class VideoProcessingThread : public wxThread {
public:
    struct VideoFrame {
        cv::Mat matBitmap;
        long    timeGet{ 0 };
        int frameNumber;
    };
    VideoProcessingThread(wxEvtHandler* eventSink, const std::string& videoFilePath, cv::CascadeClassifier* faceCascade)
        : wxThread(wxTHREAD_JOINABLE), m_eventSink(eventSink), m_videoFilePath(videoFilePath), m_faceCascade(faceCascade) {
        for (const auto& key : { "faceDetect","humanDetect","plateDetect","blurred","grayscale"}) {
            filters[key] = false;
        }
        m_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
        wxASSERT(m_eventSink);
        wxASSERT(!m_videoFilePath.empty());
        wxASSERT(m_faceCascade);

    }
    VideoProcessingThread(wxEvtHandler* eventSink, const std::string& videoFilePath)
        : wxThread(wxTHREAD_JOINABLE), m_eventSink(eventSink), m_videoFilePath(videoFilePath) {
        wxASSERT(m_eventSink);
        wxASSERT(!m_videoFilePath.empty());
    }

    int GetTotalFrames() const { return m_totalFrames; }
    int GetCurrentFrame() const { return m_currentFrame; }
    bool isPlaying() { return m_playing; }
    void Play() { m_playing = true; }
    void Pause() { m_playing = false; }
    void Seek(int frameNumber);

    std::vector<cv::Rect> detectionRectangles;
    std::vector < std::pair < cv::Rect, std::string> > speedRectangles;
    cv::Mat curFrame;
    std::unordered_map<std::string, std::atomic<bool>> filters;
protected:
    wxEvtHandler* m_eventSink{ nullptr };
    std::string m_videoFilePath;
    cv::HOGDescriptor m_hog;
    cv::CascadeClassifier* m_faceCascade{ nullptr }, * m_plateCascade{ nullptr };

    int frameWidth;
    int frameHeight;
    int m_currentFrame{ 0 };
    int m_totalFrames{ 0 };
    int m_seekFrame{ 0 };
    
    std::atomic<bool> m_seek;
    std::atomic<bool> m_playing = true;

    TrackingData tracker;
    ExitCode Entry() override;
};
