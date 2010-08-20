////////////////////////////////////////////////////////////////////////////////
// OTFans.pl IP Changer - Developed by Czepek                                 //
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

#define _WIN32_IE 0x0501
#define _WIN32_WINNT 0x0502

#include <stdio.h>
#include <winsock.h>
#include "WinGUI.h"

#include "resource.h"
#include "IpChanger.h"
#include "Tools.h"

WinGUI gui;
Tools tools;

bool saveServerList()
{
	xmlNodePtr root, listNode;
	xmlDocPtr doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)SECTION, NULL);
	root = doc->children;
	for(int i = 0; i < ListView_GetItemCount(gui.hWndIpList); i++)
	{
		char cAddress[256], cPort[10];
		ListView_GetItemText(gui.hWndIpList, i, 0, cAddress, sizeof(cAddress));
		ListView_GetItemText(gui.hWndIpList, i, 1, cPort, sizeof(cPort));

		listNode = xmlNewNode(NULL,(const xmlChar*)"Server");
		xmlSetProp(listNode, (const xmlChar*) "IP", (const xmlChar*)cAddress);
		xmlSetProp(listNode, (const xmlChar*) "Port", (const xmlChar*)cPort);
		xmlAddChild(root, listNode);
	}

	if(xmlSaveFile(tools.getFilePath(SERVER_LIST_FILE).c_str(), doc))
	{
		xmlFreeDoc(doc);
		return true;
	}
	else
	{
		xmlFreeDoc(doc);
		return false;
	}
}

bool parseCommandLine(std::vector<std::string> args)
{
	std::vector<std::string>::iterator argi = args.begin();
	if(argi != args.end())
		++argi;

	while(argi != args.end())
	{
		std::string arg = *argi;
		if(arg == "-i" || arg == "-ip")
		{
			if(++argi == args.end())
			{
				gui.messageBox(MESSAGE_TYPE_ERROR, NULL, tools.languageTable[39], arg.c_str());
				return false;
			}
			tools.cmdLineIP = new char[strlen((*argi).c_str())];
			strcpy(tools.cmdLineIP, (*argi).c_str());
		}
		else if(arg == "-p" || arg == "-port")
		{
			if(++argi == args.end())
			{
				gui.messageBox(MESSAGE_TYPE_ERROR, NULL, tools.languageTable[39], arg.c_str());
				return false;
			}
			tools.cmdLinePort = new char[strlen((*argi).c_str())];
			strcpy(tools.cmdLinePort, (*argi).c_str());
		}
		else
		{
			gui.messageBox(MESSAGE_TYPE_ERROR, NULL, tools.languageTable[38], arg.c_str());
			return false;
		}
		++argi;
	}
	return true;
}

BOOL CALLBACK ServerInfoProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message) 
	{
		case WM_INITDIALOG:
			SetTimer(hwnd, ID_TMR_SOCK_CON, 50, NULL);
			tools.bRecv = false;
			tools.bSend = false;
			break;

		case WM_TIMER:
			switch(wParam)
			{
				case ID_TMR_SOCK_CON:
				{
					char ipAddress[256], newPort[10], cStatusBar[1024];
					if(!tools.getCheckFromList())
					{
						if(!GetDlgItemText(gui.mainWindow, ID_DLG_IP, ipAddress, sizeof(ipAddress)) || !GetDlgItemText(gui.mainWindow, ID_DLG_PORT, newPort, sizeof(newPort)))
						{
							KillTimer(hwnd, ID_TMR_SOCK_CON);
							gui.messageBox(MESSAGE_TYPE_ERROR, NULL, tools.languageTable[43]);
							SendMessage(hwnd, WM_DESTROY, 0, 0);
						}
					}
					else
					{
						ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 0, ipAddress, sizeof(ipAddress));
						ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 1, newPort, sizeof(newPort));
					}

					KillTimer(hwnd, ID_TMR_SOCK_CON);
					sprintf(cStatusBar, tools.languageTable[7], ipAddress, newPort);
					SetDlgItemText(hwnd, ID_DLG_SERVER, cStatusBar);

					tools.sSock = tools.createSocket(hwnd);
					if(tools.sSock == INVALID_SOCKET)
						gui.addLineToLabel(hwnd, ID_DLG_SERVER, tools.languageTable[9]);
					else
						gui.addLineToLabel(hwnd, ID_DLG_SERVER, tools.languageTable[8]);

					tools.SA = tools.sSAddrCreate(ipAddress, (unsigned short)atoi(newPort));
					connect(tools.sSock, (LPSOCKADDR)&tools.SA, sizeof(struct sockaddr));
					SetTimer(hwnd, ID_TMR_SOCK_TIMEOUT, 10000, NULL);
					break;
				}

				case ID_TMR_SOCK_TIMEOUT:
				{
					KillTimer(hwnd, ID_TMR_SOCK_TIMEOUT);
					if(!tools.bRecv)
					{
						gui.addLineToLabel(hwnd, ID_DLG_SERVER, tools.languageTable[5]);
						if(tools.getCheckFromList())
							gui.doSetItem(gui.hWndIpList, "OFF-Line", gui.nLastItem, 2);
						SendMessage(hwnd, WM_DESTROY, 0, 0);
					}
					break;
				}
			}
			break;

		case WM_SOCKET:
		{
			if(WSAGETSELECTERROR(lParam))
			{
				if(tools.getCheckFromList())
					gui.doSetItem(gui.hWndIpList, "OFF-Line", gui.nLastItem, 2);
				gui.addLineToLabel(hwnd, ID_DLG_SERVER, tools.languageTable[5]);
				SendMessage(hwnd, WM_DESTROY, 0, 0);
			}

			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_CONNECT:
					tools.bSend = true;
					break;

				case FD_WRITE:
					if(tools.bSend)
					{
						unsigned char cPacket[8];
						cPacket[0] = 0x06;
						cPacket[1] = 0x00;
						cPacket[2] = 0xFF;
						cPacket[3] = 0xFF;
						cPacket[4] = 'i';
						cPacket[5] = 'n';
						cPacket[6] = 'f';
						cPacket[7] = 'o';
						send(tools.sSock, (const char*)cPacket, sizeof(cPacket), 0);
						tools.bSend = false;
					}
					break;

				case FD_READ:
					char buffer[1024];
					if(recv(tools.sSock, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
					{
						tools.bRecv = true;
						if(tools.getCheckFromList())
							gui.doSetItem(gui.hWndIpList, "ON-Line", gui.nLastItem, 2);
						tools.StringToke(hwnd, buffer);
						closesocket(tools.sSock);
						WSACleanup();
					}
					else
						SendMessage(hwnd, WM_CLOSE, 0, 0);
					ZeroMemory(buffer, sizeof(buffer));
					break;

				case FD_CLOSE:
					closesocket(tools.sSock);
					WSACleanup();
					break;
			}
			break;
		}

		case WM_CLOSE:
		case WM_DESTROY:
			closesocket(tools.sSock);
			WSACleanup();
			EndDialog(hwnd, 0);
			break;
	}
	return false;
}

BOOL CALLBACK OptionsWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	gui.optionsWindow = hWnd;
	switch(message) 
	{
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
					switch(wParam)
					{
						case ID_DLG_PROTOCOL_TXT:
						{
							char buffer[1024];
							GetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer, sizeof(buffer));
							strcat(buffer, "$protocol$");
							SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer);
							break;
						}

						case ID_DLG_IPADDR_TXT:
						{
							char buffer[1024];
							GetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer, sizeof(buffer));
							strcat(buffer, "$ipaddress$");
							SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer);
							break;
						}

						case ID_DLG_PORT_TXT:
						{
							char buffer[1024];
							GetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer, sizeof(buffer));
							strcat(buffer, "$port$");
							SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, buffer);
							break;
						}
					}
					break;
			}
			break;

		case WM_INITDIALOG:
		{
			tools.rsaKey = tools.readString("KeyRSA");
			if(tools.rsaKey.empty())
				SetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, OTSERV_RSA_KEY);
			else
				SetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, tools.rsaKey.c_str());

			std::string tibiaTitle = tools.readString("CustomTibiaTitle");
			if(tibiaTitle.empty())
				SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, "Tibia");
			else
				SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, tibiaTitle.c_str());

			if(tools.getUseOtherRSA())
			{
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, BM_SETCHECK, BST_CHECKED, 0);
				EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_EDIT), TRUE);
			}
			else
			{
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_EDIT), FALSE);
			}

			if(tools.getShowMessageBox())
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE, BM_SETCHECK, BST_UNCHECKED, 0);

			if(tools.getChangeTitleCmdLine())
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_CHANGE_WINDOW_CMD_LINE, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_CHANGE_WINDOW_CMD_LINE, BM_SETCHECK, BST_UNCHECKED, 0);

			if(tools.getSupportForOTServList())
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_OTSERV_SUPPORT, BM_SETCHECK, BST_CHECKED, 0);
			else
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_OTSERV_SUPPORT, BM_SETCHECK, BST_UNCHECKED, 0);

			if(tools.getShowToolTips())
			{
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_TOOLTIPS, BM_SETCHECK, BST_CHECKED, 0);
				gui.createToolTip(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_DEFAULT), tools.languageTable[49], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(gui.optionsWindow, ID_DLG_OPTIONS_CHANGE_WINDOW_CMD_LINE), tools.languageTable[46], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE), tools.languageTable[50], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(gui.optionsWindow, ID_DLG_OPTIONS_OTSERV_SUPPORT), tools.languageTable[52], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_TOOLTIPS), tools.languageTable[53], (HICON)gui.mainIcon, NAME);
			}
			else
				SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_TOOLTIPS, BM_SETCHECK, BST_UNCHECKED, 0);

			SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_DEFAULT, WM_SETTEXT, 0, (LPARAM)tools.languageTable[49]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_SAVE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[60]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_LOAD, WM_SETTEXT, 0, (LPARAM)tools.languageTable[61]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, WM_SETTEXT, 0, (LPARAM)tools.languageTable[62]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_THIS_ONE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[63]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_TITLE_SAVE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[60]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_TITLE_LOAD, WM_SETTEXT, 0, (LPARAM)tools.languageTable[61]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[66]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_CHANGE_WINDOW_CMD_LINE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[67]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_OTSERV_SUPPORT, WM_SETTEXT, 0, (LPARAM)tools.languageTable[69]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_TOOLTIPS, WM_SETTEXT, 0, (LPARAM)tools.languageTable[70]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE_SAVE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[60]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_PROTOCOL_TXT, WM_SETTEXT, 0, (LPARAM)tools.languageTable[65]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_IPADDR_TXT, WM_SETTEXT, 0, (LPARAM)tools.languageTable[64]);
			SendDlgItemMessage(gui.optionsWindow, ID_DLG_PORT_TXT, WM_SETTEXT, 0, (LPARAM)tools.languageTable[56]);
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case ID_DLG_RSA_DEFAULT:
					SetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, OTSERV_RSA_KEY);
					break;

				case ID_DLG_RSA_SAVE:
					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, BM_GETCHECK, 0, 0))
					{
						char saveRsaKey[4096];
						GetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, saveRsaKey, sizeof(saveRsaKey));

						tools.rsaKey = saveRsaKey;
						WritePrivateProfileString(SECTION, "KeyRSA", saveRsaKey, tools.getFilePath(CONFIG_FILE).c_str());
					}
					break;

				case ID_DLG_RSA_LOAD:
					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, BM_GETCHECK, 0, 0))
					{
						tools.rsaKey = tools.readString("KeyRSA");
						if(!tools.rsaKey.empty())
							SetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, tools.rsaKey.c_str());
						else
							gui.messageBox(MESSAGE_TYPE_ERROR, NAME, tools.languageTable[41]);
					}
					break;

				case ID_DLG_RSA_USE_THIS_ONE:
				{
					char saveRsaKey[4096];
					GetDlgItemText(gui.optionsWindow, ID_DLG_RSA_EDIT, saveRsaKey, sizeof(saveRsaKey));
					tools.rsaKey = saveRsaKey;
					break;
				}

				case ID_DLG_RSA_USE_OTHER:
					if(HIWORD(wParam) == BN_CLICKED)
					{
						if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_RSA_USE_OTHER, BM_GETCHECK, 0, 0))
						{
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_EDIT), TRUE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_USE_THIS_ONE), TRUE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_SAVE), TRUE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_LOAD), TRUE);
							tools.setUseOtherRSA(true);
						}
						else
						{
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_EDIT), FALSE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_USE_THIS_ONE), FALSE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_SAVE), FALSE);
							EnableWindow(GetDlgItem(gui.optionsWindow, ID_DLG_RSA_LOAD), FALSE);
							tools.setUseOtherRSA(false);
						}
					}
					break;

				case ID_DLG_TITLE_LOAD:
				{
					std::string tibiaTitle = tools.readString("CustomTibiaTitle");
					if(tibiaTitle.empty())
						SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, "Tibia");
					else
						SetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, tibiaTitle.c_str());
					break;
				}

				case ID_DLG_TITLE_SAVE:
				{
					char saveTibiaTitle[4096];
					GetDlgItemText(gui.optionsWindow, ID_DLG_OPTIONS_EDIT_TITLE, saveTibiaTitle, sizeof(saveTibiaTitle));
					WritePrivateProfileString(SECTION, "CustomTibiaTitle", saveTibiaTitle, tools.getFilePath(CONFIG_FILE).c_str());
					break;
				}

				case ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE_SAVE:
				{
					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE, BM_GETCHECK, 0, 0))
					{
						WritePrivateProfileString(SECTION, "ShowMessageBox", "1", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setShowMessageBox(true);
					}
					else
					{
						WritePrivateProfileString(SECTION, "ShowMessageBox", "0", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setShowMessageBox(false);
					}

					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE, BM_GETCHECK, 0, 0))
					{
						WritePrivateProfileString(SECTION, "ChangeTibiaTitleCmdLine", "1", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setChangeTitleCmdLine(true);
					}
					else
					{
						WritePrivateProfileString(SECTION, "ChangeTibiaTitleCmdLine", "0", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setChangeTitleCmdLine(false);
					}

					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_OTSERV_SUPPORT, BM_GETCHECK, 0, 0))
					{
						WritePrivateProfileString(SECTION, "SupportForOTServList", "1", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setSupportForOTServList(true);
						tools.addSupportForOTServList();
					}
					else
					{
						WritePrivateProfileString(SECTION, "SupportForOTServList", "0", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setSupportForOTServList(false);
						SHDeleteKey(HKEY_CLASSES_ROOT, "OTSERV");
					}

					if(SendDlgItemMessage(gui.optionsWindow, ID_DLG_OPTIONS_SHOW_TOOLTIPS, BM_GETCHECK, 0, 0))
					{
						WritePrivateProfileString(SECTION, "ShowToolTips", "1", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setShowToolTips(true);
					}
					else
					{
						WritePrivateProfileString(SECTION, "ShowToolTips", "0", tools.getFilePath(CONFIG_FILE).c_str());
						tools.setShowToolTips(false);
					}
				}
			}
			break;

		case WM_CLOSE:
		case WM_DESTROY:
			ShowWindow(gui.optionsWindow, SW_HIDE);
			break;
	}
	return false;
}

BOOL CALLBACK EditServerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
			char cAddress[256];
			char cPort[10];
			ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 0, cAddress, sizeof(cAddress));
			ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 1, cPort, sizeof(cPort));
			SetDlgItemText(hwnd, ID_DLG_EDIT_ADDRESS, cAddress);
			SetDlgItemText(hwnd, ID_DLG_EDIT_PORT, cPort);
			HFONT m_hFont;
			LOGFONT lfont;
			memset(&lfont, 0, sizeof(lfont));
			lstrcpy(lfont.lfFaceName, TEXT("Arial"));
			lfont.lfHeight = 16;
			lfont.lfWeight = FW_BOLD;
			lfont.lfItalic = false;
			lfont.lfCharSet = DEFAULT_CHARSET;
			lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lfont.lfQuality = DEFAULT_QUALITY;
			lfont.lfPitchAndFamily = DEFAULT_PITCH;
			m_hFont = CreateFontIndirect(&lfont);
			SendDlgItemMessage(hwnd, ID_DLG_EDIT_CLOSE, WM_SETFONT, (WPARAM)m_hFont, 0);
			SendDlgItemMessage(hwnd, ID_DLG_EDIT_SAVE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[60]);
			break;

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case ID_DLG_EDIT_SAVE:
				{
					char cAddress[256], cPort[10];
					GetDlgItemText(hwnd, ID_DLG_EDIT_ADDRESS, cAddress, sizeof(cAddress));
					GetDlgItemText(hwnd, ID_DLG_EDIT_PORT, cPort, sizeof(cPort));
					gui.doSetItem(gui.hWndIpList, cAddress, gui.nLastItem, 0);
					gui.doSetItem(gui.hWndIpList, cPort, gui.nLastItem, 1);
					gui.doSetItem(gui.hWndIpList, "?", gui.nLastItem, 2);
					saveServerList();

					SendMessage(hwnd, WM_CLOSE, 0, 0);
				}
				break;

				case ID_DLG_EDIT_CLOSE:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;

			case WM_CLOSE:
			case WM_DESTROY:
				EndDialog(hwnd, 0);
				break;
	}
	return false;
}

BOOL CALLBACK AddServerProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			SetDlgItemText(hwnd, ID_DLG_ADD_PORT, "7171");
			HFONT m_hFont;
			LOGFONT lfont;
			memset(&lfont, 0, sizeof(lfont));
			lstrcpy(lfont.lfFaceName, TEXT("Arial"));
			lfont.lfHeight = 16;
			lfont.lfWeight = FW_BOLD;
			lfont.lfItalic = false;
			lfont.lfCharSet = DEFAULT_CHARSET;
			lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lfont.lfQuality = DEFAULT_QUALITY;
			lfont.lfPitchAndFamily = DEFAULT_PITCH;
			m_hFont = CreateFontIndirect(&lfont);
			SendDlgItemMessage(hwnd, ID_DLG_ADD_CLOSE, WM_SETFONT, (WPARAM)m_hFont, 0);
			SendDlgItemMessage(hwnd, ID_DLG_ADD_ADD, WM_SETTEXT, 0, (LPARAM)tools.languageTable[21]);
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case ID_DLG_ADD_ADD:
				{
					char cAddress[256], cPort[10];
					if(GetDlgItemText(hwnd, ID_DLG_ADD_ADDRESS, cAddress, sizeof(cAddress)) && GetDlgItemText(hwnd, ID_DLG_ADD_PORT, cPort, sizeof(cPort)))
					{
						int nRet = ListView_GetItemCount(gui.hWndIpList);
						gui.doCreateItem(gui.hWndIpList, cAddress, nRet, 0, gui.nSelected);
						gui.doCreateItem(gui.hWndIpList, cPort, nRet, 1, gui.nSelected);
						gui.doCreateItem(gui.hWndIpList, "?", nRet, 2, gui.nSelected);
						saveServerList();

						SendMessage(hwnd, WM_CLOSE, 0, 0);
					}
				}
				break;

				case ID_DLG_ADD_CLOSE:
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;
			}
			break;

			case WM_CLOSE:
			case WM_DESTROY:
				EndDialog(hwnd, 0);
				break;
	}
	return false;
}

BOOL CALLBACK serverList(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_INITDIALOG:
		{
			gui.ipListMenu = CreatePopupMenu();
			AppendMenu(gui.ipListMenu, MF_STRING, ID_MENU_CHANGE_IP, tools.languageTable[16]);
			AppendMenu(gui.ipListMenu, MF_STRING, ID_MENU_SERVER_INFO, tools.languageTable[17]);
			AppendMenu(gui.ipListMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.ipListMenu, MF_STRING, ID_MENU_EDIT_SERVER, tools.languageTable[18]);
			AppendMenu(gui.ipListMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.ipListMenu, MF_STRING, ID_MENU_DELETE_SERVER, tools.languageTable[19]);
			AppendMenu(gui.ipListMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.ipListMenu, MF_STRING, ID_MENU_DELETE_ALL_SERVERS, tools.languageTable[20]);

			gui.ipListMenu2 = CreatePopupMenu();
			AppendMenu(gui.ipListMenu2, MF_STRING, ID_MENU_ADD_SERVER, tools.languageTable[21]);
			AppendMenu(gui.ipListMenu2, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.ipListMenu2, MF_STRING, ID_MENU_DELETE_ALL_SERVERS, tools.languageTable[20]);
			AppendMenu(gui.ipListMenu2, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.ipListMenu2, MF_STRING, ID_MENU_CLOSE_WINDOW, tools.languageTable[22]);

			SetMenuItemBitmaps(gui.ipListMenu2, ID_MENU_ADD_SERVER, MF_BYCOMMAND, gui.hbIcons[ID_ICON_ADD], gui.hbIcons[ID_ICON_ADD]);
			SetMenuItemBitmaps(gui.ipListMenu2, ID_MENU_CLOSE_WINDOW, MF_BYCOMMAND, gui.hbIcons[ID_ICON_CLOSE], gui.hbIcons[ID_ICON_CLOSE]);
			SetMenuItemBitmaps(gui.ipListMenu2, ID_MENU_DELETE_ALL_SERVERS, MF_BYCOMMAND, gui.hbIcons[ID_ICON_DELETE_ALL], gui.hbIcons[ID_ICON_DELETE_ALL]);

			SetMenuItemBitmaps(gui.ipListMenu, ID_MENU_EDIT_SERVER, MF_BYCOMMAND, gui.hbIcons[ID_ICON_EDIT], gui.hbIcons[ID_ICON_EDIT]);
			SetMenuItemBitmaps(gui.ipListMenu, ID_MENU_DELETE_ALL_SERVERS, MF_BYCOMMAND, gui.hbIcons[ID_ICON_DELETE_ALL], gui.hbIcons[ID_ICON_DELETE_ALL]);
			SetMenuItemBitmaps(gui.ipListMenu, ID_MENU_SERVER_INFO, MF_BYCOMMAND, gui.hbIcons[ID_ICON_INFORMATION], gui.hbIcons[ID_ICON_INFORMATION]);
			SetMenuItemBitmaps(gui.ipListMenu, ID_MENU_CHANGE_IP, MF_BYCOMMAND, gui.hbIcons[ID_ICON_CHANGEIP], gui.hbIcons[ID_ICON_CHANGEIP]);
			SetMenuItemBitmaps(gui.ipListMenu, ID_MENU_DELETE_SERVER, MF_BYCOMMAND, gui.hbIcons[ID_ICON_DELETE], gui.hbIcons[ID_ICON_DELETE]);

			RECT rc;
			GetClientRect(hwnd, &rc);

			gui.hWndIpList = gui.doCreateListView(hwnd, rc.left, rc.top, rc.right, rc.bottom);

			gui.doCreateColumn(gui.hWndIpList, tools.languageTable[10], 160, 0);
			gui.doCreateColumn(gui.hWndIpList, "Port", 60, 1);
			gui.doCreateColumn(gui.hWndIpList, "Status", 80, 2);

			if(tools.loadFromXmlIpList())
			{
				for(int i = 0; i < MAX_SERVERS_IN_LIST; i++)
				{
					if(!tools.IpListAddr[i].empty() && !tools.IpListPort[i].empty())
					{
						int nCount = ListView_GetItemCount(gui.hWndIpList);
						gui.doCreateItem(gui.hWndIpList, (char*)tools.IpListAddr[i].c_str(), nCount, 0, nCount);
						gui.doCreateItem(gui.hWndIpList, (char*)tools.IpListPort[i].c_str(), nCount, 1, nCount);
						gui.doCreateItem(gui.hWndIpList, "?", nCount, 2, nCount);
					}
				}
			}
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_MENU_CLOSE_WINDOW:
					saveServerList();
					SendMessage(hwnd, WM_CLOSE, 0, 0);
					break;

				case ID_MENU_ADD_SERVER:
					DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_ADD_SERVER), HWND_DESKTOP, AddServerProc);
					break;

				case ID_MENU_EDIT_SERVER:
					if(gui.nLastItem != -1)
						DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_EDIT_SERVER), HWND_DESKTOP, EditServerProc);
					break;

				case ID_MENU_DELETE_SERVER:
				{
					if(gui.nLastItem != -1)
					{
						if(gui.messageBox(MESSAGE_TYPE_YES_NO, NAME, tools.languageTable[3]))
						{
							ListView_DeleteItem(gui.hWndIpList, gui.nLastItem);
							saveServerList();
						}
					}
					break;
				}

				case ID_MENU_DELETE_ALL_SERVERS:
					if(gui.messageBox(MESSAGE_TYPE_YES_NO, NAME, tools.languageTable[11]))
					{
						ListView_DeleteAllItems(gui.hWndIpList);
						saveServerList();
					}
					break;

				case ID_MENU_SERVER_INFO:
					tools.setCheckFromList(true);
					DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_SERVER_INFO), HWND_DESKTOP, ServerInfoProc);
					break;

				case ID_MENU_CHANGE_IP:
				{
					char cAddress[256], cPort[10];
					ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 0, cAddress, sizeof(cAddress));
					ListView_GetItemText(gui.hWndIpList, gui.nLastItem, 1, cPort, sizeof(cPort));
					if(HIWORD(wParam) == BN_CLICKED)
					{
						if(SendDlgItemMessage(gui.mainWindow, ID_DLG_CHANGE_TITLE, BM_GETCHECK, 0, 0))
						{
							tools.setNewConnection(cAddress, (unsigned short)atoi(cPort), true);
						}
						else
						{
							tools.setNewConnection(cAddress, (unsigned short)atoi(cPort), false);
						}
					}
					break;
				}
			}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_RCLICK:
				{
					LPNMITEMACTIVATE lpnmItem = (LPNMITEMACTIVATE)lParam;
					if(lpnmItem->iItem != -1)
					{
						gui.nLastItem = lpnmItem->iItem;
						gui.nLastSubItem = lpnmItem->iSubItem;
						gui.doShowMenu(hwnd, gui.ipListMenu);
					}
					else
					{
						gui.nLastItem = -1;
						gui.nLastSubItem = -1;
						gui.doShowMenu(hwnd, gui.ipListMenu2);
					}
					break;
				}

				case NM_DBLCLK:
				{
					LPNMITEMACTIVATE lpnmItem = (LPNMITEMACTIVATE)lParam;
					if(lpnmItem->iItem != -1)
					{
						gui.nLastItem = lpnmItem->iItem;
						gui.nLastSubItem = lpnmItem->iSubItem;
						DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_EDIT_SERVER), HWND_DESKTOP, EditServerProc);
					}
					else
					{
						gui.nLastItem = -1;
						gui.nLastSubItem = -1;
					}
					break;
				}

				case NM_CLICK:
				case NM_RDBLCLK:
				{
					LPNMITEMACTIVATE lpnmItem = (LPNMITEMACTIVATE)lParam;
					if(lpnmItem->iItem != -1)
					{
						gui.nLastItem = lpnmItem->iItem;
						gui.nLastSubItem = lpnmItem->iSubItem;
					}
					else
					{
						gui.nLastItem = -1;
						gui.nLastSubItem = -1;
					}
					break;
				}
			}
			break;

			case WM_CLOSE:
			case WM_DESTROY:
			{
				EndDialog(hwnd, 0);
				break;
			}
	}
	return false;
}


BOOL CALLBACK MainWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	gui.mainWindow = hWnd;
	switch(message)
	{
		case WM_SHELLNOTIFY:
			if(wParam == ID_TRAY_ICON)
			{
				if(lParam == WM_LBUTTONDOWN || lParam == WM_RBUTTONDOWN)
				{
					gui.doShowMenu(hWnd, gui.trayMenu);
				}
			}
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
					if(wParam == ID_DLG_LINK)
						ShellExecute(NULL, "open", "http://otfans.pl/", NULL, NULL, SW_SHOW);
					break;
			}
			break;

		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED)
			{
				gui.minimized = true;
				ShowWindow(hWnd, SW_HIDE);
				ModifyMenu(gui.trayMenu, ID_MENU_TRAY_HIDE, MF_STRING, ID_MENU_TRAY_HIDE, tools.languageTable[12]);
			}
			break;

		case WM_INITDIALOG:
		{
			for(int i = ID_ICON_CLOSE; i < ID_ICON_LAST; i++)
				gui.hbIcons[i] = (HBITMAP)LoadImage(gui.hInst, MAKEINTRESOURCE(i), IMAGE_BITMAP, 13, 13, LR_LOADTRANSPARENT);

			LOGFONT lfont;
			ZeroMemory(&lfont, sizeof(lfont));
			lstrcpy(lfont.lfFaceName, TEXT("MS Sans Serif"));
			lfont.lfHeight = 4;
			lfont.lfWeight = FW_BOLD;
			lfont.lfItalic = false;
			lfont.lfUnderline = true;
			lfont.lfCharSet = DEFAULT_CHARSET;
			lfont.lfOutPrecision = OUT_DEFAULT_PRECIS;
			lfont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
			lfont.lfQuality = DEFAULT_QUALITY;
			lfont.lfPitchAndFamily = DEFAULT_PITCH;
			HFONT m_hFont = CreateFontIndirect(&lfont);
			SendDlgItemMessage(hWnd, ID_DLG_LINK, WM_SETFONT, (WPARAM)m_hFont, 0);

			gui.trayIcon.cbSize = sizeof(NOTIFYICONDATA);
			gui.trayIcon.hWnd = hWnd;
			gui.trayIcon.uID = ID_TRAY_ICON;
			gui.trayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			gui.trayIcon.uCallbackMessage = WM_SHELLNOTIFY;
			gui.trayIcon.hIcon = gui.mainIcon = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
			strcpy(gui.trayIcon.szTip, NAME);
			Shell_NotifyIcon(NIM_ADD, &gui.trayIcon);

			gui.trayMenu = CreatePopupMenu();
			AppendMenu(gui.trayMenu, MF_STRING, ID_MENU_IP_LIST, tools.languageTable[15]);
			AppendMenu(gui.trayMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.trayMenu, MF_STRING, ID_MENU_TRAY_HIDE, tools.languageTable[13]);
			AppendMenu(gui.trayMenu, MF_SEPARATOR, 0, 0);
			AppendMenu(gui.trayMenu, MF_STRING, ID_MENU_EXIT, tools.languageTable[14]);

			if(tools.loadFromXmlIpList())
			{
				for(int i = 0; i < MAX_SERVERS_IN_LIST; i++)
				{
					if(!tools.IpListAddr[i].empty())
					{
						SendDlgItemMessage(hWnd, ID_DLG_IP, CB_ADDSTRING, 0 , (LPARAM)tools.IpListAddr[i].c_str());
					}
				}
				SendDlgItemMessage(hWnd, ID_DLG_IP, CB_SELECTSTRING, 0, (LPARAM)tools.IpListAddr[0].c_str());
			}
			else
				SetDlgItemText(hWnd, ID_DLG_IP, "127.0.0.1");
			SetDlgItemText(hWnd, ID_DLG_PORT, "7171");

			if(tools.getShowToolTips())
			{
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_CHANGE_IP), tools.languageTable[44], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_SHOW_SERVER_INFO), tools.languageTable[45], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_CHANGE_TITLE), tools.languageTable[46], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_IP), tools.languageTable[47], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_PORT), tools.languageTable[54], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_LINK), tools.languageTable[48], (HICON)gui.mainIcon, NAME);
				gui.createToolTip(GetDlgItem(hWnd, ID_DLG_REFRESH_LIST), tools.languageTable[55], (HICON)gui.mainIcon, NAME);
			}

			gui.mainMenu = GetMenu(hWnd);
			SetMenuItemBitmaps(gui.mainMenu, ID_MENU_EXIT, MF_BYCOMMAND, gui.hbIcons[ID_ICON_CLOSE], gui.hbIcons[ID_ICON_CLOSE]);
			SetMenuItemBitmaps(gui.mainMenu, ID_MENU_OPTIONS, MF_BYCOMMAND, gui.hbIcons[ID_ICON_OPTIONS], gui.hbIcons[ID_ICON_OPTIONS]);
			SetMenuItemBitmaps(gui.mainMenu, ID_MENU_IP_LIST, MF_BYCOMMAND, gui.hbIcons[ID_ICON_IPLIST], gui.hbIcons[ID_ICON_IPLIST]);
			SetMenuItemBitmaps(gui.mainMenu, ID_MENU_UPDATE, MF_BYCOMMAND, gui.hbIcons[ID_ICON_UPDATE], gui.hbIcons[ID_ICON_UPDATE]);
			SetMenuItemBitmaps(gui.trayMenu, ID_MENU_IP_LIST, MF_BYCOMMAND, gui.hbIcons[ID_ICON_IPLIST], gui.hbIcons[ID_ICON_IPLIST]);
			SetMenuItemBitmaps(gui.trayMenu, ID_MENU_TRAY_HIDE, MF_BYCOMMAND, gui.hbIcons[ID_ICON_TRAY], gui.hbIcons[ID_ICON_TRAY]);
			SetMenuItemBitmaps(gui.trayMenu, ID_MENU_EXIT, MF_BYCOMMAND, gui.hbIcons[ID_ICON_CLOSE], gui.hbIcons[ID_ICON_CLOSE]);

			ModifyMenu(gui.mainMenu, ID_MENU_IP_LIST, MF_STRING, ID_MENU_IP_LIST, tools.languageTable[15]);
			ModifyMenu(gui.mainMenu, ID_MENU_OPTIONS, MF_STRING, ID_MENU_OPTIONS, tools.languageTable[36]);
			ModifyMenu(gui.mainMenu, ID_MENU_EXIT, MF_STRING, ID_MENU_EXIT, tools.languageTable[14]);
			ModifyMenu(gui.mainMenu, ID_MENU_UPDATE, MF_STRING, ID_MENU_UPDATE, tools.languageTable[71]);

			SendDlgItemMessage(hWnd, ID_DLG_CHANGE_TITLE, WM_SETTEXT, 0, (LPARAM)tools.languageTable[57]);
			SendDlgItemMessage(hWnd, ID_DLG_CHANGE_IP, WM_SETTEXT, 0, (LPARAM)tools.languageTable[58]);
			SendDlgItemMessage(hWnd, ID_DLG_SHOW_SERVER_INFO, WM_SETTEXT, 0, (LPARAM)tools.languageTable[17]);
			break;
		}

		case WM_COMMAND:
			switch(LOWORD(wParam)) 
			{
				case ID_MENU_EXIT:
					SendMessage(hWnd, WM_DESTROY, 0, 0);
					break;

				case ID_MENU_UPDATE:
					tools.updateXmlAddresses();
					break;

				case ID_DLG_REFRESH_LIST:
				{
					SendDlgItemMessage(hWnd, ID_DLG_IP, CB_RESETCONTENT, 0 , 0);
					if(tools.loadFromXmlIpList())
					{
						for(int i = 0; i < MAX_SERVERS_IN_LIST; i++)
						{
							if(!tools.IpListAddr[i].empty())
							{
								SendDlgItemMessage(hWnd, ID_DLG_IP, CB_ADDSTRING, 0 , (LPARAM)tools.IpListAddr[i].c_str());
							}
						}
						SendDlgItemMessage(hWnd, ID_DLG_IP, CB_SELECTSTRING, 0, (LPARAM)tools.IpListAddr[0].c_str());
					}
					else
						SetDlgItemText(hWnd, ID_DLG_IP, "127.0.0.1");
					break;
				}

				case ID_DLG_CHANGE_IP:
				{
					char ipAddress[255], port[10];
					if(GetDlgItemText(hWnd, ID_DLG_IP, ipAddress, sizeof(ipAddress)))
					{
						GetDlgItemText(hWnd, ID_DLG_PORT, port, sizeof(port));
						if(HIWORD(wParam) == BN_CLICKED)
						{
							if(SendDlgItemMessage(hWnd, ID_DLG_CHANGE_TITLE, BM_GETCHECK, 0, 0))
							{
								if(tools.setNewConnection(ipAddress, (unsigned short)atoi(port), true))
								{
									if(tools.getShowMessageBox())
										gui.messageBox(MESSAGE_TYPE_INFO, NAME, tools.languageTable[37], ipAddress, port);
								}
							}
							else
							{
								if(tools.setNewConnection(ipAddress, (unsigned short)atoi(port), false))
								{
									if(tools.getShowMessageBox())
										gui.messageBox(MESSAGE_TYPE_INFO, NAME, tools.languageTable[37], ipAddress, port);
								}
							}
						}
					}
					break;
				}

				case ID_DLG_SHOW_SERVER_INFO:
					tools.setCheckFromList(false);
					DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_SERVER_INFO), HWND_DESKTOP, ServerInfoProc);
					break;

				case ID_MENU_OPTIONS:
					ShowWindow(gui.optionsWindow, SW_SHOW);
					break;

				case ID_MENU_IP_LIST:
					DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_IP_LIST), HWND_DESKTOP, serverList);
					break;

				case ID_MENU_TRAY_HIDE:
				{
					if(gui.minimized)
					{
						ShowWindow(hWnd, SW_SHOW);
						ShowWindow(hWnd, SW_RESTORE);
						ModifyMenu(gui.trayMenu, ID_MENU_TRAY_HIDE, MF_STRING, ID_MENU_TRAY_HIDE, tools.languageTable[13]);
						gui.minimized = false;
					}
					else
					{
						ShowWindow(hWnd, SW_HIDE);
						ModifyMenu(gui.trayMenu, ID_MENU_TRAY_HIDE, MF_STRING, ID_MENU_TRAY_HIDE, tools.languageTable[12]);
						gui.minimized = true;
					}
					break;
				}
			}
			break;

		case WM_CLOSE:
		case WM_DESTROY:
		{
			DestroyMenu(gui.ipListMenu);
			DestroyMenu(gui.ipListMenu2);
			DestroyMenu(gui.trayMenu);
			Shell_NotifyIcon(NIM_DELETE, &gui.trayIcon);
			PostQuitMessage(0);
			break;
		}
	}
	return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	INITCOMMONCONTROLSEX iccex;
	iccex.dwICC = ICC_WIN95_CLASSES;
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCommonControlsEx(&iccex);

	if(!tools.loadLanguageStrings(LANGUAGE_FILE))
	{
		gui.messageBox(MESSAGE_TYPE_FATAL_ERROR, NAME, "Could not load \"%s\" file!\nFile prodably doesn't exists!", LANGUAGE_FILE);
		return 0;
	}

	if(!tools.loadFromXmlAddresses())
	{
		gui.messageBox(MESSAGE_TYPE_FATAL_ERROR, NAME, tools.languageTable[42], ADDRESSES_FILE);
		return 0;
	}

	if(tools.getSupportForOTServList())
		tools.addSupportForOTServList();

	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	char argList[10][1024];
	for(int i = 0; i < nArgs; i++)
		strcpy(argList[i], tools.WStringToString(szArglist[i]).c_str());
	LocalFree(szArglist);

	if(nArgs > 2)
	{
		if(!parseCommandLine(std::vector<std::string>(argList, argList + nArgs)))
			return 0;

		if(tools.setNewConnection(tools.cmdLineIP, (unsigned short)atoi(tools.cmdLinePort), tools.getChangeTitleCmdLine()))
		{
			if(tools.getShowMessageBox())
			{
				gui.messageBox(MESSAGE_TYPE_INFO, NAME, tools.languageTable[37], tools.cmdLineIP, tools.cmdLinePort);
			}
			return 0;
		}
	}
	else if(nArgs > 1)
	{
		char pIp[255], pPort[10], pClient[10];
		char* pStr = argList[1];

		pStr = strtok (pStr, "://");
		pStr = strtok(NULL, "/");
		strcpy(pIp, pStr);
		pStr = strtok(NULL, "/");
		strcpy(pPort, pStr);
		pStr = strtok(NULL, "/");
		strcpy(pClient, pStr);

		if(pPort != NULL && pIp != NULL && pClient != NULL)
		{
			if(tools.setNewConnection(pIp, (unsigned short)atoi(pPort), tools.getChangeTitleCmdLine()))
			{
				if(tools.getShowMessageBox())
					gui.messageBox(MESSAGE_TYPE_INFO, NAME, tools.languageTable[40], pIp, pPort, pClient);
			}
			return 0;
		}
	}
	else
	{
		WNDCLASSEX wincl;

		wincl.hInstance = hInstance;
		wincl.lpszClassName = "OTFansPlIpChangerClass";
		wincl.lpfnWndProc = NULL;
		wincl.style = CS_DBLCLKS;
		wincl.cbSize = sizeof(WNDCLASSEX);

		wincl.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON));
		wincl.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_ICON), IMAGE_ICON, 16, 16, 0);
		wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
		wincl.lpszMenuName = NULL;
		wincl.cbClsExtra = 0;
		wincl.cbWndExtra = 0;
		wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

		if(!RegisterClassEx(&wincl))
			return 0;

		gui.hInst = hInstance;
		gui.mainMenu = GetSubMenu(LoadMenu(gui.hInst, MAKEINTRESOURCE(ID_MAIN_MENU)), 0);
		CreateDialog(gui.hInst, MAKEINTRESOURCE(ID_DLG_OPTIONS_WINDOW), NULL, OptionsWindow);
		DialogBoxA(gui.hInst, MAKEINTRESOURCE(ID_DLG_MAIN_GUI), HWND_DESKTOP, MainWindowProc);
		return 0;
	}
	return 0;
}
