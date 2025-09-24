#include "TISCameraIC4.h"
#include <iostream>
#include <string>
#include <iomanip>

TISCameraIC4::TISCameraIC4() : isConnected(false), isGrabbing(false) {}

TISCameraIC4::~TISCameraIC4() {
    // stopGrabbing();
    disconnect();
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
            return -3;
		}
        
        map.setValue(ic4::PropId::UserSetSelector, "Default", ic4::Error::Ignore());
        map.executeCommand(ic4::PropId::UserSetLoad, ic4::Error::Ignore());
        
        
        
        isConnected = true;
        std::cout << "Connected to camera: " << formatDeviceInfo(dev_info) << std::endl;
        
        // Display current exposure settings
        displayExposureInfo();
        toggleAutoExposureMode();
        
        return true;
        
    } catch (const std::exception& e) {
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
        displayExposureInfo();

        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error setting exposure: " << e.what() << std::endl;
        return false;
    }
}

double TISCameraIC4::getExposure() {
    if (!isConnected) {
        std::cerr << "Camera not connected." << std::endl;
        return false;
    }

    try {

        auto map = grabber.devicePropertyMap();
        auto propExposureTime = map.find(ic4::PropId::ExposureTime);
        if (propExposureTime.is_valid() && propExposureTime.isAvailable()) {
                return propExposureTime.getValue();
                 
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error setting exposure: " << e.what() << std::endl;
        return false;
    }
}

//// Set exposure time in milliseconds (convenience method)
//bool TISCameraIC4::setExposureMs(double exposureMs) {
//    return setExposure(exposureMs * 1000.0);
//}
//
//// Enable/disable auto exposure
//bool TISCameraIC4::setAutoExposure(bool enable) {
//    if (!isConnected) {
//        std::cerr << "Camera not connected." << std::endl;
//        return false;
//    }
//    
//    try {
//        auto propAutoExposure = device.getProperty(ic4::PropId::ExposureAuto);
//        if (!propAutoExposure.isValid()) {
//            std::cerr << "Auto exposure property not available." << std::endl;
//            return false;
//        }
//        
//        // Different cameras might have different auto modes
//        // Try to set to continuous auto or off
//        if (enable) {
//            if (propAutoExposure.hasValue("Continuous")) {
//                if (!propAutoExposure.trySetValue("Continuous")) {
//                    std::cerr << "Failed to enable continuous auto exposure." << std::endl;
//                    return false;
//                }
//            } else if (propAutoExposure.hasValue("On")) {
//                if (!propAutoExposure.trySetValue("On")) {
//                    std::cerr << "Failed to enable auto exposure." << std::endl;
//                    return false;
//                }
//            } else {
//                std::cerr << "Auto exposure mode not supported." << std::endl;
//                return false;
//            }
//        } else {
//            if (!propAutoExposure.trySetValue("Off")) {
//                std::cerr << "Failed to disable auto exposure." << std::endl;
//                return false;
//            }
//        }
//        
//        std::cout << "Auto exposure " << (enable ? "enabled" : "disabled") << std::endl;
//        displayExposureInfo();
//        return true;
//        
//    } catch (const std::exception& e) {
//        std::cerr << "Error setting auto exposure: " << e.what() << std::endl;
//        return false;
//    }
//}
//
//// Get current exposure value
//double TISCameraIC4::getExposure() {
//    if (!isConnected) {
//        std::cerr << "Camera not connected." << std::endl;
//        return -1.0;
//    }
//    
//    try {
//        auto propExposure = device.getProperty(ic4::PropId::ExposureTime);
//        if (!propExposure.isValid() || !propExposure.isAvailable()) {
//            return -1.0;
//        }
//        
//        double value;
//        if (propExposure.tryGetValue(value)) {
//            return value;
//        }
//        return -1.0;
//        
//    } catch (const std::exception& e) {
//        std::cerr << "Error getting exposure: " << e.what() << std::endl;
//        return -1.0;
//    }
//}
//
//// Get exposure range
//bool TISCameraIC4::getExposureRange(double& min, double& max) {
//    if (!isConnected) {
//        std::cerr << "Camera not connected." << std::endl;
//        return false;
//    }
//    
//    try {
//        auto propExposure = device.getProperty(ic4::PropId::ExposureTime);
//        if (!propExposure.isValid() || !propExposure.isAvailable()) {
//            return false;
//        }
//        
//        ic4::PropertyInfo info;
//        if (propExposure.getInfo(info)) {
//            min = info.minimum;
//            max = info.maximum;
//            return true;
//        }
//        return false;
//        
//    } catch (const std::exception& e) {
//        std::cerr << "Error getting exposure range: " << e.what() << std::endl;
//        return false;
//    }
//}


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

        // Manual exposure time property
        auto propExposureTime = map.find(ic4::PropId::ExposureTime);
        if (propExposureTime.is_valid()) {
            if (propExposureTime.isAvailable()) {
                double currentValue = propExposureTime.getValue();
                if (!err.isError()) {
                    std::cout << "Current Exposure: " << currentValue << " mircosecond ("
                        << (currentValue / 1000.0) << " ms)" << std::endl;
                }

                
                double minVal = propExposureTime.minimum();
                double maxVal = propExposureTime.maximum();
                double step = propExposureTime.increment();
                        std::cout << "Exposure Range: " << propExposureTime.minimum() << " mircosecond to " << propExposureTime.maximum() << " mircosecond ("
                            << (minVal / 1000.0) << " ms to " << (maxVal / 1000.0) << " ms)" << std::endl;
                        std::cout << "Step size: " << step << " mircosecond" << std::endl;
                    
                }

            }
            else {
                std::cout << "ExposureTime control not available." << std::endl;
            }
        

        // Auto exposure mode (enumeration)
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

// Start grabbing frames
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
        cv::namedWindow("display");

        grabber.devicePropertyMap().setValue(ic4::PropId::Width, 640);
        grabber.devicePropertyMap().setValue(ic4::PropId::Height, 480);

        auto sink = ic4::SnapSink::create(ic4::PixelFormat::BGR8);
        grabber.streamSetup(sink);

        double exp = 5000.0;

        while (true)
        {   // Snap image from running data stream
            auto buffer = sink->snapSingle(1000);

            // Create a cv::Mat pointing into the BGR8 buffer
            auto mat = ic4interop::OpenCV::wrap(*buffer);

            cv::imshow("display", mat);
            int key = cv::waitKey(1);
            if (key == 27) { 
                std::cout << "ESC key pressed. Exiting display loop..." << std::endl;
                break;
            }
            else if (key == 'i') {
				exp += 1000.0;
                setExposure(exp);
            }
            else if (key == 'd') {

                exp -= 1000.0;
                setExposure(exp);
            }
        }

        isGrabbing = false;
        std::cout << "Stopped grabbing frames." << std::endl;
        return true;

    }

    catch (const std::exception& e) {
        std::cerr << "Error during grabbing: " << e.what() << std::endl;

        // Cleanup on error
        cv::destroyAllWindows();
        grabber.acquisitionStop();
        grabber.streamStop();
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
        
    } catch (const std::exception& e) {
        std::cerr << "Error stopping grab: " << e.what() << std::endl;
        return false;
    }
}

std::string TISCameraIC4::formatDeviceInfo(const ic4::DeviceInfo& device_info) {
    return "Model: " + device_info.modelName() + " Serial: " + device_info.serial() + " Version: " + device_info.version();
}