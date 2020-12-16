/******************************************************************************
* File Name   :	nextreaming_configuration_reader.cpp
* Description :	
*******************************************************************************
* Copyright (c) 2002-2018 NexStreaming Corp. All rights reserved.
* http://www.nexstreaming.com
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
* PURPOSE.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nextreaming_configuration_reader.h"


NexConfReader::NexConfReader(void)
 : cache(NULL), __curr(NULL), __eof(NULL), conf(NULL)
{
}

NexConfReader::~NexConfReader(void)
{
	if(cache) delete [] cache;
}

bool NexConfReader::setNexConf(NexConf *conf, const char* path)
{
	if(!readConfFile(path)) return false;

	char* line = NULL;
	CONF_STATE state = NONE_SECTION;
	LINE_RESULT result;

	while(LINE_SUCCESS == (result = getLine(&line)))
	{
		removeRemark(line);
		removeWhiteSpace(&line);
		if( strlen(line) < 1 ) continue;
		if(isSection(line))
		{
			state = getConfState(line);
			continue;
		}
		switch(state)
		{
			case LOG_SECTION:
				if(!processLogSection(conf, line)) return false;
				break;
			case PROPERTY_SECTION:
				if(!processPropertySection(conf, line)) return false;
				break;
			default:
				return false;
		}
	}

	return ( LINE_END == result );
}


bool NexConfReader::readConfFile(const char* path)
{
	FILE * fconf = fopen(path, "rb");

	if(NULL == fconf) return false;


	bool result = false;
	int len;

	fseek(fconf, 0, SEEK_END);
	len = ftell(fconf);

	cache = new char[len];
	if(NULL == cache) goto __close_file;

	__curr = cache;
	__eof = __curr + len;

	fseek(fconf, 0, SEEK_SET);
	result = (len == (int)fread(cache, 1, len, fconf));

__close_file:
	fclose(fconf);

	return result;
}

void NexConfReader::removeRemark(char* line)
{
	while( *line )
	{
		if(  '#'== *line || ';' == *line )
		{
			*line = '\0';
			return;
		}
		++line;
	}
}

void NexConfReader::removeWhiteSpace(char** line)
{
	char* out = *line;
	int i;

	i = (int)strlen(out);

	if(i < 1 ) return;
	--i;
	while (out[i] <= ' ')
	{
		out[i] = '\0';
		--i;
	}

	while (*out && *out <= ' ')
	{
		++out;
	}

	*line = out;
}


NexConfReader::LINE_RESULT NexConfReader::getLine(char** line)
{
	*line = NULL;

	while(__curr < __eof && (*__curr == 0x0d || *__curr == 0x0a)) ++__curr;
	if(__curr >= __eof) return LINE_END;
	*line = (char*)__curr;

	while(__curr < __eof && *__curr != 0x0d && *__curr != 0x0a) ++__curr;
	*__curr = '\0';

	++__curr;

	return (NULL != *line)?LINE_SUCCESS:LINE_ERROR;
}


bool NexConfReader::getItemAndValue(char* line, char** item, char** value)
{
	*item = line;
	while(*line && '=' != *line) ++line;
	if(  0 == *line ) return false;
	*line = '\0';

	++line;

	removeWhiteSpace(item);
	removeWhiteSpace(&line);

	if( '"' == *line )
	{
		int pos = strlen(line)-1;
		if( '"' != line[pos])
			return false;
		line[pos] = '\0';
		++line;
	}

	*value = line;

	return true;
}


bool NexConfReader::isSection(char* line)
{
	int last = (int)strlen(line) - 1;

	return line[0] == '[' && line[last] == ']';
}

NexConfReader::CONF_STATE NexConfReader::getConfState(char* section)
{
	if( 0 == strcasecmp("[LOG]", section) )
		return LOG_SECTION;
	else if( 0 == strcasecmp("[PROPERTY]", section) )
		return PROPERTY_SECTION;

	return NONE_SECTION;
}

bool NexConfReader::processLogSection(NexConf *conf, char* line)
{
	char *item = NULL, *value = NULL;

	if(!getItemAndValue(line, &item, &value)) return false;

	int level = atoi(value);

	if( 0 == strcasecmp("CATEGORY_ENGINE", item) )
		conf->log.engine = level;
	else if( 0 == strcasecmp("CATEGORY_CODEC", item) )
		conf->log.codec = level;
	else if( 0 == strcasecmp("CATEGORY_THEMERENDER", item) )
		conf->log.themerender = level;
	return true;
}

bool NexConfReader::processPropertySection(NexConf *conf, char* line)
{
	char *item = NULL,  *value = NULL;

	if(!getItemAndValue(line, &item, &value)) return false;

	int level = atoi(value);

	if( 0 == strcasecmp("PROPERTY_VIDEO_MAXWIDTH", item) )
		conf->properties.video_maxwidth = level;
	else if( 0 == strcasecmp("PROPERTY_VIDEO_MAXHEIGHT", item) )
		conf->properties.video_maxheight = level;
	else if( 0 == strcasecmp("PROPERTY_VIDEOBUFFER_MAXSIZE", item) )
		conf->properties.videobuffer_maxsize = level;
	else if( 0 == strcasecmp("PROPERTY_HW_DEC_COUNT", item) )
		conf->properties.hw_dec_count = level;
	else if( 0 == strcasecmp("PROPERTY_HW_ENC_COUNT", item) )
		conf->properties.hw_enc_count = level;

	return true;
}

