#ifndef VISIONMASTERPROCESSOR_H
#define VISIONMASTERPROCESSOR_H

#include <iostream>
#include <string>
#include <stdexcept>

#include "IVmSolution.h"
#include "VMException.h"
#include "IVmProcedure.h"
#include "IVmImageSource.h"
#include "IVmCalculator.h"

using namespace std;
using namespace VisionMasterSDK;
using namespace VisionMasterSDK::VmSolution;
using namespace VisionMasterSDK::VmProcedure;
using namespace VisionMasterSDK::ImageSourceModule;
using namespace VisionMasterSDK::CalculatorModule;

class VisionMasterProcessor {
private:
    IVmSolution* pVmSol;
    IVmProcedure* pVmPrc;
    ImageSourceModuleTool* ImageSourceModule;
    CalculatorModuleTool* VariableCalculation1Module;
    CalculatorResults* VC1Result;

    string solutionPath;
    string procedureName;
    string moduleImageSourceName;
    string moduleVaribleCalculation1;

public:
    // Constructor
    VisionMasterProcessor(const string& solPath = "C:\\Users\\User\\Desktop\\The-Imaging-Source-Camera-SDKs\\external\\dtx-demos\\ok_nok.solw",
        const string& procName = "Flow1",
        const string& imgSourceName = "Image Source1",
        const string& varCalcName = "Variable Calculation1");

    // Destructor
    ~VisionMasterProcessor();

    // Initialize the Vision Master solution
    bool initializeSolution();

    // Load and configure modules
    bool loadModules();

    // Run the vision processing procedure
    bool runProcedure();

    // Get and display results
    bool getResults();

    // Display detailed results
    void displayResults();

    // Run on command and return results as string
    string runAndGetResults();

    // Get results as formatted string without console output
    string getResultsAsString();

    // Getters
    IVmSolution* getSolution() const { return pVmSol; }
    IVmProcedure* getProcedure() const { return pVmPrc; }
    CalculatorResults* getCalculatorResults() const { return VC1Result; }
};

#endif // VISIONMASTERPROCESSOR_H