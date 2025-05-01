//Template Created by : William Jonathan Kusnomo
//Algorithm Created by: Cao Kai Wei   (speed/accelerating detection)
//                      Yin Shi    (face and human detection)
//                      Chen Zhuo Kai  (Plate detection)
#pragma once
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

#include "TrackingData.h"
#include "BlobDetection.h"

#include <string>
#include <iostream>

class FaceFilters {
private:
public:

    // Method to apply grayscale filter
    static void ApplyGrayscale(cv::Mat& mat) {
        if (!mat.empty()) {
            cv::cvtColor(mat, mat, cv::COLOR_BGR2GRAY);
            cv::cvtColor(mat, mat, cv::COLOR_GRAY2BGR);
        }
    }
    static void ApplyBlurred(cv::Mat& mat) {
        if (!mat.empty()) {
            cv::GaussianBlur(mat, mat, cv::Size(15, 15), 0);
        }

    }
    // Method to apply face detection filter
    static void ApplyFaceDetection(cv::Mat& mat, cv::CascadeClassifier& faceCascade, std::vector<cv::Rect>& faces) {
        if (mat.empty()) return;
        cv::Mat matGray;
        cv::cvtColor(mat, matGray, cv::COLOR_BGR2GRAY);
        faceCascade.detectMultiScale(matGray, faces, 1.1, 2,0,cv::Size(30,30));
        for (auto& face : faces) {
            face.x += cvRound(face.width*0.1);
            face.width = cvRound(face.width * 0.8);
            face.y += cvRound(face.height*0.07);
            face.height = cvRound(face.height*0.8);
            cv::rectangle(mat,face.tl(), face.br(), cv::Scalar(0, 255, 0), 1);
        }
        cv::cvtColor(matGray, matGray, cv::COLOR_GRAY2BGR);

    }
    static void ApplyHumanDetection(cv::Mat& mat, cv::HOGDescriptor& hog, std::vector < std::pair < cv::Rect, std::string> >& humans,//Human and Speed Detection
        TrackingData& tracker, timeData& tD) {//Author: Cao Kai Wei except for 64-75
        
        if (tracker.first) {
            tD.outputInterval = 0.5;  // Time interval for output in seconds  
            tD.elapsedTime = 0.0;
            tD.prevTime = cv::getTickCount();
            tD.freq = cv::getTickFrequency();
            tracker.first = false;
        }


        //These part below is written by Steven Yin
        //human detection
        std::vector<cv::Rect> found;
        humans.clear();
        hog.detectMultiScale(mat, found, 0, cv::Size(16, 16), cv::Size(64, 64), 1.05, 2);
        size_t i, j;
        for (i = 0; i < found.size(); i++) {
            cv::Rect r = found[i];
            for (j = 0; j < found.size(); j++)
                if (j != i && (r & found[j]) == r)
                    break;
            if (j == found.size())
                humans.push_back({ r,"---" });
        }
        //These part above is written by Steven Yin

        int64 currentTime = cv::getTickCount();
        float deltaTime = (currentTime - tD.prevTime) / tD.freq;
        tD.prevTime = currentTime;
        tD.elapsedTime += deltaTime;


        for (i = 0; i < humans.size(); i++) {
            cv::Rect r = humans[i].first;
            r.x += cvRound(r.width * 0.1);
            r.width = cvRound(r.width * 0.8);
            r.y += cvRound(r.height * 0.07);
            r.height = cvRound(r.height * 0.8);
            rectangle(mat, r.tl(), r.br(), cv::Scalar(0, 255, 0), 3);

            cv::Point2f center((r.tl().x + r.br().x) / 2, (r.tl().y + r.br().y) / 2);

            if (tD.elapsedTime >= tD.outputInterval) {
                int targetID = static_cast<int>(i);
                float currentSpeed = tracker.calculateSpeed(center, targetID, deltaTime);
                float currentAcceleration = tracker.calculateAcceleration(currentSpeed, targetID, deltaTime);
                std::stringstream sstm;
                if (currentAcceleration < 0) {
                    sstm << "Target " << targetID << " decelerating"<< std::endl;
                }
                else {
                    sstm << "Target " << targetID << " accelerating" << std::endl;
                }
                std::string Message = sstm.str();
                OutputDebugStringA(Message.c_str());
                humans[i].second = Message;
                //wxMessageBox(Message);
            }
        }
    }
    static void ApplyPlateDetection(cv::Mat& img,cv::CascadeClassifier& plateCascade, std::vector<cv::Rect>& plates) {
        //Author: Cao Kai Wei
        cv::Mat imgGray;
        cv::Mat imgCopy = img.clone();
        cvtColor(imgCopy, imgGray, cv::COLOR_BGR2GRAY);
        plateCascade.detectMultiScale(imgGray, plates, 1.1, 5);
        for (int i = 0; i < plates.size(); i++) {
            cv::Rect r = plates[i];
            //basically to better frame the image size
            r.x += cvRound(r.width * 0.1);
            r.width = cvRound(r.width * 0.8);
            r.y += cvRound(r.height * 0.07);
            r.height = cvRound(r.height * 0.8);
            cv::rectangle(imgCopy, r.tl(), r.br(), cv::Scalar(255, 0, 255), 1);
        }
        img = imgCopy;
    }

};

