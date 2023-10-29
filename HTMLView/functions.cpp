#ifndef E_BOUNDS
#define E_BOUNDS                         _HRESULT_TYPEDEF_(0x8000000BL)
#endif

#include <mshtmdid.h>
#include "functions.h"

extern "C"
{
	#include "CHMLIB\chm_lib.h"
}


HINSTANCE hinst = NULL;

SOptions options = {false, 0, 0, 0, 0, 0, "", ""};
LARGE_INTEGER LogFirstCount = {0};

//					   #--------------------#
//				 	   |		            |
//*********************|  CSmallStringList  |**************************
//					   |		            |
//					   #--------------------#

CSmallStringList::CSmallStringList():data(NULL)
{
}

void CSmallStringList::clear()
{
	if(data)
		delete[] data;
	data = NULL;
}
void CSmallStringList::set_size(int size)
{
	clear();
	data = new unsigned char[size];
}
void CSmallStringList::set_data(const unsigned char* buffer, int size)
{
	set_size(size);
	memcpy(data, buffer, size);
}
bool CSmallStringList::valid()
{
	return data;
}
void CSmallStringList::load_from_ini(const char* filename, const char* section, const char* key)
{
	char buffer[512];
	GetPrivateProfileString(section, key, "", buffer+1, sizeof(buffer)-1, filename);
	strcat(buffer+1, ";");
	char* next_pos;
	char* str_pos = buffer;
	while(true)
	{
		next_pos=strchr(str_pos+1, ';');
		if(next_pos==NULL || next_pos==str_pos)
			break;
		*str_pos = next_pos-str_pos-1;
		str_pos = next_pos;
	}
	*str_pos = 0;
	set_data((unsigned char*)buffer, str_pos-buffer+1);
}
void CSmallStringList::load_sign_from_ini(const char* filename, const char* section, const char* key)
{
	char buffer[512];
	unsigned char list[512];
	GetPrivateProfileString(section, key, "", buffer, sizeof(buffer), filename);
	strcat(buffer, ";");
	char* next_pos;
	char* str_pos = buffer;
	unsigned char* list_pos = list;
	while(true)
	{
		next_pos = strchr(str_pos, ';');
		if(next_pos==NULL || next_pos==str_pos)
			break;
		if(*str_pos=='\"'&&*(next_pos-1)=='\"')
		{
			*list_pos = next_pos-str_pos-2;
			memcpy(list_pos+1, str_pos+1, *list_pos);
			list_pos += *list_pos+1;
		}
		else
		{
			*list_pos = (next_pos-str_pos)/2;
			for ( const char * s = str_pos; s<=next_pos-2; s += 2 )
				sscanf(s, "%2x", ++list_pos);
			++list_pos;
		}
		str_pos = next_pos+1;
	}
	*list_pos = 0;
	set_data(list, list_pos-list+1);
}
bool CSmallStringList::find(const char* str)
{
	if(!valid())
		return false;
	int len = strlen(str);
	for(const unsigned char* str_pos = data; *str_pos; str_pos+=*str_pos+1)
		if(*str_pos==len && !memcmp(str, str_pos+1, len))
			return true;
	return false;
}
bool CSmallStringList::check_signature(const char* filename, bool skip_spaces)
{
	if(!valid())
		return false;
	FILE* file = fopen(filename, "rb");
	if(file==NULL)
		return false;
	unsigned char* buf = new unsigned char[256];
	int num_read;
	for(const unsigned char* str_pos = data; *str_pos; str_pos+=*str_pos+1)
	{
		fseek(file, 0, SEEK_SET);
		if(skip_spaces)
		{
			char c_skip;
			for(int i=0;i<64;++i)
			{
				fread(&c_skip, sizeof(char), 1, file);
				if(c_skip!=' '&&c_skip!='\r'&&c_skip!='\n')
				{
					fseek(file, -1, SEEK_CUR);
					break;
				}
			}
		}
		num_read = fread(buf, sizeof(unsigned char), *str_pos, file);
		if(num_read==*str_pos && !memicmp(buf, str_pos+1, *str_pos))
		{
			fclose(file);
			return true;
		}
	}
	fclose(file);
	return false;
}

//							#----------#
//							|		   |
//**************************|   Init   |**************************
//							|		   |
//							#----------#

void InitOptions()
{
	GetModuleFileName(hinst, options.IniFileName, sizeof(options.IniFileName));
	char* dot = strrchr(options.IniFileName, '.');
	strcpy(dot, ".ini");

	BOOL ini_exists = PathFileExists(options.IniFileName);
	if(!ini_exists)
	{
		CAtlString message = "File not found: ";
		message += options.IniFileName;
		MessageBox(NULL, message, "HTMLView", MB_OK|MB_ICONWARNING);
	}


	strcpy(options.LogIniFileName, options.IniFileName);
	strcpy(options.LogIniFileName+(dot-options.IniFileName), "_Log.ini");

	options.flags = 0;
	char temp[10];
	GetPrivateProfileString("options", "ListerTitle", "", (char*)temp, sizeof(temp), options.IniFileName);
	if(temp[0])
		options.flags |= OPT_TITLE;
	//if(GetPrivateProfileInt("options", "ShowToolbar", 0, options.IniFileName))
	//	options.flags |= OPT_TOOLBAR;
	//if(GetPrivateProfileInt("options", "UseDefaultBrowser", 0, options.IniFileName))
	//	options.flags |= OPT_DEFBROWSER;
	//if(GetPrivateProfileInt("options", "StatusBarInQuickView", 0, options.IniFileName))
	//	options.flags |= OPT_STATUS_QV;
	if(GetPrivateProfileInt("options", "UseSavePosition", 0, options.IniFileName))
		options.flags |= OPT_SAVEPOS;
	if(GetPrivateProfileInt("options", "AllowPopups", 0, options.IniFileName))
		options.flags |= OPT_POPUPS;
	if(GetPrivateProfileInt("options", "ShowDirs", 0, options.IniFileName))
		options.flags |= OPT_DIRS;
	if(GetPrivateProfileInt("Debug", "UseMozillaControl", 0, options.IniFileName))
		options.flags |= OPT_MOZILLA;
	if(GetPrivateProfileInt("Debug", "QiuckQuit", 0, options.IniFileName))
		options.flags |= OPT_QUICKQIUT;
	if(GetPrivateProfileInt("Debug", "GlobalHook", 0, options.IniFileName))
		options.flags |= OPT_GLOBALHOOK;
	if(GetPrivateProfileInt("Extensions", "SignatureSkipSpaces", 0, options.IniFileName))
		options.flags |= OPT_SIGNSKIPSPACES;
	if(GetPrivateProfileInt("Debug", "KeepHookWhenNoWindows", 0, options.IniFileName))
		options.flags |= OPT_KEEPHOOKNOWINDOWS;

	options.toolbar = 3&GetPrivateProfileInt("options", "ShowToolbar", 0, options.IniFileName);
	options.status = 3&GetPrivateProfileInt("options", "ShowStatusbar", 0, options.IniFileName);

	options.toolbar |= (3&GetPrivateProfileInt("Debug", "ToolbarBPP", 2, options.IniFileName))<<2;

	options.highlight_all_matches = GetPrivateProfileInt("options", "HighlightAllMatches", 0, options.IniFileName);

	options.dlcontrol = 0;
	if(!GetPrivateProfileInt("options", "AllowScripting", 0, options.IniFileName))
			options.dlcontrol |= DLCTL_NO_SCRIPTS;
	if(GetPrivateProfileInt("options", "ShowImages", 1, options.IniFileName))
			options.dlcontrol |= DLCTL_DLIMAGES;
	if(GetPrivateProfileInt("options", "ShowVideos", 1, options.IniFileName))
			options.dlcontrol |= DLCTL_VIDEOS;
	if(GetPrivateProfileInt("options", "PlaySounds", 1, options.IniFileName))
			options.dlcontrol |= DLCTL_BGSOUNDS;
	if(!GetPrivateProfileInt("options", "AllowJava", 0, options.IniFileName))
			options.dlcontrol |= DLCTL_NO_JAVA;
	if(!GetPrivateProfileInt("options", "AllowActiveX", 0, options.IniFileName))
			options.dlcontrol |= DLCTL_NO_DLACTIVEXCTLS | DLCTL_NO_RUNACTIVEXCTLS;
	if(GetPrivateProfileInt("options", "ForceOffline", 1, options.IniFileName))
			options.dlcontrol |= DLCTL_OFFLINE | DLCTL_FORCEOFFLINE | DLCTL_OFFLINEIFNOTCONNECTED;
	if(GetPrivateProfileInt("options", "Silent", 1, options.IniFileName))
			options.dlcontrol |= DLCTL_SILENT;
	options.valid = true;
}

//						  #-------------#
//						  |		        |
//************************|  Functions  |**************************
//						  |		        |
//						  #-------------#

CAtlString GetCHMIndex(char* FileName)
{
	CAtlString result;
	chmFile* ch = chm_open(FileName);
	if(!ch)
		return "";
	result += "mk:@msitstore:";
	result += FileName;
	result += "::/";
	unsigned char* outbuf;
	LONGINT64 outlen;
	int res;
	chmUnitInfo ui;
	bool found = false;
	if(!found)
	{
		res = chm_resolve_object(ch,"/#WINDOWS",&ui);
		if(res == CHM_RESOLVE_SUCCESS)
		{
			//unsigned long index_offset=0, default_offset=0, home_offset=0;
			//chm_retrieve_object(ch,&ui,(unsigned char*)&index_offset,0x6C,4);
			//chm_retrieve_object(ch,&ui,(unsigned char*)&default_offset,0x70,4);
			//chm_retrieve_object(ch,&ui,(unsigned char*)&home_offset,0x74,4);
			unsigned long offset;
			outlen = chm_retrieve_object(ch,&ui,(unsigned char*)&offset,0x70,4);

			if(outlen && offset)
			{
				res = chm_resolve_object(ch,"/#STRINGS",&ui);
				if(res == CHM_RESOLVE_SUCCESS)
				{
					outbuf = new unsigned char[255];
					outlen = chm_retrieve_object(ch,&ui,outbuf,offset,255);
					result += (char*)outbuf;
					delete[] outbuf;
					found = true;
				}
			}
		}
	}
	if(!found)
	{
		res = chm_resolve_object(ch,"/#SYSTEM",&ui);
		if(res == CHM_RESOLVE_SUCCESS)
		{
			outbuf=new unsigned char[ui.length];
			outlen=chm_retrieve_object(ch,&ui,outbuf,0,ui.length);
			unsigned int i=4;
			if(outlen>4) 
				while(i<outlen)
				{
					if(*(WORD*)(outbuf+i)==2 && *(char*)(outbuf+i+4))
					{
						result += (char*)(outbuf+i+4);
						found=true;
						break;
					}
					i+=*(WORD*)(outbuf+i+2)+4;
				}
			delete[] outbuf;
		}
	}
	chm_close(ch);
	return result;
}

CAtlString GetKeyName(WORD key)
{
	key &= 0xFF;
	switch (key)
	{
		case VK_CANCEL:		return "Scroll Lock";
		case VK_MBUTTON:	return "";
		case VK_BACK:		return "Backspace";
		case VK_TAB:		return "Tab";
		case VK_CLEAR:		return "Clear";
		case VK_RETURN:		return "Enter";
		case VK_SHIFT:		return "Shift";
		case VK_CONTROL:	return "Ctrl";
		case VK_MENU:		return "Alt";
		case VK_PAUSE:		return "Pause";
		case VK_CAPITAL:	return "Caps Lock";
		case VK_KANA:		return "";
		case VK_JUNJA:		return "";
		case VK_FINAL:		return "";
		case VK_HANJA:		return "";
		case VK_ESCAPE:		return "Esc";
		case VK_CONVERT:	return "";
		case VK_NONCONVERT:	return "";
		case VK_ACCEPT:		return "";
		case VK_MODECHANGE:	return "";
		case VK_SPACE:		return "Space";
		case VK_PRIOR:		return "Page Up";
		case VK_NEXT:		return "Page Down";
		case VK_END:		return "End";
		case VK_HOME:		return "Home";
		case VK_LEFT:		return "Left";
		case VK_UP:			return "Up";
		case VK_RIGHT:		return "Right";
		case VK_DOWN:		return "Down";
		case VK_SELECT:		return "Select";
		case VK_PRINT:		return "Print";
		case VK_EXECUTE:	return "Execute";
		case VK_SNAPSHOT:	return "Print Screen";
		case VK_INSERT:		return "Insert";
		case VK_DELETE:		return "Delete";
		case VK_HELP:		return "Help";
		case VK_LWIN:		return "Windows";
		case VK_RWIN:		return "Right Windows";
		case VK_APPS:		return "Applications";
		case VK_NUMPAD0:	return "Num 0";
		case VK_NUMPAD1:	return "Num 1";
		case VK_NUMPAD2:	return "Num 2";
		case VK_NUMPAD3:	return "Num 3";
		case VK_NUMPAD4:	return "Num 4";
		case VK_NUMPAD5:	return "Num 5";
		case VK_NUMPAD6:	return "Num 6";
		case VK_NUMPAD7:	return "Num 7";
		case VK_NUMPAD8:	return "Num 8";
		case VK_NUMPAD9:	return "Num 9";
		case VK_MULTIPLY:	return "Num *";
		case VK_ADD:		return "Num +";
		case VK_SEPARATOR:	return "Separator";
		case VK_SUBTRACT:	return "Num -";
		case VK_DECIMAL:	return "Num Del";
		case VK_DIVIDE:		return "Num /";
		case VK_F1:			return "F1";
		case VK_F2:			return "F2";
		case VK_F3:			return "F3";
		case VK_F4:			return "F4";
		case VK_F5:			return "F5";
		case VK_F6:			return "F6";
		case VK_F7:			return "F7";
		case VK_F8:			return "F8";
		case VK_F9:			return "F9";
		case VK_F10:		return "F10";
		case VK_F11:		return "F11";
		case VK_F12:		return "F12";
		case VK_F13:		return "F13";
		case VK_F14:		return "F14";
		case VK_F15:		return "F15";
		case VK_F16:		return "F16";
		case VK_F17:		return "F17";
		case VK_F18:		return "F18";
		case VK_F19:		return "F19";
		case VK_F20:		return "F20";
		case VK_F21:		return "F21";
		case VK_F22:		return "F22";
		case VK_F23:		return "F23";
		case VK_F24:		return "F24";
		case VK_NUMLOCK:	return "Num Lock";
		case VK_SCROLL:		return "Scroll Lock";
		case VK_LSHIFT:		return "Left Shift";
		case VK_RSHIFT:		return "Right Shift";
		case VK_LCONTROL:	return "Left Ctrl";
		case VK_RCONTROL:	return "Right Ctrl";
		case VK_LMENU:		return "Left Alt";
		case VK_RMENU:		return "Right Alt";
		case VK_PROCESSKEY:	return "";
		case VK_ATTN:		return "Attn";
		case VK_CRSEL:		return "";
		case VK_EXSEL:		return "";
		case VK_EREOF:		return "";
		case VK_PLAY:		return "Play";
		case VK_ZOOM:		return "Zoom";
		case VK_NONAME:		return "";
		case VK_PA1:		return "";
		case VK_OEM_CLEAR:	return "";
		case VK_OEM_PLUS:	return "+";
	}
	UINT lParam = MapVirtualKey(key, 2);
	if ( (lParam & 0x80000000) == 0 ) 
		return CAtlString((char)lParam);
	return CAtlString();
}
CAtlString GetFullKeyName(WORD key)
{
	CAtlString result;
	if ( GetKeyState(VK_CONTROL) < 0 ) 
		result += "Ctrl+";
	if ( GetKeyState(VK_MENU) < 0 ) 
		result += "Alt+";
	if ( GetKeyState(VK_SHIFT) < 0 ) 
		result += "Shift+";
	result += GetKeyName(key);
	return result;
}
HWND GetBrowserHostWnd(HWND child_hwnd)
{
	for(HWND hWnd=child_hwnd;hWnd;hWnd=GetParent(hWnd))
		if(GetProp(hWnd,PROP_BROWSER))
			return hWnd;
	return NULL;
}
/*
CBrowserHost* GetBrowserHost(HWND child_hwnd)
{
	void* browser_host;
	for(HWND hWnd=child_hwnd;hWnd;hWnd=GetParent(hWnd))
	{
		browser_host = GetProp(hWnd, PROP_BROWSER);
		if(browser_host)
			return (CBrowserHost*)browser_host;
	}
	return NULL;
}
*/

//							#-----------#
//  						|	  	    |
//**************************|    Log    |***************************
//							|		    |
//							#-----------#

int Log(char* Section, char* Text)
{
	char num[10];int i=0;do itoa(++i,num,10);while(GetPrivateProfileString(Section,num,"",num+6,2,options.LogIniFileName));
	WritePrivateProfileString(Section,num,Text,options.LogIniFileName);
	return i;
}
void LogTimeReset()
{
	QueryPerformanceCounter(&LogFirstCount);
}

void LogTime(char* Text)
{
	LARGE_INTEGER Frequency, Count;
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&Count);
	char time_str[80];
	gcvt((Count.QuadPart-LogFirstCount.QuadPart)/(Frequency.QuadPart/1000.),10,time_str);
	WritePrivateProfileString("Times",time_str,Text,options.LogIniFileName);
}
void LogTime(int number)
{
	char str_num[16];
	itoa(number,str_num,10);
	LogTime(str_num);
}

void DisplayLastError(void)
{
	LPVOID lpMessageBuffer;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMessageBuffer,
		0,NULL );
	MessageBox(0,(LPCSTR)lpMessageBuffer,"Error",0);
	LocalFree( lpMessageBuffer );
}
