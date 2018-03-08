#pragma once
#include <CmFile.h>
#include <memory>
#include <YXLJsonReader.h>

class Nama;

struct NC_config
{
	bool NeedConfirm = false;
	bool Press2Exit = false;
	bool OpenOutDir = false;
	bool ShowConfig = false;
	bool Preview = false;
	std::string jsonPath = "";
	std::string jsonContent = "";
};

//Array type 0
//[repeat, [repeat, type, type, ...], type, ...]
template<typename VAL>
struct ARRAY_TYPE_0
{
	std::vector<VAL> val;

	void Load(const rapidjson::Value& jsonVal, const std::string& name)
	{
		val.clear();
		if (false == jsonVal.HasMember(name.c_str()))
			return;
		auto& ref = jsonVal[name.c_str()];
		if (YXL::JSON::ValueGetter<VAL>::IsType(ref))
		{
			val.push_back(YXL::JSON::ValueGetter<VAL>::Get(ref));
			return;
		}
		if (ref.IsArray() && 0 <ref.Size() && ref.Begin()->IsInt())
			LoadArray(val, ref);
	}

private:
	void LoadArray(std::vector<VAL>& ret_val, const rapidjson::Value& jsonVal)
	{
		ret_val.clear();
		std::vector<VAL> val2;
		auto tmp = jsonVal.Begin();
		const int loop = tmp->GetInt();
		std::vector<VAL> tmp2;
		for (auto iter = ++tmp; iter != jsonVal.End(); ++iter)
			if (YXL::JSON::ValueGetter<VAL>::IsType(*iter))
				val2.push_back(YXL::JSON::ValueGetter<VAL>::Get(*iter));
			else if (iter->IsArray() && 0 < iter->Size() && iter->Begin()->IsInt())
			{
				std::vector<VAL> tmp_val;
				LoadArray(tmp_val, *iter);
				val2.insert(val2.end(), tmp_val.begin(), tmp_val.end());
			}
		ret_val.reserve(val2.size()*loop);
		for (int i(0); i != loop; ++i)
			ret_val.insert(ret_val.end(), val2.begin(), val2.end());
	}
};

//type 1(object type)
template<typename KEY, typename VAL>
struct OBJ_TYPE_0
{
	std::map<KEY, std::vector<VAL> > val;

	void Load(const rapidjson::Value& jsonVal, const std::string& name)
	{
		val.clear();
		if (false == jsonVal.HasMember(name.c_str()) || false == jsonVal[name.c_str()].IsObject())
			return;
		for (auto iter = jsonVal[name.c_str()].MemberBegin(); iter != jsonVal[name.c_str()].MemberEnd(); ++iter)
		{
			KEY key = YXL::JSON::ValueGetter<KEY>::Get(iter->name);
			ARRAY_TYPE_0<VAL> tmp;
			if (YXL::JSON::ValueGetter<VAL>::IsType(iter->value))
				tmp.val.push_back(YXL::JSON::ValueGetter<VAL>::Get(iter->value));
			else if (iter->value.IsArray())
				tmp.Load(jsonVal[name.c_str()], YXL::JSON::ValueGetter<KEY>::Get(iter->name));
			val[key] = tmp.val;
		}
	}
};

struct Source
{
	enum TYPE {
		TYPE_CAMERA,
		TYPE_IMAGE,
		TYPE_VIDEO
	};

	TYPE type=TYPE_CAMERA;

	std::shared_ptr<cv::VideoCapture> _cap = nullptr;
	//TYPE_CAMERA
	int cam_id = 0;
	cv::Size size;
	//TYPE_IMAGE
	std::vector<std::string> imgPaths;
	std::vector<cv::Mat> imgs;
	//TYPE_VIDEO
	std::string path = "";
	bool is_vertical = false;
	double fps = 0;

	int frame_cnt = 0;
	ARRAY_TYPE_0<int> frame_repeat;

	int cur_frame = -1;
	cv::Mat pre_frame;

	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val);
	void Print(const std::string offset="");
	cv::Mat GetImage(const int idx);

	void SetReplaceStr(std::string path);
};

struct Output
{
	enum TYPE {
		TYPE_NONE,
		TYPE_IMAGE,
		TYPE_VIDEO
	};

	TYPE type = TYPE_NONE;

	//TYPE_IMAGE
	std::string dir = "";
	std::string name_format = "";
	ARRAY_TYPE_0<std::string> names;
	//TYPE_VIDEO
	std::string path = "";
	float fps = -1.0f;
	int width = 0;
	int height = 0;
	std::shared_ptr<cv::VideoWriter> _writer = nullptr;

	ARRAY_TYPE_0<bool> out_flags;

	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val);
	void Print(const std::string offset = "");
	void SaveImage(cv::Mat img, const int frame_idx);
	void Finish();
	int GetSaveIndex(const int frame_idx);

	void SetReplaceStr(std::string path);
	void Replace();
};

struct Setting
{
	std::string prop_name = "";

	std::map<std::string, float> params_float;
	std::map<std::string, std::string> params_string;

	std::map<std::string, std::vector<float> > params_floats;
	std::map<std::string, std::vector<std::string> > params_strings;

	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val, const int frame_cnt);
	void Print(const std::string offset = "");
	void Replace();
};

struct PostProcess
{
	enum TYPE
	{
		TYPE_NONE,
		TYPE_CMD
	};
	struct SinglePostProcess
	{
		TYPE type = TYPE_NONE;
		std::string cmd = "";
		std::string params = "";
		std::string params2 = "";
		bool is_show = false;
		bool is_wait = true;

		void Process();
	};

	std::vector<SinglePostProcess> proc;

	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val);
	void Print(const std::string offset = "");
	void Process();
	void Replace();
};

struct AdditionCalls
{
	struct SingleCall 
	{
		bool is_before_render = true;
		std::string func="none";
		struct Param {
			bool is_int = true;
			int param_i=0;
			std::string param_str;
		};
		std::vector<Param> params;

		std::string ToString();
	};

	OBJ_TYPE_0<std::string, SingleCall> calls;

	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val);
	void Print(const std::string offset = "");
	void GetAdditionCalls(std::map<std::string, SingleCall>& calls, const int frame_idx);
	void Replace();
};

class Call
{
	Source source;
	Output output;
	PostProcess pp;
	AdditionCalls ac;
	bool need_reset = true;
	OBJ_TYPE_0<std::string, std::string> props;

	std::vector<Setting> setting;

	void GetProps(std::map<std::string, std::string>& props, const int frame_idx);
	void AdditionCall(AdditionCalls::SingleCall& call, std::shared_ptr<Nama> nama);


public:
	bool LoadFromJson(YXL::JSON::Json& json, const rapidjson::Value& val);
	void Print(const std::string offset = "");
	void Process(NC_config& config, std::shared_ptr<Nama> nama);
};

class Caller
{
	YXL::JSON::Json _json;
	bool _is_ok = false;

	std::string name;
	std::vector<Call> calls;

	bool LoadFromJson(YXL::JSON::Json& json);
	void Print(const std::string offset = "");

public:
	bool Load(NC_config& config);
	void Process(NC_config& config, std::shared_ptr<Nama> nama);
};