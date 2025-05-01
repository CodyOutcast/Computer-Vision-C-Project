#pragma once
//Created by: Yin Shi & Zheng Kai Yan (line 332-346)

#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <vector>
#include <string>
class Blob {
private:
    const int distance_threshold=65;
    double* arr;
    bool crowded;  // New attribute to track crowd status
public:
    // Constructor
    Blob(double x = 0, double y = 0):crowded(false)
    {
        arr = new double[6];
        arr[0] = x;                     // Left bound (minx)
        arr[1] = x;                     // Right bound (maxx)
        arr[2] = y;                     // Lower bound (miny)
        arr[3] = y;                     // Upper bound (maxy)
        arr[4] = 0;                     // ID
        arr[5] = 0;                     // Whether is matched
    }

    // Destructor
    ~Blob() { delete[] arr; }

    // Getters
    double get_minX() { return arr[0]; }
    double get_maxX() { return arr[1]; }
    double get_minY() { return arr[2]; }
    double get_maxY() { return arr[3]; }
    double get_area() { return  (get_maxX() - get_minX()) * (get_maxY() - get_minY()); }
    double get_centerX() { return (get_maxX() + get_minX()) * 0.5; }
    double get_centerY() { return (get_maxY() + get_minY()) * 0.5; }

    int get_id() { return (int)arr[4]; }
    bool get_matched() { return (bool)arr[5]; }

    // Distance between (x, y) and the center of the blob
    double distance(double x, double y)
    {
        double cx = get_centerX();
        double cy = get_centerY();

        double dx = cx - x;
        double dy = cy - y;

        return std::sqrt(dx * dx + dy * dy);
    }

    // Setters
    void set_id(int id) { arr[4] = id; }
    void set_matched(bool status) { arr[5] = status; }

    // The add method, add the pixel in (x, y) to the blob
    void add(double x, double y)
    {
        arr[0] = std::min(arr[0], x);
        arr[2] = std::min(arr[2], y);
        arr[1] = std::max(arr[1], x);
        arr[3] = std::max(arr[3], y);
    }
    // To verify wheter a given pixel is near the blob
    bool is_near(double x, double y)
    {
        double d = distance(x, y);
        if (d < distance_threshold) { return true; }
        else { return false; }
    }

    // Show the blob and id on the screen, for the test use
    void draw(cv::Mat& img)
    {
        cv::rectangle(img, cv::Point(arr[0], arr[2]), cv::Point(arr[1], arr[3]), cv::Scalar(255, 0, 0), 3);
        putText(img, std::to_string(get_id()), cv::Point((arr[0] + arr[1]) / 2, (arr[2] + arr[3]) / 2), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
        if (crowded) {
            std::string message = "crowded";
            //message += std::to_string(this->get_area());
            putText(img, message , cv::Point((arr[0] + arr[1]) / 2, (arr[2] + arr[3]) / 2 + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
        }
        else {
            std::string message = "normal";
            //message += std::to_string(this->get_area());
            putText(img, message, cv::Point((arr[0] + arr[1]) / 2, (arr[2] + arr[3]) / 2 + 20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 0, 0), 2);
        }
    }

    // Coppy constructor
    Blob(const Blob& src) { deep_copy(src); }

    // Assignment operator
    Blob& operator= (const Blob& src)
    {
        if (this != &src)
        {
            delete[] arr;
            deep_copy(src);
        }
        return *this;
    }

    // Method to implement deep copy
    void deep_copy(const Blob& src)
    {
        arr = new double[6];
        for (int i = 0; i < 6; i++)
        {
            arr[i] = src.arr[i];
        }
    }
    bool isCrowded() { return crowded; }
    void setCrowded(bool c) {
        crowded = c; 
        return;
    }
};

class BlobList {
    //constant
    const int INITIAL_CAPACITY = 100;
    const int grayscale_threshold = 75;
    const int distance_threshold = 65;

private:
    Blob* array;            // Dynamic array of Blob
    int capacity;           // The capacity of the BlobList
    int count;              // The count keep track of the number of the elements in the array

    void expand_capacity()
        // used to expand the capacity of the array
    {
        int previous_capacity = capacity;
        int new_capacity = capacity * 2;
        capacity = new_capacity;
        Blob* tmp_array = array;
        array = new Blob[capacity];

        for (int i = 0; i < previous_capacity; i++)
        {
            array[i] = tmp_array[i];
        }
        delete[] tmp_array;
        tmp_array = NULL;
    }

public:
    // Constructor
    BlobList()
    {
        array = new Blob[INITIAL_CAPACITY];
        capacity = INITIAL_CAPACITY;
        count = 0;                                  // The index of the last non empty block
    }

    // Destructor
    ~BlobList() { delete[] array; array = nullptr; }

    // Add a new blob to the array
    void add(const Blob& b)
    {
        if (count == capacity) expand_capacity();
        array[count++] = b;
    }

    // Delete the new blob at certain position i, shrink the size
    void erase(int i)
    {
        for (int j = i; j < count; j++)
        {
            Blob tmp = array[j];
            array[j] = array[j + 1];
            array[j + 1] = tmp;
        }
        count--;
    }

    // See whether the array is empty
    bool empty() { return (count == 0); }

    // The size of the array
    int size() { return count; }

    // Detect the blobs on the image, and maintain the array
    void detect(cv::Mat img)
    {
        cv::Mat hsvImage;
        cv::cvtColor(img, hsvImage, cv::COLOR_BGR2HSV);
        for (int x = 0; x < img.cols; x++)
        {
            for (int y = 0; y < img.rows; y++)
            {
                cv::Vec3b pixel = hsvImage.at<cv::Vec3b>(y, x);

                //What is current color
                int h = pixel[0];
                int s = pixel[1];
                int v = pixel[2];

                //If current color is similar to tracked color
                if (v < grayscale_threshold)
                {
                    bool found = false;
                    for (int i = 0; i < count; i++)
                    {
                        if (array[i].is_near(x, y))
                        {
                            array[i].add(x, y);
                            found = true;
                            break;
                        }
                    }
                    if (!found) add(Blob(x, y));         // Add new blob object
                }
            }
        }
    }

    // Filter out the blob of given size (still need to modify)
    void filter(int threshold)
    {
        for (int i = count - 1; i >= 0; i--) {
            if (array[i].get_area() < threshold) {
                erase(i);
            }
        }
    }

    // Match current blobs with given blobs
    void match(BlobList& src, int& blob_counter) {
        /*
         * This function handles blob tracking between frames.
         * It assigns IDs to new blobs and matches them with existing ones based on proximity.
         */

         // Helper lambda to calculate Euclidean distance
        auto calculate_distance = [](double x1, double y1, double x2, double y2) -> double
            {
                double dx = x2 - x1;
                double dy = y2 - y1;
                return std::sqrt(dx * dx + dy * dy);
            };

        if (src.size() == 0)
        {
            // No previous blobs, assign new IDs to all current blobs
            for (int i = 0; i < count; i++) { array[i].set_id(blob_counter++); }
            return;
        }

        // Reset "matched" status for all current blobs
        for (int i = 0; i < count; i++) { array[i].set_matched(false); }

        // Match blobs from `src` to the current list
        for (int i = 0; i < src.size(); i++) {
            double min_distance = std::numeric_limits<double>::max();
            int best_match = -1;

            for (int j = 0; j < count; j++) {
                if (!array[j].get_matched()) {
                    // Calculate distance between the centers of the blobs
                    double dist = calculate_distance(
                        array[j].get_centerX(), array[j].get_centerY(),
                        src.array[i].get_centerX(), src.array[i].get_centerY()
                    );

                    // Find the closest unmatched blob
                    if (dist < min_distance && dist < distance_threshold) {
                        min_distance = dist;
                        best_match = j;
                    }
                }
            }

            // Assign ID from the previous blob to the current blob
            if (best_match != -1)
            {
                array[best_match].set_matched(true);
                array[best_match].set_id(src.array[i].get_id());
            }
        }

        // Assign new IDs to any remaining unmatched blobs
        for (int i = 0; i < count; i++)
        {
            if (!array[i].get_matched()) {
                array[i].set_id(blob_counter++);
            }
        }
    }


    // Show the blobs for test use
    void show(cv::Mat img)
    {
        for (int i = 0; i <= count; i++)
        {
            array[i].draw(img);
        }
    }
    void writeStatus(cv::Mat img, std::string message) {
        putText(img, message, cv::Point(img.cols - 200, 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0), 2);
    }
    // Copy constructor
    BlobList(const BlobList& src) { deep_copy(src); }

    // Assignment operator
    BlobList& operator=(const BlobList& src)
    {
        if (this != &src)
        {
            delete[] array;
            deep_copy(src);
        }
        return *this;
    }

    // Method to implement deep copy
    void deep_copy(const BlobList& src)
    {
        capacity = src.capacity;
        count = src.count;

        array = new Blob[capacity];
        for (int i = 0; i < count; i++)
        {
            array[i] = src.array[i];
        }
    }
    void check_crowd(double crowdThresholdLower, double crowdThresholdUpper) {
        for (int i=0;i<=count;i++){
            double blobSize = array[i].get_area();
            if (blobSize >= crowdThresholdLower && blobSize <= crowdThresholdUpper)
            {
                array[i].setCrowded(true);
                //OutputDebugStringA(message.c_str());
            }
            else
            {
                array[i].setCrowded(false);
                //OutputDebugStringA(message.c_str());
            }
        }
    }
   
};