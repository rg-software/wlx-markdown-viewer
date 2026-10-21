// Unit tests for the layered configuration read (HTMLView/ini_settings.cpp).
//
// The plugin resolves every setting through three layers:
//   1) the user's MarkdownView.ini
//   2) the shipped MarkdownView.ini.sample in the same directory
//   3) the hardcoded default of the read site (HTMLView/settings_defaults.h)
//
// The tests cover the 2x2 matrix (sample only / user overlay / user only /
// neither file) and check that the hardcoded defaults mirror the shipped
// sample (Build/MarkdownView.ini), which is the source of truth.
//
// Build and run: tests\run_tests.bat

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "../HTMLView/ini_settings.h"
#include "../HTMLView/settings_defaults.h"

static int g_checks = 0;
static int g_failures = 0;

static void CheckStr(const char* what, const std::string& expected, const std::string& actual)
{
	g_checks++;
	if(expected != actual)
	{
		g_failures++;
		printf("FAIL %s\n  expected: \"%s\"\n  actual:   \"%s\"\n", what, expected.c_str(), actual.c_str());
	}
}

static void CheckInt(const char* what, long expected, long actual)
{
	g_checks++;
	if(expected != actual)
	{
		g_failures++;
		printf("FAIL %s\n  expected: %ld\n  actual:   %ld\n", what, expected, actual);
	}
}

static bool WriteTextFile(const std::string& path, const std::string& content)
{
	HANDLE file = CreateFileA(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(file == INVALID_HANDLE_VALUE)
		return false;
	DWORD written = 0;
	WriteFile(file, content.c_str(), (DWORD)content.size(), &written, NULL);
	CloseHandle(file);
	return true;
}

static bool FileExists(const std::string& path)
{
	return GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
}

static std::string MakeTempDir()
{
	char temp[MAX_PATH];
	GetTempPathA(MAX_PATH, temp);
	std::string dir = std::string(temp) + "mv_ini_test\\";
	CreateDirectoryA(dir.c_str(), NULL);
	return dir;
}

// Reads one value straight from a file, without the layering.
static bool ReadRaw(const std::string& file, const char* section, const char* key, std::string& value)
{
	static const char SENTINEL[] = "\x01\x02\x03\x04";
	char buffer[1024];
	GetPrivateProfileString(section, key, SENTINEL, buffer, sizeof(buffer), file.c_str());
	if(strcmp(buffer, SENTINEL) == 0)
		return false;
	value = buffer;
	return true;
}

// Reads one value through the layering.
static std::string ReadLayered(const std::string& ini_path, const char* section, const char* key,
                               const char* hardcoded_default)
{
	char buffer[1024];
	GetIniSettingString(ini_path.c_str(), section, key, hardcoded_default, buffer, sizeof(buffer));
	return buffer;
}

static int ReadLayeredInt(const std::string& ini_path, const char* section, const char* key,
                          int hardcoded_default)
{
	return GetIniSettingInt(ini_path.c_str(), section, key, hardcoded_default);
}

// ---------------------------------------------------------------------------
// 1) the 2x2 matrix
// ---------------------------------------------------------------------------

static void TestMatrix()
{
	std::string dir = MakeTempDir();
	std::string ini = dir + "MarkdownView.ini";
	std::string sample = dir + "MarkdownView.ini.sample";

	const char* SAMPLE_CONTENT =
		"[options]\r\n"
		"Overridden=from-sample\r\n"
		"Inherited=from-sample\r\n"
		"[Renderer]\r\n"
		"CustomCSS=css\\from-sample.css\r\n";
	const char* USER_CONTENT =
		"[options]\r\n"
		"Overridden=from-user\r\n";

	// --- sample only -------------------------------------------------------
	DeleteFileA(ini.c_str());
	WriteTextFile(sample, SAMPLE_CONTENT);
	CheckStr("matrix/sample only: overridden key",
		"from-sample", ReadLayered(ini, "options", "Overridden", "from-hardcoded"));
	CheckStr("matrix/sample only: inherited key",
		"from-sample", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));
	CheckStr("matrix/sample only: absent key",
		"from-hardcoded", ReadLayered(ini, "options", "Absent", "from-hardcoded"));
	CheckStr("matrix/sample only: other section",
		"css\\from-sample.css", ReadLayered(ini, "Renderer", "CustomCSS", "from-hardcoded"));

	// --- user overlay ------------------------------------------------------
	WriteTextFile(ini, USER_CONTENT);
	CheckStr("matrix/user overlay: overridden key",
		"from-user", ReadLayered(ini, "options", "Overridden", "from-hardcoded"));
	CheckStr("matrix/user overlay: inherited key",
		"from-sample", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));
	CheckStr("matrix/user overlay: absent key",
		"from-hardcoded", ReadLayered(ini, "options", "Absent", "from-hardcoded"));
	CheckStr("matrix/user overlay: other section",
		"css\\from-sample.css", ReadLayered(ini, "Renderer", "CustomCSS", "from-hardcoded"));

	// --- user only ---------------------------------------------------------
	DeleteFileA(sample.c_str());
	CheckStr("matrix/user only: overridden key",
		"from-user", ReadLayered(ini, "options", "Overridden", "from-hardcoded"));
	CheckStr("matrix/user only: inherited key",
		"from-hardcoded", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));
	CheckStr("matrix/user only: absent key",
		"from-hardcoded", ReadLayered(ini, "options", "Absent", "from-hardcoded"));
	CheckStr("matrix/user only: other section",
		"from-hardcoded", ReadLayered(ini, "Renderer", "CustomCSS", "from-hardcoded"));

	// --- neither file ------------------------------------------------------
	DeleteFileA(ini.c_str());
	CheckStr("matrix/neither: overridden key",
		"from-hardcoded", ReadLayered(ini, "options", "Overridden", "from-hardcoded"));
	CheckStr("matrix/neither: inherited key",
		"from-hardcoded", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));
	CheckStr("matrix/neither: absent key",
		"from-hardcoded", ReadLayered(ini, "options", "Absent", "from-hardcoded"));

	// the reads must not create or modify any file
	CheckInt("matrix: no ini file was created", 0, FileExists(ini) ? 1 : 0);
}

// ---------------------------------------------------------------------------
// 2) empty user file / empty section
// ---------------------------------------------------------------------------

static void TestEmptyFiles()
{
	std::string dir = MakeTempDir();
	std::string ini = dir + "MarkdownView.ini";
	std::string sample = dir + "MarkdownView.ini.sample";

	WriteTextFile(sample, "[options]\r\nInherited=from-sample\r\n");

	// an empty user file behaves like a missing one
	WriteTextFile(ini, "");
	CheckStr("empty user file: falls back to the sample",
		"from-sample", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));

	// an empty section behaves like a missing one
	WriteTextFile(ini, "[options]\r\n[Renderer]\r\n");
	CheckStr("empty section: falls back to the sample",
		"from-sample", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));

	// an empty value is a value, it does not fall through
	WriteTextFile(ini, "[options]\r\nInherited=\r\n");
	CheckStr("empty value: wins over the sample",
		"", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));

	// a value that only exists in the user file wins over the sample
	WriteTextFile(ini, "[options]\r\nOnlyUser=from-user\r\n");
	CheckStr("user only key",
		"from-user", ReadLayered(ini, "options", "OnlyUser", "from-hardcoded"));
	CheckStr("user only key does not hide the sample",
		"from-sample", ReadLayered(ini, "options", "Inherited", "from-hardcoded"));

	DeleteFileA(ini.c_str());
	DeleteFileA(sample.c_str());
}

// ---------------------------------------------------------------------------
// 3) the shipped sample and the hardcoded defaults must agree
// ---------------------------------------------------------------------------

struct Setting
{
	const char* section;
	const char* key;
	std::string default_value;
};

static std::string IntToString(int value)
{
	char buffer[32];
	sprintf(buffer, "%d", value);
	return buffer;
}

static void TestShippedSample(const char* sample_path)
{
	std::string shipped = sample_path;
	if(!FileExists(shipped))
	{
		g_checks++;
		g_failures++;
		printf("FAIL the shipped sample was not found: %s\n", shipped.c_str());
		return;
	}

	std::vector<Setting> settings;
	settings.push_back(Setting{"options", "ShowDirs", IntToString(DEFAULT_SHOW_DIRS)});
	settings.push_back(Setting{"options", "ListerTitle", DEFAULT_LISTER_TITLE});
	settings.push_back(Setting{"options", "NewWindowHTML", DEFAULT_NEW_WINDOW_HTML});
	settings.push_back(Setting{"options", "NewWindowCHM", DEFAULT_NEW_WINDOW_CHM});
	settings.push_back(Setting{"options", "AllowPopups", IntToString(DEFAULT_ALLOW_POPUPS)});
	settings.push_back(Setting{"options", "ShowStatusbar", IntToString(DEFAULT_SHOW_STATUSBAR)});
	settings.push_back(Setting{"options", "ShowToolbar", IntToString(DEFAULT_SHOW_TOOLBAR)});
	settings.push_back(Setting{"options", "UseSavePosition", IntToString(DEFAULT_USE_SAVE_POSITION)});
	settings.push_back(Setting{"options", "Silent", IntToString(DEFAULT_SILENT)});
	settings.push_back(Setting{"options", "AllowScripting", IntToString(DEFAULT_ALLOW_SCRIPTING)});
	settings.push_back(Setting{"options", "ShowImages", IntToString(DEFAULT_SHOW_IMAGES)});
	settings.push_back(Setting{"options", "ShowVideos", IntToString(DEFAULT_SHOW_VIDEOS)});
	settings.push_back(Setting{"options", "PlaySounds", IntToString(DEFAULT_PLAY_SOUNDS)});
	settings.push_back(Setting{"options", "AllowJava", IntToString(DEFAULT_ALLOW_JAVA)});
	settings.push_back(Setting{"options", "AllowActiveX", IntToString(DEFAULT_ALLOW_ACTIVEX)});
	settings.push_back(Setting{"options", "ForceOffline", IntToString(DEFAULT_FORCE_OFFLINE)});
	settings.push_back(Setting{"options", "HighlightAllMatches", IntToString(DEFAULT_HIGHLIGHT_ALL_MATCHES)});
	settings.push_back(Setting{"Extensions", "CHMExtensions", DEFAULT_CHM_EXTENSIONS});
	settings.push_back(Setting{"Extensions", "HTMLExtensions", DEFAULT_HTML_EXTENSIONS});
	settings.push_back(Setting{"Extensions", "MarkdownExtensions", DEFAULT_MARKDOWN_EXTENSIONS});
	settings.push_back(Setting{"Extensions", "DefaultSignatures", DEFAULT_SIGNATURES});
	settings.push_back(Setting{"Extensions", "SignatureSkipSpaces", IntToString(DEFAULT_SIGNATURE_SKIP_SPACES)});
	settings.push_back(Setting{"Hotkeys", "TranslationHotkeys", DEFAULT_TRANSLATION_HOTKEYS});
	settings.push_back(Setting{"Hotkeys", "TypingTranslationHotkeys", DEFAULT_TYPING_TRANSLATION_HOTKEYS});
	settings.push_back(Setting{"Renderer", "HoedownArgs", DEFAULT_HOEDOWN_ARGS});
	settings.push_back(Setting{"Renderer", "UseSmartypants", DEFAULT_USE_SMARTYPANTS});
	settings.push_back(Setting{"Renderer", "CustomCSS", DEFAULT_CUSTOM_CSS});
	settings.push_back(Setting{"Renderer", "CustomCSSDark", DEFAULT_CUSTOM_CSS_DARK});

	// every active key of the sample must have the same value as the hardcoded
	// default, otherwise the third layer would silently change the behaviour
	std::string value;
	for(size_t i=0; i<settings.size(); i++)
	{
		const Setting& setting = settings[i];
		std::string what = std::string("audit table: ") + setting.section + "/" + setting.key;
		if(!ReadRaw(shipped, setting.section, setting.key, value))
		{
			g_checks++;
			g_failures++;
			printf("FAIL %s is not an active key of the shipped sample\n", what.c_str());
			continue;
		}
		CheckStr((what + " (sample vs. hardcoded default)").c_str(), setting.default_value, value);
	}

	// the [Debug] keys are commented out in the sample and stay inactive
	const char* debug_keys[] = {"UseMozillaControl", "QiuckQuit", "GlobalHook", "ToolbarBPP", "KeepHookWhenNoWindows"};
	for(size_t i=0; i<sizeof(debug_keys)/sizeof(debug_keys[0]); i++)
	{
		if(ReadRaw(shipped, "Debug", debug_keys[i], value))
		{
			g_checks++;
			g_failures++;
			printf("FAIL [Debug]/%s must stay commented out in the shipped sample\n", debug_keys[i]);
		}
	}

	// the shipped sample used as fallback layer resolves every key to the
	// sample value, no matter whether the user ini exists
	std::string dir = MakeTempDir();
	std::string ini = dir + "MarkdownView.ini";
	std::string sample = dir + "MarkdownView.ini.sample";
	CopyFileA(shipped.c_str(), sample.c_str(), FALSE);
	for(size_t i=0; i<settings.size(); i++)
	{
		const Setting& setting = settings[i];
		std::string what = std::string("sample as fallback layer: ") + setting.section + "/" + setting.key;
		CheckStr(what.c_str(), setting.default_value,
			ReadLayered(ini, setting.section, setting.key, setting.default_value.c_str()));
	}
	DeleteFileA(sample.c_str());
	DeleteFileA(ini.c_str());
}

// ---------------------------------------------------------------------------
// 4) integer conversion keeps the GetPrivateProfileInt semantics
// ---------------------------------------------------------------------------

static void TestIntConversion()
{
	std::string dir = MakeTempDir();
	std::string ini = dir + "MarkdownView.ini";
	WriteTextFile(ini,
		"[options]\r\n"
		"Decimal=42\r\n"
		"Negative=-7\r\n"
		"Hex=0x10\r\n"
		"Text=abc\r\n"
		"Empty=\r\n"
		"Spaced= 5 \r\n"
		"Plus=+3\r\n");

	CheckInt("int: decimal", 42, ReadLayeredInt(ini, "options", "Decimal", 99));
	CheckInt("int: negative", -7, ReadLayeredInt(ini, "options", "Negative", 99));
	CheckInt("int: hexadecimal", 16, ReadLayeredInt(ini, "options", "Hex", 99));
	CheckInt("int: not a number", 0, ReadLayeredInt(ini, "options", "Text", 99));
	CheckInt("int: empty value", 99, ReadLayeredInt(ini, "options", "Empty", 99));
	CheckInt("int: leading spaces", 5, ReadLayeredInt(ini, "options", "Spaced", 99));
	CheckInt("int: leading plus", 3, ReadLayeredInt(ini, "options", "Plus", 99));
	CheckInt("int: missing key", 99, ReadLayeredInt(ini, "options", "Missing", 99));

	DeleteFileA(ini.c_str());
}

// ---------------------------------------------------------------------------
// 5) section and key names are matched case-insensitively
// ---------------------------------------------------------------------------

static void TestCaseInsensitive()
{
	std::string dir = MakeTempDir();
	std::string ini = dir + "MarkdownView.ini";
	std::string sample = dir + "MarkdownView.ini.sample";
	WriteTextFile(sample, "[Renderer]\r\nUseSmartypants=1\r\n");
	WriteTextFile(ini, "[RENDERER]\r\nusesmartypants=0\r\n");

	CheckStr("case insensitive: user value wins",
		"0", ReadLayered(ini, "Renderer", "UseSmartyPants", "1"));
	DeleteFileA(ini.c_str());
	CheckStr("case insensitive: sample value is found",
		"1", ReadLayered(ini, "Renderer", "UseSmartyPants", "0"));

	DeleteFileA(sample.c_str());
	DeleteFileA(ini.c_str());
}

int main(int argc, char** argv)
{
	const char* sample_path = argc > 1 ? argv[1] : "Build\\MarkdownView.ini";

	printf("sample under test: %s\n", sample_path);

	TestMatrix();
	TestEmptyFiles();
	TestShippedSample(sample_path);
	TestIntConversion();
	TestCaseInsensitive();

	printf("%d checks, %d failures\n", g_checks, g_failures);
	if(g_failures)
	{
		printf("FAILED\n");
		return 1;
	}
	printf("PASSED\n");
	return 0;
}
