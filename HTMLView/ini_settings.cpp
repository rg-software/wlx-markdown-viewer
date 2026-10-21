#include "ini_settings.h"
#include <string.h>
#include <stdlib.h>

// Returned by GetPrivateProfileString() when the key, the section or the file
// is missing. The Win32 API copies the default verbatim in that case; it does
// not write it back to the file and does not create a missing file.
static const char INI_KEY_MISSING[] = "\x01\x02\x03\x04";

// The sample file ships next to the plugin as "<ini file name>.sample".
#define INI_SAMPLE_SUFFIX ".sample"

// Builds the path of the shipped sample for a given ini file.
static bool GetSamplePath(const char* ini_path, char* buffer, int buffer_size)
{
	if(!ini_path || !*ini_path || !buffer || buffer_size <= 0)
		return false;
	int len = (int)strlen(ini_path);
	if(len + (int)sizeof(INI_SAMPLE_SUFFIX) > buffer_size)
		return false;
	strcpy(buffer, ini_path);
	strcat(buffer, INI_SAMPLE_SUFFIX);
	return true;
}

// Reads one layer. Returns false when the file does not hold the key.
static bool ReadIniValue(const char* file, const char* section, const char* key,
                         char* buffer, int buffer_size)
{
	if(!file || !*file || !section || !key || !buffer)
		return false;
	if(buffer_size < (int)sizeof(INI_KEY_MISSING))
		return false;	// the sentinel would not fit
	GetPrivateProfileString(section, key, INI_KEY_MISSING, buffer, buffer_size, file);
	return strcmp(buffer, INI_KEY_MISSING) != 0;
}

int GetIniSettingString(const char* ini_path, const char* section, const char* key,
                         const char* hardcoded_default, char* buffer, int buffer_size)
{
	if(!buffer || buffer_size <= 0)
		return 0;
	char sample_path[512];
	if(ReadIniValue(ini_path, section, key, buffer, buffer_size))
		return (int)strlen(buffer);	// layer 1: the user's ini
	if(ReadIniValue(GetSamplePath(ini_path, sample_path, sizeof(sample_path)) ? sample_path : NULL,
	                section, key, buffer, buffer_size))
		return (int)strlen(buffer);	// layer 2: the shipped sample
	// layer 3: the hardcoded default
	if(!hardcoded_default)
		hardcoded_default = "";
	strncpy(buffer, hardcoded_default, buffer_size-1);
	buffer[buffer_size-1] = 0;
	return (int)strlen(buffer);
}

int GetIniSettingInt(const char* ini_path, const char* section, const char* key,
                     int hardcoded_default)
{
	char buffer[64];
	char sample_path[512];
	const char* files[2];
	files[0] = ini_path;
	files[1] = GetSamplePath(ini_path, sample_path, sizeof(sample_path)) ? sample_path : NULL;
	for(int i=0; i<2; i++)
	{
		if(!ReadIniValue(files[i], section, key, buffer, sizeof(buffer)))
			continue;
		if(!buffer[0])
			return hardcoded_default;	// same as GetPrivateProfileInt()
		// Same conversion as GetPrivateProfileInt(): base 0 ("0x10" is 16),
		// truncated to 32 bits. A value that is not a number becomes 0.
		return (int)(unsigned int)strtol(buffer, NULL, 0);
	}
	return hardcoded_default;	// layer 3
}
