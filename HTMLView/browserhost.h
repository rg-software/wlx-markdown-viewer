
#define _CONVERSION_DONT_USE_THREAD_LOCALE
#include <atlbase.h>
#include <atlwin.h>
#include <atlcom.h>
#include <mshtml.h>
#include <vector>

enum FocusCatchType {fctNoCatch, fctQuickView, fctLister};

//=====================================================================//
//								CBrowserHost						   //
//=====================================================================//

class CBrowserHost : 
	public IUnknown,
	public IDispatch,
	public IOleControlSite,
	public IOleClientSite, 
	public IOleInPlaceSite
{
public:	
	//HWND mFocusWin;
	FocusCatchType mFocusType;
	HWND mParentWin;
	CComBSTR mLastSearchString;
	int mLastSearchFlags;
	//CComBSTR mLastBookmark;
	CComPtr<IHTMLTxtRange> mSearchTxtRange;
	std::vector< CComPtr<IHighlightSegment> > mSearchHighlightSegments;
	CComPtr<IHighlightSegment> mCurrentSearchHighlightSegment;
	bool mImagesHidden;
	CComPtr<IWebBrowser2> mWebBrowser;
	DWORD mEventsCookie;
	int mRefCount;
	int fSearchHighlightMode;
	DWORD fStatusBarUnlockTime;
public:
	CBrowserHost();
	~CBrowserHost();
	bool CreateBrowser(HWND hParent);
	void Quit();
	void HideShowImagesInHTMLDocument(IHTMLDocument2* lpHtmlDocument, bool remove);
	void HideShowImages(bool remove);
	void SavePosition();
	void LoadPosition();
	void Focus();
	void Resize();
	bool FindText(CComBSTR search, long search_flags, bool backward);
	bool IsSearchHighlightEnabled();
	void ClearSearchHighlight();
	void UpdateSearchHighlight();
	void SetCurrentSearchHighlight(IHTMLTxtRange* txt_range);
	bool HighlightStrings(CComBSTR search, long search_flags);
	void ProcessHotkey(UINT Msg, DWORD Key, DWORD lParam);
	void GetRect(LPRECT rect);
	void UpdateTitle();
	bool FormFocused();
	void SetStatusText(const wchar_t* str, DWORD delay=0);
public:  
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID iid, void ** ppvObject);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();

    // IOleClientSite
    STDMETHODIMP GetContainer(LPOLECONTAINER FAR* ppContainer); 
    STDMETHODIMP SaveObject(); 
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker ** ppmk); 
    STDMETHODIMP ShowObject(); 
    STDMETHODIMP OnShowWindow(BOOL fShow); 
    STDMETHODIMP RequestNewObjectLayout(); 
    // IOleWindow
    HRESULT STDMETHODCALLTYPE GetWindow(HWND * phwnd);
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode); 
    // IOleInPlaceSite
    HRESULT STDMETHODCALLTYPE CanInPlaceActivate(void); 
    HRESULT STDMETHODCALLTYPE OnInPlaceActivate(void); 
    HRESULT STDMETHODCALLTYPE OnUIActivate(void); 
    HRESULT STDMETHODCALLTYPE GetWindowContext(IOleInPlaceFrame **ppFrame,
                                               IOleInPlaceUIWindow **ppDoc, LPRECT lprcPosRect,
                                               LPRECT lprcClipRect,
                                               LPOLEINPLACEFRAMEINFO lpFrameInfo);
    HRESULT STDMETHODCALLTYPE Scroll(SIZE scrollExtant); 
    HRESULT STDMETHODCALLTYPE OnUIDeactivate(BOOL fUndoable); 
    HRESULT STDMETHODCALLTYPE OnInPlaceDeactivate(void); 
    HRESULT STDMETHODCALLTYPE DiscardUndoState(void); 
    HRESULT STDMETHODCALLTYPE DeactivateAndUndo(void); 
    HRESULT STDMETHODCALLTYPE OnPosRectChange(LPCRECT lprcPosRect); 
	// IOleControlSite
    HRESULT STDMETHODCALLTYPE OnControlInfoChanged(void); 
    HRESULT STDMETHODCALLTYPE LockInPlaceActive(BOOL fLock); 
    HRESULT STDMETHODCALLTYPE GetExtendedControl(IDispatch **ppDisp); 
    HRESULT STDMETHODCALLTYPE TransformCoords(POINTL *pPtlHimetric, POINTF *pPtfContainer, DWORD dwFlags); 
    HRESULT STDMETHODCALLTYPE TranslateAccelerator(MSG *pMsg, DWORD grfModifiers); 
    HRESULT STDMETHODCALLTYPE OnFocus(BOOL fGotFocus); 
    HRESULT STDMETHODCALLTYPE ShowPropertyFrame(void); 
	
	
    // IDispatch
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int FAR* pctinfo); 
    HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int iTInfo, LCID  lcid,
                                          ITypeInfo FAR* FAR*  ppTInfo); 
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(REFIID riid, OLECHAR FAR* FAR* rgszNames,
                                            unsigned int cNames, LCID lcid, DISPID FAR* rgDispId); 
    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags,
                                     DISPPARAMS FAR* pDispParams, VARIANT FAR* parResult,
                                     EXCEPINFO FAR* pExcepInfo, unsigned int FAR* puArgErr);
};
