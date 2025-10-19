#include "FEPythonManager.h"
using namespace FocalEngine;

#ifdef FOCAL_ENGINE_SHARED
extern "C" __declspec(dllexport) void* GetPythonManager()
{
	return FEPythonManager::GetInstancePointer();
}
#endif

FEPythonManager::FEPythonManager()
{
	
}

FEPythonManager::~FEPythonManager()
{
	Finalize();
}

std::string FEPythonManager::RemoveExcessiveIndentation(const std::string& PythonCode)
{
	std::istringstream PythonCodeStream(PythonCode);
	std::vector<std::string> CodeLines;
	CodeLines.reserve(128);

	// Read all lines
	for (std::string CurrentLine; std::getline(PythonCodeStream, CurrentLine); )
		CodeLines.emplace_back(std::move(CurrentLine));

	// If the FirstNonWhitespaceIndex line is empty, drop it
	size_t FirstLineIndex = 0;
	if (!CodeLines.empty() && CodeLines[0].find_first_not_of(" \t\r") == std::string::npos)
		FirstLineIndex = 1;

	// Find minimal common indentation across non-blank lines.
	size_t MinIndentCount = std::numeric_limits<size_t>::max();
	for (size_t i = FirstLineIndex; i < CodeLines.size(); i++)
	{
		const std::string& CurrentLine = CodeLines[i];
		size_t FirstNonWhitespaceIndex = CurrentLine.find_first_not_of(" \t\r");
		if (FirstNonWhitespaceIndex == std::string::npos)
			continue;

		size_t IndentCount = 0;
		while (IndentCount < CurrentLine.size() && (CurrentLine[IndentCount] == ' ' || CurrentLine[IndentCount] == '\t')) ++IndentCount;
		if (IndentCount < MinIndentCount) MinIndentCount = IndentCount;
	}

	if (MinIndentCount == std::numeric_limits<size_t>::max())
		MinIndentCount = 0;

	// Build output, trimming that many leading white spaces from non-blank CodeLines
	std::string TrimmedCode;
	TrimmedCode.reserve(PythonCode.size());
	for (size_t i = FirstLineIndex; i < CodeLines.size(); i++)
	{
		const std::string& CurrentLine = CodeLines[i];
		size_t FirstNonWhitespaceIndex = CurrentLine.find_first_not_of(" \t\r");
		if (FirstNonWhitespaceIndex == std::string::npos)
		{
			TrimmedCode += '\n';
			continue;
		}

		size_t CutIndex = 0;
		while (CutIndex < MinIndentCount && CutIndex < CurrentLine.size() && (CurrentLine[CutIndex] == ' ' || CurrentLine[CutIndex] == '\t'))
			++CutIndex;

		TrimmedCode += CurrentLine.substr(CutIndex);
		TrimmedCode += '\n';
	}

	return TrimmedCode;
}

std::string FEPythonManager::GetOutput()
{
	if (!IsInitialized())
		return "";

	PyObject* Main = PyImport_AddModule("__main__");
	if (Main == nullptr)
		return "";

	PyObject* Capture = PyObject_GetAttrString(Main, "Capture");
	if (Capture == nullptr)
	{
		Py_XDECREF(Main);
		return "";
	}

	PyObject* Output = PyObject_GetAttrString(Capture, "Output");
	if (Output == nullptr)
	{
		Py_XDECREF(Capture);
		return "";
	}

	PyObject* Result = PyObject_CallMethod(Output, "getvalue", nullptr);
	if (Result == nullptr)
	{
		Py_XDECREF(Output);
		Py_XDECREF(Capture);
		return "";
	}

	std::string PythonOutputText;
	if (PyUnicode_Check(Result))
		PythonOutputText = PyUnicode_AsUTF8(Result);

	Py_XDECREF(Result);
	Py_XDECREF(Output);
	Py_XDECREF(Capture);

	return PythonOutputText;
}

bool FEPythonManager::Initialize()
{
	Py_Initialize();

	if (!Py_IsInitialized())
		return false;

	const char* CaptureOutputCode = R"(
	import sys
	from io import StringIO
	class CaptureOutput:
		def __init__(self):
			self.Output = StringIO()
		def write(self, text):
			self.Output.write(text)
		def flush(self):
			pass

	Capture = CaptureOutput()
	sys.stdout = Capture
	sys.stderr = Capture
	)";

	RunSimpleString(CaptureOutputCode);

	return true;
}

bool FEPythonManager::Finalize()
{
	if (IsInitialized())
	{
		Py_Finalize();
		return true;
	}

	return false;
}

bool FEPythonManager::IsInitialized()
{
	return Py_IsInitialized();
}

bool FEPythonManager::RunSimpleString(const std::string& Command)
{
	if (!IsInitialized())
		return false;

	std::string ConvertedLine = RemoveExcessiveIndentation(Command);

	int Result = PyRun_SimpleString(ConvertedLine.c_str());
	if (Result != 0)
		return false;

	return true;
}

void FEPythonManager::Update()
{
	// Let Python threads run
	PyGILState_STATE GState = PyGILState_Ensure();

	// Dummy command to keep Python active
	PyRun_SimpleString("pass");

	PyGILState_Release(GState);
}

bool FEPythonManager::IsModuleAvailable(const std::string& ModuleName)
{
	if (!IsInitialized())
		return false;

	PyObject* Module = PyImport_ImportModule(ModuleName.c_str());
	if (Module == nullptr)
		return false;

	Py_DECREF(Module);
	return true;
}