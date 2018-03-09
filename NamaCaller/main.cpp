#include <iostream>
#include "Config.h"
#include "Nama.h"
#include <YXLHelper.h>

//opengl
#include <gl\glew.h>
#include <GLFW\glfw3.h>

#ifndef GL_LIB
#ifdef _DEBUG
#define GL_LIB(name) name "d.lib"
#else
#define GL_LIB(name) name ".lib"
#endif
#endif

#pragma comment(lib, GL_LIB("glew32"))
#pragma comment(lib, GL_LIB("glfw3"))
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")

NC_config option;

std::multimap<std::string, std::string> params;

void CmdParser(int argc, char** argv)
{
	YXL::CmdLineParser(argc, argv, [](const std::string& name, const std::string& val) {
		params.insert(std::make_pair(name, val));
		return true;
	});

	for (auto param : params)
	{
		if("-json"==param.first)
			option.jsonPath = param.second;
		else if ("-jsoncontent" == param.first)//something wrong
			option.jsonPath = param.second;
		else if ("-wait" == param.first)
			option.Press2Exit = "0" == param.second ? false : true;
		else if ("-opendir" == param.first)
			option.OpenOutDir = "0" == param.second ? false : true;
		else if ("-showconfig" == param.first)
			option.ShowConfig = "0" == param.second ? false : true;
		else if ("-confirm" == param.first)
			option.NeedConfirm = "0" == param.second ? false : true;
	}
}

static GLFWwindow* window = NULL;
void InitOpenGL()
{
	bool res = glfwInit()==GLFW_TRUE;
	if (false == res)
	{
		CV_Assert(false && "could not start GLFW3.");
	}
	const std::string title = "mask generate";

	GLFWwindow* wnd(nullptr);

	glfwWindowHint(GLFW_VISIBLE, 0);
	wnd = glfwCreateWindow(100, 100, title.c_str(), NULL, NULL);
	if (nullptr == wnd)
	{
		glfwTerminate();
		CV_Assert(false && "could not open window with GLFW3.");
	}

	glfwMakeContextCurrent(wnd);
	glfwHideWindow(wnd);

	glfwSwapInterval(0);

	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		CV_Assert(false && "glew initialization failed.");
	}
}

int main(int argc, char** argv)
{
	HANDLE hObject = CreateMutexA(NULL, true, "NamaCaller.exe");
	if (NULL== hObject || GetLastError() == ERROR_ALIAS_EXISTS)
	{
		CloseHandle(hObject);
		std::cout << "已有NamaCaller正在运行..." << std::endl;

		return 0;
	}

	if (false == CmFile::FolderExist("resources"))
	{
		if (CmFile::FolderExist("../resources"))
			CmFile::SetWkDir("../");
		else if (CmFile::FolderExist("../../resources"))
			CmFile::SetWkDir("../../");
	}

	/*std::string tmp = CmFile::BrowseFile();
	std::cout << tmp << std::endl;
	std::cout << CmFile::GetFolder(tmp) << std::endl;
	auto a = CmFile::GetFolder(tmp);
	std::cout << CmFile::GetName(a.substr(0, a.length()-1)) << std::endl;
	return 0;*/

	CmdParser(argc, argv);

	Caller caller;
	bool ret = caller.Load(option);
	if (false == ret)
	{
		return 0;
	}

	InitOpenGL();
	std::shared_ptr<Nama> nama = std::shared_ptr<Nama>(new Nama);
	nama->Init("resources/v3.bundle");
	nama->InitArdataExt("resources/ardata_ex.bundle");
	
	caller.Process(option, nama);
	
	return 0;
}