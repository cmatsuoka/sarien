// BROWSE.CPP - Simple file browser for Windows CE-based systems.
// Programmed by Vasyl Tsvirkunov.
//
#include <windows.h>
#include <commctrl.h>
// In case of HPC Pro you may have to copy this file from PocketPC devkit.
#include <aygshell.h>

#include "browse.h"

#define MENU_HEIGHT 26

// aygshell.dll is not present in some cases (HPC Pro). So, here's standard dynamic loading trick.

typedef BOOL tSHInitDialog(PSHINITDLGINFO);
typedef BOOL tSHInitExtraControls(void);
typedef WINSHELLAPI BOOL tSHHandleWMActivate(HWND hwnd, WPARAM wParam, LPARAM lParam, SHACTIVATEINFO* psai, DWORD dwFlags);
typedef WINSHELLAPI BOOL tSHHandleWMSettingChange(HWND hwnd, WPARAM wParam, LPARAM lParam, SHACTIVATEINFO* psai);

static tSHInitDialog* pSHInitDialog = NULL;
static tSHInitExtraControls* pSHInitExtraControls = NULL;
static tSHHandleWMActivate* pSHHandleWMActivate = NULL;
static tSHHandleWMSettingChange* pSHHandleWMSettingChange = NULL;

//

class cFileBrowserView
{
public:
	cFileBrowserView(BROWSEFILES* pbf);
	virtual ~cFileBrowserView();

protected:
	void InitView(HWND hwndDlg);
	void UpdateView(HWND hwndDlg, BOOL bChangePath);
	void UpdateLayout(HWND hwndDlg);
	void VerifyPath();

	void BrowserUp(HWND hwndDlg, int depth);
	void BrowserDown(HWND hwndDlg, LPTSTR item);

	void BrowseResult(HWND hwndDlg);

public:
	static BOOL CALLBACK BrowseDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	TCHAR currentFolder[MAX_PATH+1];
	SHACTIVATEINFO sai;
	
	int mnFolderImage;
	BROWSEFILES* mpbf;

	bool bInternalSelect;
};

#define CONTROL_GAP 10
#define LINE_HEIGHT 20


cFileBrowserView::cFileBrowserView(BROWSEFILES* pbf) : mpbf(pbf), bInternalSelect(false)
{
	if(mpbf->szStartDir == NULL || mpbf->szStartDir[0] == 0)
		currentFolder[0] = 0;
	else
		wcsncpy(currentFolder, mpbf->szStartDir, MAX_PATH);
}

cFileBrowserView::~cFileBrowserView()
{
}

void cFileBrowserView::InitView(HWND hwndDlg)
{
	SetWindowText(hwndDlg, mpbf->szCaption);
	HWND hwndList = GetDlgItem(hwndDlg, 1000);
	HIMAGELIST hil = ImageList_LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(mpbf->nImageId), 16, 0, CLR_DEFAULT, IMAGE_BITMAP, 0);
	ListView_SetImageList(hwndList, hil, LVSIL_SMALL);
	if(mpbf->dwFlags & BF_SELECTEXISTING)
	{
		HWND hwndEdit = GetDlgItem(hwndDlg, 1002);
		SendMessage(hwndEdit, EM_SETREADONLY, 1, 0);
	}
	for(int i=0;; i++)
	{
		if(mpbf->pExtImages[i].pExt == NULL)
		{
			mnFolderImage = mpbf->pExtImages[i].nImageIndex;
			break;
		}
	}
}

void cFileBrowserView::UpdateLayout(HWND hwndDlg)
{
	HWND hwndList = GetDlgItem(hwndDlg, 1000);
	HWND hwndCombo = GetDlgItem(hwndDlg, 1001);
	HWND hwndEdit = GetDlgItem(hwndDlg, 1002);
	HWND hwndOK = GetDlgItem(hwndDlg, IDOK);
	HWND hwndCancel = GetDlgItem(hwndDlg, IDCANCEL);

	RECT rc;
	GetClientRect(hwndDlg, &rc);
	int width = rc.right - rc.left;
	int height = rc.bottom - rc.top;

	int itemWidth = width-2*CONTROL_GAP;
	int buttonWidth = (width-3*CONTROL_GAP)/2;
	MoveWindow(hwndCombo, CONTROL_GAP, CONTROL_GAP, itemWidth, LINE_HEIGHT, TRUE);
	MoveWindow(hwndList, CONTROL_GAP, 2*CONTROL_GAP+LINE_HEIGHT, itemWidth, height-5*CONTROL_GAP-3*LINE_HEIGHT, TRUE);
	MoveWindow(hwndEdit, CONTROL_GAP, height-2*CONTROL_GAP-2*LINE_HEIGHT, itemWidth, LINE_HEIGHT, TRUE);
	MoveWindow(hwndOK, CONTROL_GAP, height-CONTROL_GAP-LINE_HEIGHT, buttonWidth, LINE_HEIGHT, TRUE);
	MoveWindow(hwndCancel, width-CONTROL_GAP-buttonWidth, height-CONTROL_GAP-LINE_HEIGHT, buttonWidth, LINE_HEIGHT, TRUE);
}

void cFileBrowserView::VerifyPath()
{
// Relative path is prepended with cwd. Path must be terminated with backslash
	if(currentFolder[0] == '.')
		currentFolder[0] = 0;

	if(currentFolder[0] != '\\')
	{
		TCHAR cwd[MAX_PATH];
		if(GetModuleFileName(GetModuleHandle(NULL), cwd, MAX_PATH))
		{
			TCHAR* lastdir = wcsrchr(cwd, '\\');
			if(lastdir)
				*(lastdir+1) = 0;
			wcscat(cwd, currentFolder);
			wcscpy(currentFolder, cwd);
		}
	}

	if(currentFolder[0] != '\\') // There was some (generally bad) error
	{
		currentFolder[0] = '\\';
		currentFolder[1] = 0;
	}

// At this point currentFolder contains absolute folder path terminated with backslash.
// Verify path existence. If it does not exist, backstep.
	TCHAR verifyPath[MAX_PATH];
	wcscpy(verifyPath, currentFolder);

	while(1)
	{
		TCHAR* lastdir = wcsrchr(verifyPath, '\\');
		if(lastdir)
			*lastdir = 0;
		if(verifyPath[0] == 0) // reached root?
			break;
		DWORD attribs = GetFileAttributes(verifyPath);
		if(attribs & FILE_ATTRIBUTE_DIRECTORY) // valid directory?
			break;
	}

// Re-terminate currentPath after backslash
	currentFolder[wcslen(verifyPath)+1] = 0;
}

void cFileBrowserView::UpdateView(HWND hwndDlg, BOOL bChangePath)
{
	HWND hwndList = GetDlgItem(hwndDlg, 1000);
	HWND hwndCombo = GetDlgItem(hwndDlg, 1001);
	HWND hwndEdit = GetDlgItem(hwndDlg, 1002);
	HWND hwndOK = GetDlgItem(hwndDlg, IDOK);

	if(bChangePath)
	{
		VerifyPath();

	// Fill combo box with items
		int depth = 0;
		TCHAR subfolder[MAX_PATH+3*16] = TEXT("My Device");
		SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
		SendMessage(hwndCombo, CB_INSERTSTRING, depth, (LPARAM)subfolder);

		TCHAR* ptr = currentFolder;
		TCHAR* nextptr = currentFolder;
		while(1)
		{
			ptr = nextptr;
			ptr ++;
			if(*ptr == 0)
				break;
			nextptr = wcschr(ptr, '\\');
			depth++;
			subfolder[0] = 0;
			for(int i=0; i<depth; i++)
				wcscat(subfolder, TEXT("   "));
			wcsncat(subfolder, ptr, nextptr-ptr);
			SendMessage(hwndCombo, CB_INSERTSTRING, depth, (LPARAM)subfolder);
		}
		SendMessage(hwndCombo, CB_SETCURSEL, depth, 0);

	// Fill list box with items
		ListView_DeleteAllItems(hwndList);

		int index = 0;

	// For non-root folders -- add UP item
		if(wcslen(currentFolder) > 1)
		{
			LVITEM lvi;
			lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
			lvi.iItem = index++;
			lvi.iSubItem = 0;
			lvi.state = 0;
			lvi.stateMask = 0;
			lvi.pszText = TEXT("..");
			lvi.cchTextMax = 0;
			lvi.iImage = mnFolderImage;
			lvi.lParam = -1;
			lvi.iIndent = 0;
			ListView_InsertItem(hwndList, &lvi);
		}

		TCHAR mask[MAX_PATH];
		wcscpy(mask, currentFolder);
		wcscat(mask, TEXT("*.*"));

		WIN32_FIND_DATA wfd;
		HANDLE hfind;
	// Directories first
		hfind = FindFirstFile(mask, &wfd);
		if(hfind != INVALID_HANDLE_VALUE)
		{
			while(1)
			{
				if(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					LVITEM lvi;
					lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
					lvi.iItem = index++;
					lvi.iSubItem = 0;
					lvi.state = 0;
					lvi.stateMask = 0;
					lvi.pszText = wfd.cFileName;
					lvi.cchTextMax = 0;
					lvi.iImage = mnFolderImage;
					lvi.lParam = -1;
					lvi.iIndent = 0;
					ListView_InsertItem(hwndList, &lvi);
				}

				if(!FindNextFile(hfind, &wfd))
					break;
			}
			FindClose(hfind);
		}

	// Then masked files
		for(int ext=0; mpbf->pExtImages[ext].pExt; ext++)
		{
			wcscpy(mask, currentFolder);
			wcscat(mask, TEXT("*."));
			wcscat(mask, mpbf->pExtImages[ext].pExt);
			int iImage = mpbf->pExtImages[ext].nImageIndex;

			hfind = FindFirstFile(mask, &wfd);
			if(hfind != INVALID_HANDLE_VALUE)
			{
				while(1)
				{
					if(!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						LVITEM lvi;
						lvi.mask = LVIF_TEXT|LVIF_IMAGE|LVIF_PARAM;
						lvi.iItem = index++;
						lvi.iSubItem = 0;
						lvi.state = 0;
						lvi.stateMask = 0;
						lvi.pszText = wfd.cFileName;
						lvi.cchTextMax = 0;
						lvi.iImage = iImage;
						lvi.lParam = 0;
						lvi.iIndent = 0;
						ListView_InsertItem(hwndList, &lvi);
					}

					if(!FindNextFile(hfind, &wfd))
						break;
				}
				FindClose(hfind);
			}
		}

	// NEED TO SORT ITEMS HERE!
	}

	EnableWindow(hwndOK, FALSE);	

	TCHAR string[MAX_PATH];
	GetWindowText(hwndEdit, string, MAX_PATH);
	bInternalSelect = true;
	if(string[0])
	{
		LV_FINDINFO lvf;
		lvf.flags = LVFI_STRING;
		lvf.psz = string;
		int item = ListView_FindItem(hwndList, 0, &lvf);
		if(item != -1)
		{
			ListView_SetItemState(hwndList, item, LVIS_SELECTED, LVIS_SELECTED);
			ListView_EnsureVisible(hwndList, item, FALSE);
		}
		EnableWindow(hwndOK, TRUE);	
	}
	else
	{
		ListView_SetItemState(hwndList, 0, LVIS_SELECTED, LVIS_SELECTED);
		ListView_EnsureVisible(hwndList, 0, FALSE);
	}
	bInternalSelect = false;
}

void cFileBrowserView::BrowserUp(HWND hwndDlg, int depth)
{
	TCHAR* ptr = currentFolder;
	for(int i=0; i<depth+1; i++)
	{
		ptr = wcschr(ptr, '\\');
		if(ptr)
			ptr++;
		else
			break;
	}
	if(ptr)
		*ptr = 0;

	SetWindowText(GetDlgItem(hwndDlg, 1002), TEXT(""));
	UpdateView(hwndDlg, TRUE);
}

void cFileBrowserView::BrowserDown(HWND hwndDlg, LPTSTR item)
{
	if(wcslen(currentFolder) <= 1 || wcscmp(TEXT(".."), item) != 0)
	{
		TCHAR fullpath[MAX_PATH];
		wcscpy(fullpath, currentFolder);
		wcscat(fullpath, item);

		DWORD attribs = GetFileAttributes(fullpath);
		if(attribs & FILE_ATTRIBUTE_DIRECTORY)
		{
			wcscpy(currentFolder, fullpath);
			wcscat(currentFolder, TEXT("\\"));

			SetWindowText(GetDlgItem(hwndDlg, 1002), TEXT(""));
			UpdateView(hwndDlg, TRUE);
		}
		else
		{
			SetWindowText(GetDlgItem(hwndDlg, 1002), item);
			UpdateView(hwndDlg, FALSE);
		}
	}
	else
	{
		TCHAR* pclose = wcsrchr(currentFolder, '\\');
		if(pclose && pclose != currentFolder) // sanity check
		{
			*pclose = 0;
			pclose = wcsrchr(currentFolder, '\\');
			pclose++;
			*pclose = 0;

			SetWindowText(GetDlgItem(hwndDlg, 1002), TEXT(""));
			UpdateView(hwndDlg, TRUE);
		}
	}
}

void cFileBrowserView::BrowseResult(HWND hwndDlg)
{
	TCHAR strName[MAX_PATH];
	GetWindowText(GetDlgItem(hwndDlg, 1002), strName, MAX_PATH);

	if(!wcschr(strName, '.') && mpbf->szDefExt)
	{
		wcsncat(strName, TEXT("."), MAX_PATH-wcslen(strName));
		wcsncat(strName, mpbf->szDefExt, MAX_PATH-wcslen(strName));
	}
	if(mpbf->pBuffer && mpbf->nBufSize)
	{
		wcsncpy(mpbf->pBuffer, currentFolder, mpbf->nBufSize);
		wcsncat(mpbf->pBuffer, strName, mpbf->nBufSize - wcslen(mpbf->pBuffer));
	}
	if(mpbf->pBufferShort && mpbf->nBufShortSize)
	{
		TCHAR* pdot = wcsrchr(strName, '.');
		if(pdot)
			*pdot = 0;
		wcsncpy(mpbf->pBufferShort, strName, mpbf->nBufShortSize);
	}
}


BOOL CALLBACK cFileBrowserView::BrowseDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	cFileBrowserView* pView = (cFileBrowserView*)GetWindowLong(hwndDlg, DWL_USER);

	switch(uMsg)
	{
	case WM_INITDIALOG:
		{
			SetWindowLong(hwndDlg, DWL_USER, lParam);
			pView = (cFileBrowserView*)lParam;

			memset(&pView->sai, 0, sizeof(SHACTIVATEINFO));

			SHINITDLGINFO shidi;
			shidi.dwMask = SHIDIM_FLAGS;
			shidi.dwFlags = SHIDIF_SIPDOWN | SHIDIF_SIZEDLG;
			shidi.hDlg = hwndDlg;
			if(pSHInitDialog)
				pSHInitDialog(&shidi);

			pView->InitView(hwndDlg);
			pView->UpdateView(hwndDlg, TRUE);
		}
		return TRUE;

	case WM_ACTIVATE:
		if(pSHHandleWMActivate)
			pSHHandleWMActivate(hwndDlg, wParam, lParam, &pView->sai, 0);
		return TRUE;

	case WM_SETTINGCHANGE:
		// 'dirtree' example differs from documentation but this one works
		if(pSHHandleWMSettingChange)
			pSHHandleWMSettingChange(hwndDlg, -1, 0, &pView->sai);
		return TRUE;

	case WM_SIZE:
		pView->UpdateLayout(hwndDlg);
		return TRUE;

	case WM_NOTIFY:
		{
			if(!pView->bInternalSelect)
			{
				LPNMHDR nmhdr = (LPNMHDR)lParam;
				HWND hwndList = GetDlgItem(hwndDlg, 1000);
				if(nmhdr->hwndFrom == hwndList && nmhdr->code == LVN_ITEMCHANGED)
				{
					LPNMLISTVIEW lplv = (LPNMLISTVIEW)lParam;
					if(lplv && (lplv->uNewState & LVIS_SELECTED))
					{
						int item = lplv->iItem;
						TCHAR itemText[MAX_PATH];
						ListView_GetItemText(hwndList, item, 0, itemText, MAX_PATH);
						pView->BrowserDown(hwndDlg, itemText);
						return TRUE;
					}
				}
			}
		}
		return FALSE;

	case WM_COMMAND:
		if(HIWORD(wParam)) // notification
		{
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				HWND hwndCombo = GetDlgItem(hwndDlg, 1001);
				if((HWND)lParam == hwndCombo)
				{
					int item = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
					pView->BrowserUp(hwndDlg, item);
					return TRUE;
				}
			}
		}
		else
		{
			switch(wParam)
			{
			case IDOK:
				pView->BrowseResult(hwndDlg);
				EndDialog(hwndDlg, 1);
				return TRUE;
			case IDCANCEL:
				EndDialog(hwndDlg, 0);
				return TRUE;
			}
		}
	default:
		return FALSE;
	}
}



bool BrowseFiles(BROWSEFILES* pbf)
{
	HMODULE hShell = LoadLibrary(TEXT("aygshell.dll"));
	if(hShell != NULL)
	{
		pSHInitDialog = (tSHInitDialog*)GetProcAddress(hShell, TEXT("SHInitDialog"));
		pSHInitExtraControls = (tSHInitExtraControls*)GetProcAddress(hShell, TEXT("SHInitExtraControls"));
		pSHHandleWMActivate = (tSHHandleWMActivate*)GetProcAddress(hShell, TEXT("SHHandleWMActivate"));
		pSHHandleWMSettingChange = (tSHHandleWMSettingChange*)GetProcAddress(hShell, TEXT("SHHandleWMSettingChange"));
	}

	if(!pbf)
		return false;
	if(!pbf->pBuffer || pbf->nBufSize == 0)
		return false;
	if(!pbf->pExtImages)
		return false;

	if(pSHInitExtraControls)
		pSHInitExtraControls(); // Required to use WC_SIPPREF control

	cFileBrowserView* pView = new cFileBrowserView(pbf);
	bool bResult = DialogBoxParam(GetModuleHandle(NULL), MAKEINTRESOURCE(pbf->nTemplateId), NULL, cFileBrowserView::BrowseDlgProc, (LPARAM)pView) == IDOK;
	delete pView; pView = NULL;
	return bResult;
}