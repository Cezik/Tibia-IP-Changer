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

#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <windows.h>
#include <Psapi.h>
#include <string>
#include <vector>
#include <winsock.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <libxml/parser.h>
#include <Tlhelp32.h>
#include <list>

#include "resource.h"
#include "IpChanger.h"

#ifdef XML_GCC_FREE
	#define xmlFree(s) free(s)
#else
	#define xmlFree(s) xmlFree(s)
#endif

#define CONFIG_FILE "IPChanger.xml"
#define SERVER_LIST_FILE "IPList.xml"
#define ADDRESSES_FILE "Addresses.xml"
#define LANGUAGE_FILE "Languages.xml"
#define SECTION "OTFans"

struct addressReading
{
	uint32_t rsaAddr;
	uint32_t ipAddr;
	uint32_t loginServers;
	char* protocol;
	bool isUsed;
};

struct languageTable_s
{
	char* language[25];
	char languageStrings[LANGUAGE_STRINGS+1][512];
	char* author[25];
};

struct serversList_s
{
	std::string ipList;
	std::string portList;
};

class Tools
{
	public:
		Tools();
		~Tools();

		bool getCheckFromList() const {return checkFromList;}

		void setCheckFromList(bool value){checkFromList = value;}

		bool fileExists(const char* fileName);
		std::string getFilePath(const char* fileName);

		void replaceString(std::string& str, const std::string sought, const std::string replacement);
		bool isCharInteger(const char* character);
		std::string WStringToString(const std::wstring s);

		SOCKET createSocket(HWND hwnd);
		SOCKADDR_IN sSAddrCreate(const char* ipAddress, uint16_t iPort);
		void getFileVersion(char* fileName, VS_FIXEDFILEINFO* pvsf);

		bool addSupportForOTServList();
		bool parseCommandLine(std::vector<std::string> args);

		void StringToke(HWND hwnd, char* szBuffer);
		std::string getClientVersion(HANDLE procHandle);

		bool writeString(HANDLE procHandle, DWORD lpBaseAddress, const char* lpBuffer);
		bool writeByte(HANDLE procHandle, DWORD lpBaseAddress, const int lpBuffer, const int nSize);

		bool setRSA(HANDLE procHandle, const DWORD rsaAddr, const char newRsaKey[]);
		bool changeIP(HANDLE procHandle, const char* newIP, const DWORD loginAddress, uint16_t maxLoginServers);
		bool changePort(HANDLE procHandle, uint16_t newPort, const DWORD loginAddress, uint16_t maxLoginServers);
		bool setNewConnection(const char* newIP, uint16_t newPort, bool changeTitle);

		bool utf8ToLatin1(char* intext, std::string& outtext);
		bool readXMLString(xmlNodePtr node, const char* tag, std::string& value);
		bool readXMLInteger(xmlNodePtr node, const char* tag, int& value);
		bool readXMLContentString(xmlNodePtr node, std::string& value);

		bool loadFromXmlIpList();
		bool setLanguage(const char* languageName);

		std::string getExeDir();

		SOCKET sSock;
		SOCKADDR_IN SA;
		bool bSend, bRecv;

		std::string rsaKey;

		char* cmdLineIP;
		char* cmdLinePort;
		char languageTable[LANGUAGE_STRINGS+1][512];

		VS_FIXEDFILEINFO vsf;
 
		std::list<serversList_s> servList;

		bool loadFromXmlAddresses();
		std::list<addressReading> addrReading;

		bool updateXmlAddresses();

		DWORD GetModuleBase(DWORD processID);
		DWORD AlignAddress(DWORD processID, DWORD address);

		bool loadLanguageStringsFromXML();
		std::list<languageTable_s> languageList;
		std::string m_defaultLanguage;

		bool loadSettingsFromXML();

		std::string getCustomRsaKey() const{return m_customRsaKey;}
		void setCustomRsaKey(std::string value){m_customRsaKey = value;}

		std::string getCustomTitle() const{return m_customTitle;}
		void setCustomTitle(std::string value){m_customTitle = value;}

		void setShowMessageBoxSetting(bool value) {m_showMessageBox = value;}
		void setChangeTitleSetting(bool value) {m_changeTitle = value;}
		void setShowToolTipsSetting(bool value) {m_showToolTips = value;}
		void setURLProtocolSetting(bool value) {m_URLProtocol = value;}
		void setOtherRsaKeySetting(bool value) {m_otherRsaKey = value;}

		bool getShowMessageBoxSetting() const {return m_showMessageBox;}
		bool getChangeTitleSetting() const {return m_changeTitle;}
		bool getShowToolTipsSetting() const {return m_showToolTips;}
		bool getURLProtocolSetting() const {return m_URLProtocol;}
		bool getOtherRsaKeySetting() const {return m_otherRsaKey;}

	protected:
		bool checkFromList;
		bool m_showMessageBox;
		bool m_changeTitle;
		bool m_showToolTips;
		bool m_URLProtocol;
		bool m_otherRsaKey;
		std::string m_customTitle;
		std::string m_customRsaKey;
};

#endif
