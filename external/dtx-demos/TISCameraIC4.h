#ifndef TISCAMERAIC4_H
#define TISCAMERAIC4_H

#include <ic4/ic4.h>
#include <ic4-interop/interop-OpenCV.h>
#include <string>
#include <opencv2/opencv.hpp>




// Define QueueSinkListener-derived class that saves all received frames in bitmap files
class GrabbingImage : public ic4::QueueSinkListener
{
private:
    int counter_;
    bool window_created_;

public:
    GrabbingImage() :
        counter_(0), window_created_(false)
    {
    }

    // Inherited via QueueSinkListener, called when there are frames available in the sink's output queue
    void framesQueued(ic4::QueueSink& sink) override
    {
        ic4::Error err;

        // Create the window on first frame arrival
        if (!window_created_)
        {
            cv::namedWindow("display");
            window_created_ = true;
        }

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
            cv::imwrite("a.png", mat);
            cv::imshow("display", mat);

            // You'll need this to actually display the window and handle events
            // Use a small delay instead of 0 to keep the window responsive
            cv::waitKey(1);

            std::cout << "Count: " << counter_ << std::endl;
            counter_ += 1;
        }
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
    bool setTriggerSource(const std::string& source = "Software"); // "Software", "Line1", etc.
    bool sendSoftwareTrigger();
    bool isTriggerModeEnabled();
    void displayTriggerInfo();
    // Trigger configuration
    bool configureTrigger(const std::string& source = "Software", double delay = 0.0);
};

#endif // TISCAMERAIC4_H