#define lc_copy			1
#define lc_newparams	2
#define lc_selectall	3
//#define lc_ieview_paste	0xFFFF0001

#define lcp_wraptext	1
#define lcp_fittowindow 2
#define lcp_ansi		4
#define lcp_ascii		8
#define lcp_variable	12

#define lcs_findfirst	1
#define lcs_matchcase	2
#define lcs_wholewords	4
#define lcs_backwards	8

#define LISTPLUGIN_OK	0
#define LISTPLUGIN_ERROR	1


HWND _stdcall ListLoad(HWND ParentWin,char* FileToLoad,int ShowFlags);
int __stdcall ListLoadNext(HWND ParentWin,HWND PluginWin,char* FileToLoad,int ShowFlags);
int _stdcall ListPrint(HWND ListWin,char* FileToPrint,int PrintFlags);
int _stdcall ListSendCommand(HWND ListWin,int Command,int Parameter);
int _stdcall ListSearchText(HWND ListWin,char* SearchString,int SearchParameter);
int _stdcall ListSearchTextW(HWND ListWin,WCHAR* SearchString,int SearchParameter);
void _stdcall ListCloseWindow(HWND ListWin);
