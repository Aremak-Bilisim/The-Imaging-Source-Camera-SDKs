#include "VisionMasterProcessor.h"
#include <sstream>

VisionMasterProcessor::VisionMasterProcessor(const string& solPath, const string& procName,
    const string& imgSourceName, const string& varCalcName)
    : solutionPath(solPath), procedureName(procName), moduleImageSourceName(imgSourceName),
    moduleVaribleCalculation1(varCalcName), pVmSol(nullptr), pVmPrc(nullptr),
    ImageSourceModule(nullptr), VariableCalculation1Module(nullptr), VC1Result(nullptr) {
    initializeSolution();
    loadModules();
}

VisionMasterProcessor::~VisionMasterProcessor() {
    // Cleanup resources if needed
    // Note: Specific cleanup depends on Vision Master SDK requirements
}

bool VisionMasterProcessor::initializeSolution() {
    pVmSol = CreateSolutionInstance();
    pVmSol = GetSolutionExistedInstance();

    pVmSol = LoadSolution(solutionPath.c_str(), "");
    if (NULL == pVmSol) {
        std::cerr << "Failed to load solution!" << std::endl;
        return false;
    }
    else {
        std::cout << "LoadSolution success!" << std::endl;
        return true;
    }

	
}

bool VisionMasterProcessor::loadModules() {
    // Load procedure
    pVmPrc = (IVmProcedure*)(*pVmSol)[procedureName.c_str()];
    if (NULL == pVmPrc) {
        std::cerr << "Failed to load procedure!" << std::endl;
        return false;
    }
    else {
        std::cout << "Procedure loaded!" << std::endl;
    }

    // Load Image Source module
    string fullImageSourcePath = procedureName + "." + moduleImageSourceName;
    ImageSourceModule = (ImageSourceModuleTool*)(*pVmSol)[fullImageSourcePath.c_str()];
    if (ImageSourceModule == nullptr) {
        std::cerr << "Failed to get Image Source module tool '" + fullImageSourcePath + "'." << std::endl;
        return false;
    }
    else {
        std::cout << "Image Source module loaded!" << std::endl;
    }

    // Load Variable Calculation module
    string fullVaraibleCalculation1Path = procedureName + "." + moduleVaribleCalculation1;
    VariableCalculation1Module = (CalculatorModuleTool*)(*pVmSol)[fullVaraibleCalculation1Path.c_str()];
    if (VariableCalculation1Module == nullptr) {
        std::cerr << "Failed to get Variable Calculation 1 module tool '" + fullVaraibleCalculation1Path + "'." << std::endl;
        return false;
    }
    else {
        std::cout << "Variable Calculation 1 module loaded!" << std::endl;
    }

    return true;
}

bool VisionMasterProcessor::runProcedure() {
    if (pVmPrc == nullptr) {
        std::cerr << "Procedure not initialized!" << std::endl;
        return false;
    }

    pVmPrc->Run();
    std::cout << "Procedure executed successfully!" << std::endl;
    return true;
}

bool VisionMasterProcessor::getResults() {
    if (VariableCalculation1Module == nullptr) {
        std::cerr << "Variable Calculation module not initialized!" << std::endl;
        return false;
    }

    VC1Result = VariableCalculation1Module->GetResult();
    if (VC1Result == nullptr) {
        std::cout << "Failed to get Variable Calculation 1 results" << std::endl;
        return false;
    }
    else {
        std::cout << "Got the results" << std::endl;
        return true;
    }
}

void VisionMasterProcessor::displayResults() {
    if (VC1Result == nullptr) {
        std::cerr << "No results available to display!" << std::endl;
        return;
    }

    std::cout << "\n--- Variable Calculation Results ---" << std::endl;

    // Get the total number of result parameters
    int resultCount = VC1Result->GetResultNum();
    std::cout << "Found " << resultCount << " result parameter(s)." << std::endl;

    // Loop through each result parameter (e.g., "myIntegers", "myFloats")
    for (int i = 0; i < resultCount; ++i) {
        // Get the result info object at the current index
        CalOutputResultInfo* resultInfo = VC1Result->GetResult(i);
        if (resultInfo != nullptr) {
            // Print the name of the result parameter
            std::cout << "\n  Parameter " << i << ": '" << resultInfo->strParamName << "'";
            std::cout << " (" << resultInfo->nValueNum << " value(s))" << std::endl;

            // Check the type of the result and print the corresponding values
            switch (resultInfo->nParamType) {
            case 1: // Integer Type
                std::cout << "  Type: Integer" << std::endl;
                if (resultInfo->pIntValue != nullptr) {
                    std::cout << "  Values: [";
                    // Loop through the array of integer values
                    for (int j = 0; j < resultInfo->nValueNum; ++j) {
                        std::cout << resultInfo->pIntValue[j] << (j == resultInfo->nValueNum - 1 ? "" : ", ");
                    }
                    std::cout << "]" << std::endl;
                }
                break;

            case 2: // Float Type
                std::cout << "  Type: Float" << std::endl;
                if (resultInfo->pFloatValue != nullptr) {
                    std::cout << "  Values: [";
                    // Loop through the array of float values
                    for (int j = 0; j < resultInfo->nValueNum; ++j) {
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
        else {
            std::cerr << "  Failed to get result at index " << i << std::endl;
        }
    }
    std::cout << "\n------------------------------------" << std::endl;
}


string VisionMasterProcessor::runAndGetResults() {
    if (!runProcedure()) {
        return "ERROR: Failed to run procedure";
    }

    if (!getResults()) {
        return "ERROR: Failed to get results";
    }

    return getResultsAsString();
}

string VisionMasterProcessor::getResultsAsString() {
    if (VC1Result == nullptr) {
        return "ERROR: No results available";
    }

    string resultString = "\n--- Variable Calculation Results ---\n";

    // Get the total number of result parameters
    int resultCount = VC1Result->GetResultNum();
    resultString += "Found " + to_string(resultCount) + " result parameter(s).\n";

    // Loop through each result parameter
    for (int i = 0; i < resultCount; ++i) {
        CalOutputResultInfo* resultInfo = VC1Result->GetResult(i);
        if (resultInfo != nullptr) {
            resultString += "\n  Parameter " + to_string(i) + ": '" + string(resultInfo->strParamName) + "'";
            resultString += " (" + to_string(resultInfo->nValueNum) + " value(s))\n";

            // Check the type of the result and format the corresponding values
            switch (resultInfo->nParamType) {
            case 1: // Integer Type
                resultString += "  Type: Integer\n";
                if (resultInfo->pIntValue != nullptr) {
                    resultString += "  Values: [";
                    for (int j = 0; j < resultInfo->nValueNum; ++j) {
                        resultString += to_string(resultInfo->pIntValue[j]);
                        if (j < resultInfo->nValueNum - 1) resultString += ", ";
                    }
                    resultString += "]\n";
                }
                break;

            case 2: // Float Type
                resultString += "  Type: Float\n";
                if (resultInfo->pFloatValue != nullptr) {
                    resultString += "  Values: [";
                    for (int j = 0; j < resultInfo->nValueNum; ++j) {
                        resultString += to_string(resultInfo->pFloatValue[j]);
                        if (j < resultInfo->nValueNum - 1) resultString += ", ";
                    }
                    resultString += "]\n";
                }
                break;

            case 3: // String Type
                resultString += "  Type: String\n";
                resultString += "  Note: String data access not implemented in current struct.\n";
                break;

            default:
                resultString += "  Type: Unknown or unsupported (Type code: " + to_string(resultInfo->nParamType) + ")\n";
                break;
            }
        }
        else {
            resultString += "  Failed to get result at index " + to_string(i) + "\n";
        }
    }
    resultString += "------------------------------------\n";

    return resultString;
}