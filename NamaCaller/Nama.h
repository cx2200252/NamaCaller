#pragma once
#include "CmFile.h"

class Nama
{
public:
	Nama();
	void Init(std::string v3Path);

	void InitArdataExt(std::string path);

	void ClearState();

	cv::Mat Process(cv::Mat img, std::vector<int> props);

	int GetPropHandle(std::string& path);

	void SetParam(int prop_id, std::string param, double val);
	void SetParam(int prop_id, std::string param, std::string val);

private:
	static int CreateProp(std::string & path);

private:
	int _frameID = 0;
	std::map<std::string, int> _propHandles;
	std::string _ardata_ext_path = "";
};