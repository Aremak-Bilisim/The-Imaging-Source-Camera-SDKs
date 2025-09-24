#ifndef TISCAMERAIC4_H
#define TISCAMERAIC4_H

#include <ic4/ic4.h>
#include <ic4-interop/interop-OpenCV.h>
#include <string>
#include <opencv2/opencv.hpp>

class TISCameraIC4 {
private:
    ic4::Grabber grabber;
    bool isConnected;
    bool isGrabbing;

    ic4::DeviceInfo device;

    std::string formatDeviceInfo(const ic4::DeviceInfo& device_info);

    // Exposure control variables
    double minExposure;
    double maxExposure;
    double currentExposure;
    int sliderValue;

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
};

#endif // TISCAMERAIC4_H