# description
A demo of Nama ([a SDK about face](http://www.faceunity.com)).

# functions
- multiple input sources: image, video etc.
- per-frame control: repeat frame, parameters vary from frame to frame etc.
- using json as workflow
- saving results as image/video

# requirement
- VS2015
- OpenCV
- [Nama SDK](https://github.com/Faceunity)
- some of my code: [YXL](https://github.com/cx2200252/YXL_code/tree/master/YXL)  

# cmdline parameters
```C
NamaCaller.exe -json=path -wait=0 -opendir=0 -showconfig=0 -confirm=0
```
- -json: path of workflow file
- -jsoncontent: content of workflow, has something wrong since the limitation of cmdline parameter's length
- -wait: press any key to exit
- -opendir: open the out folder
- -showconfig: show the workflow
- -confirm: confirm before run the workflow

# detail about workflow file (json)
## basic structure
```C
{
	"name": "123",
	"calls": [
		//job 0
		{
			//optional
			//reset Nama's tracking state
			//target to multiple jobs
			"need_reset": false, //default false
			"source":{
				"type": "camera",
				...
			},
			//optional
			"props":{
				"prop_name1": "prop_path",
				...
			},
			//optional
			"setting": [
				{
					"prop_name": "prop_name1",
					"params":{
						"param1": value,
						...
					}
				}
			],
			//optional
			"addition_calls":[
				"call1": type_0(addition_call_type)
			],
			"output": {
				"type": "image",
				...
			},
			//optional
			"post_process":{
				"proc_1": {
					"type": "cmd",
					"cmd": "rm",
					"params": "..."
				}
			}
		},
		//job 1
		{

		}
	]
}
```

## avaliable type
```C
//can use recursively
type_0(type):[repeat_times, [repeat_times, type, type, ...], type, ...]

/*
example:
type_0(int) in json:
[2, [3, 4,5], 6]
will expand to an array:
[4,5,4,5,4,5,6, 4,5,4,5,4,5,6]
*/

addition_call_type:
{
	"is_before_render": true, //default true
	"func": "function_name",
	//parameter list, can mix of int and string
	"params": [int, string, ...,]
}

```

## configuration of each type
- source
```C
/*
camera
*/
"source":{
	"type": "camera",
	"cam_id": 0, //default 0
	"width": 1280, //default 1280
	"height": 720, //default 720
	//total input frame
	"frame_cnt": 0, //defaut 0
	//times of each frame to use
	"frame_repeat": type_0(int) //default 1
}

/*
image
*/
"source":{
	"type": "image",
	//if set, will ignore "is_pick_file" and "is_pick_folder"
	"path": type_0(string),
	//pick an image by GUI
	"is_pick_file": false,
	//pick an folder by GUI and treat all image (*.jpg, *.png, *.bmp)
	//within it (excluding sub-folder) as input image 
	"is_pick_folder": false,
	// is_pick_file¡¢is_pick_folder can use at the same time

	//in this source type, if not set or set to 0£º 
	//	frame_cnt = sum(frame_repeat[i], i=0~#image-1)
	"frame_cnt": 0,
	//#frame_repeat can less than #image
	//if less, will repeat the last value to make sure #frame_repeat==#image
	"frame_repeat": type_0(int)
}

/*
video
*/
"source":{
	"type": "video",
	//if set, will ignore "is_pick_file"
	"path": string
	//pick a video by GUI
	"is_pick_file": false,
	//whether the video is vertical or not
	//I came across that using opencv to read vertical video and it return a horizontal image
	//but I'm not sure right now.
	"is_vertical": true,
	//in this source type, if not set or set to 0£º 
	// frame_cnt = sum(frame_repeat[i], i=0~#video_frame-1)
	"frame_cnt": 0,
	//#frame_repeat can less than #video_frame
	//if less, will repeat the last value to make sure #frame_repeat==#video_frame
	"frame_repeat": type_0(int)
}
```
- output
```C
/*
image
*/
"output": {
	"type": "image",
	//output folder
	"dir": string,
	//output filenames
	//if #names < frame_cnt, the rest of output frame will use the name_foramt
	"names": type_0(string),
	//output filename format, index is based on current output image count
	"name_format": "%03d.jpg", //default %03d.jpg
	//indicate whether a frame is output or not
	//#out_flags can less than frame_cnt
	//if less, will repeat the last value to make sure #out_flags==frame_cnt
	"out_flags": type_0(bool)
}

/*
video
*/
"output": {
	"type": "video",
	//output video path
	//please use *.avi, other format may have problems
	"path": string,
	//if not set or negative and source type is video, set to fps of source video
	"fps": 25.0, //default 25.0
	"width": 1280,
	"height": 720,
	//if width or height not set or negative, use same size of source
	
	//same as output image
	"out_flags": type_0(bool)
}
```
- props
```C
"props":{
	//#prop_name1==1 indicates using this prop for all frames
	//if #prop_name1<frame_cnt,
	//the remain frame (last frame_cnt-#prop_name1) will ignore this prop
	//if a prop is not exist, it will be ignore, for example,
	//"prop_name1": [2, path_a, ""]
	//means that the 1st, 3rd frame will use path_a,
	//while 2nd, 4th to frame_cnt frame will use no props
	"prop_name1": type_0(string),
	"prop_name2": type_0(string),
	...
}
```
- setting
```C
"setting": [
	{
		"prop_name": "prop_name1",
		"params":{
			//#param0 can less than frame_cnt
			//if less, will repeat the last value to make sure #param0==frame_cnt
			"param0": type_0(float),
			"param1": type_0(string),
			//only integer can used
			//the index is base on #output_frame
			//for example, if expanded save flags are [true, false, true, false, true]
			//the index will be [0, -1, 1, -1, 2, 3, 4, ...]
			"param2": "%d",
			"param3": {
				"from": 0.0,
				//"step" is optional
				//if not set, [0, frame_cnt] will map to [from, to]
				//if set, param3[frame_i]=min(from+frame_i*step, to)
				"step": 1.0,
				"to": 100.0
			}
		}		
	},
	{
		"prop_name": "prop_name2",
		"params":{
			...
		}
	}
]
```

- post_process
```C
//run after a job is done
//only type "cmd" for now
"post_process":{
	"proc_1": {
		"type": "cmd",
		"cmd": "bat_script_path",
		//optional
		"params": "parameters",
		//optional
		"is_show": false, //default false
		//optional
		"is_wait": true, //default true
	}
}
```

- addition_calls
```C
/*
not used for now
*/
"addition_calls":[
	"call1": type_0(addition_call_type)
]
```

- some marco
```C
/*
source
*/
//only set when source type is image or video
//in case of image, all values are set based on the first image
%source.dir%
%source.dir_name%
%source.extension%
%source.name_ne% //without extension
%source.path% //folder or file based on source type
%source.path_ne% //folder or file based on source type

/*
output
*/
//in case of image, all values are set based on the first frame even if its output flag is false
%output.dir%
%output.dir_name%
%output.extension%
%output.name_ne%
%output.path%
%output.path_ne%
```
# example workflow
- attach filter to an image
	- pick image by GUI
    - save to an folder (%image_name%_filter) next to the selected image
```C
{
	"name": "filter effect",
	"calls": [
		{
			"source":{
				"type": "image",
				"frame_cnt": 10,
				"is_pick_file": true
			},
			"props": {
				"beauty": "./resources/face_beautification.bundle"
			},
			"setting": [
				{
					"prop_name": "beauty",
					"params": {
						"color_level": 0.0,
						"red_level": 0.0,
						"blur_level": 0.0,
						"face_shape_level": 0.0,
						"filter_name":[1, 
							"abao", 
							"autumn",
							"blackwhite",
							"boardwalk",
							"cloud",
							"cold",
							"concrete",
							"crimson",
							"cruz",
							"cyan"]
					}
				}				
			],
			"output": {
				"type": "image",
				"dir": "%source.path_ne%_filter/",
				"names": [1,
					"abao.jpg",
					"autumn.jpg",
					"blackwhite.jpg",
					"boardwalk.jpg",
					"cloud.jpg",
					"cold.jpg",
					"concrete.jpg",
					"crimson.jpg",
					"cruz.jpg",
					"cyan.jpg"]
			}
		}
	]
}
```
- attach face beautification effect to an video
  - pick video by GUI
  - save an result video (%video_name%_beauty.avi) next to the source video
  - add blur (blur_level is set to 5.0)
```C
{
	"name": "video_beauty",
	"calls": [
		{
			"source":{
				"type": "video",
				"is_pick_file": true,
				//repeat first frame 120 times 
				//for Nama to get better result since beginning
				"frame_repeat": [1, 120, 1]
			},
			"output": {
				"type": "video",
				"path": "%source.path_ne%_beauty.avi",
				//since the source's frame repeat, 
				//additional 119 frames are added, not save
				"out_flags": [1, [119, false], true]
			},
			"props":{
				"beauty": "./resources/face_beautification.bundle"
			},
			"setting":[
				{
					"prop_name": "beauty",
					"params": {
						"color_level": 0.0,
						"red_level": 0.0,
						"blur_level": 5.0,
						"face_shape_level": 0.0
					}
				}	
			]
		}
	]
}
```