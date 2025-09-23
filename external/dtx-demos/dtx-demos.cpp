#include <ic4/ic4.h>
#include <iostream>
#include <random>
#include <opencv2/opencv.hpp>


std::string format_device_info(const ic4::DeviceInfo& device_info)
{
    return "Model: " + device_info.modelName() + " Serial: " + device_info.serial() + " Version: " + device_info.version();
}

void print_device_list()
{
    std::cout << "Enumerating all attached video capture devices in a single list..." << std::endl;

    auto device_list = ic4::DeviceEnum::enumDevices();

    if (device_list.empty())
    {
        std::cout << "No devices found" << std::endl;
        return;
    }

    std::cout << "Found " << device_list.size() << " devices:" << std::endl;

    for (auto&& dev_info : device_list)
    {
        std::cout << "\t" << format_device_info(dev_info) << std::endl;
    }

    std::cout << std::endl;
}



int main()
{
    ic4::initLibrary();
    print_device_list();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);

    // Create a 640x480 image with 3 channels (BGR)
    cv::Mat random_image(480, 640, CV_8UC3);

    // Fill the image with random colors
    for (int i = 0; i < random_image.rows; i++)
    {
        for (int j = 0; j < random_image.cols; j++)
        {
            random_image.at<cv::Vec3b>(i, j)[0] = dis(gen); // Blue
            random_image.at<cv::Vec3b>(i, j)[1] = dis(gen); // Green
            random_image.at<cv::Vec3b>(i, j)[2] = dis(gen); // Red
        }
    }

    // Create a window and display the random image
    cv::namedWindow("Random Image", cv::WINDOW_AUTOSIZE);
    cv::imshow("Random Image", random_image);

    std::cout << "Displaying random image. Press any key to exit..." << std::endl;

    // Wait for a key press
    cv::waitKey(0);

    // Clean up
    cv::destroyAllWindows();
}