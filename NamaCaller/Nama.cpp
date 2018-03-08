#include "Nama.h"

#include <funama.h>
#include <authpack.h>

#pragma comment(lib, "nama.lib")

#include <YXLHelper.h>


Nama::Nama()
{
}

void Nama::Init(std::string v3Path)
{
	std::vector<char> v3data;
	CV_Assert(true == YXL::LoadFileContentBinary(v3Path, v3data));
	int ret = fuSetup((float*)&v3data[0], NULL, g_auth_package, sizeof(g_auth_package));

	fuSetExpressionCalibration(0);
}

void Nama::InitArdataExt(std::string path)
{
	if (_ardata_ext_path != path && false == CmFile::FileExist(path))
	{
		return;
	}
	std::vector<char> data;
	CV_Assert(true == YXL::LoadFileContentBinary(path, data));
	std::cout << "load ardata ext." << std::endl;
	fuLoadExtendedARData(&data[0], static_cast<int>(data.size()));
	_ardata_ext_path = path;
}

void Nama::ClearState()
{
	fuOnCameraChange();
	cv::Mat tmp(10, 10, CV_8UC4, cv::Scalar::all(0));
	int h = 0;
	fuRenderItemsEx(FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(tmp.data),
		FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(tmp.data),
		tmp.cols, tmp.rows, _frameID, &h, 1);

	fuDestroyAllItems();
	fuSetExpressionCalibration(0);
}

cv::Mat Nama::Process(cv::Mat img, std::vector<int> props)
{
	if (3 == img.channels())
	{
		cv::cvtColor(img, img, CV_BGR2BGRA);
	}
	int* ptr = props.empty() ? nullptr : &props[0];

	fuRenderItemsEx(FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(img.data),
		FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(img.data),
		img.cols, img.rows, _frameID, ptr, static_cast<int>(props.size()));
	
	//fuRenderItems(0, reinterpret_cast<int*>(img.data), img.cols, img.rows, _frameID, &props[0], props.size());
	++_frameID;
	return img;
}

int Nama::GetPropHandle(std::string & path)
{
	if (_propHandles.find(path) == _propHandles.end())
	{
		_propHandles[path] = CreateProp(path);
	}

	return _propHandles[path];
}

void Nama::SetParam(int prop_id, std::string param, double val)
{
	fuItemSetParamd(prop_id, &param[0], val);
}

void Nama::SetParam(int prop_id, std::string param, std::string val)
{
	fuItemSetParams(prop_id, &param[0], &val[0]);
}

int Nama::CreateProp(std::string & path)
{
	std::vector<char> data;
	if (false == YXL::LoadFileContentBinary(path, data))
	{
		return 0;
	}

	std::cout << "load bundle data: "<<path << std::endl;

	int handle = fuCreateItemFromPackage(&data[0], static_cast<int>(data.size()));
	return handle;
}
