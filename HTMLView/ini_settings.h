#ifndef _INI_SETTINGS_H_
#define _INI_SETTINGS_H_

#include <windows.h>

// Layered lookup of the plugin configuration.
//
// Layer 1: the user's MarkdownView.ini           (<ini_path>)
// Layer 2: the shipped MarkdownView.ini.sample   (<ini_path>.sample)
// Layer 3: the hardcoded default of the read site
//
// The first layer that holds the section/key wins. A key that is in neither
// file resolves to the hardcoded default, so a missing or unreadable sample
// never changes the behaviour of the plugin.
//
// Section and key names are matched case-insensitively, exactly as the Win32
// profile API does.

// Returns the length of the resolved value (like GetPrivateProfileString does).
int GetIniSettingString(const char* ini_path, const char* section, const char* key,
                         const char* hardcoded_default, char* buffer, int buffer_size);
int GetIniSettingInt(const char* ini_path, const char* section, const char* key,
                     int hardcoded_default);

#endif //  _INI_SETTINGS_H_
