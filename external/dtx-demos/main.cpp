#include "TISCameraIC4.h"
#include <iostream>
#include <string>
#include <thread>   // for sleep
#include <chrono>

int main() {
    ic4::initLibrary();

    TISCameraIC4 camera;
    camera.listCameras();

    int index;
    std::cout << "Enter camera index: ";
    std::cin >> index;

    if (!camera.connect(index)) {
        std::cerr << "Failed to connect to camera." << std::endl;
        return -1;
    }

    

    camera.displayTriggerInfo();
    if (!camera.configureTrigger("Software", 0.0)) {
        std::cerr << "Failed to configure trigger." << std::endl;
        return -1;
    }

    if (!camera.startGrabbing()) {
        std::cerr << "Failed to start grabbing." << std::endl;
        return -1;
    }
    

    camera.stopGrabbing();

    ic4::exitLibrary(); // don’t forget cleanup
    return 0;
}