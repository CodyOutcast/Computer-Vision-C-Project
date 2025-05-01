//Algorithm created by: Chen Zuo Kai
//Template by: William Jonathan Kusnomo
//
#pragma once
#include "heatmap.h"
#include <iostream>
cv::Mat heatmapThread::createRoadMask(const cv::Mat& frame) {
    cv::Mat mask = cv::Mat::zeros(frame.size(), CV_8UC1);

    // Assuming the user draws lines in pairs to form polygons
    std::vector<std::vector<cv::Point>> polygons;

    // Group lines into polygons (you can adjust this logic as needed)
    if (m_lanes.size() >= 2 )
    {
        for (size_t i = 0; i < m_lanes.size(); i += 2)
        {
            std::vector<cv::Point> polygon;
            polygon.push_back(m_lanes[i][0]);
            polygon.push_back(m_lanes[i][1]);
            if (i + 1 < m_lanes.size())
            {
                polygon.push_back(m_lanes[i + 1][1]);
                polygon.push_back(m_lanes[i + 1][0]);
            }
            polygons.push_back(polygon);
        }
    }

    // Draw the polygons on the mask
    for (const auto& polygon : polygons)
    {
        cv::fillConvexPoly(mask, polygon, cv::Scalar(255));
    }

    return mask;
}
void heatmapThread::SetRoadMask(std::vector<std::vector<cv::Point>> lanes) {
    m_lanes=lanes;
}
void heatmapThread::UpdateRoadMask(cv::Mat mat) {
    cv::Mat gray;
    cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    cv::Mat blur;
    cv::GaussianBlur(gray, blur, cv::Size(blur_kernel_size, blur_kernel_size), 0);
    cv::Mat edges;
    cv::Canny(blur, edges, canny_low_threshold, canny_high_threshold);

    // Hough Line Transform to detect lines
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(edges, lines, 1, CV_PI / 180, 50, 50, 10);

    // Create an image to draw the lines on
    cv::Mat line_image = cv::Mat::zeros(mat.size(), mat.type());

    // Draw the lines on the line image
    for (size_t i = 0; i < lines.size(); i++)
    {
        cv::Vec4i l = lines[i];
        cv::line(line_image, cv::Point(l[0], l[1]),
            cv::Point(l[2], l[3]), cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
    }

    // Create road_mask from detected lines
    cv::Mat line_mask;
    cv::cvtColor(line_image, line_mask, cv::COLOR_BGR2GRAY);
    cv::threshold(line_mask, road_mask, 50, 255, cv::THRESH_BINARY_INV);
}

void heatmapThread::initializeHeatMap(int rows, int cols) {
    heatmap = cv::Mat::zeros(rows, cols, CV_8UC1);
}

cv::Mat heatmapThread::applyHeatmap(const cv::Mat& frame, const cv::Mat& road_mask, cv::Mat& heatmap, const cv::Mat& motion_mask,
    double opacity, int decay_rate, int increase_value) {
    cv::subtract(heatmap, cv::Scalar(decay_rate), heatmap);
    cv::Mat motion_road_mask;
    cv::bitwise_and(motion_mask, road_mask, motion_road_mask);
    cv::add(heatmap, cv::Scalar(increase_value), heatmap, motion_road_mask);
    cv::threshold(heatmap, heatmap, 255, 255, cv::THRESH_TRUNC);
    cv::threshold(heatmap, heatmap, 0, 255, cv::THRESH_TOZERO);
    cv::Mat heatmap_color(frame.size(), frame.type());
    cv::Vec3b base_color = cv::Vec3b(255, 200, 100); // Light blue (B, G, R)
    cv::Vec3b hot_color = cv::Vec3b(0, 0, 255);      // Red (B, G, R)

    // Map heatmap values to colors between base_color and hot_color
    for (int y = 0; y < heatmap.rows; y++)
    {
        for (int x = 0; x < heatmap.cols; x++)
        {
            uchar heat_value = heatmap.at<uchar>(y, x);
            double alpha_heat = heat_value / 255.0;

            cv::Vec3b color_pixel;
            for (int c = 0; c < 3; c++)
            {
                // Ensure valid color calculation
                color_pixel[c] = static_cast<uchar>(
                    (1.0 - alpha_heat) * base_color[c] + alpha_heat * hot_color[c]);
            }

            heatmap_color.at<cv::Vec3b>(y, x) = color_pixel;
        }
    }
    // Apply the heatmap to the road regions using the road mask
    cv::Mat final_image = frame.clone();

    heatmap_color.copyTo(final_image, road_mask);

    // Apply opacity to blend the heatmap with the original frame
    cv::addWeighted(final_image, opacity, frame, 1.0 - opacity, 0, final_image);

    return final_image;
}
cv::Mat heatmapThread::detectMovingObjects(const cv::Mat& frame, cv::Ptr<cv::BackgroundSubtractor> pBackSub) {
    cv::Mat fgMask;
    // Apply background subtraction
    pBackSub->apply(frame, fgMask);

    // Threshold to eliminate shadows (if any)
    cv::threshold(fgMask, fgMask, 200, 255, cv::THRESH_BINARY);

    // Optional: Morphological operations to remove noise
    cv::erode(fgMask, fgMask, cv::Mat(), cv::Point(-1, -1), 1);
    cv::dilate(fgMask, fgMask, cv::Mat(), cv::Point(-1, -1), 2);

    return fgMask;
}

wxThread::ExitCode heatmapThread::Entry(){
    cv::VideoCapture capture(m_videoFilePath);
    cv::Mat matframe;
    m_totalFrames = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_COUNT));
    capture >> matframe;
    initializeHeatMap(matframe.rows, matframe.cols);
    wxStopWatch stopWatch;
    while (!TestDestroy()) {
        int currentBlur = blur_kernel_size.load();
        int currentCannyLow = canny_low_threshold.load();
        int currentCannyHigh = canny_high_threshold.load();
        VideoFrame* frame = nullptr;
        if (drawRoad) {
            frame = new VideoFrame;
            capture >> frame->matBitmap;
            initialFrame = frame->matBitmap;
            wxThreadEvent* evt = new wxThreadEvent(wxEVT_VIDEO_FRAME);

            evt->SetPayload(frame);
            m_eventSink->QueueEvent(evt);
            wxMilliSleep(30);
            m_playing = false;
            drawRoad = false;
        }
        if (m_seek) {
            capture.set(cv::CAP_PROP_POS_FRAMES, m_seekFrame);  // Seek to the desired frame
            m_seek = false;
            m_currentFrame = m_seekFrame;
        }
        else if (!m_playing) {
            wxMilliSleep(100);  // Sleep for a short time to avoid busy-waiting
            continue;
        }

        road_mask = createRoadMask(initialFrame);


        try {
            frame = new VideoFrame;
            stopWatch.Start();
            capture >> frame->matBitmap;  // Get the next frame from the video

            frame->timeGet = stopWatch.Time();
            if (!frame->matBitmap.empty()) {
                double opacity = opacity_value / 100.0;
                if(m_lanes.size()<2)UpdateRoadMask(frame->matBitmap);
                cv::Mat gray, mat;
                mat = frame->matBitmap;
                cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
                cv::Mat blur;
                cv::GaussianBlur(gray, blur, cv::Size(blur_kernel_size, blur_kernel_size), 0);
                cv::Mat edges;
                cv::Canny(blur, edges, canny_low_threshold, canny_high_threshold);
                cv::Mat motion_mask = detectMovingObjects(mat, pBackSub);

                std::vector<std::vector<cv::Point>> contours;
                cv::findContours(motion_mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
                cv::Mat frame_with_contours = mat.clone();
                cv::Mat final_output = applyHeatmap(frame_with_contours, road_mask, heatmap, motion_mask, opacity, decay_rate, increase_value);
                frame->matBitmap = final_output;

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