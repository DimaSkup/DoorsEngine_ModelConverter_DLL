#define MODEL_CONVERTER_DLL_ENTRY_EXPORTS

#include "ModelConverterDLLEntry.h"
#include "ModelConverterInterface.h"
#include "ModelConverterForObjTypeClass.h"
#include <iostream>
#include <cassert>


bool ModelConverter::ImportModelFromFile(
	const char* inputFilename,      // full path to the model's input data file 
	const char* outputFilename)     // full path to the model's output data file
{
	// check input data
	assert((inputFilename != nullptr) && (inputFilename[0] != '\0'));
	assert((outputFilename != nullptr) && (outputFilename[0] != '\0'));


	// check if we already have an OUTPUT data file for the model of such type
	//std::ifstream fin(outputFilename, std::ios::in | std::ios::binary);
	//bool executeModelConvertation = executeModelConvertation = fin.fail();


	std::unique_ptr<ModelConverterInterface> pModelConverter = std::make_unique<ModelConverterInterface>();

	std::cout << "\n\n\n-----   START OF THE CONVERTATION PROCESS:   -----\n";

	bool result = pModelConverter->Convert(inputFilename, outputFilename);
	if (!result)
	{
		std::cout << "can't convert a model by file:\n" << inputFilename << std::endl;
		return false;
	}

	return true;
}
