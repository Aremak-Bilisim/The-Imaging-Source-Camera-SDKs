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
    //bool setExposure(double exposureUs);

    //// Set exposure time in milliseconds (convenience method)
    //bool setExposureMs(double exposureMs);

    //// Enable/disable auto exposure
    //bool setAutoExposure(bool enable);

    //// Get current exposure value
    //double getExposure();

    //// Get exposure range
    //bool getExposureRange(double& min, double& max);

    // Display current exposure information
    void displayExposureInfo();

    // Start grabbing frames
    bool startGrabbing();

    // Stop grabbing
    bool stopGrabbing();

    // Check if camera is connected
    bool connected() const { return isConnected; }

    // Check if camera is grabbing
    bool grabbing() const { return isGrabbing; }
};

#endif // TISCAMERAIC4_H