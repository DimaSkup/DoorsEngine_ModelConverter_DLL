#define MODEL_CONVERTER_DLL_ENTRY_EXPORTS

#include "ModelConverterDLLEntry.h"
#include "ModelConverterClass.h"
#include <iostream>
#include <cassert>


void ModelConverter::ImportModelFromFile(
	const char* inputFilename,      // full path to the model's input data file 
	const char* outputFilename)     // full path to the model's output data file
{
	assert((inputFilename != nullptr) && (inputFilename[0] != '\0'));
	assert((outputFilename != nullptr) && (outputFilename[0] != '\0'));


	// check if we already have an OUTPUT data file for the model of such type
	std::ifstream fin(outputFilename, std::ios::in | std::ios::binary);
	bool executeModelConvertation = executeModelConvertation = fin.fail();

	// if we need to convert external file model data into the internal model format
	if (true)
	{
		std::unique_ptr<ModelConverterClass> pModelConverter = std::make_unique<ModelConverterClass>();

		bool result = pModelConverter->ConvertFromObj(inputFilename, outputFilename);
		if (!result)
		{
			std::cout << "can't convert .obj into the internal model format" << std::endl;
			return;
		}
	}

}
