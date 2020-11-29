
// main include
#include "IOShell.h"
#include <stdio.h>

// constructor
SHELL_IO::SHELL_IO(): m_cRef(0)
{
    HRESULT InitStatus = OleInitialize(NULL);
    RegisterDragDrop(GetActiveWindow(), this);
    //printf("Res = %d\n", InitStatus);
}

// destructor
SHELL_IO::~SHELL_IO()
{
    OleUninitialize();
}

// for IUnknown
HRESULT SHELL_IO::QueryInterface(REFIID riid, LPVOID * ppvObj)
{
    // Always set out parameter to NULL, validating it first.
    if (!ppvObj)
        return E_INVALIDARG;
    *ppvObj = NULL;
    if(IsEqualIID(riid, IID_IDropTarget) || IsEqualIID(riid, IID_IUnknown)) //if (riid == IID_IUnknown)// || riid == IID_IMAPIProp || riid == IID_IMAPIStatus)
    {
        // Increment the reference count and return the pointer.
        *ppvObj = (LPVOID) this;
        AddRef();
        return NOERROR;
    }
    return E_NOINTERFACE;
}

ULONG SHELL_IO::AddRef()
{
    InterlockedIncrement(&m_cRef);
    return m_cRef;
}

ULONG SHELL_IO::Release()
{
    // Decrement the object's internal counter.
    ULONG ulRefCount = InterlockedDecrement(&m_cRef);
    if (0 == m_cRef)
    {
        delete this;
    }
    return ulRefCount;
}

// allow all
HRESULT STDMETHODCALLTYPE SHELL_IO::DragEnter(IDataObject __RPC_FAR *pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect)
{
	return S_OK;
}

// allow all
HRESULT STDMETHODCALLTYPE SHELL_IO::DragOver(DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect)
{
	return S_OK;
}

// allow
HRESULT STDMETHODCALLTYPE SHELL_IO::DragLeave()
{
	return S_OK;
}

// called when drop was released
HRESULT STDMETHODCALLTYPE SHELL_IO::Drop(IDataObject __RPC_FAR *pDataObj, DWORD grfKeyState, POINTL pt, DWORD __RPC_FAR *pdwEffect)
{
    // temp vars
	LPENUMFORMATETC pefEtc = 0;
	STGMEDIUM		stgM;
	FORMATETC		fEtc;
	ULONG			ulFetched	= 0L;
	UINT			nFileCount  = 0;
	HRESULT			hr = S_OK;

    // get data format
	hr = pDataObj->EnumFormatEtc(DATADIR_GET, &pefEtc);
	if(SUCCEEDED(hr))
	{
		hr = pefEtc->Reset();
		if(SUCCEEDED(hr))
		{
			//	disable 'conditional expression is not constant'
			while(TRUE)
			{
				hr = pefEtc->Next(1, &fEtc, &ulFetched);
				if(FAILED(hr) || (ulFetched <= 0))
					break;

				fEtc.cfFormat	= CF_HDROP;
				fEtc.dwAspect	= DVASPECT_CONTENT;
				fEtc.lindex		= -1;
				fEtc.ptd		= NULL;
				fEtc.tymed		= TYMED_HGLOBAL;

				hr = pDataObj->GetData(&fEtc, &stgM);
				if(SUCCEEDED(hr))
				{
					if(stgM.tymed == TYMED_HGLOBAL)
					{
                        // dump to file
						nFileCount = DragQueryFile((HDROP)stgM.hGlobal, (UINT)INVALID_HANDLE_VALUE, NULL, 0);
						if(nFileCount >= 1)
						{
                            // load
							HWND hwndTV = 0;
							POINT _pt;
							_pt.x = pt.x;
							_pt.y = pt.y;

							// send to hwnd
							hwndTV	= WindowFromPoint(_pt);
							SendMessage(hwndTV, WM_DROPFILES, (WPARAM)stgM.hGlobal, (LPARAM)0);
                            break;
						}
					}
				}
			}
		}
	}

	if(pefEtc)
		pefEtc->Release();

	return hr;
}

