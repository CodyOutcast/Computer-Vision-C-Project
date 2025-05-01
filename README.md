# Computer-Vision-C-Project
Our team has decided to develop a C++ program that provides automated analysis of objects, specifically humans and vehicles, in video footage and real-time camera views using the OpenCV library. This program will be designed to extract, store, and manipulate video data captured in locations such as mall entrances, highways, and sidewalks.

Contribution（Each member accounts for 20%)
William Jonathan Kusnomo 123040050:
Packaged the aforementioned functions, organized them into logical groups, and assigned them to the designated interface.  
Developed a user-friendly interface with toolboxes that enable users to apply various operations, input parameters, and select different analysis modes using wxWidgets library.  

Zhuokai Chen
Composed the project proposal and project report.
Implemented heatmap visualization to indicate the frequency of object passage.  
Included an option to extract cropped images of selected objects and save them to local devices. 

Kaiwei Cao
Created a matrix transformation algorithm to calculate the approximate acceleration of an object, which was later used to indicate motion status.

Shi Yin
Implemented face detection and object tracking pre-trained models.
Wrote a blob detection class which was used to count objects and analyze object status.

Kaiyan Zheng
Building on top of the blob detection class, he created an algorithm to assess the status of whether a blob is crowded and the change of object amount in a range of frame.
