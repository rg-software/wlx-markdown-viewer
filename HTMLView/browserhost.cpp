
#include <ExDispID.h>
#include <mshtmdid.h>
#include <comutil.h>

#include "browserhost.h"
#include "functions.h"
#include <boost/nowide/convert.hpp>

CBrowserHost::CBrowserHost() :
	//mFocusWin(NULL),
	//mLastBookmark(NULL),
	mImagesHidden(false),
	mEventsCookie(0),
	mRefCount(0),
	fSearchHighlightMode(1),
	fStatusBarUnlockTime(0)
{
	mLastSearchString.Empty();
	mLastSearchFlags = 0;
}
CBrowserHost::~CBrowserHost()
{
}
void CBrowserHost::Quit()
{
	if(!mWebBrowser)
		return;
	AtlUnadvise((IUnknown*)mWebBrowser, DIID_DWebBrowserEvents, mEventsCookie);

	mWebBrowser->Stop();
	mWebBrowser->Quit();

	CComQIPtr<IOleObject> ole_object(mWebBrowser);
	if(ole_object)
		ole_object->Close(0);

	if(!(options.flags&OPT_MOZILLA))
		mWebBrowser.Release();
	Release();
}

CLSID CLSID_MozillaBrowser = {0x1339B54C,0x3453,0x11D2,{0x93,0xB9,0x00,0x00,0x00,0x00,0x00,0x00}};
bool CBrowserHost::CreateBrowser(HWND hParent)
{
	HRESULT hr;
	AddRef();
	hr = mWebBrowser.CoCreateInstance((options.flags&OPT_MOZILLA)?CLSID_MozillaBrowser:CLSID_WebBrowser/*, NULL, CLSCTX_INPROC*/);
	if(hr!=S_OK)
		return false;
	CComQIPtr<IOleObject> ole_object(mWebBrowser);
	hr = ole_object->SetClientSite((IOleClientSite*)this);
	hr = AtlAdvise(CComQIPtr<IUnknown, &IID_IUnknown>(mWebBrowser), (IDispatch*)this, DIID_DWebBrowserEvents, &mEventsCookie);

	mParentWin=hParent;
	Resize();
	MSG msg;
	RECT rect;
	GetRect(&rect);
	ole_object->DoVerb(OLEIVERB_INPLACEACTIVATE, &msg, (IOleClientSite*)this, 0, mParentWin, &rect);
	mImagesHidden = false;
	return true;
}
void CBrowserHost::GetRect(LPRECT rect)
{
	RECT status_rc;
	::GetClientRect(mParentWin, rect);
	HWND status = (HWND)GetProp(mParentWin, PROP_STATUS);
	if(status)
	{
		GetWindowRect(status, &status_rc);
		rect->bottom -= (status_rc.bottom-status_rc.top);
	}
	HWND toolbar = (HWND)GetProp(mParentWin, PROP_TOOLBAR);
	if(toolbar)
	{
		GetWindowRect(toolbar, &status_rc);
		rect->top += (status_rc.bottom-status_rc.top);
	}
}
void CBrowserHost::Resize()
{
	RECT rect;
	GetRect(&rect);
	CComQIPtr<IOleInPlaceObject> inplace_object(mWebBrowser);
	inplace_object->SetObjectRects(&rect, &rect);
}

void CBrowserHost::Focus()
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(html_disp)
	{
		CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
		if(html_doc2)
		{
			CComPtr<IHTMLWindow2> html_win;
			html_doc2->get_parentWindow(&html_win);
			if(html_win)
			{
				html_win->focus();
				return;
			}
		}
		CComQIPtr<IHTMLDocument4> html_doc(html_disp);
		if(html_doc)
		{
			html_doc->focus();
			return;
		}
	}
	CComQIPtr<IOleWindow> ole_win(mWebBrowser);
	if(ole_win)
	{
		HWND win;
		ole_win->GetWindow(&win);
		::SetFocus(win);
	}
}

void CBrowserHost::SavePosition()
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return;
	CComPtr<IHTMLElement> body_el;
    html_doc2->get_body( &body_el );
	if(!body_el)
		return;
	CComQIPtr<IHTMLElement2> body_el2(body_el);
	if(!body_el2)
		return;
	long position;
    body_el2->get_scrollTop( &position );

	BSTR loc_wide;
	char loc_char[320];
	mWebBrowser->get_LocationURL(&loc_wide);
	WideCharToMultiByte(CP_ACP,NULL,loc_wide,-1,loc_char,320,NULL,FALSE);
	SysFreeString(loc_wide);

	char ini_path[512];
	strcpy(ini_path, options.IniFileName);
	strcpy(strrchr(ini_path, '\\'), "\\positions.ini");
	char str_pos[16];
	ltoa(position,str_pos,10);
	WritePrivateProfileString("HTML", loc_char, str_pos, ini_path);
}
void CBrowserHost::LoadPosition()
{
	BSTR loc_wide;
	char loc_char[320];
	mWebBrowser->get_LocationURL(&loc_wide);
	WideCharToMultiByte(CP_ACP,NULL,loc_wide,-1,loc_char,320,NULL,FALSE);
	SysFreeString(loc_wide);

	char ini_path[512];
	strcpy(ini_path,options.IniFileName);
	strcpy(strrchr(ini_path, '\\'),"\\positions.ini");
	long position = GetPrivateProfileInt("HTML", loc_char, 0, ini_path);
	
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return;
	CComPtr<IHTMLElement> body_el;
    html_doc2->get_body( &body_el );
	if(!body_el)
		return;
	CComQIPtr<IHTMLElement2> body_el2(body_el);
	if(!body_el2)
		return;
    body_el2->put_scrollTop( position );
}

void CBrowserHost::SetStatusText(const wchar_t* str, DWORD delay)
{
	HWND status_wnd = (HWND)GetProp(mParentWin, PROP_STATUS);
	SetWindowTextW(status_wnd, str);
	fStatusBarUnlockTime = delay ? GetTickCount()+delay : 0;
}

bool CBrowserHost::IsSearchHighlightEnabled()
{
	if(fSearchHighlightMode==0)
		return false;
	else if(fSearchHighlightMode==2)
		return true;
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return false;
	fSearchHighlightMode = options.highlight_all_matches;
	if(options.highlight_all_matches==1)
	{
		VARIANT doc_mod = {0};
		DISPPARAMS params = {0};
		HRESULT hres = html_disp->Invoke(1104, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &params, &doc_mod, NULL, NULL);
		if(hres==S_OK)
			fSearchHighlightMode = 2;
		else if(hres==DISP_E_MEMBERNOTFOUND)
			fSearchHighlightMode = 0;
		return hres==S_OK;
	}
	else if(options.highlight_all_matches==2)
		return true;
	else
		return false;
}

void CBrowserHost::UpdateSearchHighlight()
{
	ClearSearchHighlight();
	HighlightStrings(mLastSearchString, mLastSearchFlags);
}

void CBrowserHost::ClearSearchHighlight()
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	CComQIPtr<IHighlightRenderingServices> hl_services(html_disp);
	if(!hl_services)
		return;
	int segm_count = mSearchHighlightSegments.size();
	for(int i=0;i<segm_count;++i)
		if(mSearchHighlightSegments[i])
			hl_services->RemoveSegment(mSearchHighlightSegments[i]);
	mSearchHighlightSegments.clear();
	if(mCurrentSearchHighlightSegment)
	{
		hl_services->RemoveSegment(mCurrentSearchHighlightSegment);
		mCurrentSearchHighlightSegment.Release();
	}
}

CAtlStringW GetSearchStatusString(int number, bool finished)
{
	if(number<0)
		return CAtlStringW(finished ? L"" : L"Searching...");
	else if(number==0)
	{	
		if(finished)
			return CAtlStringW(L"No occurrences have been found");
		else
			return CAtlStringW(L"Searching... (no occurrences have been found)");
	}
	else if(number==1)
	{
		if(finished)
			return CAtlStringW(L"1 occurrence has been found");
		else
			return CAtlStringW(L"Searching... (1 occurrence has been found)");
	}
	else
	{
		if(finished)
			return CComVariant(number)+CAtlStringW(L" occurrences have been found");
		else
			return CAtlStringW(L"Searching... (")+CComVariant(number)+CAtlStringW(L" occurrences have been found)");
	}
}

bool CBrowserHost::HighlightStrings(CComBSTR search, long search_flags)
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return false;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return false;
	CComPtr<IHTMLElement> body_el;
    html_doc2->get_body(&body_el);
	if(!body_el)
		return false;
	CComQIPtr<IHTMLBodyElement> body_element(body_el);
	if(!body_element)
		return false;
	CComPtr<IHTMLTxtRange> txt_range;
	body_element->createTextRange(&txt_range);
	if(!txt_range)
		return false;

	CComQIPtr<IHTMLDocument4> html_doc4(html_disp);
	CComQIPtr<IMarkupServices> markup_services(html_disp);
	CComQIPtr<IDisplayServices> displ_services(html_disp);
	CComQIPtr<IHighlightRenderingServices> hl_services(html_disp);
	if(!html_doc4 || !markup_services || !displ_services || !hl_services)
		return false;

	CComPtr<IHTMLRenderStyle> render_style;
	html_doc4->createRenderStyle(NULL, &render_style);
	render_style->put_defaultTextSelection(CComBSTR(L"false"));
	render_style->put_textColor(CComVariant(CComBSTR(L"transparent")));
	render_style->put_textBackgroundColor(CComVariant(0xFFFF77));

	CComPtr<IMarkupPointer> markup_start;
	CComPtr<IMarkupPointer> markup_end;
	markup_services->CreateMarkupPointer(&markup_start);
	markup_services->CreateMarkupPointer(&markup_end);

	CComPtr<IDisplayPointer> displ_ptr_start;
	CComPtr<IDisplayPointer> displ_ptr_end;
	displ_services->CreateDisplayPointer(&displ_ptr_start);
	displ_services->CreateDisplayPointer(&displ_ptr_end);

	SetStatusText(GetSearchStatusString(-1, false));
	DWORD last_status_update_time = GetTickCount();
	while(true)
	{
		VARIANT_BOOL bFound = VARIANT_FALSE;
		txt_range->findText((BSTR)search, 0x10000000, search_flags, &bFound);
		if(!bFound)
			break;
		markup_services->MovePointersToRange(txt_range, markup_start, markup_end);
		displ_ptr_start->MoveToMarkupPointer(markup_start, NULL);
		displ_ptr_end->MoveToMarkupPointer(markup_end, NULL);
		CComPtr<IHighlightSegment> hl_segment;
		hl_services->AddSegment(displ_ptr_start, displ_ptr_end, render_style, &hl_segment);
		mSearchHighlightSegments.push_back(hl_segment);
		if(GetTickCount()>last_status_update_time+500)
		{
			SetStatusText(GetSearchStatusString(mSearchHighlightSegments.size(), false));
			last_status_update_time = GetTickCount();
		}
		long t;
		//txt_range->collapse(VARIANT_TRUE);
		//txt_range->move((BSTR)CComBSTR("character"), 1, &t);
		txt_range->moveStart((BSTR)CComBSTR("character"), 1, &t);
		if(!t)
			break;
	}
	SetStatusText(GetSearchStatusString(mSearchHighlightSegments.size(), true), 1000);
	return true;
}

void CBrowserHost::SetCurrentSearchHighlight(IHTMLTxtRange* txt_range)
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return;

	CComQIPtr<IHTMLDocument4> html_doc4(html_disp);
	CComQIPtr<IMarkupServices> markup_services(html_disp);
	CComQIPtr<IDisplayServices> displ_services(html_disp);
	CComQIPtr<IHighlightRenderingServices> hl_services(html_disp);
	if(!html_doc4 || !markup_services || !displ_services || !hl_services)
		return;
	CComPtr<IHTMLRenderStyle> render_style;
	html_doc4->createRenderStyle(NULL, &render_style);

	render_style->put_defaultTextSelection(CComBSTR(L"false"));
	render_style->put_textColor(CComVariant(CComBSTR(L"transparent")));
	render_style->put_textBackgroundColor(CComVariant(0xFF9966));
	render_style->put_renderingPriority(5);

	CComPtr<IMarkupPointer> markup_start;
	CComPtr<IMarkupPointer> markup_end;
	markup_services->CreateMarkupPointer(&markup_start);
	markup_services->CreateMarkupPointer(&markup_end);

	CComPtr<IDisplayPointer> displ_ptr_start;
	CComPtr<IDisplayPointer> displ_ptr_end;
	displ_services->CreateDisplayPointer(&displ_ptr_start);
	displ_services->CreateDisplayPointer(&displ_ptr_end);

	markup_services->MovePointersToRange(txt_range, markup_start, markup_end);

	displ_ptr_start->MoveToMarkupPointer(markup_start, NULL);
	displ_ptr_end->MoveToMarkupPointer(markup_end, NULL);

	if(mCurrentSearchHighlightSegment)
	{
		hl_services->RemoveSegment(mCurrentSearchHighlightSegment);
		mCurrentSearchHighlightSegment.Release();
	}
	hl_services->AddSegment(displ_ptr_start, displ_ptr_end, render_style, &mCurrentSearchHighlightSegment);
}

bool CBrowserHost::FindText(CComBSTR search, long search_flags, bool backward)
{
	bool is_new_search = mLastSearchString!=search || mLastSearchFlags!=search_flags;
	if(is_new_search)
		mSearchTxtRange.Release();
	if(mSearchTxtRange)
	{
		CComPtr<IHTMLElement> search_range_elem;
		HRESULT hr = mSearchTxtRange->parentElement(&search_range_elem);
		if(hr!=S_OK || !search_range_elem)
		{
			is_new_search = true;
			mSearchTxtRange.Release();
		}
	}
	mLastSearchString = search;
	mLastSearchFlags = search_flags;
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return false;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return false;
	CComPtr<IHTMLElement> body_el;
    html_doc2->get_body( &body_el );
	if(!body_el)
		return false;
	CComQIPtr<IHTMLBodyElement> body_element(body_el);
	if(!body_element)
		return false;
	CComPtr<IHTMLTxtRange> txt_range;
	body_element->createTextRange(&txt_range);
	if(!txt_range)
		return false;

	bool is_highlight_enabled = IsSearchHighlightEnabled();

	long t;
	VARIANT_BOOL bTest = VARIANT_FALSE;
	HRESULT hr;
	if(mSearchTxtRange)
	{
		txt_range->moveStart(CComBSTR("textedit"), 0, &t);
		txt_range->moveEnd(CComBSTR("textedit"), 1, &t);
		hr = txt_range->setEndPoint(backward?CComBSTR("EndToEnd"):CComBSTR("StartToStart"), mSearchTxtRange);
		if(hr==S_OK)
		{
			if(backward)
				txt_range->moveEnd(CComBSTR("character"), -1, &t);
			else
				txt_range->moveStart(CComBSTR("character"), 1, &t);
		}
		else
		{
			is_new_search = true;
			mSearchTxtRange.Release();
		}
	}
	if(!mSearchTxtRange)
	{
		txt_range->moveStart(CComBSTR("textedit"), 0, &t);
		txt_range->moveEnd(CComBSTR("textedit"), 1, &t);
	}

	if(is_highlight_enabled)
		SetStatusText(GetSearchStatusString(-1, false));
	VARIANT_BOOL bFound = VARIANT_FALSE;
	if(backward)
	{
		CComPtr<IHTMLTxtRange> txt_range_curr;
		CComPtr<IHTMLTxtRange> txt_range_found;
		hr = txt_range->duplicate(&txt_range_curr);
		VARIANT_BOOL curr_found = VARIANT_FALSE;
		hr = txt_range_curr->findText(search, -0x10000000, search_flags, &curr_found);
		hr = txt_range_curr->setEndPoint(CComBSTR("EndToEnd"), txt_range);
		if(!curr_found)
		{
			CComPtr<IHTMLTxtRange> txt_range_body;
			body_element->createTextRange(&txt_range_body);
			hr = txt_range_curr->setEndPoint(CComBSTR("StartToStart"), txt_range_body);
		}
		while(true)
		{
			hr = txt_range_curr->findText(search, 0, search_flags, &curr_found);
			if(hr!=S_OK || !curr_found)
				break;
			txt_range_found.Release();
			txt_range_curr->duplicate(&txt_range_found);
			txt_range_curr->setEndPoint(CComBSTR("EndToEnd"), txt_range);
			txt_range_curr->moveStart(CComBSTR("character"), 1, &t);
		}
		if(txt_range_found)
		{
			bFound = VARIANT_TRUE;
			txt_range = txt_range_found;
		}
	}
	else
		hr = txt_range->findText(search, 0x10000000, search_flags, &bFound);
	if(is_highlight_enabled)
		SetStatusText(L"");
	if(bFound)
	{
		txt_range->scrollIntoView(TRUE);
		if(is_highlight_enabled)
		{
			CComPtr<IHTMLSelectionObject> selection;
			html_doc2->get_selection(&selection);
			if(selection)
				selection->empty();
			if(is_new_search || mSearchHighlightSegments.empty())
			{
				SetStatusText(GetSearchStatusString(-1, false));
				UpdateSearchHighlight();
				SetStatusText(GetSearchStatusString(mSearchHighlightSegments.size(), true), 1000);
			}
			SetCurrentSearchHighlight(txt_range);
		}
		else
			txt_range->select();
		mSearchTxtRange = txt_range;
		//mSearchTxtRange.Release();
		//txt_range->duplicate(&mSearchTxtRange);
		return true;
	}
	else if(mSearchTxtRange)
	{
		mSearchTxtRange.Release();
		return FindText(search, search_flags, backward);
	} 
	else
	{
		if(is_highlight_enabled)
			ClearSearchHighlight();
		MessageBox(mParentWin, "Cannot find the string.", "HTMLView", MB_OK|MB_ICONEXCLAMATION);
		return true;
	}
}
void CBrowserHost::HideShowImagesInHTMLDocument(IHTMLDocument2* lpHtmlDocument, bool remove)
{
	HRESULT hr;

	CComPtr<IHTMLElementCollection> html_elem_coll;
	lpHtmlDocument->get_all(&html_elem_coll);
	if(!html_elem_coll)
		return;
	long elements_len;
	html_elem_coll->get_length(&elements_len);
	VARIANT var;
	var.vt=VT_I4;
	for(var.intVal=0;var.intVal<elements_len;++var.intVal)
	{
		CComPtr<IDispatch> html_elem_disp;
		html_elem_coll->item(var,var,&html_elem_disp);
		if(html_elem_disp)
		{
			CComQIPtr<IHTMLDOMNode> html_dom_node(html_elem_disp);
			if(!html_dom_node)
				continue;
			CComBSTR dom_name;
			html_dom_node->get_nodeName(&dom_name);
			if(dom_name)
			{
				if(dom_name==L"imagedata")
				{
					CComBSTR parent_dom_name;
					CComPtr<IHTMLDOMNode> html_parent_node;
					html_dom_node->get_parentNode(&html_parent_node);
					hr = html_parent_node->get_nodeName(&parent_dom_name);
					if(parent_dom_name==L"shape")
						html_dom_node = html_parent_node;
					else
						html_dom_node.Release();
				}
				else if(dom_name!=L"IMG")
					html_dom_node.Release();
				if(html_dom_node)
				{
					if(remove)
					{
						html_dom_node->removeNode(VARIANT_TRUE, NULL);
						--var.intVal;
					}
					else
					{
						CComQIPtr<IHTMLElement> html_elem(html_dom_node);
						CComPtr<IHTMLStyle> html_style;
						html_elem->get_style(&html_style);
						//html_style->get_visibility(&xx);
						if(mImagesHidden)
							html_style->put_visibility(CComBSTR(L"inherit"));
						else
							html_style->put_visibility(CComBSTR(L"hidden"));
						//html_style->get_visibility(&xx);
					}
				}
			}
		}
	}

	CComPtr<IHTMLFramesCollection2> frames_coll;
	lpHtmlDocument->get_frames(&frames_coll);
	if(!frames_coll)
		return;
	long frames_len;
	frames_coll->get_length(&frames_len);
	if(!frames_len)
		return;
	VARIANT res_var;
	for(var.intVal=0;var.intVal<frames_len;++var.intVal)
		if(frames_coll->item(&var, &res_var) == S_OK)
		{
			CComQIPtr<IHTMLWindow2> html_window(res_var.pdispVal);
			if(!html_window)
				continue;
			CComPtr<IHTMLDocument2> html_doc;
			html_window->get_document(&html_doc);
			if(html_doc)
				HideShowImagesInHTMLDocument(html_doc, remove);
		}
}
void CBrowserHost::HideShowImages(bool remove)
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return;
	HideShowImagesInHTMLDocument(html_doc2, remove);
	mImagesHidden = !mImagesHidden;
}
void CBrowserHost::ProcessHotkey(UINT Msg, DWORD Key, DWORD Info)
{
	char str_action[80];
	CAtlString full_name = GetFullKeyName(Key);
	if((full_name=="F3" || full_name=="Shift+F3") && IsSearchHighlightEnabled())
		mWebBrowser->ExecWB(OLECMDID_CLEARSELECTION, OLECMDEXECOPT_DONTPROMPTUSER, NULL, NULL);
	if((Msg==WM_KEYDOWN||Msg==WM_SYSKEYDOWN)&&GetPrivateProfileString("Hotkeys", (const char*)full_name, "", str_action, sizeof(str_action), options.IniFileName))
	{
		if(!strcmpi(str_action,"Back"))
			mWebBrowser->GoBack();
		else if(!strcmpi(str_action,"Forward"))
			mWebBrowser->GoForward();
		else if(!strcmpi(str_action,"Stop"))
			mWebBrowser->Stop();
		else if(!strcmpi(str_action,"Refresh"))
			mWebBrowser->Refresh();
		else if(!strcmpi(str_action,"SavePosition"))
			SavePosition();
		else if(!strcmpi(str_action,"LoadPosition"))
			LoadPosition();
		else if(!strcmpi(str_action,"RemoveImages"))
			HideShowImages(true);
		else if(!strcmpi(str_action,"HideShowImages"))
			HideShowImages(false);
		else if(!strnicmp(str_action, "Cmd", 3))
		{
			static const int zoom_factors_cnt = 14;
			static const int zoom_factors[zoom_factors_cnt] = {15, 25, 33, 50, 75, 100, 125, 150, 200, 300, 400, 500, 700, 1000};
			CComQIPtr<IOleCommandTarget, &IID_IOleCommandTarget> pCmd = mWebBrowser;
			VARIANTARG v;
			HRESULT res;
			if(pCmd)
			{
				if(!stricmp(str_action, "CmdFind"))
					res = pCmd->Exec(NULL, OLECMDID_FIND, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdPrint"))
					res = pCmd->Exec(NULL, OLECMDID_PRINT, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdPrintPreview"))
					res = pCmd->Exec(NULL, OLECMDID_PRINTPREVIEW, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdPageSetup"))
					res = pCmd->Exec(NULL, OLECMDID_PAGESETUP, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdProperties"))
					res = pCmd->Exec(NULL, OLECMDID_PROPERTIES, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdSaveAs"))
					res = pCmd->Exec(NULL, OLECMDID_SAVEAS, 0, NULL, &v);
				else if(!stricmp(str_action, "CmdZoomIn"))
				{
					res = pCmd->Exec(NULL, OLECMDID_OPTICAL_ZOOM, 0, NULL, &v);
					VARIANTARG vi;
					vi.vt = VT_I4;
					//vi.intVal = v.intVal*1.5;
					vi.intVal = v.intVal;
					for(int i=0;i<zoom_factors_cnt;++i)
						if(zoom_factors[i]>v.intVal)
						{
							vi.intVal = zoom_factors[i];
							break;
						}
					res = pCmd->Exec(NULL, OLECMDID_OPTICAL_ZOOM, 0, &vi, &v);
				}
				else if(!stricmp(str_action, "CmdZoomOut"))
				{
					res = pCmd->Exec(NULL, OLECMDID_OPTICAL_ZOOM, 0, NULL, &v);
					VARIANTARG vi;
					vi.vt = VT_I4;
					//vi.intVal = v.intVal/1.5;
					vi.intVal = v.intVal;
					for(int i=0;i<zoom_factors_cnt;++i)
						if(zoom_factors[i]<v.intVal)
							vi.intVal = zoom_factors[i];
						else
							break;
					res = pCmd->Exec(NULL, OLECMDID_OPTICAL_ZOOM, 0, &vi, &v);
				}
				else if(!stricmp(str_action, "CmdZoomDef"))
				{
					VARIANTARG vi;
					vi.vt = VT_I4;
					vi.intVal = 100;
					res = pCmd->Exec(NULL, OLECMDID_OPTICAL_ZOOM, 0, &vi, &v);
				}
			}
			/*
			if(pCmd)
			{
				const GUID GrID = {0xED016940, 0xBD5B, 0x11CF, {0xBA, 0x4E, 0x00, 0xC0, 0x4F, 0xD7, 0x08, 0x16}};
				pCmd->Exec(&GrID, 1, NULL, NULL,NULL);
			}
			*/
		}
	}
}
void CBrowserHost::UpdateTitle()
{
	char buf[1024];
	if(mFocusType!=fctQuickView && GetPrivateProfileString("options", "ListerTitle", "",buf , sizeof(buf), options.IniFileName))
	{
		CAtlStringW atlstr_text = buf;
		BSTR title_wide=NULL;

		CComPtr<IDispatch> html_disp;
		mWebBrowser->get_Document(&html_disp);
		if(html_disp)
		{
			CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
			if(html_doc2)
				html_doc2->get_title(&title_wide);
		}
		if(!title_wide)
			mWebBrowser->get_LocationName(&title_wide);
		else if(!wcslen(title_wide))
		{
			SysFreeString(title_wide);
			mWebBrowser->get_LocationName(&title_wide);
		}
		if(title_wide)
		{
			atlstr_text.Replace(L"%TITLE",title_wide);
			SysFreeString(title_wide);
		}

		BSTR loc_wide=NULL;
		mWebBrowser->get_LocationURL(&loc_wide);
		if(loc_wide)
		{
			wchar_t name[262],ext[262],dir[262],drive[5];
			_wsplitpath(loc_wide,drive,dir,name,ext);
			SysFreeString(loc_wide);
			atlstr_text.Replace(L"%DRIVE",drive);
			atlstr_text.Replace(L"%DIR",dir);
			atlstr_text.Replace(L"%NAME",name);
			atlstr_text.Replace(L"%EXT",ext);
		}
		
		SetWindowTextW(GetParent(mParentWin),atlstr_text);
	}
}
bool CBrowserHost::FormFocused()
{
	CComPtr<IDispatch> html_disp;
	mWebBrowser->get_Document(&html_disp);
	if(!html_disp)
		return false;
	CComQIPtr<IHTMLDocument2> html_doc2(html_disp);
	if(!html_doc2)
		return false;
	CComPtr<IHTMLElement> html_elem;
	html_doc2->get_activeElement(&html_elem);
	if(!html_elem)
		return false;	
	//CComQIPtr<IHTMLDOMNode> html_dom_node(html_elem);
	//if(!html_dom_node)
	//	return false;
	BSTR dom_name;
	html_elem->get_tagName(&dom_name);
	if(!dom_name)
		return false;

	bool result = false;
	if(!wcsicmp(dom_name,L"Textarea"))
		result = true;
	else if(!wcsicmp(dom_name,L"Input"))
	{
		VARIANT input_type;
		html_elem->getAttribute(L"Type", 0, &input_type);
		if(input_type.bstrVal)
		{
			if(!wcsicmp(input_type.bstrVal, L"text"))
				result = true;
			SysFreeString(input_type.bstrVal);
		}
		//result = true;
	}
	SysFreeString(dom_name);
	return result;
}

//---------------------------=|  IUnknown  |=---------------------------
STDMETHODIMP CBrowserHost::QueryInterface(REFIID riid, void ** ppvObject)
{
    if(ppvObject == NULL) 
		return E_INVALIDARG;
    else if(riid == IID_IUnknown)
        *ppvObject = (IUnknown*)this;
    else if(riid == IID_IDispatch)
		*ppvObject = (IDispatch*)this;
    else if(riid == IID_IOleClientSite)
		*ppvObject = (IOleClientSite*)this;
    else if(riid == IID_IOleInPlaceSite)
		*ppvObject = (IOleInPlaceSite*)this;
    //else if(riid == IID_IDocHostUIHandler)
	//	*ppvObject = (IDocHostUIHandler*)this;
    //else if(riid == IID_IDocHostUIHandler2)
	//	*ppvObject = (IDocHostUIHandler2*)this;
	else
	{
		*ppvObject = NULL;
		return E_NOINTERFACE;
	}
    AddRef(); 
	return S_OK;
}

ULONG STDMETHODCALLTYPE CBrowserHost::AddRef()
{ 
	return ++mRefCount; 
}

ULONG STDMETHODCALLTYPE CBrowserHost::Release()
{
    if(!--mRefCount)
        delete this;
    return mRefCount;
}

//---------------------------=|  IOleClientSite  |=---------------------------
STDMETHODIMP CBrowserHost::GetContainer(LPOLECONTAINER FAR* ppContainer) { return E_NOTIMPL; }
STDMETHODIMP CBrowserHost::SaveObject() { return E_NOTIMPL; }
STDMETHODIMP CBrowserHost::GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk) { return E_NOTIMPL; }
STDMETHODIMP CBrowserHost::ShowObject() { return S_OK; }
STDMETHODIMP CBrowserHost::OnShowWindow(BOOL fShow) { return E_NOTIMPL; }
STDMETHODIMP CBrowserHost::RequestNewObjectLayout() { return E_NOTIMPL; }
//---------------------------=|  IOleWindow  |=---------------------------
HRESULT STDMETHODCALLTYPE CBrowserHost::GetWindow(HWND * phwnd)
{ 
	*phwnd = mParentWin; 
	return S_OK; 
}
HRESULT STDMETHODCALLTYPE CBrowserHost::ContextSensitiveHelp(BOOL fEnterMode) { return E_NOTIMPL; }
//---------------------------=|  IOleInPlaceSite  |=---------------------------
HRESULT STDMETHODCALLTYPE CBrowserHost::CanInPlaceActivate(void) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnInPlaceActivate(void) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnUIActivate(void) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::GetWindowContext(IOleInPlaceFrame **ppFrame,
                                           IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect,
                                           LPRECT lprcClipRect,
                                           LPOLEINPLACEFRAMEINFO lpFrameInfo)
{ 
	GetRect(lprcPosRect);
	GetRect(lprcClipRect);
	return S_OK; 
}
HRESULT STDMETHODCALLTYPE CBrowserHost::Scroll(SIZE scrollExtant) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnUIDeactivate(BOOL fUndoable) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnInPlaceDeactivate(void) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::DiscardUndoState(void) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::DeactivateAndUndo(void) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnPosRectChange(LPCRECT lprcPosRect) { return E_NOTIMPL; }
//---------------------------=|  IOleControlSite  |=---------------------------
HRESULT STDMETHODCALLTYPE CBrowserHost::OnControlInfoChanged(void) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::LockInPlaceActive(BOOL fLock) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::GetExtendedControl(IDispatch **ppDisp) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::TranslateAccelerator(MSG *pMsg, DWORD grfModifiers) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::OnFocus(BOOL fGotFocus) { return S_OK; }
HRESULT STDMETHODCALLTYPE CBrowserHost::ShowPropertyFrame(void) { return S_OK; }
//---------------------------=|  IDispatch  |=---------------------------
HRESULT STDMETHODCALLTYPE CBrowserHost::GetTypeInfoCount(unsigned int FAR* pctinfo) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::GetTypeInfo(unsigned int iTInfo, LCID  lcid,
                                      ITypeInfo FAR* FAR*  ppTInfo) { return E_NOTIMPL; }
HRESULT STDMETHODCALLTYPE CBrowserHost::GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames,
                                        unsigned int cNames, LCID lcid, DISPID FAR* rgDispId) { return E_NOTIMPL; }


struct SNewWindowThread
{
	CComBSTR path;
	bool def_browser;
};

DWORD WINAPI NewWindowThreadFunc(LPVOID param)
{
	SNewWindowThread* thread_struct = (SNewWindowThread*)param;
	if(thread_struct->def_browser)
		ShellExecuteW(NULL, L"open", thread_struct->path, NULL, NULL, SW_SHOW);
	else
	{
	    PROCESS_INFORMATION pi;
		STARTUPINFOW si;
		ZeroMemory(&si, sizeof(si));
		ZeroMemory(&pi, sizeof(pi));
		si.cb = sizeof(si);
		si.dwFlags = 0;
		CreateProcessW(NULL, thread_struct->path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	}
	delete thread_struct;
	return 0;
}

void CBrowserHost::LoadWebBrowserFromStreamWrapper(const char* html)
{
	auto wstrTo(boost::nowide::widen(html));

	IDispatch* pHtmlDoc;
	mWebBrowser->get_Document(&pHtmlDoc);
	if (!pHtmlDoc)
		return;
	
	CComPtr<IHTMLDocument2> doc2;
	doc2.Attach((IHTMLDocument2*)pHtmlDoc);
	if (!doc2)
		return;
	
	// Creates a new one-dimensional array
	SAFEARRAY* psaStrings = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	if (!psaStrings)
		return;

	BSTR bstrt = SysAllocString(wstrTo.c_str());
	VARIANT* param;
	if (SUCCEEDED(SafeArrayAccessData(psaStrings, (LPVOID*)&param)))
	{
		param->vt = VT_BSTR;
		param->bstrVal = bstrt;
		if (SUCCEEDED(SafeArrayUnaccessData(psaStrings)))
		{
			doc2->write(psaStrings);
			doc2->close();
		}
	}

	if (psaStrings)
		SafeArrayDestroy(psaStrings);
}

HRESULT STDMETHODCALLTYPE CBrowserHost::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid,
                                                    WORD wFlags, DISPPARAMS FAR* pDispParams,
                                                    VARIANT FAR* pVarResult,
                                                    EXCEPINFO FAR* pExcepInfo,
                                                    unsigned int FAR* puArgErr) 
{
	switch (dispIdMember)
	{
		case DISPID_BEFORENAVIGATE:
		case DISPID_DOWNLOADBEGIN:
			if(options.highlight_all_matches)
				ClearSearchHighlight();
			mSearchTxtRange.Release();
			break; 
		case DISPID_DOCUMENTCOMPLETE:
		case DISPID_NAVIGATECOMPLETE:
		case DISPID_NAVIGATECOMPLETE2:
			//mImagesHidden = false;
		case 0U-726:
			//ShowWindow(mParentWin,SW_SHOW);
			if ( mFocusType == fctLister ) 
				Focus();
			//else if(mFocusType == fctQuickView/* && !::GetFocus()*/)
			//	::SetFocus(mFocusWin);
			break;

		case DISPID_NEWWINDOW:
		case DISPID_NEWWINDOW2:
			READYSTATE m_ReadyState;
			mWebBrowser->get_ReadyState(&m_ReadyState);
			if(m_ReadyState!=READYSTATE_COMPLETE && m_ReadyState!=READYSTATE_INTERACTIVE && !(options.flags&OPT_POPUPS))
				*pDispParams->rgvarg[0].pboolVal=VARIANT_TRUE;
            else if(dispIdMember==DISPID_NEWWINDOW)
			{
				char temp[MAX_PATH+3];
				if(!wcsnicmp(pDispParams->rgvarg[5].bstrVal, L"mk:@msitstore:", 14))
					GetPrivateProfileString("options", "NewWindowCHM", "", (char*)temp, sizeof(temp), options.IniFileName);
				else
					GetPrivateProfileString("options", "NewWindowHTML", "", (char*)temp, sizeof(temp), options.IniFileName);
				if(temp[0]=='\0')
					return S_OK;

				*pDispParams->rgvarg[0].pboolVal=VARIANT_TRUE;
				SNewWindowThread* thread_struct = new SNewWindowThread;
				if(!stricmp(temp, "default"))
				{
					thread_struct->path = pDispParams->rgvarg[5].bstrVal;
					thread_struct->def_browser = true;
				}
				else
				{
					thread_struct->path = temp;
					thread_struct->path += L" \"";
					thread_struct->path += pDispParams->rgvarg[5].bstrVal;
					thread_struct->path += L"\"";
					thread_struct->def_browser = false;
				}
				CreateThread(NULL, 0, NewWindowThreadFunc, (void*)(thread_struct), 0, NULL);
			}
			break;
		case DISPID_STATUSTEXTCHANGE:
			if(fStatusBarUnlockTime==0 || GetTickCount()>fStatusBarUnlockTime)
			{
				SetWindowTextW((HWND)GetProp(mParentWin, PROP_STATUS), pDispParams->rgvarg[0].bstrVal);
				fStatusBarUnlockTime = 0;
			}
			break;
		case DISPID_TITLECHANGE:
			if(options.flags&OPT_TITLE)
				UpdateTitle();
			break;
		case DISPID_COMMANDSTATECHANGE:
			if( options.toolbar && (pDispParams->rgvarg[1].intVal==1 || pDispParams->rgvarg[1].intVal==2))
			{
				HWND toolbar = (HWND)GetProp(mParentWin, PROP_TOOLBAR);
				if(toolbar)
					PostMessage(toolbar, TB_SETSTATE, (pDispParams->rgvarg[1].intVal==1)?TBB_FORWARD:TBB_BACK, (pDispParams->rgvarg[0].boolVal)?TBSTATE_ENABLED:TBSTATE_INDETERMINATE);
			}
			break;
		case DISPID_AMBIENT_DLCONTROL:
			pVarResult->vt = VT_I4;
			pVarResult->lVal =  options.dlcontrol;
			break;
		default:
			return DISP_E_MEMBERNOTFOUND;
	}
	return S_OK;
}
