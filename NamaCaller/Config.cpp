#include "Config.h"
#include "Nama.h"
#include <YXLHelper.h>
#pragma comment( lib, CV_LIB("videoio"))

std::map<std::string, std::string> g_replace_str;


template <typename ValType> void PrintVector(std::vector<ValType>& vec)
{
	std::cout << "[";
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		if (iter != vec.begin())
		{
			std::cout << ", ";
		}
		std::cout << *iter;
	}
	std::cout << "]\n";
}

template <typename ValTypeA, typename ValTypeB> void PrintVector(std::vector<std::pair<ValTypeA, ValTypeB>>& vec)
{
	std::cout << "[";
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		if (iter != vec.begin())
		{
			std::cout << ", ";
		}
		std::cout << "("<<iter->first<<": "<<iter->second<<")";
	}
	std::cout << "]\n";
}

template <typename ValTypeA, typename ValTypeB> void PrintMap(std::map<ValTypeA, ValTypeB>& vec, std::string offset="")
{
	for (auto iter = vec.begin(); iter != vec.end(); ++iter)
	{
		std::cout << offset << "" << iter->first << ": " << iter->second << std::endl;;
	}
}

namespace YXL
{
	namespace JSON
	{
		template<>
		struct ValueGetter<AdditionCalls::SingleCall> {
			static AdditionCalls::SingleCall Get(const rapidjson::Value & val) {
				AdditionCalls::SingleCall call;
				if (val.HasMember("is_before_render") && val["is_before_render"].IsBool())
				{
					call.is_before_render = val["is_before_render"].GetBool();
				}
				call.func = val["func"].GetString();
				if (val.HasMember("params") && val["params"].IsArray())
				{
					for (auto iter = val["params"].Begin(); iter != val["params"].End(); ++iter)
					{
						AdditionCalls::SingleCall::Param param;
						if (iter->IsInt())
						{
							param.is_int = true;
							param.param_i = iter->GetInt();
							call.params.push_back(param);
						}
						else if (iter->IsString())
						{
							param.is_int = false;
							param.param_str = iter->GetString();
							call.params.push_back(param);
						}
					}
				}
				return call;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsObject() && val.HasMember("func") && val["func"].IsString();
			}
		};
	}
}

int FrameIndexToTargetIndex(std::vector<int>& config, int frame_idx)
{
	int target_idx(0);
	if (config.empty())
	{
		target_idx = frame_idx;
	}
	else
	{
		int acc(0), i(-1);
		while (acc <= frame_idx)
		{
			++i;
			if (i < config.size())
			{
				acc += config[i];
			}
			else
			{
				acc += *config.rbegin();
			}
		}
		target_idx = i;
	}
	return target_idx;
}

int FrameIndexToTargetIndex(std::vector<bool>& config, int frame_idx)
{
	bool is_save(true);
	if (false == config.empty())
	{
		is_save = frame_idx < config.size() ? config[frame_idx] : *config.rbegin();
	}
	if (false == is_save)
	{
		return -1;
	}

	int target_idx(0);
	{
		if (config.empty())
		{
			target_idx = frame_idx;
		}
		else
		{
			for (int i(0); i < config.size() && i <= frame_idx; ++i)
			{
				if (config[i])
				{
					++target_idx;
				}
			}
			target_idx += static_cast<int>((frame_idx + 1 > config.size()) ? frame_idx + 1 - config.size() : 0);
			--target_idx;
		}
	}
	return target_idx;
}

template<typename T>
T FrameIndexToTarget(std::vector<T>& config, int frame_idx,  const T& def_val=T())
{
	return frame_idx < config.size() ? config[frame_idx] : def_val;
}

void AddImage(std::vector<std::string>& ret, std::string path)
{
	if (CmFile::FolderExist(path))
	{
		if (!(*path.rbegin() == '/' || *path.rbegin() == '\\'))
		{
			path += "\\";
		}
		vecS tmp1, tmp2, tmp3;
		CmFile::GetNames(path + "*.jpg", tmp1);
		CmFile::GetNames(path + "*.png", tmp2);
		CmFile::GetNames(path + "*.bmp", tmp3);
		
		for (auto iter = tmp1.begin(); iter != tmp1.end(); ++iter)
		{
			ret.push_back(path + *iter);
		}
		for (auto iter = tmp2.begin(); iter != tmp2.end(); ++iter)
		{
			ret.push_back(path + *iter);
		}
		for (auto iter = tmp3.begin(); iter != tmp3.end(); ++iter)
		{
			ret.push_back(path + *iter);
		}
	}
	else  if (CmFile::FileExist(path))
	{
		ret.push_back(path);
	}
	else
	{
		std::cout << "file not exist: " << path << std::endl;
	}
}




/*
source
*/



bool Source::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val)
{
	std::string strType = json.ReadValue<std::string>("type", "none", val);
	if ("none" == strType)
	{
		std::cout << "no source type specified..." << std::endl;
		return false;
	}
	frame_cnt = json.ReadValue("frame_cnt", 0, val);

	frame_repeat.Load(val, "frame_repeat");

	if ("camera" == strType)
	{
		type = TYPE_CAMERA;
		if (val.HasMember("cam_id") && val["cam_id"].IsInt())
		{
			cam_id = val["cam_id"].GetInt();
		}

		if (val.HasMember("width") && val["width"].IsInt())
		{
			size.width = val["width"].GetInt();
		}
		if (val.HasMember("height") && val["height"].IsInt())
		{
			size.height = val["height"].GetInt();
		}
	}
	else if ("image" == strType)
	{
		imgPaths.clear();
		type = TYPE_IMAGE;

		ARRAY_TYPE_0<std::string> path;
		path.Load(val, "path");
		if (path.val.empty())
		{
			auto is_pick_file = json.ReadValue("is_pick_file", false, val);
			auto is_pick_folder = json.ReadValue("is_pick_folder", false, val);
			if (is_pick_file)
			{
				std::string tmp = CmFile::BrowseFile();
				if ("" != tmp)
				{
					AddImage(imgPaths, tmp);
				}
			}
			if (is_pick_folder)
			{
				std::string tmp = CmFile::BrowseFolder();
				if ("" != tmp)
				{
					tmp += "\\";
					AddImage(imgPaths, tmp);
				}
			}
		}
		else
		{
			for (int i(0); i != path.val.size(); ++i)
			{
				AddImage(imgPaths, path.val[i]);
			}
		}
		if (imgPaths.empty())
		{
			std::cout << "no image specified..." << std::endl;
			return false;
		}
		SetReplaceStr(imgPaths[0]);
		if (0 == frame_cnt)
		{
			auto def_val = frame_repeat.val.empty() ? 1 : *frame_repeat.val.rbegin();
			for (int i(0); i != imgPaths.size(); ++i)
			{
				if (frame_repeat.val.size() <= i)
					frame_cnt += def_val;
				else
					frame_cnt += frame_repeat.val[i];
			}
		}
	}
	else if ("video" == strType)
	{
		type = TYPE_VIDEO;

		path = json.ReadValue<std::string>("path", "", val);
		auto is_pick_file = json.ReadValue("is_pick_file", false, val);
		if (""==path && is_pick_file)
		{
			path = CmFile::BrowseFile("video (*.avi;*.mp4)\0*.avi;*.mp4\0\0");
		}
		if ("" == path)
		{
			std::cout << "no source video path specified..." << std::endl;
			return false;
		}
		if (false == CmFile::FileExist(path))
		{
			std::cout << "file not exist: " << path << std::endl;
			return false;
		}
		SetReplaceStr(path);
		is_vertical = json.ReadValue("is_vertical", false, val);

		cv::VideoCapture cap(path);
		if (cap.isOpened() == true)
		{
			if (0 == frame_cnt)
			{
				auto def_val = frame_repeat.val.empty() ? 1 : *frame_repeat.val.rbegin();
				for (int i(0); i != imgPaths.size(); ++i)
				{
					if (frame_repeat.val.size() <= i)
						frame_cnt += def_val;
					else
						frame_cnt += frame_repeat.val[i];
				}
			}
			fps = cap.get(CV_CAP_PROP_FPS);
		}
		else
		{
			std::cout << "can not open this video: " << path << std::endl;
			return false;
		}
	}

	return true;
}

void Source::Print(const std::string offset)
{
	std::cout << offset << "---------source---------" << std::endl;
	std::string typeStr[] = { "camera", "image", "video" };
	std::cout << offset << "source type: " << typeStr[type] << std::endl;
	switch (type)
	{
	case TYPE_CAMERA:
		std::cout << offset << "camera id: " << cam_id << std::endl;
		break;
	case TYPE_IMAGE:
		std::cout << offset << "source images: (count: "<<imgPaths.size()<<")" << std::endl;
		for (auto iter = imgPaths.begin(); iter != imgPaths.end(); ++iter)
		{
			std::cout << offset << "\t" << *iter << std::endl;
		}
		break;
	case TYPE_VIDEO:
		std::cout << offset << "source video: " << path << std::endl;
		std::cout << offset << "is vertical: " << (is_vertical ? "true" : "false") << std::endl;
		std::cout << offset << "fps: " << fps << std::endl;
		break;
	}
	std::cout << offset << "#out frame: " << frame_cnt << std::endl;
	if (false == frame_repeat.val.empty())
	{
		std::cout << offset << "frame repeat: ";
		PrintVector(frame_repeat.val);
	}
}

cv::Mat Source::GetImage(const int idx)
{
	int target_idx = FrameIndexToTargetIndex(frame_repeat.val, idx);

	cv::Mat res;
	switch (type)
	{
	case TYPE_CAMERA:
		if (nullptr == _cap)
		{
			_cap = std::shared_ptr<cv::VideoCapture>(new cv::VideoCapture(cam_id));
			_cap->set(CV_CAP_PROP_FRAME_WIDTH, size.width);
			_cap->set(CV_CAP_PROP_FRAME_HEIGHT, size.height);
		}
		if (target_idx > cur_frame)
		{
			*_cap >> res;
			++cur_frame;
		}
		else
		{
			res = pre_frame.clone();
		}
		break;
	case TYPE_IMAGE:
	{
		if (true == imgs.empty())
		{
			imgs.resize(imgPaths.size());
		}
		const int real_idx = std::min(target_idx, int(imgs.size() - 1));
		if (true == imgs[real_idx].empty())
		{
			imgs[real_idx] = cv::imread(imgPaths[real_idx], -1);
		}
		res = imgs[real_idx].clone();
	}
		break;
	case TYPE_VIDEO:
		if (nullptr == _cap)
		{
			_cap = std::shared_ptr<cv::VideoCapture>(new cv::VideoCapture(path));
		}
		if (target_idx > cur_frame)
		{
			*_cap >> res;
			++cur_frame;
		}
		if (true == res.empty())
		{
			res = pre_frame.clone();
		}
		else if(is_vertical)
		{
			cv::transpose(res, res);
		}
		break;
	}

	pre_frame = res.clone();
	return res;
}

void Source::SetReplaceStr(std::string path)
{
	if (g_replace_str.find("%source.path%") == g_replace_str.end())
	{
		path = YXL::ToWindowsPath(path);
		g_replace_str["%source.path%"] = path;
		g_replace_str["%source.path_ne%"] = CmFile::GetPathNE(path);
		g_replace_str["%source.name_ne%"] = CmFile::GetNameNE(path);
		g_replace_str["%source.extension%"] = CmFile::GetExtention(path);
		auto dir = CmFile::GetFolder(path);
		dir = dir.substr(0, dir.length() - 1);
		g_replace_str["%source.dir%"] = dir;
		g_replace_str["%source.dir_name%"] = CmFile::GetName(dir);
	}
}


/*
output
*/

bool Output::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val)
{
	std::string strType = json.ReadValue<std::string>("type", "none", val);
	if ("none"==strType)
	{
		std::cout << "no output type specified..." << std::endl;
		return false;
	}
	if ("image" == strType)
	{
		type = TYPE_IMAGE;
		std::string tmp = json.ReadValue<std::string>("dir", "", val);
		if (""==tmp)
		{
			std::cout << "no image output folder specified..." << std::endl;
			return false;
		}
		dir = CmFile::GetFolder(tmp);
		name_format = json.ReadValue<std::string>("name_format", "%03d.jpg", val);

		names.Load(val, "names");
		SetReplaceStr(dir + (names.val.empty() ? name_format : names.val[0]));
	}
	else if ("video" == strType)
	{
		type = TYPE_VIDEO;
		path = json.ReadValue<std::string>("path", "", val);
		if (""==path)
		{
			std::cout << "no video output path specified..." << std::endl;
			return false;
		}
		path = ".avi" != CmFile::GetExtention(path) ? (path + ".avi") : path;
		fps = json.ReadValue("fps", 25.0f, val);
		width = json.ReadValue("width", 1280, val);
		height = json.ReadValue("height", 720, val);
		
		SetReplaceStr(path);
	}
	else
	{
		std::cout << "output type error..." << std::endl;
		return false;
	}

	out_flags.Load(val, "out_flags");

	return true;
}

void Output::Print(const std::string offset)
{
	std::cout << offset << "---------output---------" << std::endl;
	std::string typeStr[] = { "none", "image", "video" };
	std::cout << offset << "output type: " << typeStr[type] << std::endl;
	switch (type)
	{
	case TYPE_IMAGE:
		std::cout << offset << "output folder: " << dir << std::endl;
		std::cout << offset << "name format: " << name_format << std::endl;
		std::cout << offset << "names: ";
		PrintVector(names.val);
		break;
	case TYPE_VIDEO:
		std::cout << offset << "output video: " << path << std::endl;
		std::cout << offset << "fps: " << fps << std::endl;
		std::cout << offset << "width: " << width << std::endl;
		std::cout << offset << "height: " << height << std::endl;
		break;
	}

	if (false == out_flags.val.empty())
	{
		std::cout << offset << "out flags: " << std::endl;
		int from(0);
		for (int i(1); i < out_flags.val.size(); ++i)
		{
			if (out_flags.val[i] != out_flags.val[i - 1])
			{
				std::cout << offset << "\t[" << from << ", " << i << "): " << (out_flags.val[from] ? "true" : "false") << std::endl;
				from = i;
			}
		}
		std::cout << offset << "\t[" << from << ", " << out_flags.val.size() << "): " << (out_flags.val[from] ? "true" : "false") << std::endl;
	}

}

void Output::SaveImage(cv::Mat img, const int frame_idx)
{
	int target_idx = FrameIndexToTargetIndex(out_flags.val, frame_idx);
	if (0 > target_idx)
	{
		return;
	}
	if (img.empty())
	{
		return;
	}

	switch (type)
	{
	case TYPE_IMAGE:
	{
		std::string name_str = FrameIndexToTarget(names.val, target_idx, name_format);
		
		name_str = cv::format(name_str.c_str(), target_idx);
		if (false == CmFile::FolderExist(CmFile::GetFolder(dir + name_str)))
		{
			CmFile::MkDir(CmFile::GetFolder(dir + name_str));
		}
		cv::imwrite(dir+name_str, img);
	}
		break;
	case TYPE_VIDEO:
	{
		cv::Size size(width, height);
		if (0 == width || 0 == height)
		{
			size = img.size();
		}
		if (nullptr == _writer)
		{
			_writer = std::shared_ptr<cv::VideoWriter>(new cv::VideoWriter(path, CV_FOURCC('X', 'V', 'I', 'D'), fps, size));
		}
		if (4 == img.channels())
		{
			cv::cvtColor(img, img, CV_BGRA2BGR);
		}
		cv::Mat img2;
		cv::resize(img, img2, size);
		*_writer << img2;
	}
		break;
	}
}

void Output::Finish()
{
	if (_writer)
	{
		_writer = nullptr;
	}
}

int Output::GetSaveIndex(const int frame_idx)
{
	int target_idx = FrameIndexToTargetIndex(out_flags.val, frame_idx);
	return target_idx;
}

void Output::SetReplaceStr(std::string path)
{
	if (g_replace_str.find("%output.path%") == g_replace_str.end())
	{
		path = YXL::ToWindowsPath(path);
		g_replace_str["%output.path%"] = path;
		g_replace_str["%output.path_ne%"] = CmFile::GetPathNE(path);
		g_replace_str["%output.name_ne%"] = CmFile::GetNameNE(path);
		g_replace_str["%output.extension%"] = CmFile::GetExtention(path);
		auto dir = CmFile::GetFolder(path);
		dir = dir.substr(0, dir.length() - 1);
		g_replace_str["%output.dir%"] = dir;
		g_replace_str["%output.dir_name%"] = CmFile::GetName(dir);
	}
}

void Output::Replace()
{
	std::vector<std::string*> to_do;
	to_do.push_back(&dir);
	to_do.push_back(&name_format);
	for (auto iter = names.val.begin(); iter != names.val.end(); ++iter)
	{
		to_do.push_back(&(*iter));
	}
	to_do.push_back(&path);

	for (auto iter = to_do.begin(); iter != to_do.end(); ++iter)
	{
		**iter = YXL::ReplaceStrings(**iter, g_replace_str);
	}
}

/*
setting
*/

bool Setting::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val, const int frame_cnt)
{
	prop_name = json.ReadValue<std::string>("prop_name", "", val);
	if (""==prop_name)
	{
		std::cout << "no prop_name specified..." << std::endl;
		return false;
	}

	if (false == val.HasMember("params") || false == val["params"].IsObject())
	{
		std::cout << "empty parametere list..." << std::endl;
		return true;
	}

	for (auto iter = val["params"].MemberBegin(); iter != val["params"].MemberEnd(); ++iter)
	{
		std::string name = JsonGetStr(iter->name);
		if (iter->value.IsFloat())
		{
			auto tmp = iter->value.GetFloat();
			params_float.insert(std::make_pair(name, tmp));
		}
		if (iter->value.IsInt())
		{
			float tmp = static_cast<float>(iter->value.GetInt());
			params_float.insert(std::make_pair(name, tmp));
		}
		else if (iter->value.IsString())
		{
			auto tmp = JsonGetStr(iter->value);
			params_string.insert(std::make_pair(name, tmp));
		}
		else if (iter->value.IsBool())
		{
			auto tmp = iter->value.GetBool()==false ? 0.0f: 1.0f;
			params_float.insert(std::make_pair(name, tmp));
		}
		else if (iter->value.IsArray())
		{
			ARRAY_TYPE_0<float> f;
			ARRAY_TYPE_0<std::string> s;
			f.Load(val["params"], name);
			s.Load(val["params"], name);

			if (f.val.empty() && false == s.val.empty())
			{
				std::vector<std::string> tmps;
				for (auto iter2 = s.val.begin(); iter2 != s.val.end(); ++iter2)
				{
					tmps.push_back(*iter2);
				}
				params_strings.insert(std::make_pair(name, tmps));
			}
			else if (false == f.val.empty() && s.val.empty())
			{
				std::vector<float> tmps;
				for (auto iter2 = f.val.begin(); iter2 != f.val.end(); ++iter2)
				{
					tmps.push_back(*iter2);
				}
				params_floats.insert(std::make_pair(name, tmps));
			}
			else
			{
				std::cout << "string & float mix used in param: "<<name << std::endl;
			}
		}
		else if (iter->value.IsObject())
		{
			if (iter->value.HasMember("from") && iter->value["from"].IsFloat() && iter->value.HasMember("to") && iter->value["to"].IsFloat())
			{
				const float from = iter->value["from"].GetFloat();
				const float to = iter->value["to"].GetFloat();
				const float step = (iter->value.HasMember("step") && iter->value["step"].IsFloat()) ? iter->value["step"].GetFloat() 
					: (to - from) / frame_cnt;

				float cur = from;
				std::vector<float> tmps;
				for (int i(0); i != frame_cnt; ++i)
				{
					tmps.push_back(cur);
					if (cur < to)
					{
						cur += step;
					}
				}
				params_floats.insert(std::make_pair(name, tmps));
			}
		}
	}

	return true;
}

void Setting::Print(const std::string offset)
{
	std::cout << offset << "------setting------" << std::endl;
	std::cout << offset << "prop name: "<<prop_name << std::endl;
	std::cout << offset << "parameters: " << std::endl;
	std::cout << offset << "\tfloat:" << std::endl;
	for (auto iter = params_float.begin(); iter != params_float.end(); ++iter)
	{
		std::cout << offset << "\t\t" << iter->first << " = " << iter->second << std::endl;
	}
	std::cout << offset << "\tstring:" << std::endl;
	for (auto iter = params_string.begin(); iter != params_string.end(); ++iter)
	{
		std::cout << offset << "\t\t" << iter->first << " = " << iter->second << std::endl;
	}
	
	std::cout << offset << "\tfloat (changing):" << std::endl;
	for (auto iter = params_floats.begin(); iter != params_floats.end(); ++iter)
	{
		std::cout << offset << "\t\t" << iter->first << " = [";
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			if (iter2 != iter->second.begin())
			{
				std::cout << ", ";
			}
			std::cout << *iter2;
		}
		std::cout << "]" << std::endl;
	}

	std::cout << offset << "\tstring (changing):" << std::endl;
	for (auto iter = params_strings.begin(); iter != params_strings.end(); ++iter)
	{
		std::cout << offset << "\t\t" << iter->first << " = [";
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			if (iter2 != iter->second.begin())
			{
				std::cout << ", ";
			}
			std::cout << *iter2;
		}
		std::cout << "]" << std::endl;
	}
}

void Setting::Replace()
{
	std::vector<std::string*> to_do;
	for (auto iter = params_string.begin(); iter != params_string.end(); ++iter)
	{
		to_do.push_back(&(iter->second));
	}
	for (auto iter = params_strings.begin(); iter != params_strings.end(); ++iter)
	{
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			to_do.push_back(&(*iter2));
		}
	}

	for (auto iter = to_do.begin(); iter != to_do.end(); ++iter)
	{
		**iter = YXL::ReplaceStrings(**iter, g_replace_str);
	}
}

/*
post process
*/

void PostProcess::SinglePostProcess::Process()
{
	switch (type)
	{
	case PostProcess::TYPE_CMD:
	{
		CmFile::RunProgram(cmd, params2, is_wait, is_show);
	}
		break;
	default:
		break;
	}
}

bool PostProcess::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val)
{
	if (false == val.HasMember("post_process") || false == val["post_process"].IsObject())
	{
		return true;
	}

	proc.clear();
	for (auto iter = val["post_process"].MemberBegin(); iter != val["post_process"].MemberEnd(); ++iter)
	{
		SinglePostProcess tmp;
		std::string type = json.ReadValue<std::string>("type", "", iter->value);
		if (""==type)
			continue;
		if ("cmd" == type)
		{
			tmp.type = TYPE_CMD;
			tmp.cmd = json.ReadValue<std::string>("cmd", "", iter->value);
			if (""==tmp.cmd)
				continue;
			tmp.params = json.ReadValue<std::string>("params", "", iter->value);
			tmp.is_show = json.ReadValue("is_show", false, iter->value);
			tmp.is_wait = json.ReadValue("is_wait", true, iter->value);

			proc.push_back(tmp);
		}
	}

	return true;
}

void PostProcess::Print(const std::string offset)
{
	std::cout << offset << "post process:" << std::endl;
	std::string txt[] = { "none", "cmd" };
	for (auto iter = proc.begin(); iter != proc.end(); ++iter)
	{
		if (TYPE_NONE == iter->type)
		{
			continue;
		}
		std::cout << offset << "\ttype: " << txt[iter->type] << std::endl;
		switch (iter->type)
		{
		case TYPE_CMD:
			std::cout << offset << "\tcmd: " << iter->cmd << std::endl;
			std::cout << offset << "\tparams: " << iter->params << std::endl;
			std::cout << offset << "\tactual params: " << iter->params2 << std::endl;
			std::cout << offset << "\tis show: " << (iter->is_show?"true":"false") << std::endl;
			std::cout << offset << "\tis wait: " << (iter->is_wait ? "true" : "false") << std::endl;
			break;
		default:
			break;
		}
	}
}

void PostProcess::Process()
{
	for (auto iter = proc.begin(); iter != proc.end(); ++iter)
	{
		iter->Process();
	}
}

void PostProcess::Replace()
{
	std::vector<std::string*> to_do;
	for (auto iter = proc.begin(); iter != proc.end(); ++iter)
	{
		to_do.push_back(&(iter->params));
		to_do.push_back(&(iter->params2));
	}

	for (auto iter = to_do.begin(); iter != to_do.end(); ++iter)
	{
		**iter = YXL::ReplaceStrings(**iter, g_replace_str);
	}
}

/*
addition calls
*/

bool AdditionCalls::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val)
{
	if (false == val.HasMember("addition_calls"))
	{
		return true;
	}
	calls.Load(val, std::string("addition_calls"));

	return true;
}

void AdditionCalls::Print(const std::string offset)
{
	std::cout << offset << "addition calls: " << std::endl;
	for (auto iter = calls.val.begin(); iter != calls.val.end(); ++iter)
	{
		std::cout << offset << "\t" << iter->first << std::endl;
		for (auto iter2 = iter->second.begin(); iter2 != iter->second.end(); ++iter2)
		{
			std::cout << offset << "\t\t" << iter2->ToString() << std::endl;
		}
	}
}

void AdditionCalls::GetAdditionCalls(std::map<std::string, SingleCall>& ret, const int frame_idx)
{
	ret.clear();
	SingleCall def_val;
	def_val.func = "none";

	for (auto iter = calls.val.begin(); iter != calls.val.end(); ++iter)
	{
		SingleCall val = FrameIndexToTarget(iter->second, frame_idx, def_val);
		if ("none" == val.func && false == iter->second.empty())
		{
			val = *iter->second.rbegin();
		}
		ret[iter->first] = val;
	}
}

void AdditionCalls::Replace()
{
	std::vector<std::string*> to_do;
	for (auto iter = calls.val.begin(); iter != calls.val.end(); ++iter)
	{
		auto& single_calls = iter->second;
		for (auto iter2 = single_calls.begin(); iter2 != single_calls.end(); ++iter2)
		{
			for (auto iter3 = iter2->params.begin(); iter3 != iter2->params.end(); ++iter3)
			{
				to_do.push_back(&(iter3->param_str));
			}
		}
	}

	for (auto iter = to_do.begin(); iter != to_do.end(); ++iter)
	{
		**iter = YXL::ReplaceStrings(**iter, g_replace_str);
	}
}

std::string AdditionCalls::SingleCall::ToString()
{
	std::string ret = func + "(";
	for (auto iter = params.begin(); iter != params.end(); ++iter)
	{
		if (iter != params.begin())
		{
			ret += ", ";
		}
		if (true == iter->is_int)
		{
			ret += cv::format("%d", iter->param_i);
		}
		else
		{
			ret += iter->param_str;
		}
	}
	ret += ")";
	return ret;
}


/*
call
*/


void Call::GetProps(std::map<std::string, std::string>& ret, const int frame_idx)
{
	ret.clear();
	for (auto iter = props.val.begin(); iter != props.val.end(); ++iter)
	{
		std::string val = FrameIndexToTarget(iter->second, frame_idx, std::string(""));
		if ("" == val && iter->second.size() == 1)
		{
			val = *iter->second.rbegin();
		}
		if (true == CmFile::FileExist(val))
		{
			ret[iter->first] = val;
		}
	}
}

void Call::AdditionCall(AdditionCalls::SingleCall & call, std::shared_ptr<Nama> nama)
{
	if ("" == call.func)
	{

	}
	else
	{
		std::cout << "no match nama function: " << call.ToString() << std::endl;
	}
}

bool Call::LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value & val)
{
	if (false == val.HasMember("source") || false == val["source"].IsObject())
	{
		std::cout << "no source specified..." << std::endl;
		return false;
	}
	bool ret = source.LoadFromJson(json, val["source"]);
	if (false == ret)
	{
		std::cout << "source configuration errors..." << std::endl;
		return false;
	}

	if (val.HasMember("output") && val["output"].IsObject())
	{
		bool ret = output.LoadFromJson(json, val["output"]);
		if (false == ret)
		{
			std::cout << "output configuration errors..." << std::endl;
			return false;
		}
	}

	if (val.HasMember("need_reset") && val["need_reset"].IsBool())
	{
		need_reset = val["need_reset"].GetBool();
	}

	if (val.HasMember("setting") && val["setting"].IsArray())
	{
		for (auto iter = val["setting"].Begin(); iter != val["setting"].End(); ++iter)
		{
			Setting tmp;
			bool ret = tmp.LoadFromJson(json, *iter, source.frame_cnt);
			if (false == ret)
			{
				std::cout << "setting configuration errors..." << std::endl;
				continue;
			}
			setting.push_back(tmp);
		}
	}
	
	props.Load(val, "props");
	/*for (auto iter = props.val.begin(); iter != props.val.end(); )
	{
		for (int i(0); i < iter->second.size();)
		{
			if (false == CmFile::FileExist(iter->second[i]))
			{
				std::cout << "file not exist: " << iter->second[i] << std::endl;
				iter->second.erase(iter->second.begin() + i);
				continue;
			}
			++i;
		}
		auto old = iter++;
		if (old->second.empty())
		{
			props.val.erase(old);
		}
	}*/

	ret = pp.LoadFromJson(json, val);
	if (false == ret)
	{
		std::cout << "post process configuration errors..." << std::endl;
		return false;
	}

	ret = ac.LoadFromJson(json, val);
	if (false == ret)
	{
		std::cout << "addition calls configuration errors..." << std::endl;
		return false;
	}

	for (auto iter = setting.begin(); iter != setting.end(); ++iter)
	{
		iter->Replace();
	}

	output.Replace();
	pp.Replace();
	ac.Replace();

	return true;
}

void Call::Print(const std::string offset)
{
	std::cout << offset << "------------call------------" << std::endl;
	source.Print(offset+"\t");
	std::cout << std::endl;
	output.Print(offset+"\t");
	std::cout << std::endl;
	for (auto iter = setting.begin(); iter != setting.end(); ++iter)
	{
		iter->Print(offset+"\t");
	}
	std::cout << std::endl;
	std::cout << offset << "------------others------------" << std::endl;
	std::cout << offset << "need_reset: " << (need_reset ? "true" : "false") << std::endl;
	std::cout << offset << "props:" << std::endl;

	for (auto iter = props.val.begin(); iter != props.val.end(); ++iter)
	{
		std::cout << offset << "\t" << iter->first << " = ";
		PrintVector(iter->second);
	}

	if (false == pp.proc.empty())
	{
		std::cout << std::endl;
		pp.Print(offset);
	}

	if (false == ac.calls.val.empty())
	{
		std::cout << std::endl;
		ac.Print(offset);
	}
	std::cout << offset << "replace string:" << std::endl;
	PrintMap(g_replace_str, offset + "\t");
}

void Call::Process(NC_config& config, std::shared_ptr<Nama> nama)
{
	if (need_reset)
	{
		nama->ClearState();
	}

	if (output.fps < 0)
	{
		if (Source::TYPE_VIDEO == source.type)
			output.fps = static_cast<float>(source.fps);
		if (output.fps <= 0)
			output.fps = 25.0f;
	}

	const bool is_preview = Source::TYPE_CAMERA == source.type || config.Preview;

	std::map<std::string, std::string> propPathList;
	std::map<std::string, int> props;
	std::vector<int> propList;
	for (int i(0); i != source.frame_cnt; ++i)
	{
		//source
		cv::Mat img = source.GetImage(i);
		//prepare items for rendering
		GetProps(propPathList, i);
		props.clear();
		propList.clear();
		for (auto iter = propPathList.begin(); iter != propPathList.end(); ++iter)
		{
			props[iter->first] = nama->GetPropHandle(iter->second);
			if (0 == props[iter->first])
			{
				continue;
			}
			propList.push_back(props[iter->first]);
		}
		//
		const int save_idx = output.GetSaveIndex(i);
		//set parameters
		for (auto iter = setting.begin(); iter != setting.end(); ++iter)
		{
			int handle = props[iter->prop_name];
			if (0 == handle)
			{
				continue;
			}

			for (auto iter2 = iter->params_float.begin(); iter2 != iter->params_float.end(); ++iter2)
			{
				nama->SetParam(handle, iter2->first, iter2->second);
			}
			for (auto iter2 = iter->params_string.begin(); iter2 != iter->params_string.end(); ++iter2)
			{
				nama->SetParam(handle, iter2->first, cv::format(iter2->second.c_str(), save_idx));
			}
			for (auto iter2 = iter->params_floats.begin(); iter2 != iter->params_floats.end(); ++iter2)
			{
				double val;
				if (i < iter2->second.size())
				{
					val = iter2->second[i];
				}
				else
				{
					val = *(iter2->second.rbegin());
				}
				nama->SetParam(handle, iter2->first, val);
			}
			for (auto iter2 = iter->params_strings.begin(); iter2 != iter->params_strings.end(); ++iter2)
			{
				std::string val;
				if (i < iter2->second.size())
				{
					val = iter2->second[i];
				}
				else
				{
					val = *(iter2->second.rbegin());
				}
				val = cv::format(val.c_str(), save_idx);
				nama->SetParam(handle, iter2->first, val);
			}
		}
		//renderitems
		//if (false == propList.empty())
		{
			std::map<std::string, AdditionCalls::SingleCall> acs;
			ac.GetAdditionCalls(acs, i);
			//call functions before renderitems
			for (auto iter = acs.begin(); iter != acs.end(); ++iter)
			{
				if (iter->second.is_before_render)
				{
					AdditionCall(iter->second, nama);
				}
			}
			img = nama->Process(img, propList);
			//call functions after renderitems
			for (auto iter = acs.begin(); iter != acs.end(); ++iter)
			{
				if (false == iter->second.is_before_render)
				{
					AdditionCall(iter->second, nama);
				}
			}
		}
		//output
		output.SaveImage(img, i);
		//preview result
		if (is_preview)
		{
			cv::imshow("preview", img);
			cv::waitKey(1);
		}

		std::cout << i + 1 << "/" << source.frame_cnt << "\r";
	}
	//finish output (close videowriter etc.)
	output.Finish();
	if (is_preview)
	{
		cv::destroyWindow("preview");
	}
	//post process
	pp.Process();

	if (true == config.OpenOutDir)
	{
		std::string dir;
		if (output.type == Output::TYPE_IMAGE)
		{
			dir = output.dir;
		}
		else if (output.type == Output::TYPE_VIDEO)
		{
			dir = CmFile::GetFolder(output.path);
		}
		for (auto iter = dir.begin(); iter != dir.end(); ++iter)
		{
			if ('/' == *iter)
			{
				*iter = '\\';
			}
		}
		CmFile::RunProgram("explorer.exe", dir);
	}
}

bool Caller::LoadFromJson(YXL::JSON::Json& json)
{
	auto& val = json.GetRoot();
	name = json.ReadValue<std::string>("name", "");

	if (val.HasMember("calls") && val["calls"].IsArray())
	{
		for (auto iter = val["calls"].Begin(); iter != val["calls"].End(); ++iter)
		{
			Call tmp;
			bool ret = tmp.LoadFromJson(json, *iter);
			if (false == ret)
			{
				std::cout << "call configuration errors..." << std::endl;
				continue;
			}
			calls.push_back(tmp);
		}
	}

	return true;
}

void Caller::Print(const std::string offset)
{
	std::cout << offset << "---------------Calls---------------" << std::endl;
	std::cout << offset << "name: " << name << std::endl;
	int idx(0);
	for (auto iter = calls.begin(); iter != calls.end(); ++iter, ++idx)
	{
		std::cout << offset << "call " << idx << std::endl;
		iter->Print(offset+"\t");
	}

	std::cout << offset << "------------------------------------" << std::endl;
}

bool Caller::Load(NC_config & config)
{
	if ("" == config.jsonContent)
	{
		if (false == CmFile::FileExist(config.jsonPath))
		{
			std::cout << "file not exist: " << config.jsonPath << std::endl;
			_is_ok = false;
			return false;
		}
		_json.Load(config.jsonPath);
	}
	else
	{
		_json.LoadFronJsonContent(config.jsonContent);
	}
	_is_ok = LoadFromJson(_json);
	if (false == _is_ok)
	{
		return false;
	}

	if (true == config.ShowConfig)
	{
		Print();
	}

	if (true == config.NeedConfirm)
	{
		char ch;
		std::cout << "continue (Y/n) ?" << std::endl;
		std::cin >> ch;
		if (!('Y' == ch || 'y' == ch))
		{
			_is_ok = false;
			return false;
		}
	}
	return true;
}

void Caller::Process(NC_config& config, std::shared_ptr<Nama> nama)
{
	if (false == _is_ok)
	{
		return;
	}

	for (auto iter = calls.begin(); iter != calls.end(); ++iter)
	{
		iter->Process(config, nama);
	}

	if (true == config.Press2Exit)
	{
		system("pause");
	}
}

