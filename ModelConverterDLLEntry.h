#pragma once

#include "Log.h"

namespace ModelConverter
{
	// when the MODEL_CONVERTER_DLL_EXPORTS macro is defined, the MODEL_CONVERTER_API
	// macro sets the __declspec(dllexport) modifier on the function declarations. This
	// modifier tells the compiler and linker to export a function or variable from the DLL
	// for use by other applications. Then MODEL_CONVERTER_DLL_EXPORTS is undefined, for 
	// example, wneh the header file is included by a client application,
	// MODEL_CONVERTER_API applies the __declspec(dllimport) modifier to the declarations.
	// This modifier optimizes the import of the function or variable in an application.
	#ifdef MODEL_CONVERTER_DLL_ENTRY_EXPORTS
		#define MODEL_CONVERTER_API __declspec(dllexport)
	#else
		#define MODEL_CONVERTER_API __declspec(dllimport)
	#endif


	static Log log;


	// DEFINE THE DLL's INTERFACE

	//#ifdef __cplusplus    // if used by C++ code,
	//	extern "C" {      // we need to export the C interface
	//#endif


	extern "C" MODEL_CONVERTER_API bool ImportModelFromFile(
		const char* inputFilename,      // full path to the model's input data file 
		const char* outputFilename);    // full path to the model's output data file

	//#ifdef __cplusplus    // if used by C++ code,
	//	}                 // the end of "extern C" declaration
	//#endif
}




