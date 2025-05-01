//Created by: Yin Shi & Zheng Kaiyan(line 25-55)
#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include "blob.h"
class BlobTracker {
public:
    BlobTracker(): blobCounter(0), frameCount(0), previousBlobCount10FramesAgo(0){}

    void processFrame(cv::Mat& img) {
        currentBlobs=BlobList();
        // Detect blobs of current fram
        currentBlobs.detect(img);
        // Remove the small blobs
        currentBlobs.filter(75);
        // Match currentBlobs with blobs, change the id of currentBlobs.
        currentBlobs.match(previousBlobs, blobCounter);
        // show the blobs for test
        
        // Copy current blob to previous blob;
        previousBlobs.deep_copy(currentBlobs);
        currentBlobs.check_crowd(crowdThresholdLower,crowdThresholdUpper);
        currentBlobs.show(img);
        // Update the current blob count
        int currentBlobCount = currentBlobs.size();

        // Every 10 frames, check the trend based on the blob count from 10 frames ago
        if (frameCount % 10 == 0) {
            
            // Compare current blob count with the count from 10 frames ago
            if (currentBlobCount > previousBlobCount10FramesAgo) {
                message = "The number of people is increasing.";
            }
            else if (currentBlobCount < previousBlobCount10FramesAgo) {
                message = "The number of people is decreasing.";
            }
            else {
                message = "The number of people remains the same.";
            }
            int baseline = 0;
            cv::Size textSize = getTextSize(message, cv::FONT_HERSHEY_SIMPLEX, 0.5, 2, &baseline);

            // Ensure the text fits within the image bounds
            x = std::max(0, img.cols - textSize.width - 10); // Keep 10-pixel padding from the right edge
            y = std::max(textSize.height + 10, 30);         // Ensure the text is at least 30 pixels from the top
            
            // Update the 10-frame-old blob count
            previousBlobCount10FramesAgo = currentBlobCount;
        }
        putText(img, message, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
        // Increment frame count
        frameCount++;

        // Display the image
        return;
    }

private:
    int x, y;
    BlobList previousBlobs;
    BlobList currentBlobs;
    int frameCount;
    int blobCounter;
    int crowdThresholdLower = 2000;
    int crowdThresholdUpper = 10000;
    int previousBlobCount10FramesAgo;
    std::string message;
};
