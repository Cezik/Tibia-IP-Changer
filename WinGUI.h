////////////////////////////////////////////////////////////////////////////////
// Czepek's IP Changer - Developed by Czepek								  //
////////////////////////////////////////////////////////////////////////////////
// This program is free software; you can redistribute it and/or			  //
// modify it under the terms of the GNU General Public License				  //
// as published by the Free Software Foundation; either version 2			  //
// of the License, or (at your option) any later version.					  //
//																			  //
// This program is distributed in the hope that it will be useful,			  //
// but WITHOUT ANY WARRANTY; without even the implied warranty of			  //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the			  //
// GNU General Public License for more details.								  //
//																			  //
// You should have received a copy of the GNU General Public License		  //
// along with this program; if not, write to the Free Software Foundation,	  //
// Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.			  //
////////////////////////////////////////////////////////////////////////////////

#ifndef __WINGUI_H__
#define __WINGUI_H__

#include <windows.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <Psapi.h>
#include <stdio.h>
#include "resource.h"
#include "Tools.h"

enum MesageBoxType_t
{
	MESSAGE_TYPE_FATAL_ERROR,
	MESSAGE_TYPE_ERROR,
	MESSAGE_TYPE_INFO,
	MESSAGE_TYPE_NO_ICON,
	MESSAGE_TYPE_YES_NO
};

class WinGUI
{
	public:
		WinGUI();
		~WinGUI(){};

		void createToolTip(HWND hParent, const char* tipText, HICON iconID, const char* tipTitle = NULL);
		void doSetItem(HWND hLview, char* szBuffer, int nItem, int nSubItem);
		void doCreateItem(HWND hLview, char* szBuffer, int nItem, int nSubItem, int nData);
		void doCreateColumn(HWND hLview, char* szBuffer, int nWidth, int nColumnNumber);
		HWND doCreateListView(HWND hParent, int x, int y, int nWidth, int nHeight);
		void doShowMenu(HWND hWnd, HMENU rMenu);
		bool messageBox(MesageBoxType_t type, const char* title, const char* text, ...);
		void addLineToLabel(HWND hwnd, int ResourceID, const char* text, ...);

		HINSTANCE hInst;
		NOTIFYICONDATA trayIcon;
		HMENU ipListMenu, ipListMenu2, trayMenu, mainMenu;
		HWND mainWindow, hWndIpList, optionsWindow, languageWindow;
		LVITEM lvi;
		HBITMAP hbIcons[ID_ICON_LAST];
		HICON mainIcon;
		bool minimized;
		int nLastSubItem, nSelected, nLastItem;
};
#endif
