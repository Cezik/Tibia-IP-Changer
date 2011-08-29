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

#include "Tools.h"
#include "WinGUI.h"

extern WinGUI gui;

Tools::Tools()
{
	bSend = true;
	bRecv = false;
	useOtherRSA = false;
	checkFromList = false;
	setShowMessageBox(readBoolean("ShowMessageBox"));
	setChangeTitleCmdLine(readBoolean("ChangeTibiaTitleCmdLine"));
	setSupportForOTServList(readBoolean("SupportForOTServList"));
	setShowToolTips(readBoolean("ShowToolTips"));
	setURLToAddresses(readString("UpdateURL"));
}

Tools::~Tools()
{
	//
}

/// <summary>
/// Checking that file exists
/// </summary>
bool Tools::fileExists(const char* fileName)
{
	std::ifstream file(getFilePath(fileName).c_str());
	if(!file.fail())
		return true;
	return false;
}

/// <summary>
/// Reads integer value from configuration file
/// </summary>
long Tools::readInteger(const char* key)
{
	return GetPrivateProfileInt(SECTION, key, -1, getFilePath(CONFIG_FILE).c_str());
}

/// <summary>
/// Reads string value from configuration file
/// </summary>
std::string Tools::readString(const char* key)
{
	char buffer[4096];
	GetPrivateProfileString(SECTION, key, "", buffer, sizeof(buffer), getFilePath(CONFIG_FILE).c_str());
	return buffer;
}

/// <summary>
/// Reads string value from *.ini file
/// </summary>
std::string Tools::readStringFromFile(const char* fileName, const char* section, const char* key)
{
	char buffer[4096];
	GetPrivateProfileString(section, key, "", buffer, sizeof(buffer), getFilePath(fileName).c_str());
	return buffer;
}

/// <summary>
/// Reads boolean value from configuration file
/// </summary>
bool Tools::readBoolean(const char* key)
{
	if(readInteger(key) > 0)
		return true;
	return false;
}

/// <summary>
/// Replace string for another one
/// </summary>
void Tools::replaceString(std::string& str, const std::string sought, const std::string replacement)
{
	size_t pos = 0, start = 0, soughtLen = sought.length(),	replaceLen = replacement.length();
	while((pos = str.find(sought, start)) != std::string::npos)
	{
		str = str.substr(0, pos) + replacement + str.substr(pos + soughtLen);
		start = pos + replaceLen;
	}
}

/// <summary>
/// Check that string is a number
/// </summary>
bool Tools::isCharInteger(const char* character)
{
	switch(character[0])
	{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return true;
			break;
		default:
			return false;
	}
}

/// <summary>
/// Converts "WString" to "String"
/// </summary>
std::string Tools::WStringToString(const std::wstring s)
{
	std::string temp(s.begin(), s.end());
	return temp;
}

/// <summary>
/// Creating socket connection
/// </summary>
SOCKET Tools::createSocket(HWND hwnd)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	SOCKET sSock = socket(AF_INET, SOCK_STREAM, 0);
	WSAAsyncSelect(sSock, hwnd, WM_SOCKET, (FD_CLOSE | FD_CONNECT | FD_READ | FD_WRITE));
	return sSock;
}

/// <summary>
/// Adding IP and port to socket connection
/// </summary>
SOCKADDR_IN Tools::sSAddrCreate(const char* ipAddress, unsigned short iPort)
{
	SOCKADDR_IN SA;
	HOSTENT* HE = gethostbyname(ipAddress);

	if(!HE)
		SA.sin_addr.s_addr = inet_addr(ipAddress);
	else
		SA.sin_addr = *((in_addr*)HE->h_addr);

	SA.sin_family = AF_INET;
	SA.sin_port = htons(iPort);
	return SA;
}

/// <summary>
/// Getting file version
/// </summary>
void Tools::getFileVersion(char* fileName, VS_FIXEDFILEINFO* pvsf)
{
	DWORD dwHandle;
	DWORD cchver = GetFileVersionInfoSize(fileName, &dwHandle);
	char* pver = new char[cchver];
	GetFileVersionInfo(fileName, dwHandle, cchver, pver);
	UINT uLen;
	void *pbuf;
	VerQueryValue(pver, "\\", &pbuf, &uLen);
	memcpy(pvsf, pbuf, sizeof(VS_FIXEDFILEINFO));
	delete[] pver;
}

/// <summary>
/// Adding registry key to program to be supported with otservlist.org [URL Protocol]
/// </summary>
bool Tools::addSupportForOTServList()
{
	HKEY hKey, hKey2;
	DWORD dwDisp, dwDisp2, dwData;
	if(RegCreateKeyEx(HKEY_CLASSES_ROOT, "OTSERV\\shell\\open\\command", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey, &dwDisp) == ERROR_SUCCESS)
	{
		char szAppPath[MAX_PATH];
		GetModuleFileName(0, szAppPath, sizeof(szAppPath));
		strcat(szAppPath, " %1");
		if(RegSetValueEx(hKey, NULL, 0, REG_SZ, (LPBYTE)szAppPath, (DWORD)strlen(szAppPath)) == ERROR_SUCCESS)
		{
			if(RegCreateKeyEx(HKEY_CLASSES_ROOT, "OTSERV", 0, 0, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 0, &hKey2, &dwDisp2) == ERROR_SUCCESS)
			{
				char newValue[MAX_PATH];
				strcpy(newValue, "URL:Open Tibia Server");
				if(RegSetValueEx(hKey2, NULL, 0, REG_SZ, (LPBYTE)newValue, (DWORD)strlen(newValue)) == ERROR_SUCCESS)
				{
					ZeroMemory(newValue, sizeof(newValue));
					if(RegSetValueEx(hKey2, "Source Filter", 0, REG_SZ, NULL, 0) == ERROR_SUCCESS)
					{
						if(RegSetValueEx(hKey2, "URL Protocol", 0, REG_SZ, NULL, 0) == ERROR_SUCCESS)
						{
							dwData = 2;
							if(RegSetValueEx(hKey2, "EditFlags", 0, REG_DWORD, (LPBYTE)&dwData, sizeof(DWORD)) == ERROR_SUCCESS)
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

/// <summary>
/// Parsing server information
/// </summary>
void Tools::StringToke(HWND hwnd, char* szBuffer)
{
	 char cSeparator[] = "\"\"";
	 char* token;
	 char* LastToken;
	 char* Uptime = TEXT(languageTable[4]);
	 char* Ip = TEXT(languageTable[4]);
	 char* ServerName = TEXT(languageTable[4]);
	 char* Port = TEXT(languageTable[4]);
	 char* Location = TEXT(languageTable[4]);
	 char* Website = TEXT(languageTable[4]);
	 char* ServerType = TEXT(languageTable[4]);
	 char* Version = TEXT(languageTable[4]);
	 char* Protocol = TEXT(languageTable[4]);
	 char* Owner = TEXT(languageTable[4]);
	 char* Email = TEXT(languageTable[4]);
	 char* PlayersOnline = TEXT(languageTable[4]);
	 char* PlayersMax = TEXT(languageTable[4]);
	 char* PlayersPeak = TEXT(languageTable[4]);
	 char* MonstersTotal = TEXT(languageTable[4]);
	 char* MapName = TEXT(languageTable[4]);
	 char* MapAuthor = TEXT(languageTable[4]);
	 char* MapWidth = TEXT(languageTable[4]);
	 char* MapHeight = TEXT(languageTable[4]);
	 char* Motd = TEXT(languageTable[4]);
	 unsigned long nSeconds = 0, nDays = 0, nHours = 0, nMinutes = 0, nHOut = 0, nMOut = 0, nSOut = 0;
	 token = strtok(szBuffer, cSeparator);

	 while(token != NULL)
	 {
		 LastToken = token;
		 token = strtok(NULL, cSeparator);
		 if(token == NULL)
			 break;

		 if(strcmp(LastToken, "><serverinfo uptime=") == 0)
			 Uptime = token;
		 if(strcmp(LastToken, " ip=") == 0)
			 Ip = token;
		 if(strcmp(LastToken, " servername=") == 0)
			 ServerName = token;
		 if(strcmp(LastToken, " port=") == 0)
			 Port = token;
		 if(strcmp(LastToken, " location=") == 0)
			 Location = token;
		 if(strcmp(LastToken, " url=" ) == 0)
			 Website = token;
		 if(strcmp(LastToken, " server=") == 0)
			 ServerType = token;
		 if(strcmp(LastToken, " version=") == 0)
			 Version = token;
		 if(strcmp(LastToken, " client=") == 0)
			 Protocol = token;
		 if(strcmp(LastToken, "/><owner name=") == 0)
			 Owner = token;
		 if(strcmp(LastToken, " email=") == 0)
			 Email = token;
		 if(strcmp(LastToken, "/><players online=") == 0)
			 PlayersOnline = token;
		 if(strcmp(LastToken, " max=") == 0)
			 PlayersMax = token;
		 if(strcmp(LastToken, " peak=") == 0)
			 PlayersPeak = token;
		 if(strcmp(LastToken, "/><monsters total=") == 0)
			 MonstersTotal = token;
		 if(strcmp(LastToken, "/><map name=") == 0)
			 MapName = token;
		 if(strcmp(LastToken, " author=") == 0)
			 MapAuthor = token;
		 if(strcmp(LastToken, " width=") == 0)
			 MapWidth = token;
		 if(strcmp(LastToken, " height=") == 0)
			 MapHeight = token;
		 if(token[0] == '/' && token[3] == 'm' && token[4] == 'o' && token[5] == 't')
			 Motd = token;
	 }

	 if(!isCharInteger(&Uptime[1]))
		 Uptime = TEXT(languageTable[4]);
	 else
	 {
		 std::istringstream iss(Uptime);
		 if(iss >> nSeconds)
		 {
			 nMinutes = nSeconds / 60;
			 nHours = nMinutes / 60;
			 nDays = nHours / 24;
			 nHOut = nHours - (nDays * 24);
			 nMOut = nMinutes - (nHours * 60);
			 nSOut = nSeconds - (nMinutes * 60);
		 }
	 }

	 if(!isCharInteger(&Ip[1]))
		 Ip = TEXT(languageTable[4]);
	 if(ServerName[5] == '=')
		 ServerName = TEXT(languageTable[4]);
	 if(!isCharInteger(&Port[1]))
		 Port = TEXT(languageTable[4]);
	 if(Location[4] == '=')
		 Location = TEXT(languageTable[4]);
	 if(Website[7] == '=')
		 Website = TEXT(languageTable[4]);
	 if(ServerType[8] == '=')
		 ServerType = TEXT(languageTable[4]);
	 if(!isCharInteger(&Protocol[0]))
		 Protocol = TEXT(languageTable[4]);
	 if(Owner[6] == '=')
		 Owner = TEXT(languageTable[4]);
	 if(Email[0] == '/')
		 Email = TEXT(languageTable[4]);
	 if(!isCharInteger(&PlayersOnline[0]))
		 PlayersOnline = TEXT(languageTable[4]);
	 if(!isCharInteger(&PlayersMax[0]))
		 PlayersMax = TEXT(languageTable[4]);
	 if(!isCharInteger(&PlayersPeak[0]))
		 PlayersPeak = TEXT(languageTable[4]);
	 if(!isCharInteger(&MonstersTotal[0]))
		 MonstersTotal = TEXT(languageTable[4]);
	 if(MapName[7] == '=')
		 MapName = TEXT(languageTable[4]);
	 if(MapAuthor[6] == '=')
		 MapAuthor = TEXT(languageTable[4]);
	 if(!isCharInteger(&MapWidth[0]))
		 MapWidth = TEXT(languageTable[4]);
	 if(!isCharInteger(&MapHeight[0]))
		 MapHeight = TEXT(languageTable[4]);

	 char cMotd[4086];
	 char cMotdOut[4086];
	 sprintf(cMotd, "%s", Motd);

	 for(int i = 8; i < 1024; i++)
	 {
		 if(cMotd[i] == '<')
			 break;
		 cMotdOut[i-8] = cMotd[i];
	 }

	 std::stringstream str;
	 str << "Uptime: " << nDays << "d " << nHOut << "h " << nMOut << "m " << nSOut << "s"
			<< "\r\nIP: " << Ip
			<< "\r\nPort: " << Port
			<< "\r\n" << languageTable[35] << " " << Protocol
			<< "\r\n" << languageTable[34] << " " << Location
			<< "\r\n\r\n" << languageTable[33] << " " << ServerName
			<< "\r\n" << languageTable[32] << " " << Website
			<< "\r\n" << languageTable[31] << " " << Owner
			<< "\r\n" << languageTable[30] << " " << Email
			<< "\r\n\r\n" << languageTable[29] << " " << PlayersOnline << "/" << PlayersMax
			<< "\r\n" << languageTable[28] << " " << PlayersPeak
			<< "\r\n" << languageTable[27] << " " << MonstersTotal
			<< "\r\n" << languageTable[26] << " " << MapAuthor
			<< "\r\n" << languageTable[25] << " " << MapName
			<< "\r\n" << languageTable[24] << " " << MapWidth << "x" << MapHeight
			<< "\r\n\r\n" << languageTable[23] << " " << cMotdOut;
	 SetDlgItemText(hwnd, ID_DLG_SERVER, str.str().c_str());
}

/// <summary>
/// Getting a program version
/// </summary>
std::string Tools::getClientVersion(HANDLE procHandle)
{
	char filePath[MAX_PATH], fileVersion[5];
	GetModuleFileNameEx(procHandle, NULL, filePath, sizeof(filePath));
	getFileVersion(filePath, &vsf);
	sprintf(fileVersion, "%u%u%u", vsf.dwFileVersionMS >> 16, vsf.dwFileVersionMS & 0xFFFF, vsf.dwFileVersionLS >> 16);
	return fileVersion;
}

/// <summary>
/// Replace address for string value
/// </summary>
bool Tools::writeString(HANDLE procHandle, DWORD lpBaseAddress, const char* lpBuffer)
{
	if(WriteProcessMemory(procHandle, (LPVOID)lpBaseAddress, lpBuffer, strlen(lpBuffer) + 1, NULL))
		return true;
	return false;
}

/// <summary>
/// Replace address for integer value
/// </summary>
bool Tools::writeByte(HANDLE procHandle, DWORD lpBaseAddress, const int lpBuffer, const int nSize)
{
	if(WriteProcessMemory(procHandle, (LPVOID)lpBaseAddress, &lpBuffer, nSize, NULL))
		return true;
	return false;
}

/// <summary>
/// Changing RSA key of Tibia client
/// </summary>
bool Tools::setRSA(HANDLE procHandle, const DWORD rsaAddr, const char newRsaKey[])
{
	DWORD oldProtection, newProtection;
	if(VirtualProtectEx(procHandle, (LPVOID)rsaAddr, strlen(newRsaKey), PAGE_EXECUTE_READWRITE, &oldProtection))
	{
		if(WriteProcessMemory(procHandle, (LPVOID)rsaAddr, newRsaKey, strlen(newRsaKey), NULL))
		{
			if(VirtualProtectEx(procHandle, (LPVOID)rsaAddr, strlen(newRsaKey), oldProtection, &newProtection))
				return true;
		}
	}
	return false;
}

/// <summary>
/// Changing IP of Tibia client
/// </summary>
bool Tools::changeIP(HANDLE procHandle, const char* newIP, const DWORD loginAddress, unsigned short maxLoginServers)
{
	DWORD addr = loginAddress;
	for(int i = 0; i < maxLoginServers; i++)
	{
		if(!writeString(procHandle, addr, newIP))
			return false;

		addr += IpDistance;
	}
	return true;
}

/// <summary>
/// Changing port of Tibia client
/// </summary>
bool Tools::changePort(HANDLE procHandle, unsigned short newPort, const DWORD loginAddress, unsigned short maxLoginServers)
{
	DWORD addr = loginAddress;
	for(int i = 0; i < maxLoginServers; i++)
	{
		if(!writeByte(procHandle, addr + PortDistance, newPort, 4))
			return false;

		addr += IpDistance;
	}
	return true;
}

/// <summary>
/// Changing IP, port, RSA key and title of Tibia client
/// </summary>
bool Tools::setNewConnection(const char* newIP, unsigned short newPort, bool changeTitle)
{
	HWND tibiaWindow = FindWindow("TibiaClient", NULL);
	if(tibiaWindow != NULL)
	{
		DWORD pID;
		GetWindowThreadProcessId(tibiaWindow, &pID);
		HANDLE procHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);

		if(procHandle == NULL)
			return false;

		char* clientVersion;
		clientVersion = new char[getClientVersion(procHandle).size()];
		strcpy(clientVersion, getClientVersion(procHandle).c_str());

		for(int i = 0; i <= MAX_AMOUNT_OF_PROTOCOLS; i++)
		{
			if(strcmp(clientVersion, rAddr[i].protocol) == 0)
			{
				if(rAddr[i].isUsed)
				{
					const DWORD rsaAddr = rAddr[i].rsaAddr;
					const DWORD ipAddr = rAddr[i].ipAddr;
					const short loginServers = rAddr[i].loginServers;
					if(rsaAddr != 0x00)
					{
						if(getUseOtherRSA())
							setRSA(procHandle, rsaAddr, rsaKey.c_str());
						else
							setRSA(procHandle, rsaAddr, OTSERV_RSA_KEY);
					}
					if(changeIP(procHandle, newIP, ipAddr, loginServers) && changePort(procHandle, newPort, ipAddr, loginServers))
					{
						if(changeTitle)
						{
							std::string getCustomTibiaText = readString("CustomTibiaTitle");
							replaceString(getCustomTibiaText, "$protocol$", clientVersion);
							replaceString(getCustomTibiaText, "$ipaddress$", newIP);
							char newPort2[10];
							_itoa(newPort, newPort2, 10);
							replaceString(getCustomTibiaText, "$port$", newPort2);
							SetWindowTextA(tibiaWindow, getCustomTibiaText.c_str());
						}
						else
							SetWindowTextA(tibiaWindow, "Tibia");
						CloseHandle(procHandle);
						return true;
					}
				}
			}
		}
	}
	else
	{
		gui.messageBox(MESSAGE_TYPE_FATAL_ERROR, NULL, languageTable[1]);
		return false;
	}
	return false;
}

bool Tools::utf8ToLatin1(char* intext, std::string& outtext)
{
	outtext.clear();
	if(intext == NULL)
		return false;
	size_t inlen  = strlen(intext);
	if(inlen == 0)
		return false;
	size_t outlen = inlen*2;
	unsigned char* outbuf = new unsigned char[outlen];
	int res = UTF8Toisolat1(outbuf, (int*)&outlen, (unsigned char*)intext, (int*)&inlen);
	if(res < 0)
	{
		delete[] outbuf;
		return false;
	}
	outbuf[outlen] = '\0';
	outtext = (char*)outbuf;
	delete[] outbuf;
	return true;
}

/// <summary>
/// Reads string value from *.xml file
/// </summary>
bool Tools::readXMLString(xmlNodePtr node, const char* tag, std::string& value)
{
	char* nodeValue = (char*)xmlGetProp(node, (xmlChar*)tag);
	if(nodeValue)
	{
		if(!utf8ToLatin1(nodeValue, value))
			value = nodeValue;
		xmlFree(nodeValue);
		return true;
	}

	return false;
}

/// <summary>
/// Reads integer value from *.xml file
/// </summary>
bool Tools::readXMLInteger(xmlNodePtr node, const char* tag, int& value)
{
	char* nodeValue = (char*)xmlGetProp(node, (xmlChar*)tag);
	if(nodeValue)
	{
		value = atoi(nodeValue);
		xmlFree(nodeValue);
		return true;
	}

	return false;
}

/// <summary>
/// Loading servers from *.xml file
/// </summary>
bool Tools::loadFromXmlIpList()
{
	for(int i = 0; i < MAX_SERVERS_IN_LIST; i++)
	{
		IpListAddr[i].clear();
		IpListPort[i].clear();
	}

	xmlDocPtr doc = xmlParseFile(getFilePath(SERVER_LIST_FILE).c_str());
	if(!doc)
		return false;
	xmlNodePtr root, p;
	root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)SECTION) != 0)
	{
		xmlFreeDoc(doc);
		return false;
	}

	p = root->children;
	int cID = 0;
	while(p)
	{
		std::string strVal;
		if(xmlStrcmp(p->name, (const xmlChar*)"Server") == 0)
		{
			if(readXMLString(p, "IP", strVal))
				IpListAddr[cID] = strVal;
			if(readXMLString(p, "Port", strVal))
				IpListPort[cID] = strVal;
			cID++;
		}
		p = p->next;
	}
	xmlFreeDoc(doc);
	return true;
}

/// <summary>
/// Loading language file with strings
/// </summary>
bool Tools::loadLanguageStrings(const char* fileName)
{
	if(!fileExists(fileName))
		return false;

	for(int i = 1; i <= LANGUAGE_STRINGS; i++)
	{
		char key[5];
		_itoa(i, key, 10);
		std::string str = readStringFromFile(fileName, "Language", key);
		replaceString(str, "\\n", "\xa");
		strcpy(languageTable[i], str.c_str());
	}

	return true;
}

/// <summary>
/// Returns a path where is *.exe file
/// </summary>
std::string Tools::getExeDir()
{
	char buffer[MAX_PATH];
	if(GetModuleFileName(NULL, buffer, sizeof(buffer)))
	{
		if(PathRemoveFileSpec(buffer))
		{
			PathAddBackslash(buffer);
			return buffer;
		}
	}
	return "";
}

/// <summary>
/// Returns a path where is *.exe file with file name
/// </summary>
std::string Tools::getFilePath(const char* fileName)
{
	char filePath[MAX_PATH];
	sprintf(filePath, "%s\\%s", getExeDir().c_str(), fileName);
	return filePath;
}

/// <summary>
/// Loads Tibia addresses from *.xml file
/// </summary>
bool Tools::loadFromXmlAddresses()
{
	xmlDocPtr doc = xmlParseFile(getFilePath(ADDRESSES_FILE).c_str());
	if(!doc)
		return false;

	xmlNodePtr root, p;
	root = xmlDocGetRootElement(doc);
	if(xmlStrcmp(root->name,(const xmlChar*)SECTION) != 0)
	{
		xmlFreeDoc(doc);
		return false;
	}

	p = root->children;
	int cID = 0;
	while(p)
	{
		std::string strVal;
		int intVal;
		if(xmlStrcmp(p->name, (const xmlChar*)"Protocol") == 0)
		{
			if(readXMLString(p, "Version", strVal))
			{
				rAddr[cID].protocol = new char[strVal.size()+1];
				strcpy(rAddr[cID].protocol, strVal.c_str());
				rAddr[cID].isUsed = true;
			}
			else
				rAddr[cID].isUsed = false;
			if(readXMLString(p, "rsaAddr", strVal))
			{
				if(strcmp(rAddr[cID].protocol, "910") == 0)
				{
					DWORD tempAddress;
					sscanf(strVal.c_str(), "0x%X", &tempAddress);
					rAddr[cID].rsaAddr = AlignAddress(tempAddress);
				}
				else
					sscanf(strVal.c_str(), "0x%X", &rAddr[cID].rsaAddr);
			}
			if(readXMLString(p, "ipAddr", strVal))
			{
				if(strcmp(rAddr[cID].protocol, "910") == 0)
				{
					DWORD tempAddress;
					sscanf(strVal.c_str(), "0x%X", &tempAddress);
					rAddr[cID].ipAddr = AlignAddress(tempAddress);
				}
				else
					sscanf(strVal.c_str(), "0x%X", &rAddr[cID].ipAddr);
			}
			if(readXMLInteger(p, "loginServers", intVal))
			{
				rAddr[cID].loginServers = (unsigned short)intVal;
			}
			cID++;
		}
		p = p->next;
	}
	xmlFreeDoc(doc);
	return true;
}

/// <summary>
/// Checks for Addresses.xml file update
/// </summary>
bool Tools::updateXmlAddresses()
{
	/* TODO (#1#): Rewrite this function */
	return false;
}

DWORD Tools::GetModuleBase()
{
	MODULEENTRY32 moduleEntry = {0};
	HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
	DWORD base = 0;
 
	if(!snapShot)
		return 0;
 
	moduleEntry.dwSize = sizeof(moduleEntry);
	BOOL currentModule = Module32First(snapShot, &moduleEntry);
 
	if(currentModule)
	{
		static char hold[1000];
		memcpy(hold, moduleEntry.szModule, strlen(moduleEntry.szModule) + 1);
 
		if(std::string(hold).find(".exe") == std::string(hold).size() - 4)
			base = (DWORD)moduleEntry.modBaseAddr;
	}
 
	CloseHandle(snapShot);
	return base;
}

DWORD Tools::AlignAddress(DWORD address)
{
	static int base = (int)GetModuleBase();
	static int XPBase = 0x400000;
	address += (base - XPBase);
	return address;
}
