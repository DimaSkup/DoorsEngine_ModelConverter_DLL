///////////////////////////////////////////////////////////////////////////////
// Filename: log.cpp
// There is a log system source file
///////////////////////////////////////////////////////////////////////////////
#include "Log.h"


Log* Log::m_instance = nullptr;
HANDLE Log::handle = GetStdHandle(STD_OUTPUT_HANDLE);
FILE* Log::m_file = nullptr;


Log::Log(void)
{
	if (!m_instance) // we can have only one instance of logger
	{
		m_instance = this;
		m_init();

		printf("Log::Log(): a log system for the MODEL CONVERTER is created successfully\n");
	}
	else
	{
		printf("Log::Log(): there is already exists one instance of the log system for the MODEL CONVERTER\n");
	}
}

Log::~Log(void)
{
	if (!m_file)
		return;

	m_close();
	fflush(m_file);
	fclose(m_file);

	printf("Log::~Log(): the log system is destroyed\n");
}


// returns a pointer to the instance of the Log class
Log* Log::Get() { return m_instance; }



// make and open a logger text file
void Log::m_init(void)
{
	if (fopen_s(&m_file, "log_model_converter.txt", "w") == 0)
	{
		printf("Log::m_init(): the log file is created successfully\n");

		char time[9];
		char date[9];

		_strtime_s(time, 9);
		_strdate_s(date, 9);

		fprintf(m_file, "%s : %s| the log file is created\n", time, date);
		fprintf(m_file, "-------------------------------------------\n\n");
	}
	else
	{
		printf("Log::m_init(): can't create the log file\n");
	}
}


// print message about closing of the logger file
void Log::m_close(void)
{
	char time[9];
	char date[9];

	_strtime_s(time, 9);
	_strdate_s(date, 9);

	fprintf(m_file, "\n-------------------------------------------\n");
	fprintf(m_file, "%s : %s| the end of the log file\n", time, date);
}


// prints a usual message
void Log::Print(char* message, ...)
{
	va_list args;
	int len = 0;
	char* buffer = nullptr;

	va_start(args, message);

	len = _vscprintf(message, args) + 1;	// +1 together with '\0'
	try
	{
		buffer = new char[len];

		vsprintf_s(buffer, len, message, args);


		SetConsoleTextAttribute(Log::handle, 0x000A);
		Log::m_print("", buffer);
		SetConsoleTextAttribute(Log::handle, 0x0007);
	}
	catch (std::bad_alloc & e)
	{
		printf("Log::Print(): ERROR: %s", e.what());
		printf("Log::Print(): can't allocate memory for the buffer");
		va_end(args);

		return;
	}

	delete[] buffer;
	buffer = nullptr;

	va_end(args);
}


void Log::Debug(const char* funcName, int codeLine, const std::string & message)
{
#ifdef _DEBUG
	std::stringstream ss;
	ss << funcName << "() (line: " << codeLine << "): " << message;
	m_print("DEBUG: ", ss.str().c_str());

	return;
#endif
}

// prints a debug message
void Log::Debug(const char* funcName, int codeLine, const char* message)
{
#ifdef _DEBUG
	std::stringstream ss;
	ss << funcName << "() (line: " << codeLine << "): " << message;
	m_print("DEBUG: ", ss.str().c_str());

	return;
#endif
}





// prints an error message
void Log::Error(char* message, ...)
{
	va_list args;
	int len = 0;
	char* buffer = nullptr;
	SetConsoleTextAttribute(Log::handle, 0x0004);  // set console text color to red

	va_start(args, message);

	len = _vscprintf(message, args) + 1;	// +1 together with '\0'

	// try to allocate memory for the symbols buffer
	try
	{
		buffer = new char[len];
	}
	catch (std::bad_alloc & e)
	{
		printf("Log::Error(char* message, ...): ERROR: %s", e.what());
		printf("Log::Error(char* message, ...): can't allocate memory for the buffer");
		va_end(args);
		return;
	}

	// fill in the buffer with data
	vsprintf_s(buffer, len, message, args);

	// print the error message into the console and write it into the log file
	Log::m_print("ERROR: ", buffer);
	SetConsoleTextAttribute(Log::handle, 0x0007);

	// free memory
	delete[] buffer;
	buffer = nullptr;

	va_end(args);

	return;
}




// a helper for printing messages into the command prompt and into the logger text file
void Log::m_print(char* levtext, const char* text)
{
	clock_t cl = clock();
	char time[9];

	_strtime_s(time, 9);
	printf("%s::%d|\t%s%s\n", time, cl, levtext, text);

	if (m_file)
	{
		fprintf(m_file, "%s::%d|\t%s %s\n", time, cl, levtext, text);
	}
}