#include "TISCameraIC4.h"
#include <iostream>
#include <string>
#include <iomanip>

TISCameraIC4::TISCameraIC4() : isConnected(false), isGrabbing(false),
minExposure(0), maxExposure(100000),
currentExposure(5000), sliderValue(50) {
}

TISCameraIC4::~TISCameraIC4() {
    disconnect();
}



// Initialize parameter control window
void TISCameraIC4::initParameterControlWindow() {
    // Create a resizable window
    cv::namedWindow("Parameter Control", cv::WINDOW_NORMAL);

    // Resize window to something bigger
    cv::resizeWindow("Parameter Control", 800, 200);

    // Get exposure range
    getExposureRange(minExposure, maxExposure);
    updateSliderFromExposure();

    // Create exposure slider with large range
    cv::createTrackbar("Exposure (µs)", "Parameter Control",
        &sliderValue, 100000,
        onExposureChange, this);

    // Make a larger control panel image
    cv::Mat controlPanel = cv::Mat::zeros(200, 800, CV_8UC3);

    // Display current exposure value instead of slider value
    std::string exposureText = "Current Exposure: " +
        std::to_string(currentExposure / 1000.0).substr(0, 6) +
        " ms";

    cv::putText(controlPanel, exposureText, cv::Point(20, 60),
        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(controlPanel, "ESC to exit", cv::Point(20, 120),
        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(200, 200, 200), 2);

    cv::imshow("Parameter Control", controlPanel);
}

// Update slider position from current exposure
void TISCameraIC4::updateSliderFromExposure() {
    if (maxExposure > minExposure) {
        double exposureRange = maxExposure - minExposure;

        // Map current exposure into 0–100000 range
        sliderValue = static_cast<int>(
            (currentExposure - minExposure) / exposureRange * 100000.0);

        sliderValue = std::max(0, std::min(100000, sliderValue));
    }
}

// Add this method to update the display with current exposure value
void TISCameraIC4::updateParameterDisplay() {
    cv::Mat controlPanel = cv::Mat::zeros(200, 800, CV_8UC3);

    std::string exposureText = "Current Exposure: " + std::to_string(currentExposure / 1000.0).substr(0, 6) +
        " ms";

    cv::putText(controlPanel, exposureText, cv::Point(20, 60),
        cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
    cv::putText(controlPanel, "ESC to exit", cv::Point(20, 120),
        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(200, 200, 200), 2);

    cv::imshow("Parameter Control", controlPanel);
}

// Also update the onExposureChange to refresh the display
void TISCameraIC4::onExposureChange(int value, void* userdata) {
    TISCameraIC4* camera = static_cast<TISCameraIC4*>(userdata);

    // Convert slider value (0–100000) to exposure range
    double exposureRange = camera->maxExposure - camera->minExposure;
    double newExposure = camera->minExposure +
        (exposureRange * value / 10000000.0);  // Fixed: removed extra zero

    camera->currentExposure = newExposure;
    camera->setExposure(newExposure);

    // Update the display with the new exposure value
    camera->updateParameterDisplay();

    std::cout << "Exposure set to: " << newExposure << " μs ("
        << (newExposure / 1000.0) << " ms)" << std::endl;
}

// List available cameras using IC4
void TISCameraIC4::listCameras() {
    std::cout << "Enumerating all attached video capture devices..." << std::endl;

    auto device_list = ic4::DeviceEnum::enumDevices();

    if (device_list.empty()) {
        std::cout << "No devices found" << std::endl;
        return;
    }

    std::cout << "Found " << device_list.size() << " devices:" << std::endl;

    for (size_t i = 0; i < device_list.size(); ++i) {
        auto&& dev_info = device_list[i];
        std::cout << "[" << i << "] " << formatDeviceInfo(dev_info) << std::endl;
    }
    std::cout << std::endl;
}

// Connect to camera by index
bool TISCameraIC4::connect(int cameraIndex) {
    try {
        auto device_list = ic4::DeviceEnum::enumDevices();

        if (device_list.empty()) {
            std::cerr << "No devices found." << std::endl;
            return false;
        }

        if (cameraIndex < 0 || cameraIndex >= device_list.size()) {
            std::cerr << "Camera index " << cameraIndex << " out of range." << std::endl;
            return false;
        }

        ic4::Error err;
        auto&& dev_info = device_list[cameraIndex];

        // Open the device
        if (!grabber.deviceOpen(dev_info, err)) {
            std::cerr << "Failed to open camera device." << std::endl;
            return false;
        }

        auto map = grabber.devicePropertyMap(err);
        if (err.isError()) {
            std::cerr << "Failed to get device property map: " << err.message() << std::endl;
            return false;
        }

        map.setValue(ic4::PropId::UserSetSelector, "Default", ic4::Error::Ignore());
        map.executeCommand(ic4::PropId::UserSetLoad, ic4::Error::Ignore());

        isConnected = true;
        std::cout << "Connected to camera: " << formatDeviceInfo(dev_info) << std::endl;

        // Display current exposure settings
        displayExposureInfo();
        toggleAutoExposureMode();

        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Error connecting to camera: " << e.what() << std::endl;
        return false;
    }
}

// Disconnect camera
void TISCameraIC4::disconnect() {
    if (isConnected) {
        grabber.deviceClose();
        isConnected = false;
        std::cout << "Camera disconnected." << std::endl;
    }
}

bool TISCameraIC4::toggleAutoExposureMode() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();
        map.setValue(ic4::PropId::ExposureAuto, "Off");
        displayExposureInfo();
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error setting exposure: " << e.what() << std::endl;
        return false;
    }
}

// Set exposure time in microseconds
bool TISCameraIC4::setExposure(double exposureUs) {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();
        map.setValue(ic4::PropId::ExposureTime, exposureUs);
        currentExposure = exposureUs;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error setting exposure: " << e.what() << std::endl;
        return false;
    }
}

double TISCameraIC4::getExposure() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return -1.0;
    }

    try {
        auto map = grabber.devicePropertyMap();
        auto propExposureTime = map.find(ic4::PropId::ExposureTime);
        if (propExposureTime.is_valid() && propExposureTime.isAvailable()) {
            currentExposure = propExposureTime.getValue();
            return currentExposure;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting exposure: " << e.what() << std::endl;
    }
    return -1.0;
}

// Get exposure range
bool TISCameraIC4::getExposureRange(double& min, double& max) {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();
        auto propExposureTime = map.find(ic4::PropId::ExposureTime);
        if (propExposureTime.is_valid() && propExposureTime.isAvailable()) {
            min = propExposureTime.minimum();
            max = propExposureTime.maximum();
            minExposure = min;
            maxExposure = max;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error getting exposure range: " << e.what() << std::endl;
    }
    return false;
}

// Display current exposure information
void TISCameraIC4::displayExposureInfo() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return;
    }

    try {
        ic4::Error err;
        auto map = grabber.devicePropertyMap(err);
        if (err.isError()) {
            std::cerr << "Failed to get device property map: " << err.message() << std::endl;
            return;
        }

        std::cout << std::endl << "=== Exposure Information ===" << std::endl;

        auto propExposureTime = map.find(ic4::PropId::ExposureTime);
        if (propExposureTime.is_valid()) {
            if (propExposureTime.isAvailable()) {
                double currentValue = propExposureTime.getValue();
                std::cout << "Current Exposure: " << currentValue << " microseconds ("
                    << (currentValue / 1000.0) << " ms)" << std::endl;

                double minVal = propExposureTime.minimum();
                double maxVal = propExposureTime.maximum();
                double step = propExposureTime.increment();
                std::cout << "Exposure Range: " << minVal << " microseconds to " << maxVal << " microseconds ("
                    << (minVal / 1000.0) << " ms to " << (maxVal / 1000.0) << " ms)" << std::endl;
                std::cout << "Step size: " << step << " microseconds" << std::endl;
            }
        }
        else {
            std::cout << "ExposureTime control not available." << std::endl;
        }

        auto propAuto = map.find(ic4::PropId::ExposureAuto);
        if (propAuto.is_valid() && propAuto.isAvailable()) {
            std::string autoMode = propAuto.getValue(err);
            if (!err.isError()) {
                std::cout << "Auto Exposure Mode: " << autoMode << std::endl;
            }
        }

        std::cout << "============================" << std::endl << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error displaying exposure info: " << e.what() << std::endl;
    }
}

// Start grabbing frames with parameter control window
bool TISCameraIC4::startGrabbing() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    if (isGrabbing) {
        std::cout << "Already grabbing." << std::endl;
        return true;
    }

    try {
        // Initialize the parameter control window
        initParameterControlWindow();

        grabber.devicePropertyMap().setValue(ic4::PropId::Width, width);
        grabber.devicePropertyMap().setValue(ic4::PropId::Height, height);

        ic4::Error err;

        // Create the frame listener
        frameListener = std::make_shared<GrabbingImage>();

        // Create the queue sink with our listener
        queueSink = ic4::QueueSink::create(*frameListener, ic4::PixelFormat::BGR8, err);

        if (!queueSink) {
            std::cerr << "Failed to create sink: " << err.message() << std::endl;
            return false;
        }

        // Start the video stream into the sink
        if (!grabber.streamSetup(queueSink, ic4::StreamSetupOption::AcquisitionStart, err)) {
            std::cerr << "Failed to setup stream: " << err.message() << std::endl;
            return false;
        }

        isGrabbing = true;

        std::cout << "Starting camera feed. Use the exposure slider in 'Parameter Control' window." << std::endl;
        std::cout << "Press ESC to exit, 't' to trigger (if trigger mode enabled)..." << std::endl;

        // Create display window
        cv::namedWindow("Camera Feed", cv::WINDOW_NORMAL);
        cv::resizeWindow("Camera Feed", width, height);

        // Main display loop
        while (isGrabbing) {
            cv::Mat currentFrame;

            // Get the latest frame from the listener
            if (frameListener->getLatestFrame(currentFrame)) {
                // Display the frame
                cv::imshow("Camera Feed", currentFrame);
            }
            else {
                // If no frame available, show a waiting message
                cv::Mat waitingImage = cv::Mat::zeros(480, 640, CV_8UC3);
                cv::putText(waitingImage, "Waiting for frames...", cv::Point(50, 240),
                    cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2);
                cv::imshow("Camera Feed", waitingImage);
            }

            // Handle key presses
            int key = cv::waitKey(1) & 0xFF;

            // Check for ESC key to exit
            if (key == 27) {  // ESC key
                std::cout << "ESC key pressed. Exiting display loop..." << std::endl;
                break;
            }

            // Handle software trigger if trigger mode is enabled
            if ((key == 't' || key == 'T') && triggerModeEnabled) {
                if (!sendSoftwareTrigger()) {
                    std::cerr << "Failed to send software trigger." << std::endl;
                }
            }

            // Handle exposure changes from slider (OpenCV trackbar callbacks are handled automatically)
            // Update parameter display periodically
            static int displayUpdateCounter = 0;
            if (displayUpdateCounter++ % 30 == 0) { // Update every 30 frames
                updateParameterDisplay();

                // Optional: Print frame rate info
                static auto lastTime = std::chrono::steady_clock::now();
                auto currentTime = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime).count();

                if (elapsed > 1000) { // Every second
                    int frameCount = frameListener->getFrameCount();
                    std::cout << "Frames received: " << frameCount << std::endl;
                    lastTime = currentTime;
                }
            }

            // Small delay to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        // Cleanup after loop exits
        isGrabbing = false;
        cv::destroyWindow("Camera Feed");
        cv::destroyWindow("Parameter Control");

        // Stop the stream
        if (!grabber.streamStop(err)) {
            std::cerr << "Error stopping stream: " << err.message() << std::endl;
        }

        std::cout << "Stopped grabbing frames." << std::endl;
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Error during grabbing: " << e.what() << std::endl;

        // Cleanup on error
        cv::destroyAllWindows();
        try {
            grabber.streamStop();
        }
        catch (...) {
            // Ignore cleanup errors
        }
        isGrabbing = false;

        return false;
    }
}
// Stop grabbing
bool TISCameraIC4::stopGrabbing() {
    if (!isGrabbing) {
        return true;
    }

    try {
        grabber.streamStop();
        isGrabbing = false;
        std::cout << "Stopped grabbing frames." << std::endl;
        return true;

    }
    catch (const std::exception& e) {
        std::cerr << "Error stopping grab: " << e.what() << std::endl;
        return false;
    }
}

std::string TISCameraIC4::formatDeviceInfo(const ic4::DeviceInfo& device_info) {
    return "Model: " + device_info.modelName() + " Serial: " + device_info.serial() + " Version: " + device_info.version();
}

// Enable trigger mode
bool TISCameraIC4::enableTriggerMode() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();

        // Set trigger mode to On
        map.setValue(ic4::PropId::TriggerMode, "On");

        triggerModeEnabled = true;
        std::cout << "Trigger mode enabled." << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error enabling trigger mode: " << e.what() << std::endl;
        return false;
    }
}

// Disable trigger mode (free-run mode)
bool TISCameraIC4::disableTriggerMode() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();

        // Set trigger mode to Off
        map.setValue(ic4::PropId::TriggerMode, "Off");

        triggerModeEnabled = false;
        std::cout << "Trigger mode disabled (free-run mode)." << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error disabling trigger mode: " << e.what() << std::endl;
        return false;
    }
}

// Set trigger source
bool TISCameraIC4::setTriggerSource(const std::string& source) {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();

        // First set trigger selector to frame start (most common)
        map.setValue(ic4::PropId::TriggerSelector, "FrameStart");

        // Set trigger source
        if (!map.setValue(ic4::PropId::TriggerSource, &source));
            std::cout << "ERROR";

        currentTriggerSource = source;
        std::cout << "Trigger source set to: " << source << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error setting trigger source: " << e.what() << std::endl;
        return false;
    }
}

// Send software trigger
bool TISCameraIC4::sendSoftwareTrigger() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    if (!triggerModeEnabled) {
        std::cerr << "Trigger mode not enabled." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();

        // Execute software trigger command
        if (!map.executeCommand(ic4::PropId::TriggerSoftware)) {
            std::cout << "ERROR" << std::endl;
                
        }

        std::cout << "Software trigger sent." << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error sending software trigger: " << e.what() << std::endl;
        return false;
    }
}

// Check if trigger mode is enabled
bool TISCameraIC4::isTriggerModeEnabled() {
    if (!isConnected) {
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();
        auto prop = map.find(ic4::PropId::TriggerMode);

        if (prop.is_valid() && prop.isAvailable()) {
            std::string mode = prop.getValue();
            triggerModeEnabled = (mode == "On");
            return triggerModeEnabled;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error checking trigger mode: " << e.what() << std::endl;
    }

    return false;
}

// Configure trigger with common settings
bool TISCameraIC4::configureTrigger(const std::string& source, double delay) {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {
        auto map = grabber.devicePropertyMap();
        map.setValue(ic4::PropId::TriggerMode, "Off");

        ic4::Error err;

        if (delay > 0.0) {
            map.setValue(ic4::PropId::TriggerDelay, delay);
        }

        // Enable trigger mode
        map.setValue(ic4::PropId::TriggerMode, "On");

        triggerModeEnabled = true;
        currentTriggerSource = source;

        std::cout << "Trigger configured - Source: " << source
            << ", Delay: " << delay << " μs" << std::endl;

        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Error configuring trigger: " << e.what() << std::endl;
        return false;
    }
}

// Display trigger information
void TISCameraIC4::displayTriggerInfo() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return;
    }

    try {
        auto map = grabber.devicePropertyMap();

        std::cout << std::endl << "=== Trigger Information ===" << std::endl;

        // Trigger Mode
        auto propMode = map.find(ic4::PropId::TriggerMode);
        if (propMode.is_valid() && propMode.isAvailable()) {
            std::string mode = propMode.getValue();
            std::cout << "Trigger Mode: " << mode << std::endl;
        }

        // Trigger Source
        auto propSource = map.find(ic4::PropId::TriggerSource);
        if (propSource.is_valid() && propSource.isAvailable()) {
            std::string source = propSource.getValue();
            std::cout << "Trigger Source: " << source << std::endl;
        }

        // Trigger Selector
        auto propSelector = map.find(ic4::PropId::TriggerSelector);
        if (propSelector.is_valid() && propSelector.isAvailable()) {
            std::string selector = propSelector.getValue();
            std::cout << "Trigger Selector: " << selector << std::endl;
        }

        // Trigger Delay
        auto propDelay = map.find(ic4::PropId::TriggerDelay);
        if (propDelay.is_valid() && propDelay.isAvailable()) {
            double delay = propDelay.getValue();
            std::cout << "Trigger Delay: " << delay << " μs" << std::endl;
        }

        std::cout << "===========================" << std::endl << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error displaying trigger info: " << e.what() << std::endl;
    }
}