//Created by: William Jonathan Kusnomo
#pragma once
#include <wx/wx.h>
#include <wx/choicdlg.h>
#include <wx/wx.h>
#include <opencv2/opencv.hpp>

#include "bmpfromocvpanel.h"    
#include "convertmattowxbmp.h"
#include "face_filters.h"
#include "TrackingData.h"
#include "BlobDetection.h"

wxDEFINE_EVENT(wxEVT_CAMERA_FRAME, wxThreadEvent);// Could not retrieve a frame, consider connection to the camera lost.
wxDEFINE_EVENT(wxEVT_CAMERA_EMPTY, wxThreadEvent);// An exception was thrown in the camera thread.
wxDEFINE_EVENT(wxEVT_CAMERA_EXCEPTION, wxThreadEvent);// A frame was retrieved from WebCam

class CameraThread : public wxThread {
public:
    struct CameraFrame {
        cv::Mat matBitmap;
        long    timeGet{ 0 };
    };
    CameraThread(wxEvtHandler* eventSink, cv::VideoCapture* camera, cv::CascadeClassifier* faceCascade)
        : wxThread(wxTHREAD_JOINABLE), m_eventSink(eventSink), m_camera(camera), m_faceCascade(faceCascade){
        wxASSERT(m_eventSink);
        wxASSERT(m_camera);
        wxASSERT(m_faceCascade);
        m_hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
        for (const auto& key : { "faceDetect","humanDetect","plateDetect","blurred","grayscale","blobDetect"}) {
            filters[key] = false;
        }
    }
    std::vector < std::pair < cv::Rect, std::string> >speedRectangles;
    std::vector<cv::Rect> detectionRectangles;
    cv::Mat curFrame;
    std::unordered_map<std::string, std::atomic<bool>> filters;
protected:
    wxEvtHandler* m_eventSink{ nullptr };
    cv::VideoCapture* m_camera{ nullptr };
    cv::CascadeClassifier* m_faceCascade{ nullptr };
    cv::HOGDescriptor m_hog;
    TrackingData tracker;

    ExitCode Entry() override;
};

wxThread::ExitCode CameraThread::Entry(){
    wxStopWatch  stopWatch;
    timeData tD;
    while (!TestDestroy()){
        CameraFrame* frame = nullptr;
        try {
            frame = new CameraFrame;
            stopWatch.Start();
            (*m_camera) >> frame->matBitmap;
            frame->timeGet = stopWatch.Time();

            if (!frame->matBitmap.empty()) {
                // Perform face detection in the worker thread
                curFrame = frame-> matBitmap.clone();
                if (filters["blurred"]) {
                    FaceFilters::ApplyBlurred(frame->matBitmap);
                }
                if (filters["grayscale"]) {
                    FaceFilters::ApplyGrayscale(frame->matBitmap);
                }
                if (filters["faceDetect"]) {
                    FaceFilters::ApplyFaceDetection(frame->matBitmap, *m_faceCascade, detectionRectangles);
                }
                if (filters["humanDetect"]) {
                    FaceFilters::ApplyHumanDetection(frame->matBitmap, m_hog, speedRectangles, tracker,tD);
                }else {
                    tracker.first = true;
                }
                // Send the frame with face detection results to the main thread
                wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_FRAME);

                evt->SetPayload(frame);
                m_eventSink->QueueEvent(evt);
                wxMilliSleep(30);
            }
            else { // connection to camera lost
                m_eventSink->QueueEvent(new wxThreadEvent(wxEVT_CAMERA_EMPTY));
                wxDELETE(frame);
                break;
            }
        }
        catch (const std::exception& e){
            wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_EXCEPTION);

            wxDELETE(frame);
            evt->SetString(e.what());
            m_eventSink->QueueEvent(evt);
            break;
        }
        catch (...){
            wxThreadEvent* evt = new wxThreadEvent(wxEVT_CAMERA_EXCEPTION);

            wxDELETE(frame);
            evt->SetString("Unknown exception");
            m_eventSink->QueueEvent(evt);
            break;
        }
    }

    return static_cast<wxThread::ExitCode>(nullptr);
}
