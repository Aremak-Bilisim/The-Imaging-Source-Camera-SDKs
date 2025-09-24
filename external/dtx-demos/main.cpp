#include "TISCameraIC4.h"
#include <iostream>
#include <string>
#include <limits>

// Function to display usage information
void showHelp() {
    std::cout << "\n=== TIS Camera Control Application (IC4) ===" << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  list        - List available cameras" << std::endl;
    std::cout << "  connect [n] - Connect to camera by index" << std::endl;
    std::cout << "  exp [us]    - Set exposure time in microseconds" << std::endl;
    std::cout << "  expms [ms]  - Set exposure time in milliseconds" << std::endl;
    std::cout << "  auto [0/1]  - Disable (0) or enable (1) auto exposure" << std::endl;
    std::cout << "  info        - Display current exposure information" << std::endl;
    std::cout << "  start       - Start grabbing frames" << std::endl;
    std::cout << "  stop        - Stop grabbing frames" << std::endl;
    std::cout << "  help        - Show this help message" << std::endl;
    std::cout << "  exit        - Exit application" << std::endl;
    std::cout << "============================================" << std::endl;
}

int main() {
    try {
        ic4::initLibrary();

        std::cout << "TIS Camera Basic Application Starting (IC4)..." << std::endl;

        TISCameraIC4 camera;
        bool running = true;

        showHelp();

        while (running) {
            std::cout << "\nEnter command: ";
            std::string command;
            std::cin >> command;

            if (command == "list") {
                camera.listCameras();
            }
            else if (command == "connect") {
                int index;
                std::cout << "Enter camera index: ";
                std::cin >> index;
                camera.connect(index);
            }
            else if (command == "exp") {
                double exposure;
                std::cout << "Enter exposure time (µs): ";
                std::cin >> exposure;
                // camera.setExposure(exposure);
            }
            else if (command == "expms") {
                double exposure;
                std::cout << "Enter exposure time (ms): ";
                std::cin >> exposure;
                // camera.setExposureMs(exposure);
            }
            else if (command == "auto") {
                int enable;
                std::cout << "Enable auto exposure (0=No, 1=Yes): ";
                std::cin >> enable;
                // camera.setAutoExposure(enable != 0);
            }
            else if (command == "info") {
                camera.displayExposureInfo();
            }
            else if (command == "start") {
                 camera.startGrabbing();
            }
            else if (command == "stop") {
                 camera.stopGrabbing();
            }
            else if (command == "help") {
                showHelp();
            }
            else if (command == "exit") {
                running = false;
            }
            else {
                std::cout << "Unknown command. Type 'help' for available commands." << std::endl;
            }

            // Clear any remaining input
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        std::cout << "Application exiting..." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}