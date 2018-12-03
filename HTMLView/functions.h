#ifndef _IEVIEW_FUNCTIONS_H_
#define _IEVIEW_FUNCTIONS_H_

#define _CONVERSION_DONT_USE_THREAD_LOCALE
#include <windows.h>
#include <atlstr.h>

#define MAIN_WINDOW_CLASS "IEViewMainWindowClass"
#define TBB_BACK 0
#define TBB_FORWARD 1
#define TBB_STOP 2
#define TBB_REFRESH 3
#define TBB_PRINT 4
#define TBB_COPY 5
#define TBB_PASTE 6
#define TBB_SEARCH 7
#define PROP_BROWSER "HTMLView_Host"
#define PROP_STATUS "HTMLView_StatusBar"
#define PROP_TOOLBAR "HTMLView_Toolbar"
#define WM_IEVIEW_COMMAND (WM_USER+501)
#define WM_IEVIEW_SEARCH (WM_USER+502)
#define WM_IEVIEW_PRINT (WM_USER+503)
#define WM_IEVIEW_HOTKEY (WM_USER+504)
#define WM_IEVIEW_SEARCHW (WM_USER+505)

#define OPT_TOOLBAR			0x1
#define OPT_TITLE			0x2
#define OPT_SAVEPOS			0x4
#define OPT_STATUS_QV		0x8
#define OPT_POPUPS			0x10
//#define OPT_DEFBROWSER	0x20
#define OPT_MOZILLA			0x40
#define OPT_QUICKQIUT		0x80
#define OPT_GLOBALHOOK		0x100
#define OPT_DIRS			0x200
#define OPT_SIGNSKIPSPACES	0x400
#define OPT_KEEPHOOKNOWINDOWS	0x800

class CBrowserHost;

struct CSmallStringList
{
protected:
	unsigned char* data;
public:
	CSmallStringList();
	void clear();
	void set_size(int size);
	void set_data(const unsigned char* buffer, int size);
	bool valid();
	void load_from_ini(const char* filename, const char* section, const char* key);
	void load_sign_from_ini(const char* filename, const char* section, const char* key);
	bool find(const char* str);
	bool check_signature(const char* filename, bool skip_spaces);
};

struct SOptions
{
	bool			valid;
	long			flags;
	long			dlcontrol;
	unsigned char	toolbar;
	unsigned char	status;
	unsigned char	highlight_all_matches;
	char			IniFileName[320], LogIniFileName[320];
	//SOptions():RefCount(0), dlcontrol(0){*IniFileName=*LogIniFileName=0;}
};
extern SOptions options;
extern HINSTANCE hinst;

void InitOptions();
void InitProc();
HWND GetBrowserHostWnd(HWND child_hwnd);
CAtlString GetFullKeyName(WORD key);
CAtlString GetCHMIndex(char* FileName);

int Log(char* Section,char* Text);
void LogTime(char* Text);
void LogTime(int number);
void LogTimeReset();
void DisplayLastError(void);

#endif //  _IEVIEW_FUNCTIONS_H_