#include "ModelConverterClass.h"

#include <cstdio>   // for using a remove() method for deleting files



ModelConverterClass::ModelConverterClass(void)
{
	inputLineBuffer_ = new char[ModelConverterClass::INPUT_LINE_SIZE_]{ '\0' };   // during execution of the getline() function we put here a one single text line
	isPrintConvertProcessMessages_ = true;
}

ModelConverterClass::~ModelConverterClass(void)
{
	Shutdown();
}




// ----------------------------------------------------------------------------------- //
//
//                          PUBLIC METHODS
//
// ----------------------------------------------------------------------------------- //

void ModelConverterClass::Shutdown(void)
{
	/*
	
	delete[] pPoint3D_;
	pPoint3D_ = nullptr;

	delete[] pTexCoord_;
	pTexCoord_ = nullptr;

	delete[] pNormal_;
	pNormal_ = nullptr;
	*/

	delete[] inputLineBuffer_;
	inputLineBuffer_ = nullptr;
}


// converts a model of the ".obj" type into the internal model format
bool ModelConverterClass::ConvertFromObj(const char* inputFilename, const char* outputFilename)
{
	bool result = false;

	//this->GetOutputModelFilename(outputFilename, inputFilename);
	this->PrintDebugFilenames(inputFilename, outputFilename);

	// open the input file and create an output file
	std::ifstream fin(inputFilename, std::ios::in | std::ios::binary);	// input data file (.obj)
	std::ofstream fout(outputFilename, std::ios::out);                  // ouptput data file (.txt)


	// If it could not open the input file then exit
	if (fin.fail())
	{
		std::string errorMsg{ "can't open input data file: " + std::string(inputFilename) };
		PrintError(ERROR_MSG, errorMsg.c_str());
		return false;
	}

	// if it could not open the output file then exit
	if (fout.bad())
	{
		PrintError(ERROR_MSG, "can't open the output file");
		return false;
	}
	
	// convert the model
	result = this->ConvertFromObjHelper(fin, fout);
	if (!result)
	{
		PrintError(ERROR_MSG, "Model's data has been converted successfully");
		return false;
	}
	

	// close the .obj input file and the output file
	fin.close();
	fout.close();

	return true;
}

void ModelConverterClass::PrintDebugFilenames(const char* inputFilename, const char* outputFilename) const
{
	// generate a debug message with input/output filenames

	std::stringstream ss;
	ss << "FILES:\n " << "IN: " << inputFilename << "\n " << "OUT: " << outputFilename;
	std::cout << "ModelConverterClass::PrintDebugFilenames():\n" << ss.str() << "\n\n";
}

// ----------------------------------------------------------------------------------- //
//
//                          PRIVATE METHODS / HELPERS
//
// ----------------------------------------------------------------------------------- //

// help us to convert .obj file model data into the internal model format
bool ModelConverterClass::ConvertFromObjHelper(ifstream& fin, ofstream& fout)
{
	bool result = false;

	this->ReadCounts(fin);

	// print positions
	std::cout << "POSITIONS:\n";
	std::cout << "before VERTICES: " << posBeforeVerticesData_ << '\n';
	std::cout << "before TEXTURES: " << posBeforeTexturesData_ << '\n';
	std::cout << "before NORMALS:  " << posBeforeNormalsData_ << '\n';
	std::cout << "before FACES:    " << posBeforeFacesData_ << '\n';

	std::cout << "\n\n";

	// print counts
	std::cout << "COUNTS:\n";
	std::cout << "VERTICES COUNT: " << verticesCount_ << '\n';
	std::cout << "TEXTURES COUNT: " << textureCoordsCount_ << '\n';
	std::cout << "NORMALS COUNT:  " << normalsCount_ << '\n';
	std::cout << "FACES COUNT:    " << facesCount_ << std::endl;



	fout << "Vertex Count: " << verticesCount_ << "\n";
	fout << "Indices Count: " << vertexIndicesArray_.size() << "\n";
	fout << "Textures Count: " << textureCoordsCount_ << "\n\n";


	fin.clear();             // because we've went up to the end of the file (EOF) we have to clear ifstream state
	fin.seekg(0, fin.beg);   // go to the beginning of the file


	if (!ReadInAndWriteVerticesData(fin, fout))
	{
		PrintError(ERROR_MSG, "can't read in vertices data");
		return false;
	}

	fin.clear();             // because we've went up to the end of the file (EOF) we have to clear ifstream state
	fin.seekg(0, fin.beg);   // go to the beginning of the file

	if (!ReadInAndWriteTexturesData(fin, fout))
	{
		PrintError(ERROR_MSG, "can't read in textures data");
		return false;
	}


	std::cout << "CONVERTATION IS FINISHED" << std::endl;
	



	//Log::Debug(THIS_FUNC, "START of the convertation process");

/*
	

	if (!ReadInTextureData(fin))
	{
		PrintError(ERROR_MSG, "can't read in textures data");
		return false;
	}

	if (!ReadInNormalsData(fin))
	{
		PrintError(ERROR_MSG, "can't read in normals data");
		return false;
	}

	if (!ReadInFacesData(fin))
	{
		PrintError(ERROR_MSG, "can't read in faces data");
		return false;
	}

	// write model data in an internal model format into the output data file
	if (!WriteDataIntoOutputFile(fout)) 
	{
		PrintError(ERROR_MSG, "can't write data into output file");
		return false;
	}

*/

	// after each convertation we MUST reset the state of the converter for proper later convertations
	//ResetConverterState();        

	//Log::Print(THIS_FUNC, "Convertation is finished successfully!");

	return true;
}


void ModelConverterClass::SkipUntilVerticesData(ifstream & fin)
{
	// contains a position right before the vertices data
	streampos posBeforeVerticesData = 0;    

	// Read up to the vertex values
	while (inputLineBuffer_[0] != 'v')
	{
		posBeforeVerticesData = fin.tellg();                                  // store the position in input sequence
		fin.getline(inputLineBuffer_, ModelConverterClass::INPUT_LINE_SIZE_); // get a line from the file						
	}

	// we got to the vertices data so return a stream pointer to the position right before the vertices data
	fin.seekg(posBeforeVerticesData);

	return;
}

void ModelConverterClass::ReadCounts(ifstream & fin)
{
	this->SkipUntilVerticesData(fin);

	posBeforeVerticesData_ = fin.tellg();	 // later we'll return to this position so save it
	fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_); // read the first line of vertices data


	this->CalculateCount(fin, verticesCount_, posBeforeTexturesData_, "VERTICES", "v ", "vt");
	this->CalculateCount(fin, textureCoordsCount_, posBeforeNormalsData_, "TEXTURES", "vt", "vn");
	this->CalculateCount(fin, normalsCount_, posBeforeFacesData_, "NORMALS", "vn", "f");
	this->CalculateCount(fin, facesCount_, streampos(), "FACES", "f ", "  ");

	

	return;
}


// calculate counts for some particular data block 
// (for instance: count of vertices, position before the next block, etc.)
void ModelConverterClass::CalculateCount(ifstream & fin,
	int & countOfData,               // 1. how many lines this block of data has
	streampos & posBeforeNextBlock,  // 2. file ptr position right before next block of data
	std::string dataType,            // 3. which type of data we're reading from the current data block
	std::string prefix,              // 4. each line of the current data block starts with this prefix
	std::string skipUntilPrefix)     // 5. skip input symbols until we come across this prefix (the next data block)
{
	std::cout << "GO THROUGH " << dataType << ":\n\n";

	// while we're reading lines which start with prefix
	while ((inputLineBuffer_[0] == prefix[0]) && (inputLineBuffer_[1] == prefix[1]))
	{
		countOfData++;
		//std::cout << inputLineBuffer_ << "\n";
		fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
	}

	std::cout << "\n\n";

	// skip lines until the next block of data
	while ((inputLineBuffer_[0] != skipUntilPrefix[0]) && 
		   (inputLineBuffer_[1] != skipUntilPrefix[1]))
	{
		std::cout << "skip line: " << inputLineBuffer_ << std::endl;
		fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
	}

	// calculate the file ptr position right before the next data block
	posBeforeNextBlock = fin.tellg();
	posBeforeNextBlock -= strlen(inputLineBuffer_);

	std::cout << "\n\n";

	return;
}



// in this function we read in vertices data from the input data file 
// and right after it we write this data into the output data file
bool ModelConverterClass::ReadInAndWriteVerticesData(ifstream & fin, ofstream & fout)
{
	VERTEX3D  vertex3D_;
	char input;                          // for reading the '\n' symbol
	fin.seekg(posBeforeVerticesData_);   // return to the position before the vertices data 
	fout << "\nVertices Data:\n";        // write into the output file that the following data block is vertices data
	

	for (size_t i = 0; i < verticesCount_; i++)
	{
		// read vertex data from the input data file 
		fin.ignore(2);                                              // skip the "v" and " " (space) symbols at the beginning of the line
		fin >> vertex3D_.x >> vertex3D_.y >> vertex3D_.z >> input;  // read in x, y, z vertex coordinates and the '\n' symbol
		
		// write this vertex data into the output data file
		fout.setf(ios::fixed, ios::floatfield);
		fout.precision(6);

		fout << vertex3D_.x << " "       // write the vertex coordinates
			 << vertex3D_.y << " "
			 << vertex3D_.z * -1.0f      // invert the value to use it in the left handed coordinate system
			 << "\n";  
	}
	fout << "\n\n";                      // in the output data file: make a separation space before the next data block 

	return true;
}

bool ModelConverterClass::ReadInAndWriteTexturesData(ifstream & fin, ofstream & fout)
{
	TEXTURE_COORDS texCoords_;
	char input;                          // for reading the '\n' symbol
	fin.seekg(posBeforeTexturesData_, fin.beg);   // return to the position before the textures data 
	fout << "\nTextures Data:\n";        // write into the output file that the following data block is vertices data
	
	/*
	
	std::cout << "\n\nTEXTURES: " << std::endl;
	while (!fin.eof())
	{
		std::cout << (char)fin.get();
	}

	std::cout << std::endl;
	
	*/

	for (size_t i = 0; i < textureCoordsCount_; i++)
	{
		// read texture data from the input data file 
		fin.ignore(3);                   // ignore the "vt " symbols in the beginning of line
		fin >> texCoords_.tu >> texCoords_.tv;

		std::cout << "tex[" << i << "]: " << texCoords_.tu << " : " << texCoords_.tv << std::endl;

		// write this texture coords data into the output data file
		fout.setf(ios::fixed, ios::floatfield);
		fout.precision(6);

		fout << texCoords_.tu << " "
			 << 1.0f - texCoords_.tv   // invert the value to use it in the left handed coordinate system
			 << "\n";  
	}

	return true;
}

bool ModelConverterClass::ReadInNormalsData(ifstream& fin)
{
	//Log::Debug(THIS_FUNC_EMPTY);

	//constexpr int lineSize = 80;
	char input;
	//char inputLine[lineSize];
	size_t posBeforeNormalsData = 0;
	std::string firstReadingDataLine{ "" };
	std::string lastReadingDataLine{ "" };


	fin.seekg(-1, ios::cur); // move at the position before the symbol "v"
	posBeforeNormalsData = fin.tellg(); // later we'll return to this position to read in normals data


	// ----------- CALCULATE THE COUNT OF NORMALS  ---------------- //
	fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_, '\n');
	firstReadingDataLine = inputLineBuffer_;
	std::cout << "first normal line: " << firstReadingDataLine << std::endl;

	while (inputLineBuffer_[0] == 'v')   // while we don't get to the end of normals data
	{
		normalsCount_++;
		fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
	}

	lastReadingDataLine = inputLineBuffer_;
	std::cout << "last normal line: " << lastReadingDataLine << std::endl;

	/*
	
	
	
	// allocate the memory for this count of normals
	try
	{
		pNormal_ = new DirectX::XMFLOAT3[normalsCount_];
	}
	catch (std::bad_alloc & e)
	{
		PrintError(ERROR_MSG, e.what());
		PrintError(ERROR_MSG, "can't allocate memory for normals");
		return false;
	}

	// --------------------- READ IN NORMALS DATA ---------------------- //
	fin.seekg(posBeforeNormalsData); // return back to the position before normals data

	for (size_t i = 0; i < normalsCount_; i++) //  reading in of each normal
	{
		fin.ignore(2); // ignore "vn " in the beginning of line
		fin >> pNormal_[i].x >> pNormal_[i].y >> pNormal_[i].z >> input; // read in the normal data and a new line symbol
	}


	
	*/
	return true;
}




bool ModelConverterClass::ReadInFacesData(ifstream & fin)
{
	int vertexIndex = 0;
	int textureIndex = 0;
	int normalIndex = 0;

	inputLineBuffer_[0] = '\0';
	//char input[2];


	// skip data until we get to the 'f' and ' ' (space) symbols
	while (inputLineBuffer_[0] != 'f' && inputLineBuffer_[1] != ' ')
	{
		fin.getline(inputLineBuffer_, ModelConverterClass::INPUT_LINE_SIZE_);
	};



	// store the file pointer position
	size_t posBeforeFaceCommand = fin.tellg();
	//posBeforeFaceCommand -= strlen(inputLineBuffer_) + 1; // come back at the beginning of line (size of the string + null character)
	fin.seekg(posBeforeFaceCommand);	// now we at the position before the beginning of polygonal face data

	// define how many faces we have
	fin.getline(inputLineBuffer_, ModelConverterClass::INPUT_LINE_SIZE_);
	while (!fin.eof())
	{
		facesCount_++;
		fin.getline(inputLineBuffer_, ModelConverterClass::INPUT_LINE_SIZE_);
	};


	fin.clear();
	fin.seekg(posBeforeFaceCommand);

	// go through each face
	for (size_t faceIndex = 0; faceIndex < facesCount_; faceIndex++)
	{
		fin.ignore(2);  // skip the 'f' and ' ' (space) symbols at the beginning of the line

						// go through each vertex of the current face
		for (size_t faceVertex = 1; faceVertex <= 3; faceVertex++)
		{
			// read in a vertex index
			fin >> vertexIndex;
			if (fin.bad())
			{
				PrintError(ERROR_MSG, "error about reading of the vertex index");
				return false;
			}
			fin.ignore();  // ignore "/"

						   // read in a texture index
			fin >> textureIndex;
			if (fin.bad())
			{
				PrintError(ERROR_MSG, "error about reading of the texture index");
				return false;
			}
			fin.ignore();  // ignore "/"

						   // read in an index of the normal vector
			fin >> normalIndex;
			if (fin.bad())
			{
				PrintError(ERROR_MSG, "error about reading of the normal index");
				return false;
			}
			fin.get();     // read up the space (or '\n') after each set of v/vt/vn

			// write point/texture/normal data into the vertexArray
			vertexIndex--;
			textureIndex--;
			normalIndex--;

			vertexIndicesArray_.push_back(vertexIndex);   // write the index of a vertex coord
			textureIndicesArray_.push_back(textureIndex);  // write the index of a texture coord
		}
	}
	return true;
}


// here we write into the output model file the model data
bool ModelConverterClass::WriteDataIntoOutputFile(ofstream & fout)
{
	


	this->WriteIndicesIntoOutputFile(fout);
	this->WriteVerticesIntoOutputFile(fout);
	this->WriteTexturesIntoOutputFile(fout);


	return true;
}


bool ModelConverterClass::WriteIndicesIntoOutputFile(ofstream & fout)
{
	// VERTEX INDICES WRITING
	fout << "Vertex Indices Data:" << "\n\n";

	for (size_t it = 0; it < vertexIndicesArray_.size() - 2; it += 3)
	{
		fout << vertexIndicesArray_[it + 2] << ' ';
		fout << vertexIndicesArray_[it + 1] << ' ';
		fout << vertexIndicesArray_[it] << endl;
	}
	fout.seekp(-1, ios::cur);
	fout << "\n\n";


	// TEXTURE INDICES WRITING
	fout << "Texture Indices Data:" << "\n\n";

	for (size_t it = 0; it < textureIndicesArray_.size() - 2; it += 3)
	{
		fout << textureIndicesArray_[it + 2] << ' ';
		fout << textureIndicesArray_[it + 1] << ' ';
		fout << textureIndicesArray_[it] << endl;
	}

	return true;
}


// VERTICES DATA WRITING
bool ModelConverterClass::WriteVerticesIntoOutputFile(ofstream & fout)
{
	

	return true;
}


// TEXTURES DATA WRITING
bool ModelConverterClass::WriteTexturesIntoOutputFile(ofstream & fout)
{
	/*
	
	fout << "Textures Data:" << "\n\n";

	for (size_t index = 0; index < textureCoordsCount_; index++)
	{
		fout.setf(ios::fixed, ios::floatfield);
		fout.precision(6);

		fout << setprecision(4)
			<< pTexCoord_[index].x << " "
			<< 1.0f - pTexCoord_[index].y << " ";  // invert the value to use it in the left handed coordinate system

		fout << "\n";
	}

	
	
	*/
	return true;
}









// after each convertation we MUST reset the state of the converter 
// for proper later convertations
bool ModelConverterClass::ResetConverterState()
{
	/*
	
	delete[] pPoint3D_;
	pPoint3D_ = nullptr;

	delete[] pTexCoord_;
	pTexCoord_ = nullptr;

	delete[] pNormal_;
	pNormal_ = nullptr;

	*/
	verticesCount_ = 0;
	textureCoordsCount_ = 0;
	normalsCount_ = 0;
	facesCount_ = 0;

	//modelData.clear();
	vertexIndicesArray_.clear();
	textureIndicesArray_.clear();
	inputLineBuffer_[0] = '\0';

	return true;
}


// makes a final name for the file where we'll place model data
/*
bool ModelConverterClass::GetOutputModelFilename(string & fullFilename, const string & rawFilename)
{
	size_t pointPos = rawFilename.rfind('.');
	fullFilename = { rawFilename.substr(0, pointPos) + ".txt" };

	return true;
}
*/



void ModelConverterClass::PrintError(char* message, ...)
{
	va_list args;
	int len = 0;
	char* buffer = nullptr;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, 0x0004);  // set console text color to red

	va_start(args, message);
	len = _vscprintf(message, args) + 1;	// +1 together with '/0'

	try
	{
		buffer = new char[len];
	}
	catch (std::bad_alloc & e)
	{
		printf("ModelConverterClass: PrintError(): %s", e.what());
		printf("ModelConverterClass: PrintError(): can't allocate memory for the buffer");
		va_end(args);
		return;
	}


	vsprintf_s(buffer, len, message, args);

	clock_t cl = clock();
	char time[9];

	_strtime_s(time, 9);
	printf("%s::%d|\t%s%s\n\n", time, cl, "ERROR: ", buffer);

	delete[] buffer;
	buffer = nullptr;

	SetConsoleTextAttribute(handle, 0x0007);   // set console text color to white

	return;
}





/*
// write in vertices data
vtnData[faceIndex].x = pPoint3D_[vertexNum].x;
vtnData[faceIndex].y = pPoint3D_[vertexNum].y;
vtnData[faceIndex].z = pPoint3D_[vertexNum].z * -1.0f;    // invert the value to use it in the left handed coordinate system

// write in texture coordinates data
vtnData[faceIndex].tu = pTexCoord_[textureNum].tu;
vtnData[faceIndex].tv = 1.0f - pTexCoord_[textureNum].tv; // invert the value to use it in the left handed coordinate system

// write in normals data
vtnData[faceIndex].nx = pNormal_[normalNum].nx;
vtnData[faceIndex].ny = pNormal_[normalNum].ny;
vtnData[faceIndex].nz = pNormal_[normalNum].nz * -1.0f;   // invert the value to use it in the left handed coordinate system





string progressSymbols{ "|/-\\" };
size_t progressSymbolsIndex = 0;
size_t facesCount = modelData.size();
fout << "Vertex Count: " << facesCount << "\n\n"; // to build a face we need 3 vertices
fout << "Data:" << "\n\n";
for (size_t i = 0; i < facesCount; i++)
{
// print information about the writing progress into the console
if (ModelConverterClass::PRINT_CONVERT_PROCESS_MESSAGES_)
{
if (i % 2000 == 0 || (i == facesCount - 1))
{
float percentage = (float)(i + 1) / (float)facesCount * 100.0f;  // calculate the percentage of the writing progress
std::cout << "\t\tWriting faces data into the file: ";
std::cout << (int)percentage << "%  ";
std::cout << progressSymbols[progressSymbolsIndex];
std::cout << '\r';
if (progressSymbolsIndex == progressSymbols.size())
progressSymbolsIndex = 0;
else
progressSymbolsIndex++;
}
}
*/