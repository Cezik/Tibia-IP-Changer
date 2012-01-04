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

#include "resource.h"
#include "IpChanger.h"

#ifdef XML_GCC_FREE
	#define xmlFree(s) free(s)
#else
	#define xmlFree(s) xmlFree(s)
#endif

#define CONFIG_FILE "IPChanger.ini"
#define SERVER_LIST_FILE "IPList.xml"
#define ADDRESSES_FILE "Addresses.xml"
#define LANGUAGE_FILE "Language.ini"
#define SECTION "OTFans"

struct addressReading
{
	uint32_t rsaAddr;
	uint32_t ipAddr;
	uint32_t loginServers;
	char* protocol;
	bool isUsed;
};

class Tools
{
	public:
		Tools();
		~Tools();

		bool getUseOtherRSA() const {return useOtherRSA;}
		bool getCheckFromList() const {return checkFromList;}
		bool getShowMessageBox() const {return showMessageBox;}
		bool getChangeTitleCmdLine() const {return changeTitleCmdLine;}
		bool getSupportForOTServList() const {return supportForOTServList;}
		bool getShowToolTips() const {return showToolTips;}

		void setUseOtherRSA(bool value){useOtherRSA = value;}
		void setCheckFromList(bool value){checkFromList = value;}
		void setShowMessageBox(bool value){showMessageBox = value;}
		void setChangeTitleCmdLine(bool value){changeTitleCmdLine = value;}
		void setSupportForOTServList(bool value){supportForOTServList = value;}
		void setShowToolTips(bool value){showToolTips = value;}

		bool fileExists(const char* fileName);
		std::string getFilePath(const char* fileName);

		int32_t readInteger(const char* key);
		std::string readString(const char* key);
		std::string readStringFromFile(const char* fileName, const char* section, const char* key);
		bool readBoolean(const char* key);

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

		bool loadFromXmlIpList();
		bool loadLanguageStrings(const char* fileName);

		std::string getExeDir();

		SOCKET sSock;
		SOCKADDR_IN SA;
		bool bSend, bRecv;

		std::string rsaKey;

		char* cmdLineIP;
		char* cmdLinePort;
		char languageTable[LANGUAGE_STRINGS+1][512];

		VS_FIXEDFILEINFO vsf;

		std::string IpListAddr[MAX_SERVERS_IN_LIST], IpListPort[MAX_SERVERS_IN_LIST];

		bool loadFromXmlAddresses();
		addressReading rAddr[MAX_AMOUNT_OF_PROTOCOLS];

		bool updateXmlAddresses();

		DWORD GetModuleBase();
		DWORD AlignAddress(DWORD address);

	protected:
		bool useOtherRSA, checkFromList, showMessageBox, changeTitleCmdLine, supportForOTServList, showToolTips;
};

#endif
