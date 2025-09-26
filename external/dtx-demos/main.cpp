#include "TISCameraIC4.h"
#include <iostream>
#include <string>
#include <thread>   // for sleep
#include <chrono>


#include "IVmSolution.h"
#include "VMException.h"
#include "IVmProcedure.h"
#include "IVmImageSource.h"
#include "IVmCalculator.h"

using namespace VisionMasterSDK;
using namespace VisionMasterSDK::VmSolution;
using namespace VisionMasterSDK::VmProcedure;
using namespace VisionMasterSDK::ImageSourceModule;
using namespace VisionMasterSDK::CalculatorModule;



int main() {

    cv::Mat colorImage = cv::imread("a.png", cv::IMREAD_COLOR);
    if (colorImage.empty()) {
        cerr << "ERROR: Failed to load image 'a.png'!" << endl;
        return 4;
    }

	const string& solutionPath = "C:\\Users\\User\\Desktop\\The-Imaging-Source-Camera-SDKs\\external\\dtx-demos\\ok_nok.solw";
    const string& procedureName = "Flow1";
	const string& moduleImageSourceName = "Image Source1";
	const string& moduleVaribleCalculation1 = "Variable Calculation1";


    IVmSolution* pVmSol = CreateSolutionInstance();
    pVmSol = GetSolutionExistedInstance();

    pVmSol = LoadSolution(solutionPath.c_str(), "");
    if (NULL == pVmSol) {
        return IMVS_EC_NULL_PTR;
    }
    else {
		std::cout << "LoadSolution success!" << std::endl;
    }

    IVmProcedure* pVmPrc = (IVmProcedure*)(*pVmSol)[procedureName.c_str()];
    if (NULL == pVmSol) {
        return IMVS_EC_NULL_PTR;
    }
    else {
        std::cout << "Procedure loaded!" << std::endl;
    }

    string fullImageSourcePath = procedureName + "." + moduleImageSourceName;
    ImageSourceModuleTool* ImageSourceModule = (ImageSourceModuleTool*)(*pVmSol)[fullImageSourcePath.c_str()];
    if (ImageSourceModule == nullptr) {
        throw runtime_error("Failed to get 1D Image Source module tool '" + fullImageSourcePath + "'.");
    }
    else
    {
		std::cout << "Image Source module loaded!" << std::endl;
    }

    string fullVaraibleCalculation1Path = procedureName + "." + moduleVaribleCalculation1;
    CalculatorModuleTool* VariableCalculation1Module = (CalculatorModuleTool*)(*pVmSol)[fullVaraibleCalculation1Path.c_str()];
    if (VariableCalculation1Module == nullptr) {
        throw runtime_error("Failed to get Varible Calculation 1 module tool '" + fullVaraibleCalculation1Path + "'.");
    }
    else {
        std::cout << "Varible Calculation 1 module loaded!" << std::endl;
    }

    CalculatorResults* VC1Result = nullptr;

    while (true) {
        pVmPrc->Run();

        VC1Result = VariableCalculation1Module->GetResult();
        if (VC1Result == nullptr) {
            std::cout << "Failed to get Varible Calculation 1 results" << std::endl;
        }
        else {
            std::cout << "Got the results" << std::endl;
        }




        // TODO I WANT TO SEE MY RESULT HERE
        std::cout << "\n--- Variable Calculation Results ---" << std::endl;

        // Get the total number of result parameters
        int resultCount = VC1Result->GetResultNum();
        std::cout << "Found " << resultCount << " result parameter(s)." << std::endl;

        // Loop through each result parameter (e.g., "myIntegers", "myFloats")
        for (int i = 0; i < resultCount; ++i)
        {
            // Get the result info object at the current index
            CalOutputResultInfo* resultInfo = VC1Result->GetResult(i);
            if (resultInfo != nullptr)
            {
                // Print the name of the result parameter
                std::cout << "\n  Parameter " << i << ": '" << resultInfo->strParamName << "'";
                std::cout << " (" << resultInfo->nValueNum << " value(s))" << std::endl;

                // Check the type of the result and print the corresponding values
                switch (resultInfo->nParamType)
                {
                case 1: // Integer Type
                    std::cout << "  Type: Integer" << std::endl;
                    if (resultInfo->pIntValue != nullptr)
                    {
                        std::cout << "  Values: [";
                        // Loop through the array of integer values
                        for (int j = 0; j < resultInfo->nValueNum; ++j)
                        {
                            std::cout << resultInfo->pIntValue[j] << (j == resultInfo->nValueNum - 1 ? "" : ", ");
                        }
                        std::cout << "]" << std::endl;
                    }
                    break;

                case 2: // Float Type
                    std::cout << "  Type: Float" << std::endl;
                    if (resultInfo->pFloatValue != nullptr)
                    {
                        std::cout << "  Values: [";
                        // Loop through the array of float values
                        for (int j = 0; j < resultInfo->nValueNum; ++j)
                        {
                            std::cout << resultInfo->pFloatValue[j] << (j == resultInfo->nValueNum - 1 ? "" : ", ");
                        }
                        std::cout << "]" << std::endl;
                    }
                    break;

                case 3: // String Type
                    std::cout << "  Type: String" << std::endl;
                    std::cout << "  Note: The provided struct does not contain a member to access string data." << std::endl;
                    // String data might need to be retrieved using a different function if available in your SDK version.
                    break;

                default:
                    std::cout << "  Type: Unknown or unsupported (Type code: " << resultInfo->nParamType << ")" << std::endl;
                    break;
                }
            }
            else
            {
                std::cerr << "  Failed to get result at index " << i << std::endl;
            }
        }
        std::cout << "\n------------------------------------" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }



    ObjectList procedureObjList = pVmSol->GetAllProcedureObjects();
    ProcessInfoList* procedureList = pVmSol->GetAllProcedureList();
    ModuleInfoList* moduInfoList = pVmSol->GetAllModuleList();

    std::cout << std::endl;
    std::cout << "=== Module Information List ===" << std::endl;
    std::cout << "Number of modules: " << moduInfoList->nNum << std::endl;
    std::cout << "Maximum capacity: " << MAX_MODULE_NUM << std::endl;

    for (unsigned int i = 0; i < moduInfoList->nNum && i < MAX_MODULE_NUM; i++) {
        const ModuInfo& module = moduInfoList->astModuleInfo[i];

        std::cout << "\n--- Module " << i << " ---" << std::endl;
        std::cout << "Module ID: " << module.nModuleID << std::endl;
        std::cout << "Process ID: " << module.nProcessID << std::endl;
        std::cout << "Display Name: " << module.strDisplayName << std::endl;
        std::cout << "Module Name: " << module.strModuleName << std::endl;

        // Check if reserved fields contain data
        bool hasNonZeroReserved = false;
        for (int j = 0; j < 128; j++) {
            if (module.nReserved[j] != 0) {
                hasNonZeroReserved = true;
                break;
            }
        }
        std::cout << "Reserved Fields: " << (hasNonZeroReserved ? "Contains data" : "All zeros") << std::endl;
    }

    // Print list reserved fields
    std::cout << "\nList Reserved Fields: ";
    for (int i = 0; i < 4; i++) {
        std::cout << "0x" << std::hex << moduInfoList->nReserved[i] << std::dec;
        if (i < 3) std::cout << ", ";
    }
    std::cout << std::endl;


    


    /*ic4::initLibrary();

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

    ic4::exitLibrary(); // don’t forget cleanup*/
    
    return 0;
}