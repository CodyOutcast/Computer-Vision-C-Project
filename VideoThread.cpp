//Created by: William Jonathan Kusnomo
#pragma once
#include "VideoThread.h"

wxDEFINE_EVENT(wxEVT_VIDEO_FRAME, wxThreadEvent);// A frame was retrieved from a video source.
wxDEFINE_EVENT(wxEVT_VIDEO_EMPTY, wxThreadEvent);// Could not retrieve a frame, consider the video ended or file error.
wxDEFINE_EVENT(wxEVT_VIDEO_EXCEPTION, wxThreadEvent);// An exception was thrown in the video thread.
wxThread::ExitCode VideoProcessingThread::Entry(){
    cv::VideoCapture capture(m_videoFilePath);
    frameWidth = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
    frameHeight = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
    m_totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    wxStopWatch stopWatch;

    if (!capture.isOpened()) {
        wxThreadEvent* evt = new wxThreadEvent(wxEVT_VIDEO_EXCEPTION);
        evt->SetString("Failed to open video file: " + m_videoFilePath);
        m_eventSink->QueueEvent(evt);
        return static_cast<wxThread::ExitCode>(nullptr);
    }
    timeData td;
    BlobTracker m_blobtracker;
    while (!TestDestroy()) {
        if (m_seek) {
            capture.set(cv::CAP_PROP_POS_FRAMES, m_seekFrame);  // Seek to the desired frame
            m_seek = false;
            m_currentFrame = m_seekFrame;
        }
        else if (!m_playing) {
            wxMilliSleep(100);  // Sleep for a short time to avoid busy-waiting
            continue;
        }

        VideoFrame* frame = nullptr;
        try {
            frame = new VideoFrame;
            stopWatch.Start();
            capture >> frame->matBitmap;  // Get the next frame from the video
            frame->timeGet = stopWatch.Time();
            curFrame = frame->matBitmap.clone();
            m_currentFrame = static_cast<int>(capture.get(cv::CAP_PROP_POS_FRAMES));
            if (!frame->matBitmap.empty()) {
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
                    FaceFilters::ApplyHumanDetection(frame->matBitmap, m_hog, speedRectangles, tracker, td);
                    std::vector<cv::Rect> tmp;
                    for (auto a : speedRectangles) {
                        tmp.push_back(a.first);
                    }
                    detectionRectangles = tmp;
                }
                else {
                    tracker.first = true;
                }
                if (filters["plateDetect"]) {
                    FaceFilters::ApplyPlateDetection(frame->matBitmap,*m_plateCascade,detectionRectangles);
                }
                if (filters["blobDetect"]) {
                    m_blobtracker.processFrame(frame->matBitmap);
                }
                wxThreadEvent* evt = new wxThreadEvent(wxEVT_VIDEO_FRAME);
                evt->SetPayload(frame);
                m_eventSink->QueueEvent(evt);
                wxMilliSleep(30);
            }
        }
        catch (const std::exception& e) {
            wxThreadEvent* evt = new wxThreadEvent(wxEVT_VIDEO_EXCEPTION);
            wxDELETE(frame);
            evt->SetString(e.what());
            m_eventSink->QueueEvent(evt);
            break;
        }
        catch (...) {
            wxThreadEvent* evt = new wxThreadEvent(wxEVT_VIDEO_EXCEPTION);
            wxDELETE(frame);
            evt->SetString("Unknown exception occurred");
            m_eventSink->QueueEvent(evt);
            break;
        }
    }

    return static_cast<wxThread::ExitCode>(nullptr);
}
void VideoProcessingThread::Seek(int frameNumber) {
    m_seekFrame = frameNumber;
    m_seek = true;
}