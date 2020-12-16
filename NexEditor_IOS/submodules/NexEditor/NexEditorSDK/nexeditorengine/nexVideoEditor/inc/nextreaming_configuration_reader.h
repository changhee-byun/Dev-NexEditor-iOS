
#ifndef NEXCONFREADER_H
#define NEXCONFREADER_H

#define DEFAULT_TRACE_NEXEDITOR_ENGINE				(1)
#define DEFAULT_TRACE_NEXEDITOR_CODEC				(0)
#define DEFAULT_TRACE_NEXEDITOR_THEMERENDER		(2)

#define DEFAULT_PROPERTY_VIDEO_MAXWIDTH				(4096)
#define DEFAULT_PROPERTY_VIDEO_MAXHEIGHT			(2176)
#define DEFAULT_PROPERTY_VIDEOBUFFER_MAXSIZE		(4096*2176)
#define DEFAULT_PROPERTY_HW_DEC_COUNT				(2)
#define DEFAULT_PROPERTY_HW_ENC_COUNT				(1)

class NexConf
{
private:

public:
	struct _log
	{
		_log()
		  :	engine(DEFAULT_TRACE_NEXEDITOR_ENGINE),
			codec(DEFAULT_TRACE_NEXEDITOR_CODEC),
			themerender(DEFAULT_TRACE_NEXEDITOR_THEMERENDER)
		{
		}
		int engine;
		int codec;
		int themerender;
	} log;

	struct _property
	{
		_property() :
			video_maxwidth(DEFAULT_PROPERTY_VIDEO_MAXWIDTH),
			video_maxheight(DEFAULT_PROPERTY_VIDEO_MAXHEIGHT),
			videobuffer_maxsize(DEFAULT_PROPERTY_VIDEOBUFFER_MAXSIZE),
			hw_dec_count(DEFAULT_PROPERTY_HW_DEC_COUNT),
			hw_enc_count(DEFAULT_PROPERTY_HW_ENC_COUNT)
		{
		}
		int video_maxwidth;
		int video_maxheight;
		int videobuffer_maxsize;
		int hw_dec_count;
		int hw_enc_count;
	} properties;

	NexConf()
	{
	}

	~NexConf()
	{
	}
};



class NexConfReader
{
	enum CONF_STATE
	{
		NONE_SECTION,
		LOG_SECTION,
		PROPERTY_SECTION
	};

	enum LINE_RESULT
	{
		LINE_SUCCESS,
		LINE_ERROR,
		LINE_END,
	};

private:
	char *cache, *__curr, *__eof;
	NexConf* conf;

public:
	NexConfReader(void);
	~NexConfReader(void);

	bool setNexConf(NexConf* conf, const char* path);
	bool readConfFile(const char* path);
	void removeRemark(char* line);
	void removeWhiteSpace(char** line);
	LINE_RESULT getLine(char** line);
	bool isSection(char* line);
	CONF_STATE getConfState(char* section);
	bool getItemAndValue(char* line, char** item, char** value);
	bool processLogSection(NexConf *conf, char* line);
	bool processPropertySection(NexConf *conf, char* line);
};

#endif // NEXCONFREADER_H
