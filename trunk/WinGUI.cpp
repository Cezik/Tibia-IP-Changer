////////////////////////////////////////////////////////////////////////////////
// Czepek's IP Changer - Developed by Czepek                                  //
////////////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or              //
// modify it under the terms of the GNU General Public License                //
// as published by the Free Software Foundation; either version 2             //
// of the License, or (at your option) any later version.                     //
//                                                                            //
// This program is distributed in the hope that it will be useful,            //
// but WITHOUT ANY WARRANTY; without even the implied warranty of             //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              //
// GNU General Public License for more details.                               //
//                                                                            //
// You should have received a copy of the GNU General Public License          //
// along with this program; if not, write to the Free Software Foundation,    //
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.            //
////////////////////////////////////////////////////////////////////////////////

#include "WinGUI.h"

extern Tools tools;

WinGUI::WinGUI()
{
	minimized = false;
	nLastItem = -1;
}

void WinGUI::createToolTip(HWND hParent, const char* tipText, HICON iconID, const char* tipTitle)
{
	if(!hParent || tipText == NULL)
		return;

	if(!tools.getShowToolTips())
		return;

	HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL, WS_POPUP | TTS_BALLOON | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hParent, NULL, hInst, NULL);
	SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
	TOOLINFO ti;
	ti.cbSize = sizeof(TOOLINFO);
	ti.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
	ti.hwnd = mainWindow;
	ti.hinst = NULL;
	ti.uId = (UINT_PTR)hParent;
	ti.lpszText = (LPSTR)tipText;
	SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
	SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 275);
	if(tipTitle != NULL && iconID > 0)
		SendMessage(hwndTT, TTM_SETTITLE, (WPARAM)iconID, (LPARAM)tipTitle);
}

void WinGUI::doSetItem(HWND hLview, char* szBuffer, int nItem, int nSubItem)
{
	lvi.state = 0;
	lvi.stateMask = 0;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = nItem;
	lvi.iSubItem = nSubItem;
	lvi.pszText = szBuffer;
	ListView_SetItem(hLview, &lvi);
}

void WinGUI::doCreateItem(HWND hLview, char* szBuffer, int nItem, int nSubItem, int nData)
{
	lvi.state = 0;
	lvi.stateMask = 0;

	if(nSubItem > 0)
		lvi.mask = LVIF_TEXT;
	else
	{
		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.lParam = (LPARAM)nData;
	}

	lvi.iItem = nItem;
	lvi.iSubItem = nSubItem;
	lvi.pszText = szBuffer;

	if(nSubItem > 0)
		ListView_SetItem(hLview, &lvi);
	else
		ListView_InsertItem(hLview, &lvi);
}

void WinGUI::doCreateColumn(HWND hLview, char* szBuffer, int nWidth, int nColumnNumber)
{
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.iSubItem = nColumnNumber;
	lvc.pszText = szBuffer;
	lvc.cx = nWidth;
	lvc.fmt = LVCFMT_LEFT;
	ListView_InsertColumn(hLview, nColumnNumber, &lvc);
}

HWND WinGUI::doCreateListView(HWND hParent, int x, int y, int nWidth, int nHeight)
{
	HWND hLview;
	RECT rParent;
	char szBuffer[256];
	ZeroMemory(szBuffer, sizeof(szBuffer));
	GetClientRect(hParent, &rParent);
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	if(!InitCommonControlsEx(&icex))
	   messageBox(MESSAGE_TYPE_FATAL_ERROR, NAME, "Error while creating user interface.");

	hLview = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, "", WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_AUTOARRANGE | WS_BORDER | LVS_SHOWSELALWAYS, x, y, nWidth, nHeight, hParent, (HMENU)ID_DLG_LISTVIEW, 0, 0);

	ListView_SetExtendedListViewStyle(hLview, LVS_EX_FULLROWSELECT);
	return hLview;
}

void WinGUI::doShowMenu(HWND hWnd, HMENU rMenu)
{
	POINT pMouse;
	GetCursorPos(&pMouse);
	SetForegroundWindow(hWnd);
	TrackPopupMenu(rMenu, TPM_LEFTALIGN, pMouse.x, pMouse.y, 0, hWnd, 0);
	PostMessage(hWnd, WM_NULL, 0, 0);
}

bool WinGUI::messageBox(MesageBoxType_t type, const char* title, const char* text, ...)
{
	va_list list;
	va_start(list, text);
	char buffer[4096];
	_vsnprintf(buffer, sizeof(buffer), text, list);
	va_end(list);

	UINT mType;
	switch(type)
	{
		case MESSAGE_TYPE_FATAL_ERROR:
			mType = MB_OK | MB_ICONERROR;
			break;

		case MESSAGE_TYPE_ERROR:
			mType = MB_OK | MB_ICONEXCLAMATION;
			break;

		case MESSAGE_TYPE_INFO:
			mType = MB_OK | MB_ICONINFORMATION;
			break;

		case MESSAGE_TYPE_NO_ICON:
			mType = MB_OK;
			break;

		case MESSAGE_TYPE_YES_NO:
			mType = MB_YESNO;
			if(mType == MB_YESNO)
			{
				if(MessageBoxA(HWND_DESKTOP, buffer, title, mType) == IDYES)
					return true;
				else
					return false;
			}
			break;

		default:
			mType = MB_OK;
			break;
	}
	MessageBoxA(HWND_DESKTOP, buffer, title, mType);
	return true;
}

void WinGUI::addLineToLabel(HWND hwnd, int ResourceID, const char* text, ...)
{
	char oldBuffer[4096], newBuffer[4096], buffer[4096];
	va_list list;
	va_start(list, text);
	_vsnprintf(buffer, sizeof(buffer), text, list);
	va_end(list);

	GetDlgItemText(hwnd, ResourceID, oldBuffer, sizeof(oldBuffer));
	sprintf(newBuffer, "%s\r\n%s", oldBuffer, buffer);
	SetDlgItemText(hwnd, ResourceID, newBuffer);
}
