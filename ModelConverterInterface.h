#pragma once

#include "ModelConverterForObjTypeClass.h"

class ModelConverterInterface
{
public:
	bool Convert(const char* inputFilename, const char* outputFilename)
	{


		std::unique_ptr<ModelConverterForObjTypeClass> pModelConverter = std::make_unique<ModelConverterForObjTypeClass>();

		bool result = pModelConverter->ConvertFromObj(inputFilename, outputFilename);
		if (!result)
		{
			std::cout << "can't convert .obj into the internal model format" << std::endl;
			return false;
		}

		return true;
	}

	
};