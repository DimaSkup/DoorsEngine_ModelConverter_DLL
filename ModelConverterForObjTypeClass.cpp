#include "ModelConverterForObjTypeClass.h"

#include <cstdio>   // for using a remove() method for deleting files
#include <cassert>


ModelConverterForObjTypeClass::ModelConverterForObjTypeClass(void)
{
	try
	{
		inputLineBuffer_ = new char[ModelConverterForObjTypeClass::INPUT_LINE_SIZE_]{ '\0' };   // during execution of the getline() function we put here a one single text line
	}
	catch (std::bad_alloc & e)
	{
		PrintError(ERROR_MSG, e.what());
		PrintError(ERROR_MSG, "can't allocate memory for some members of the class");
		exit(-1);
	}
}

ModelConverterForObjTypeClass::~ModelConverterForObjTypeClass(void)
{
	Shutdown();
}




// ----------------------------------------------------------------------------------- //
//
//                          PUBLIC METHODS
//
// ----------------------------------------------------------------------------------- //

void ModelConverterForObjTypeClass::Shutdown(void)
{
	delete[] inputLineBuffer_;
	inputLineBuffer_ = nullptr;
}


// converts a model of the ".obj" type into the internal model format
bool ModelConverterForObjTypeClass::ConvertFromObj(const char* inputFilename, const char* outputFilename)
{
	// print names of the input/output file
	this->PrintInOutFilenames(inputFilename, outputFilename);

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
	bool result = this->ConvertFromObjHelper(fin, fout);
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


// print into console/log file names of the input/output data file
void ModelConverterForObjTypeClass::PrintInOutFilenames(const char* inputFilename, const char* outputFilename) const
{
	// generate a debug message with input/output filenames
	std::stringstream ss;

	ss << "DATA FILES:\n " << "IN: " << inputFilename << "\n " << "OUT: " << outputFilename;
	std::cout << ss.str() << "\n\n";

	return;
}







// ----------------------------------------------------------------------------------- //
//
//                          PRIVATE METHODS / HELPERS
//
// ----------------------------------------------------------------------------------- //

// help us to convert .obj file model data into the internal model format
bool ModelConverterForObjTypeClass::ConvertFromObjHelper(ifstream& fin, ofstream& fout)
{
	bool result = false;

	// read the count of vertices, textures coordinates and the count of faces as well
	this->ReadCounts(fin);

#ifdef _DEBUG 

	// print positions of input file pointers before some particular data block
	std::cout << "FILE PTR POSITIONS:\n";
	std::cout << "before VERTICES: " << posBeforeVerticesData_ << '\n';
	std::cout << "before TEXTURES: " << posBeforeTexturesData_ << '\n';
	std::cout << "before NORMALS:  " << posBeforeNormalsData_ << '\n';
	std::cout << "before FACES:    " << posBeforeFacesData_ << '\n';

	std::cout << "\n";

	// print counts of vertices/texture coords/normals/faces
	std::cout << "COUNTS:\n";
	std::cout << "VERTICES COUNT: " << verticesCount_ << '\n';
	std::cout << "TEXTURES COUNT: " << textureCoordsCount_ << '\n';
	std::cout << "NORMALS COUNT:  " << normalsCount_ << '\n';
	std::cout << "FACES COUNT:    " << facesCount_ << std::endl;

	std::cout << "\n\n";

#endif


	// write the number of vertices/indices/texture coords into the output data file
	fout << "Vertex Count: " << verticesCount_ << "\n";
	fout << "Indices Count: " << facesCount_ * 3 << "\n";         // each face has 3 vertices
	fout << "Textures Count: " << textureCoordsCount_ << "\n\n";


	// because earlier we've went up to the end of the file (EOF) 
	// during reading of counts we have to clear ifstream state
	fin.clear();
	fin.seekg(0, fin.beg);   // go to the beginning of the file

	// handle vertices data
	if (!ReadInAndWriteVerticesData(fin, fout))
	{
		PrintError(ERROR_MSG, "can't read/write vertices data");
		return false;
	}
	std::cout << "VERTICES DATA WAS HANDLED CORRECTLY" << std::endl;


	// handle texture coords data
	if (!ReadInAndWriteTexturesData(fin, fout))
	{
		PrintError(ERROR_MSG, "can't read/write textures data");
		return false;
	}
	std::cout << "TEXTURE DATA WAS HANDLED CORRECTLY" << std::endl;


	/*
	// handle normals data
	if (!ReadInAndWriteNormalsData(fin, fout))
	{
		PrintError(ERROR_MSG, "can't read/write normals data");
		return false;
	}

	std::cout << "NORMALS DATA WAS HANDLED CORRECTLY" << std::endl;
	*/


	// read in faces data
	if (!ReadInFacesData(fin))
	{
		PrintError(ERROR_MSG, "can't read in faces data");
		return false;
	}
	std::cout << "FACES DATA WAS READ IN SUCCESSFULLY" << std::endl;

	// because earlier we've went up to
	// the end of the file (EOF) we have to clear ifstream state
	fin.clear();             

	// write faces data
	if (!this->WriteIndicesIntoOutputFile(fout))
	{
		PrintError(ERROR_MSG, "can't read/write indices data");
		return false;
	}

	std::cout << "FACES DATA WAS WRITTEN SUCCESSFULLY" << std::endl;
	std::cout << "\n-----   CONVERTATION IS FINISHED   -----\n\n" << std::endl;

	return true;
}


void ModelConverterForObjTypeClass::SkipUntilVerticesData(ifstream & fin)
{
	// contains a position right before the vertices data
	streampos posBeforeVerticesData = 0;

	// Read up to the vertex values
	while (inputLineBuffer_[0] != 'v')
	{
		posBeforeVerticesData = fin.tellg();                                  // store the position in input sequence
		fin.getline(inputLineBuffer_, ModelConverterForObjTypeClass::INPUT_LINE_SIZE_); // get a line from the file						
	}

	// we got to the vertices data so return a stream pointer to the position right before the vertices data
	fin.seekg(posBeforeVerticesData);

	return;
}


// read the count of vertices, textures coordinates and the count of faces as well
void ModelConverterForObjTypeClass::ReadCounts(ifstream & fin)
{
	this->SkipUntilVerticesData(fin);

	posBeforeVerticesData_ = fin.tellg();	 // later we'll return to this position so save it
	fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_); // read the first line of vertices data


	this->CalculateCount(fin, verticesCount_, posBeforeTexturesData_, "VERTICES", "v ", "vt");
	this->CalculateCount(fin, textureCoordsCount_, posBeforeNormalsData_, "TEXTURES", "vt", "vn");
	this->CalculateCount(fin, normalsCount_, posBeforeFacesData_, "NORMALS", "vn", "f");

	// calculate the number of faces in a separate way because faces data can be
	// separated with some technical lines or empty space
	while (!fin.eof())  
	{
		if ((inputLineBuffer_[0] == 'f') && (inputLineBuffer_[1] == ' '))
		{
			facesCount_++;
		}
		fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
	}

	return;
}


// calculate counts for some particular data block 
// (for instance: count of vertices, position before the next block, etc.)
void ModelConverterForObjTypeClass::CalculateCount(ifstream & fin,
	int & countOfData,               // 1. how many lines this block of data has
	streampos & posBeforeNextBlock,  // 2. file ptr position right before next block of data
	std::string dataType,            // 3. which type of data we're reading from the current data block
	std::string prefix,              // 4. each line of the current data block starts with this prefix
	std::string skipUntilPrefix)     // 5. skip input symbols until we come across this prefix (the next data block)
{
	try
	{
		// while we're reading lines which start with prefix
		while ((inputLineBuffer_[0] == prefix[0]) && (inputLineBuffer_[1] == prefix[1]))
		{
			countOfData++;
			//std::cout << inputLineBuffer_ << "\n";
			fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
		}
		
		// skip lines until the next block of data
		while ((inputLineBuffer_[0] != skipUntilPrefix[0]) &&
			   (inputLineBuffer_[1] != skipUntilPrefix[1]))
		{
			// std::cout << "skip line: " << inputLineBuffer_ << std::endl;   // for debug purpose
			fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_);
		}

		// calculate the file ptr position right before the next data block
		posBeforeNextBlock = fin.tellg();
		posBeforeNextBlock -= strlen(inputLineBuffer_);

	}
	catch (std::ifstream::failure e)
	{
		this->PrintError(ERROR_MSG, "Exception reading file:");
		this->PrintError(ERROR_MSG, e.what());
	}

	return;
}



// in this function we read in vertices data from the input data file 
// and right after it we write this data into the output data file
bool ModelConverterForObjTypeClass::ReadInAndWriteVerticesData(ifstream & fin, ofstream & fout)
{
	char input;                              // for reading the '\n' symbol
	VERTEX3D  vertex3D;                      // will contain vertex data

	try
	{
		fin.seekg(posBeforeVerticesData_);   // return to the position before the vertices data 
		fout << "\nVertices Data:\n";        // write into the output file that the following data block is vertices data


		for (size_t i = 0; i < verticesCount_; i++)
		{
			// read vertex data from the input data file 
			fin.ignore(1);                                              // skip the "v" symbol at the beginning of the line
			fin >> vertex3D.x >> vertex3D.y >> vertex3D.z >> input;     // read in x, y, z vertex coordinates and the '\n' symbol

			fout.setf(ios::fixed, ios::floatfield);
			fout.precision(6);

			// write this vertex coordinates into the output data file
			fout << vertex3D.x << " " 
				 << vertex3D.y << " "
				 << vertex3D.z * -1.0f    // invert the value to use it in the left handed coordinate system
				 << "\n";
		}

		fout << "\n\n";                   // in the output data file: make a separation space before the next data block 
	}
	catch (std::ifstream::failure & e)
	{
		this->PrintError(ERROR_MSG, "Exception reading/writing file:");
		this->PrintError(ERROR_MSG, e.what());
	}

	return true;
}

bool ModelConverterForObjTypeClass::ReadInAndWriteTexturesData(ifstream & fin, ofstream & fout)
{
	TEXTURE_COORDS texCoords;
	std::string vt{ "" };          // we will write here "vt" symbols in the beginning of the line
	
	try
	{
		fin.seekg(posBeforeTexturesData_, fin.beg);    // return to the position before the textures data 
		fout << "\nTextures Data:\n";                  // write into the output file that the following data block is textures data

		for (size_t i = 0; i < textureCoordsCount_; i++)
		{
			// read texture data from the input data file 
			fin >> vt >> texCoords.tu >> texCoords.tv;    // read in texture coords data

			fout.setf(ios::fixed, ios::floatfield);
			fout.precision(6);

			// write this texture coords data into the output data file
			fout << texCoords.tu << " "
				 << 1.0f - texCoords.tv       // invert the value to use it in the left handed coordinate system
				 << "\n";
		}

		fout << "\n\n";                       // in the output data file: make a separation space before the next data block 
	}
	catch (std::ifstream::failure & e)
	{
		this->PrintError(ERROR_MSG, "Exception reading/writing file:");
		this->PrintError(ERROR_MSG, e.what());
	}

	return true;
}


bool ModelConverterForObjTypeClass::ReadInAndWriteNormalsData(ifstream& fin, ofstream & fout)
{
	NORMAL normal;
	char input;

	try
	{
		fin.seekg(posBeforeNormalsData_, fin.beg);   // return to the position before the normals data 
		fout << "\nNormals Data:\n";                 // write into the output file that the following data block is normals data

		for (size_t i = 0; i < normalsCount_; i++)
		{
			// read normal data from the input data file 
			fin.ignore(2);                                       // skip "vn " symbols in the beginning of line
			fin >> normal.nx >> normal.ny >> normal.nz >> input; // read in the normal data and a new line symbol

			fout.setf(ios::fixed, ios::floatfield);
			fout.precision(6);
			
			// write this normal data into the output data file
			fout << normal.nx << ' '
				 << normal.ny << ' '
				 << normal.nz * -1.0f   // invert the value to use it in the left handed coordinate system
				 << '\n';
		}
	}
	catch (std::ifstream::failure & e)
	{
		this->PrintError(ERROR_MSG, "Exception reading/writing file:");
		this->PrintError(ERROR_MSG, e.what());
	}

	return true;
}




bool ModelConverterForObjTypeClass::ReadInFacesData(ifstream & fin)
{
	// clear the arrays if we had some data in it before
	delete[] pVertexIndicesArray_;
	delete[] pTextureIndicesArray_;
	pVertexIndicesArray_ = nullptr;
	pTextureIndicesArray_ = nullptr;

	// allocate memory for vertices/texture coords indices
	pVertexIndicesArray_ = new UINT[facesCount_ * 3]{ 0 };   // each face has 3 vertices
	pTextureIndicesArray_ = new UINT[facesCount_ * 3]{ 0 };  // each face has 3 texture coords


	UINT vertexIndex = 0;
	UINT textureIndex = 0;
	UINT normalIndex = 0;
	UINT arrayIndex = 0;

	char slash{ ' ' };   // we'll write here a slash symbol

	inputLineBuffer_[0] = '\0';
	
	fin.seekg(posBeforeFacesData_, fin.beg);	// now we at the position before the beginning of polygonal face data
	fin.seekg(-1, fin.cur);
	

	// go through each face until the end of the file (EOF)
	while (!fin.eof())
	{
		fin.getline(inputLineBuffer_, INPUT_LINE_SIZE_, '\n');

		// if the current line doesn't contain data of a face we just skip this line
		if (inputLineBuffer_[0] != 'f')
		{
			//std::cout << "skip line: " << inputLineBuffer_ << std::endl;    // for debug purpose
		}
		// this line contains face data
		else  
		{
			// turn back at the beginning of this line
			fin.seekg(-1 - strlen(inputLineBuffer_), fin.cur);  
			
			std::string symbolsAtLineBeginning;
			fin >> symbolsAtLineBeginning;

			//std::cout << "symbols: " << symbolsAtLineBeginning << std::endl;
			
			// go through each vertex of the current face
			for (size_t faceVertex = 1; faceVertex <= 3; faceVertex++)
			{
				
				fin >> vertexIndex;      // read in a VERTEX index
				if (fin.bad())
				{
					PrintError(ERROR_MSG, "error about reading of the vertex index");
					return false;
				}
				fin.ignore();            // ignore "/"
		  

				fin >> textureIndex;     // read in a TEXTURE index
				if (fin.bad())
				{
					PrintError(ERROR_MSG, "error about reading of the texture index");
					return false;
				}
				fin.ignore();            // ignore "/"
		   

				fin >> normalIndex;      // read in an index of the NORMAL vector
				if (fin.bad())
				{
					PrintError(ERROR_MSG, "error about reading of the normal index");
					return false;
				}

				fin.get();     // read up the space (or '\n') after each set of v/vt/vn


				
				vertexIndex--;
				textureIndex--;
				normalIndex--;

				// write vertex/texture data into the vertex/texture indices arrays
				pVertexIndicesArray_[arrayIndex] = vertexIndex;
				pTextureIndicesArray_[arrayIndex] = textureIndex;
				arrayIndex++;

			} // for
		} // else
	} // while(!fin.eof())

	return true;
}


// write vertex/texture coords indices into the output data file
bool ModelConverterForObjTypeClass::WriteIndicesIntoOutputFile(ofstream & fout)
{
	// VERTEX INDICES WRITING
	fout << "Vertex Indices Data:" << "\n\n";

	for (size_t it = 0; it < facesCount_ * 3 - 2; it += 3)
	{
		fout << pVertexIndicesArray_[it + 2] << ' ';
		fout << pVertexIndicesArray_[it + 1] << ' ';
		fout << pVertexIndicesArray_[it] << endl;
	}
	fout.seekp(-1, ios::cur);
	fout << "\n\n";


	// TEXTURE INDICES WRITING
	fout << "Texture Indices Data:" << "\n\n";

	for (size_t it = 0; it < facesCount_ * 3 - 2; it += 3)
	{
		fout << pTextureIndicesArray_[it + 2] << ' ';
		fout << pTextureIndicesArray_[it + 1] << ' ';
		fout << pTextureIndicesArray_[it] << endl;
	}

	return true;
}

// for DEBUG purpose
void ModelConverterForObjTypeClass::PrintError(char* message, ...)
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
		std::cout << "ModelConverterForObjTypeClass: PrintError(): " << e.what() << '\n';
		std::cout << "ModelConverterForObjTypeClass: PrintError(): can't allocate memory for the buffer" << std::endl;

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
if (ModelConverterForObjTypeClass::PRINT_CONVERT_PROCESS_MESSAGES_)
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