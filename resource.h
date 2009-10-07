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

#define TTM_SETTITLE (WM_USER + 32)

#define NAME "IP Changer"
#define CURRENT_VERSION  1,0,8,32
#define CURRENT_VERSION2 "1.0.8.32\0"

#define ID_STATIC -1

#define ID_ICON_CLOSE 10
#define ID_ICON_OPTIONS 11
#define ID_ICON_IPLIST 12
#define ID_ICON_TRAY 13
#define ID_ICON_ADD 14
#define ID_ICON_EDIT 15
#define ID_ICON_DELETE 16
#define ID_ICON_DELETE_ALL 17
#define ID_ICON_INFORMATION 18
#define ID_ICON_CHANGEIP 19

#define ID_ICON_LAST ID_ICON_CHANGEIP + 1

#define ID_DLG_LISTVIEW 99
#define ID_ICON 100
#define ID_TRAY_ICON 101

#define WM_SOCKET WM_USER+1
#define WM_SHELLNOTIFY WM_USER+5

#define ID_MAIN_MENU 2000
#define ID_MENU_EXIT 2001
#define ID_MENU_OPTIONS 2002
#define ID_MENU_IP_LIST 2003
#define ID_MENU_CLOSE_WINDOW 2004
#define ID_MENU_ADD_SERVER 2005
#define ID_MENU_EDIT_SERVER 2006
#define ID_MENU_DELETE_SERVER 2007
#define ID_MENU_SERVER_INFO 2008
#define ID_MENU_CHANGE_IP 2009
#define ID_MENU_DELETE_ALL_SERVERS 2010
#define ID_MENU_TRAY_HIDE 2011

#define ID_DLG_MAIN_GUI 101
#define ID_DLG_IP 102
#define ID_DLG_PORT 103
#define ID_DLG_CHANGE_IP 104
#define ID_DLG_LINK 105
#define ID_DLG_CHANGE_TITLE 106
#define ID_DLG_SHOW_SERVER_INFO 107
#define ID_DLG_REFRESH_LIST 108

#define ID_DLG_SERVER_INFO 200
#define ID_DLG_SERVER 201

#define ID_DLG_OPTIONS_WINDOW 202
#define ID_DLG_RSA_SAVE 203
#define ID_DLG_RSA_EDIT 204
#define ID_DLG_RSA_DEFAULT 205
#define ID_DLG_RSA_USE_OTHER 206
#define ID_DLG_RSA_LOAD 207
#define ID_DLG_RSA_USE_THIS_ONE 208
#define ID_DLG_OPTIONS_EDIT_TITLE 209
#define ID_DLG_PROTOCOL_TXT 210
#define ID_DLG_IPADDR_TXT 211
#define ID_DLG_PORT_TXT 212

#define ID_DLG_IP_LIST 213
#define ID_DLG_ADD_SERVER 214
#define ID_DLG_ADD_ADD 215
#define ID_DLG_ADD_CLOSE 216
#define ID_DLG_ADD_ADDRESS 217
#define ID_DLG_ADD_PORT 218

#define ID_DLG_EDIT_SERVER 219
#define ID_DLG_EDIT_ADDRESS 220
#define ID_DLG_EDIT_PORT 221
#define ID_DLG_EDIT_SAVE 222
#define ID_DLG_EDIT_CLOSE 223
#define ID_DLG_TITLE_SAVE 224
#define ID_DLG_TITLE_LOAD 225
#define ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE 226
#define ID_DLG_OPTIONS_SHOW_WINDOW_CMD_LINE_SAVE 227
#define ID_DLG_OPTIONS_CHANGE_WINDOW_CMD_LINE 228
#define ID_DLG_OPTIONS_OTSERV_SUPPORT 229
#define ID_DLG_OPTIONS_SHOW_TOOLTIPS 230

#define ID_TMR_SOCK_CON 1000
#define ID_TMR_SOCK_TIMEOUT 1001
