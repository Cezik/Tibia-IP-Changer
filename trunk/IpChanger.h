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

#ifndef __IP_CHANGER_H__
#define __IP_CHANGER_H__

#include <string>

#define MAX_SERVERS_IN_LIST 200
#define LANGUAGE_STRINGS 73
#define MAX_AMOUNT_OF_PROTOCOLS 50

const int16_t PortDistance = 100;
const int16_t IpDistance = 112;
const char OTSERV_RSA_KEY[] = "109120132967399429278860960508995541528237502902798129123468757937266291492576446330739696001110603907230888610072655818825358503429057592827629436413108566029093628212635953836686562675849720620786279431090218017681061521755056710823876476444260558147179707119674283982419152118103759076030616683978566631413";

#define ADDRESSES_CHECK "http://czepekipchanger.svn.sourceforge.net/viewvc/czepekipchanger/trunk/Addresses.xml"

#ifdef _MSC_VER
typedef signed long long int64_t;
typedef unsigned long uint32_t;
typedef signed long int32_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned char uint8_t;
typedef signed char int8_t;
#endif

#endif
