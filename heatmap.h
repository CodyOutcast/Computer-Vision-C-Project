//Algorithm created by : Chen Zuo Kai
//Template by: William Jonathan Kusnomo
#pragma once
#include "VideoThread.h"

class heatmapThread : public VideoProcessingThread {
public:
    heatmapThread(wxEvtHandler* eventSink, const std::string& videoFilePath)
        : VideoProcessingThread(eventSink, videoFilePath){
        pBackSub = cv::createBackgroundSubtractorMOG2();
    }
    cv::Mat road_mask, initialFrame;
    cv::Ptr<cv::BackgroundSubtractor> pBackSub;
    cv::Mat heatmap;
    int GetTotalFrames() const { return m_totalFrames; }
    int GetCurrentFrame() const { return m_currentFrame; }
    bool first = false;
    void SetBlur(int x) {
        blur_kernel_size.store(x);
    }
    void SetCannyLow(int x) {
        canny_low_threshold.store(x);
    }
    void SetCannyHigh(int x) {
        canny_high_threshold.store(x);
    }
    void SetOpacity(int x) {
        opacity_value.store(x);
    }
    void SetDecay(int x) {
        decay_rate.store(x);
    }
    void SetIncrease(int x) {
        increase_value.store(x);
    }
    bool drawRoad = true;
    void SetRoadMask(std::vector<std::vector<cv::Point>> lanes);
    cv::Mat createRoadMask(const cv::Mat& frame);
    void UpdateRoadMask(cv::Mat mat);
    void initializeHeatMap(int rows, int cols);
    cv::Mat applyHeatmap(const cv::Mat& frame, const cv::Mat& road_mask, cv::Mat& heatmap, const cv::Mat& motion_mask, 
        double opacity, int decay_rate, int increase_value);
    cv::Mat detectMovingObjects(const cv::Mat& frame, cv::Ptr<cv::BackgroundSubtractor> pBackSub);
    ExitCode Entry() override;
private:
    std::atomic<int> canny_low_threshold = 10;
    std::atomic<int> canny_high_threshold = 60;
    std::atomic<int> blur_kernel_size = 5;
    std::atomic<int> opacity_value = 30;
    std::atomic<int> decay_rate = 2;
    std::atomic<int> increase_value = 60;
    std::vector<std::vector<cv::Point>> m_lanes;
};