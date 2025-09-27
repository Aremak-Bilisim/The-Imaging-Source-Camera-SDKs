#ifndef TISCAMERAIC4_H
#define TISCAMERAIC4_H

#include <ic4/ic4.h>
#include <ic4-interop/interop-OpenCV.h>
#include "VisionMasterProcessor.h"
#include <string>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <atomic>

// Define QueueSinkListener-derived class that stores the latest frame
class GrabbingImage : public ic4::QueueSinkListener
{
private:
    std::mutex frame_mutex_;
    cv::Mat latest_frame_;
    std::atomic<bool> new_frame_available_{ false };
    VisionMasterProcessor processor;

public:
    GrabbingImage() = default;

    // Inherited via QueueSinkListener, called when there are frames available in the sink's output queue
    void framesQueued(ic4::QueueSink& sink) override
    {
        ic4::Error err;

        while (true)
        {
            // Remove a buffer from the sink's output queue
            auto buffer = sink.popOutputBuffer(err);
            if (buffer == nullptr)
            {
                // No more buffers available, return
                return;
            }

            auto mat = ic4interop::OpenCV::wrap(*buffer);
            processor.runProcedure();
            processor.getResults();


            // Store the latest frame with thread safety
            {
                std::lock_guard<std::mutex> lock(frame_mutex_);
                latest_frame_ = mat.clone(); // Clone to ensure we have our own copy
                new_frame_available_ = true;
            }
            
        }
    }

    float getResult() {
        CalculatorResults* res = processor.getCalculatorResults();
        return res->GetResult(0)->pFloatValue[0];
    }

    // Get the latest frame (thread-safe)
    bool getLatestFrame(cv::Mat& frame)
    {
        std::lock_guard<std::mutex> lock(frame_mutex_);
        if (!latest_frame_.empty()) {
            frame = latest_frame_.clone();
            new_frame_available_ = false;
            return true;
        }
        return false;
    }

    // Check if a new frame is available
    bool isNewFrameAvailable() const
    {
        return new_frame_available_;
    }

};

class TISCameraIC4 {
private:
    ic4::Grabber grabber;
    bool isConnected;
    bool isGrabbing;
    bool triggermodeEnabled = true;
    ic4::DeviceInfo device;
    std::string formatDeviceInfo(const ic4::DeviceInfo& device_info);

    // Exposure control variables
    double minExposure;
    double maxExposure;
    double currentExposure;
    int sliderValue;
    int width = 640;
    int height = 480;
    bool triggerModeEnabled = false;
    std::string currentTriggerSource = "Software";

    // Frame grabbing
    std::shared_ptr<GrabbingImage> frameListener;
    std::shared_ptr<ic4::QueueSink> queueSink;

    // Static callback function for trackbar
    static void onExposureChange(int value, void* userdata);

public:
    TISCameraIC4();
    ~TISCameraIC4();

    // List available cameras using IC4
    void listCameras();
    // Connect to camera by index
    bool connect(int cameraIndex = 0);
    // Disconnect camera
    void disconnect();
    // Set exposure time in microseconds
    bool setExposure(double exposureUs);
    // Get current exposure value
    double getExposure();
    // Get exposure range
    bool getExposureRange(double& min, double& max);
    // Display current exposure information
    void displayExposureInfo();
    // Start grabbing frames with parameter control window
    bool startGrabbing();
    // Stop grabbing
    bool stopGrabbing();
    // Check if camera is connected
    bool connected() const { return isConnected; }
    // Check if camera is grabbing
    bool grabbing() const { return isGrabbing; }
    bool toggleAutoExposureMode();
    // Initialize parameter control window
    void initParameterControlWindow();
    // Update slider from current exposure value
    void updateSliderFromExposure();
    void updateParameterDisplay();
    bool enableTriggerMode();
    bool disableTriggerMode();
    bool toggleTriggerMode();
    bool setTriggerSource(const std::string& source = "Software"); // "Software", "Line1", etc.
    bool sendSoftwareTrigger();
    bool isTriggerModeEnabled();
    void displayTriggerInfo();
    // Trigger configuration
    bool configureTrigger(const std::string& source = "Software", double delay = 0.0);
};
#endif // TISCAMERAIC4_H