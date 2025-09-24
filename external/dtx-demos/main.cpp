#include "TISCameraIC4.h"
#include <iostream>
#include <string>


int main() {

    ic4::initLibrary();


    TISCameraIC4 camera;
    bool running = true;

    camera.listCameras();

    int index;
    std::cout << "Enter camera index: ";
    std::cin >> index;
    camera.connect(index);

    camera.startGrabbing();
    camera.stopGrabbing();


    //ic4::exitLibrary();

    return 0;
}