//Created by: Cao Kai Wei 
#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <unordered_map>
struct timeData {
    double outputInterval;  // Time interval for output in seconds  
    double elapsedTime;
    int64 prevTime;
    double freq;
};

class TrackingData {
private:
    cv::Mat perspectiveMatrix;
    std::unordered_map<int, cv::Point2f> prevPositions;
    std::unordered_map<int, float> prevSpeeds;

public:
    bool first;// Boolena to initalize timeData initial variable
    TrackingData() {
        first = true;
        initPerspectiveMatrix();
    }

    void initPerspectiveMatrix() {
        cv::Point2f srcPoints[4] = { cv::Point2f(0.25, 0), cv::Point2f(2.75, 0),
                                     cv::Point2f(0.75, 1.5), cv::Point2f(2.25, 1.5) };
        cv::Point2f dstPoints[4] = { cv::Point2f(0, 0), cv::Point2f(2, 0),
                                     cv::Point2f(0, 2), cv::Point2f(2, 2) };
        perspectiveMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    }

    float calculateSpeed(cv::Point2f currentCenter, int targetID, float deltaTime, float actualHeight = 1.71f) {
        if (perspectiveMatrix.empty()) {
            initPerspectiveMatrix();
        }

        float pixelHeight = actualHeight / currentCenter.y;

        std::vector<cv::Point2f> src = { currentCenter };
        std::vector<cv::Point2f> dst;
        cv::perspectiveTransform(src, dst, perspectiveMatrix);
        dst[0].y *= pixelHeight;

        if (prevPositions.find(targetID) == prevPositions.end()) {
            prevPositions[targetID] = dst[0];
            return 0.0f;
        }

        cv::Point2f prevPosition = prevPositions[targetID];
        float distance = sqrt(pow(dst[0].x - prevPosition.x, 2) + pow(dst[0].y - prevPosition.y, 2));
        prevPositions[targetID] = dst[0];

        return distance / deltaTime;
    }

    float calculateAcceleration(float currentSpeed, int targetID, float deltaTime) {
        if (prevSpeeds.find(targetID) == prevSpeeds.end()) {
            prevSpeeds[targetID] = currentSpeed;
            return 0.0f;
        }

        float prevSpeed = prevSpeeds[targetID];
        float acceleration = (currentSpeed - prevSpeed) / deltaTime;
        prevSpeeds[targetID] = currentSpeed;

        return acceleration;
    }
};