#include <direct.h>
#include <windows.h>
#include <fstream>
#include <sstream>
#include "functions.h"
#include "browserhost.h"
#include "ListerPlugin.h"
#include "resource.h"
#include <ExDispID.h>
#include "common.h"
#include <locale>
#include <codecvt>
#include <algorithm>

HHOOK hook_keyb = NULL;
HIMAGELIST img_list = NULL;
int num_lister_windows = 0;

char* INPUT_STRING;
char* SP_INPUT_STRING;
char* OUTPUT_STRING;
char* SP_OUTPUT_STRING;
extern "C" int hoedown_main(int argc, const char **argv);
extern "C" int smartypants_main(int argc, char **argv);
extern "C" int smartypants_main_null(int argc, char **argv);

char result_file_name_copy[MAX_PATH];

CSmallStringList html_extensions;
CSmallStringList markdown_extensions;
CSmallStringList chm_extensions;
CSmallStringList def_signatures;
CSmallStringList trans_hotkeys;
CSmallStringList typing_trans_hotkeys;
char hoedown_args[512];
char smartypants_args[512];
char html_template[512];

LRESULT CALLBACK HookKeybProc(int nCode,WPARAM wParam,LPARAM lParam)
{
	if (nCode<0/* || dbg_DontExecHook*/) 
		return CallNextHookEx(hook_keyb, nCode, wParam, lParam);
	HWND BrowserWnd=GetBrowserHostWnd(GetFocus());
	if(BrowserWnd)
		SendMessage(BrowserWnd,WM_IEVIEW_HOTKEY,wParam,lParam);
	return CallNextHookEx(hook_keyb, nCode, wParam, lParam);
}

void InitProc()
{
	if(!options.valid)
		InitOptions();
	if(!hook_keyb&&(options.flags&OPT_KEEPHOOKNOWINDOWS))
		hook_keyb = SetWindowsHookEx(WH_KEYBOARD, HookKeybProc, hinst, (options.flags&OPT_GLOBALHOOK)?0:GetCurrentThreadId());
	if(!img_list)
	{
		unsigned char toolbar_bpp = (options.toolbar>>2)&3;
		if(toolbar_bpp==2)
		{
			OSVERSIONINFO osvi;
			ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&osvi);
			toolbar_bpp = (osvi.dwMajorVersion>5||osvi.dwMajorVersion==5&&osvi.dwMinorVersion>=1)?1:0;
		}
		if(toolbar_bpp==1)
			img_list = ImageList_LoadImage(hinst, MAKEINTRESOURCE(IDB_BITMAP2), 24, 0, CLR_NONE, IMAGE_BITMAP, LR_CREATEDIBSECTION);
		else
			img_list = ImageList_LoadImage(hinst, MAKEINTRESOURCE(IDB_BITMAP1), 24, 0, CLR_DEFAULT, IMAGE_BITMAP, LR_CREATEDIBSECTION);
	}
	
	if(!markdown_extensions.valid())
		markdown_extensions.load_from_ini(options.IniFileName, "Extensions", "MarkdownExtensions");
	if(!html_extensions.valid())
		html_extensions.load_from_ini(options.IniFileName, "Extensions", "HTMLExtensions");
	if(!chm_extensions.valid())
		chm_extensions.load_from_ini(options.IniFileName, "Extensions", "CHMExtensions");
	if(!def_signatures.valid())
		def_signatures.load_sign_from_ini(options.IniFileName, "Extensions", "DefaultSignatures");
	if(!typing_trans_hotkeys.valid())
		typing_trans_hotkeys.load_from_ini(options.IniFileName, "Hotkeys", "TypingTranslationHotkeys");
	if(!trans_hotkeys.valid())
		trans_hotkeys.load_from_ini(options.IniFileName, "Hotkeys", "TranslationHotkeys");
	
	GetPrivateProfileString("Renderer", "HoedownArgs", "", &hoedown_args[0], 512, options.IniFileName);
	GetPrivateProfileString("Renderer", "UseSmartyPants", "", &smartypants_args[0], 512, options.IniFileName);
	GetPrivateProfileString("Renderer", "CustomCSS", "", &html_template[0], 512, options.IniFileName);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_CREATE)
	{
	}
	else if(message==WM_DESTROY && !(options.flags&OPT_QUICKQIUT))
	{
		HWND status = (HWND)GetProp(hWnd, PROP_STATUS);
		HWND toolbar = (HWND)GetProp(hWnd, PROP_TOOLBAR);
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		RemoveProp(hWnd, PROP_BROWSER);
		RemoveProp(hWnd, PROP_STATUS);
		RemoveProp(hWnd, PROP_TOOLBAR);
		if(status)
			DestroyWindow(status);
		if(toolbar)
			DestroyWindow(toolbar);
		if(browser_host)
		{
			if(options.flags&OPT_SAVEPOS)
				browser_host->SavePosition();
			browser_host->Quit();
		}
	}
	else if(message==WM_SIZE)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		if(browser_host)
			browser_host->Resize();
		HWND status = (HWND)GetProp(hWnd, PROP_STATUS);
		if(status)
		{
			RECT status_rc,rc;
			GetClientRect(hWnd,&rc);
			GetWindowRect(status,&status_rc);
			MoveWindow(status,0,rc.bottom-(status_rc.bottom-status_rc.top),rc.right-rc.left,status_rc.bottom-status_rc.top,TRUE);
			InvalidateRect(status,NULL,TRUE);
		}
		HWND toolbar = (HWND)GetProp(hWnd, PROP_TOOLBAR);
		if(toolbar)
		{
			RECT toolbar_rc;
			GetWindowRect(toolbar, &toolbar_rc);
			MoveWindow(toolbar, 0, 0, LOWORD(lParam), toolbar_rc.bottom-toolbar_rc.top, TRUE);
			InvalidateRect(toolbar,NULL,TRUE);
		}
	}
	else if(message==WM_SETFOCUS)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		if(browser_host)
			browser_host->Focus();
	}
	else if(message==WM_COMMAND)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		if(browser_host && lParam==(LPARAM)GetProp(hWnd, PROP_TOOLBAR))
		{
			switch(LOWORD(wParam))
			{
			case TBB_BACK:
				browser_host->mWebBrowser->GoBack();
				break;
			case TBB_FORWARD:
				browser_host->mWebBrowser->GoForward();
				break;
			case TBB_STOP:
				browser_host->mWebBrowser->Stop();
				break;
			case TBB_REFRESH:
				browser_host->mWebBrowser->Refresh();	// actually we'll need to reload the page here
				break;
			case TBB_PRINT:
				SendMessage(hWnd, WM_IEVIEW_PRINT, 0, 0);
				break;
			case TBB_COPY:
				SendMessage(hWnd, WM_IEVIEW_COMMAND, lc_copy, 0);
				break;
			/*case TBB_PASTE:
				SendMessage(hWnd, WM_IEVIEW_COMMAND, lc_ieview_paste, 0);
				break;*/
			case TBB_SEARCH:
				if(browser_host->mFocusType==fctQuickView)
					SetFocus(hWnd);
				SendMessage(hWnd, WM_KEYDOWN, VK_F3, 0);
				//SendMessage(hWnd, WM_IEVIEW_SEARCH, 0, 0);
				break;
			}
		}
	}
	else if(message==WM_IEVIEW_SEARCH||message==WM_IEVIEW_SEARCHW)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd ,PROP_BROWSER);
		if(browser_host)
		{
			long flags = 0;
			//if(lParam&lcs_findfirst)
			//	flags |= ;
			if(lParam&lcs_matchcase)
				flags |= 4;
			if(lParam&lcs_wholewords)
				flags |= 2;
			//if(lParam&lcs_backwards)
			//	flags |= 1;
			if(message==WM_IEVIEW_SEARCH)
				browser_host->FindText(CComBSTR((char*)wParam), flags, lParam&lcs_backwards);
			else if(message==WM_IEVIEW_SEARCHW)
				browser_host->FindText(CComBSTR((WCHAR*)wParam), flags, lParam&lcs_backwards);
		}
	}
	else if(message==WM_IEVIEW_PRINT)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd ,PROP_BROWSER);
		CComQIPtr<IOleCommandTarget, &IID_IOleCommandTarget> pCmd = browser_host->mWebBrowser;
		if ( pCmd ) 
			pCmd->Exec(NULL, OLECMDID_PRINT, OLECMDEXECOPT_DODEFAULT, NULL,NULL);
	}
	else if(message==WM_IEVIEW_COMMAND)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		if ( browser_host ) 
		{
			CComQIPtr<IOleCommandTarget, &IID_IOleCommandTarget> pCmd = browser_host->mWebBrowser;
			if ( pCmd ) 
			{
				if(wParam==lc_selectall)
					pCmd->Exec(NULL, OLECMDID_SELECTALL, OLECMDEXECOPT_DODEFAULT, NULL,NULL);
				else if(wParam==lc_copy)
					pCmd->Exec(NULL, OLECMDID_COPY, OLECMDEXECOPT_DODEFAULT, NULL,NULL);
				/*else if(wParam==lc_ieview_paste)
					pCmd->Exec(NULL, OLECMDID_PASTE, OLECMDEXECOPT_DODEFAULT, NULL,NULL);*/
			}
		}
	}
	else if(message==WM_IEVIEW_HOTKEY)
	{
		CBrowserHost* browser_host = (CBrowserHost*)GetProp(hWnd,PROP_BROWSER);
		if ( browser_host ) 
		{
			bool alt_down = 0x20000000&lParam;
			bool key_down = 0x80000000&lParam;
			UINT Msg = key_down?(alt_down?WM_SYSKEYUP:WM_KEYUP):(alt_down?WM_SYSKEYDOWN:WM_KEYDOWN);
			CAtlString key_name = GetFullKeyName(wParam);
			if(key_name=="Ctrl+Insert")
				SendMessage(hWnd, WM_IEVIEW_COMMAND, lc_copy, 0);
			if(browser_host->FormFocused())
			{
				if(typing_trans_hotkeys.find(key_name)&&!GetCapture()) 
					SendMessage(hWnd, Msg, wParam, lParam);
			}
			else
			{
				if(trans_hotkeys.find(key_name)&&!GetCapture()) 
					SendMessage(hWnd, Msg, wParam, lParam);
			}
			browser_host->ProcessHotkey(Msg, wParam, lParam);
		}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

HWND Create_Toolbar(HWND ListWin)
{
	TBBUTTON tb_buttons[10] = 
	{
		{0, TBB_BACK,		TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{1, TBB_FORWARD,	TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{2, TBB_STOP,		TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{3, TBB_REFRESH,	TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{-1, -1,			TBSTATE_ENABLED, BTNS_SEP,	  NULL},
		{5, TBB_COPY,		TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		//{6, TBB_PASTE,		TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{-1, -1,			TBSTATE_ENABLED, BTNS_SEP,	  NULL},
		{4, TBB_PRINT,		TBSTATE_ENABLED, BTNS_BUTTON, NULL},
		{-1, -1,			TBSTATE_ENABLED, BTNS_SEP,	  NULL},
		{7, TBB_SEARCH,		TBSTATE_ENABLED, BTNS_BUTTON, NULL}
	};

	char parent_class_name[64];
	GetClassName(GetParent(ListWin), parent_class_name, 64);
	if(strncmp(parent_class_name, "TFormViewUV", 11)==0)
		tb_buttons[5].fsState = tb_buttons[6].fsState = tb_buttons[9].fsState = TBSTATE_HIDDEN;

	HWND toolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD|CCS_TOP, 0, 0, 0, 0, ListWin, NULL, hinst, NULL); 
	SendMessage(toolbar, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0); 
	SendMessage(toolbar, TB_SETIMAGELIST, 0, (LPARAM)img_list);
	SendMessage(toolbar, TB_ADDBUTTONS, 10, (LPARAM)&tb_buttons);
	SendMessage(toolbar, TB_AUTOSIZE, 0, 0);

	/*
	RECT rect, toolbar_rect;
	GetWindowRect(ListWin, &rect); 
	GetWindowRect(toolbar, &toolbar_rect);
	MoveWindow(toolbar, 0, 0, rect.right-rect.left, toolbar_rect.bottom-toolbar_rect.top, TRUE);
	GetWindowRect(toolbar, &toolbar_rect);
	*/
	ShowWindow(toolbar, SW_SHOW);
	return toolbar;
}

CComBSTR GetUrlFromFilename(char* FileToLoad)
{
	CAtlString url;
	char ext[MAX_PATH];
	_splitpath(FileToLoad, NULL, NULL, NULL, ext);
	strlwr(ext);
	if((options.flags&OPT_DIRS)&&FileToLoad[strlen(FileToLoad)-1]=='\\')
		url = FileToLoad;
	else if( html_extensions.find(ext+1) )
		url = FileToLoad;
	else if( chm_extensions.find(ext+1) )
		url = GetCHMIndex(FileToLoad);
	else if(def_signatures.check_signature(FileToLoad, options.flags&OPT_SIGNSKIPSPACES))
		url = FileToLoad;
	if(url.IsEmpty() || url.Right(3)=="..\\")
		return NULL;
	return CComBSTR(url);
}

//							#------------#
//  						|	  	     |
//**************************|   Lister   |***************************
//							|		     |
//							#------------#

void do_events()
{
	MSG msg;
	BOOL result;

	while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		result = ::GetMessage(&msg, NULL, 0, 0);
		if (result == 0) // WM_QUIT
		{
			::PostQuitMessage(msg.wParam);
			break;
		}
		else if (result == -1)
		{
			// Handle errors/exit application, etc.
		}
		else
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}
}

void prepare_browser(CBrowserHost* browser_host)
{
	browser_host->mWebBrowser->Navigate(L"about:blank", NULL, NULL, NULL, NULL);

	// it's really a bad method, but in practice we won't have to wait, so 
	// perhaps the loop will never get executed
	READYSTATE rs;
	do
	{
		browser_host->mWebBrowser->get_ReadyState(&rs);
		do_events();
	} while (rs != READYSTATE_COMPLETE);
}

char* read_file(const char* FileToLoad)
{
	std::ifstream t;
	int length;
	t.open(FileToLoad);      
	t.seekg(0, std::ios::end);
	length = t.tellg();       
	t.seekg(0, std::ios::beg); 
	char* buffer = (char*)calloc(length + 1, 1);
	t.read(buffer, length);
	t.close();
	return buffer;
}

void browser_show_file(CBrowserHost* browser_host, const char* FileToLoad)
{
	INPUT_STRING = read_file(FileToLoad);

	std::vector<std::string> hoedown_args_list;
	std::istringstream f(hoedown_args);
	std::string s;
	while(f >> s) // space-separated list
		hoedown_args_list.push_back(s); 
	
	const char* hoedown_argv[512];
	for (int i = 0; i < hoedown_args_list.size(); ++i)
		hoedown_argv[i] = hoedown_args_list[i].c_str();

	hoedown_main(hoedown_args_list.size(), hoedown_argv);

	SP_INPUT_STRING = OUTPUT_STRING;

	char* smartypants_argv[] = { "smartypants" };
	
	if(smartypants_args[0] == '1')	// should use smartypants
		smartypants_main(1, smartypants_argv);
	else
		smartypants_main_null(1, smartypants_argv);

	CHAR file_path[MAX_PATH];
	CHAR file_url[MAX_PATH];
	DWORD path_len = MAX_PATH;
	strcpy(file_path, FileToLoad);
	PathRemoveFileSpec(file_path);	// no file name, no trailing slash
	UrlCreateFromPath(file_path, file_url, &path_len, NULL);
	strcat(file_url, "/");


	// read HTML template
	CHAR path[MAX_PATH];
	GetModuleFileName(hinst, path, MAX_PATH);
	PathRemoveFileSpec(path);
	strcat(path, "\\");
	strcat(path, html_template);

	char* html_template_string = read_file(path);
	std::string css(html_template_string);
	std::string result = "<HTML><HEAD><base href=\"" + 
						 std::string(file_url) + "\"></base><style>" + 
						 css + "</style></HEAD><BODY>" + 
						 std::string(SP_OUTPUT_STRING) + 
						 "</BODY></HTML>";

	// std::ofstream os("log.txt"); 
	// os << result;

	prepare_browser(browser_host);
	browser_host->LoadWebBrowserFromStreamWrapper(result.c_str());
	
	free(INPUT_STRING);
	free(OUTPUT_STRING);
	free(SP_OUTPUT_STRING);
	free(html_template_string);
}

bool is_markdown(const char* FileToLoad)
{
	CAtlString url;
	char ext[MAX_PATH];
	_splitpath(FileToLoad, NULL, NULL, NULL, ext);
	strlwr(ext);

	return markdown_extensions.find(ext + 1);
}

int __stdcall ListLoadNext(HWND ParentWin, HWND PluginWin, char* FileToLoad, int ShowFlags)
{
	if (!is_markdown(FileToLoad))
		return LISTPLUGIN_ERROR;

	CBrowserHost* browser_host = (CBrowserHost*)GetProp(PluginWin, PROP_BROWSER);
	if(!browser_host)
		return LISTPLUGIN_ERROR;
	
	browser_show_file(browser_host, FileToLoad);
	return LISTPLUGIN_OK;
}

HWND __stdcall ListLoad(HWND ParentWin, char* FileToLoad, int ShowFlags)
{
	OleInitialize(NULL);
	InitProc();

	if (!is_markdown(FileToLoad))
		return NULL;

	RECT Rect;
	GetClientRect(ParentWin, &Rect);

	HWND ListWin;
	HWND status;
	HWND toolbar;
	CBrowserHost* browser_host;
	bool qiuck_view = WS_CHILD&GetWindowLong(ParentWin, GWL_STYLE);
	bool need_toolbar = (!qiuck_view&&(options.toolbar&1))||(qiuck_view&&(options.toolbar&2));
	bool need_statusbar = (!qiuck_view&&(options.status&1))||(qiuck_view&&(options.status&2));
	
	ListWin = CreateWindow(MAIN_WINDOW_CLASS, "IEViewMainWindow", WS_VISIBLE|WS_CHILD|WS_CLIPCHILDREN, 0, 0, Rect.right, Rect.bottom, ParentWin, NULL, hinst, NULL);
	if(!ListWin)
		return NULL;
	if( need_statusbar )
		status = CreateStatusWindow(WS_CHILD|WS_VISIBLE,"",ListWin,0);
	else 
		status = NULL;
	SetProp(ListWin, PROP_STATUS, status);
	if( need_toolbar )
		toolbar = Create_Toolbar(ListWin);
	else 
		toolbar = NULL;
	SetProp(ListWin, PROP_TOOLBAR, toolbar);
	browser_host = new CBrowserHost;
	
	browser_host->mFocusType = qiuck_view?fctQuickView:fctLister;
	if(!browser_host->CreateBrowser(ListWin))
	{
		DestroyWindow(ListWin);
		return NULL;
	}

	browser_show_file(browser_host, FileToLoad);
	
	SetProp(ListWin, PROP_BROWSER, browser_host);
	
	if(/*!(options.flags&OPT_KEEPHOOKNOWINDOWS)&&*/hook_keyb==NULL/*&&num_lister_windows==0*/)
		hook_keyb = SetWindowsHookEx(WH_KEYBOARD, HookKeybProc, hinst, (options.flags&OPT_GLOBALHOOK)?0:GetCurrentThreadId());
	++num_lister_windows;

	return ListWin;
}

int __stdcall ListSendCommand(HWND ListWin,int Command,int Parameter)
{
	if(Command==lc_copy || Command==lc_selectall)
	{
		SendMessage(ListWin, WM_IEVIEW_COMMAND, Command, Parameter);
		return LISTPLUGIN_OK;
	}
	return LISTPLUGIN_ERROR;
}

int _stdcall ListSearchText(HWND ListWin, char* SearchString, int SearchParameter)
{
	SendMessage(ListWin, WM_IEVIEW_SEARCH, (WPARAM)SearchString, SearchParameter);
	return LISTPLUGIN_OK;
}

int _stdcall ListSearchTextW(HWND ListWin, WCHAR* SearchString, int SearchParameter)
{
	SendMessage(ListWin, WM_IEVIEW_SEARCHW, (WPARAM)SearchString, SearchParameter);
	return LISTPLUGIN_OK;
}

void __stdcall ListCloseWindow(HWND ListWin)
{
    DestroyWindow(ListWin);
	OleUninitialize();
	remove(result_file_name_copy);

	--num_lister_windows;
	if(!(options.flags&OPT_KEEPHOOKNOWINDOWS)&&hook_keyb&&num_lister_windows==0)
	{
		UnhookWindowsHookEx(hook_keyb);
		hook_keyb = NULL;
	}
	return;
}

int __stdcall ListPrint(HWND ListWin,char* FileToPrint,char* DefPrinter,int PrintFlags,RECT* Margins)
{
	SendMessage(ListWin, WM_IEVIEW_PRINT, (WPARAM)FileToPrint,0);
	return LISTPLUGIN_OK;
}

//							#---------#
//							|		  |
//**************************|   DLL   |***************************
//							|		  |
//							#---------#

BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  reason_for_call, 
					  LPVOID lpReserved
					  )
{
	if(reason_for_call==DLL_PROCESS_ATTACH)
	{
		hinst = (HINSTANCE)hModule;
		num_lister_windows = 0;
		WNDCLASS wc = {	0,//CS_HREDRAW | CS_VREDRAW,
						(WNDPROC)WndProc,0,0,hinst,NULL,
						LoadCursor(NULL, IDC_ARROW),
						NULL,NULL,MAIN_WINDOW_CLASS};
		RegisterClass(&wc);
	}
	else if(reason_for_call==DLL_PROCESS_DETACH)
	{
		if(hook_keyb)
			UnhookWindowsHookEx(hook_keyb);
		if(img_list)
			ImageList_Destroy(img_list);
	}
	return TRUE;
}
