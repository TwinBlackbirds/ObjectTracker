#include <iostream>


#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/core/mat.hpp>
#include <opencv4/opencv2/xobjdetect.hpp>
#include <opencv4/opencv2/core/types.hpp>
#include <opencv4/opencv2/opencv.hpp>

#include <sys/shm.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

int main() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open display" << std::endl;
        return 1;
    }

    // Get the root window (main window that contains all screens)
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);
    int width = 1000;
    int height = 800;

    /*
     * Share memory between X server and OpenCV (verify)
     */
    // store shared memory info
    XShmSegmentInfo shmInfo;
    // create shared memory image
    // display, visual, depth, format, data, shmInfo, width, height
    XImage* image = XShmCreateImage(display, DefaultVisual(display, screen), 24, ZPixmap, nullptr, &shmInfo, width, height);

    // get shared memory, (private means new), size in bytes, and create/permissions
    shmInfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
    // route shared memory addr
    shmInfo.shmaddr = image->data = static_cast<char *>(shmat(shmInfo.shmid, nullptr, 0));
    // allow writing
    shmInfo.readOnly = false;

    if (!XShmAttach(display, &shmInfo)) {
        std::cerr << "Failed to attach shared memory segment" << std::endl;
        return 1;
    }

    // tracking loop
    while (true) {
        // capture the screen
        XShmGetImage(display, root, image, 0, 0, AllPlanes);
        cv::Mat frame(height, width, 24, image->data);

        // Convert frame from BGRA to HSV for color tracking
        cv::Mat hsv;
        cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);

        // Set color values for tracking (since it's color tracking)
        // Here we use black
        cv::Scalar lower(0, 0, 0);   // Lower HSV threshold (Hue, Sat, Value)
        cv::Scalar upper(0, 0, 0); // Upper HSV threshold


        // Create mask
        cv::Mat mask;
        cv::inRange(hsv, lower, upper, mask);

        // Find contours (outlines) of objects in the mask
        std::vector<std::vector<cv::Point>> contours;
        // Find out what RETR_EXTERNAL and CHAIN_APPROX_SIMPLE do
        cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Draw bounding boxes according to found contours
        for (const auto& contour : contours) {
            if (contourArea(contour) > 500) {  // Ignore small noise
                // detect bounding box
                cv::Rect boundingBox = cv::boundingRect(contour);
                // detect center point of object
                // cv::Point center(boundingBox.x + boundingBox.width / 2, boundingBox.y + boundingBox.height / 2);

                // Draw bounding box
                cv::rectangle(frame, boundingBox, cv::Scalar(0, 255, 0), 2);
                // Draw center point
                // cv::circle(frame, center, 5, cv::Scalar(255, 0, 0), -1);
            }
        }

        // Show the window which replicates the display, but shows trackers
        imshow("Object Tracker", frame);
        // Update this so that it saves it as a recording instead

        if (cv::waitKey(1) == 'q') break;
    }

    XCloseDisplay(display);
    return 0;
}