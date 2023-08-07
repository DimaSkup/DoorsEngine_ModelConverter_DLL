/////////////////////////////////////////////////////////////////////
// Filename:     modelconverterclass.h
// Description:  this class is used to convert models data from others
//               formats into the internal model data format
//
// Revising:     25.04.22
/////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// INCLUDES
//////////////////////////////////
//#include "../Engine/Log.h"
//#include "../Engine/Settings.h"   // here we get some settings for the convertation process

#define ERROR_MSG "%s() (%d): %s",__FUNCTION__, __LINE__

#include <windows.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <memory>
#include <string>


using namespace std;

//////////////////////////////////
// Class name: ModelConverterClass
//////////////////////////////////
class ModelConverterClass
{
public:
	ModelConverterClass(void);
	~ModelConverterClass(void);

	void Shutdown(void);	// releases the memory

							// converts .obj file model data into the internal model format
	bool ConvertFromObj(const char* inputFilename, const char* outputFilename);

private:
	bool ConvertFromObjHelper(ifstream & fin, ofstream & fout);

	// input data file reading handlers  
	void SkipUntilVerticesData(ifstream & fin);
	
	void ReadCounts(ifstream & fin);   // read the count of vertices, textures coordinates and the count of faces as well

	// calculate counts for particular data block
	void CalculateCount(ifstream & fin,
		int & countOfData,               // 1. how many lines this block of data has
		streampos & posBeforeNextBlock,  // 2. file ptr position right before next block of data
		std::string dataType,            // 3. which type of data we're reading from the current data block
		std::string prefix,              // 4. each line of the current data block starts with this prefix
		std::string skipUntilPrefix);    // 5. after the current data block we may have some 
										 //    technical lines or empty lines so skip it until 
										 //    the next data block which starts with symbols are
										 //    stored in the skipUntilPrefix variable);
	bool ReadInAndWriteVerticesData(ifstream & fin, ofstream & fout);
	bool ReadInAndWriteTexturesData(ifstream & fin, ofstream & fout);
	bool ReadInAndWriteNormalsData(ifstream & fin, ofstream & fout);
	bool ReadInFacesData(ifstream & fin);

	bool ResetConverterState();                   // after each convertation we MUST reset the state of the converter for proper later convertations

	void PrintDebugFilenames(const char* inputFilename, const char* outputFilename) const;
	void PrintError(char* message, ...);

	// output data file writing handlers 
	bool WriteDataIntoOutputFile(ofstream & fout);  // write model data in an internal model format into the output data file
	bool WriteIndicesIntoOutputFile(ofstream & fout);



private:
	struct VERTEX3D
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	};

	struct TEXTURE_COORDS
	{
		float tu = 0.0f;
		float tv = 0.0f;
	};

	struct NORMAL
	{
		float nx = 0.0f;
		float ny = 0.0f;
		float nz = 0.0f;
	};

	
	
	NORMAL            normal_;

	char* inputLineBuffer_ = nullptr;                   // during execution of the getline() function we put here a one single text line

	streampos posBeforeVerticesData_ = 0;
	streampos posBeforeTexturesData_ = 0;
	streampos posBeforeNormalsData_ = 0;
	streampos posBeforeFacesData_ = 0;

	int verticesCount_ = 0;
	int textureCoordsCount_ = 0;
	int normalsCount_ = 0;
	int facesCount_ = 0;

	std::vector<UINT> vertexIndicesArray_;
	std::vector<UINT> textureIndicesArray_;

	bool isPrintConvertProcessMessages_ = false;  // defines whether to print or not in the console messages about the convertation process    

												  // constants
	const int INPUT_LINE_SIZE_ = 80;              // how many symbols can read the getline() function as one sinle text line
};

