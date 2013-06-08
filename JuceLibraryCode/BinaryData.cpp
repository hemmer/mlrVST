/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== CHANGES ==================
static const unsigned char temp_ef27c340[] =
"September 28, 2005\r\n"
"------------------\r\n"
"\r\n"
"Compared to the previous official snapshot (November 2004) the \r\n"
"current version of oscpack includes a re-written set of network \r\n"
"classes and some changes to the syntax of the networking code. It no \r\n"
"longer uses threads, which means that you don't need to use sleep() \r\n"
"if you are writing a simple single-threaded server, or you need to \r\n"
"spawn your own threads in a more complex application.\r\n"
"\r\n"
"The list below summarises the changes if you are porting code from \r\n"
"the previous release.\r\n"
"\r\n"
"    - there are no longer any threads in oscpack. if you need to \r\n"
"    set up an asynchronous listener you can create your own thread \r\n"
"    and call Run on an instance of SocketReceiveMultiplexer or \r\n"
"    UdpListeningReceiveSocket (see ip/UdpSocket.h) yourself.\r\n"
"    \r\n"
"    - host byte order is now used for network (IP) addresses\r\n"
"        \r\n"
"    - functions which used to take two parameters <address, port> \r\n"
"    now take an instance of IpEndpointName (see \r\n"
"    ip/IpEndpointName.h) this class has a number of convenient \r\n"
"    constructors for converting numbers and strings to internet \r\n"
"    addresses. For example there is one which takes a string and \r\n"
"    another that take the dotted address components as separate \r\n"
"    parameters.\r\n"
"    \r\n"
"    - The UdpTransmitPort class, formerly in UdpTransmitPort.h, is \r\n"
"    now called UdpTransmitSocket, which is simply a convenience \r\n"
"    class derived from UdpSocket (see ip/UdpSocket.h). Where you \r\n"
"    used to use the constructor UdpTransmitPort( address, port) now \r\n"
"    you can use UdpTransmitSocket( IpEndpointName( address, port ) \r\n"
"    ) or you can any of the other possible ctors to IpEndpointName\r\n"
"    () (see above). The Send() method is unchanged.\r\n"
"    \r\n"
"    - The packet listener base class is now located in \r\n"
"    ip/PacketListener.h instead of PacketListenerPort.h. The \r\n"
"    ProcessPacket method now has an additional parameter indicating \r\n"
"    the remote endpoint\r\n"
"    \r\n"
"    - The preferred way to set up listeners is with \r\n"
"    SocketReceiveMultiplexer (in ip/UdpSocket.h), this also allows \r\n"
"    attaching periodic timers. For simple applications which only \r\n"
"    listen to a single socket with no timers you can use \r\n"
"    UdpListeningReceiveSocket (also in UdpSocket.h) See \r\n"
"    osc/OscReceiveTest.cpp or osc/OscDump.cpp for examples of this. \r\n"
"    This is more or less equivalent to the UdpPacketListenerPort \r\n"
"    object in the old oscpack versions except that you need to \r\n"
"    explicitly call Run() before it will start receiving packets \r\n"
"    and it runs in the same thread, not a separate thread so Run() \r\n"
"    won't usually return.\r\n"
"    \r\n"
"    - Explicit calls to InitializeNetworking() and \r\n"
"    TerminateNetworking() are no longer required for simple \r\n"
"    applications (more complex windows applications should \r\n"
"    instantiate NetworkInitializer in main() or WinMain (see \r\n"
"    ip/NetworkingUtils.h/.cpp)\r\n"
"    \r\n"
"    - The OscPacketListener base class (OscPacketListener.h) was \r\n"
"    added to make traversing OSC packets easier, it handles bundle \r\n"
"    traversal automatically so you only need to process messages in \r\n"
"    your derived classes.\r\n"
"    \r\n"
"    - On Windows be sure to link with ws2_32.lib or you will see\r\n"
"    a linker error about WSAEventSelect not being found. Also you \r\n"
"    will need to link with winmm.lib for timeGetTime()\r\n"
"\r\n";

const char* CHANGES = (const char*) temp_ef27c340;

//================== LICENSE ==================
static const unsigned char temp_cd0ce4be[] =
"oscpack -- Open Sound Control packet manipulation library\n"
"http://www.audiomulch.com/~rossb/code/oscpack\n"
"\n"
"Copyright (c) 2004 Ross Bencina <rossb@audiomulch.com>\n"
"\n"
"Permission is hereby granted, free of charge, to any person obtaining\n"
"a copy of this software and associated documentation files\n"
"(the \"Software\"), to deal in the Software without restriction,\n"
"including without limitation the rights to use, copy, modify, merge,\n"
"publish, distribute, sublicense, and/or sell copies of the Software,\n"
"and to permit persons to whom the Software is furnished to do so,\n"
"subject to the following conditions:\n"
"\n"
"The above copyright notice and this permission notice shall be\n"
"included in all copies or substantial portions of the Software.\n"
"\n"
"Any person wishing to distribute modifications to the Software is\n"
"requested to send the modifications to the original developer so that\n"
"they can be incorporated into the canonical version.\n"
"\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,\n"
"EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF\n"
"MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.\n"
"IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR\n"
"ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF\n"
"CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION\n"
"WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.\n"
"\n";

const char* LICENSE = (const char*) temp_cd0ce4be;

//================== README ==================
static const unsigned char temp_7bfbf171[] =
"oscpack -- Open Sound Control packet manipulation library\n"
"http://www.audiomulch.com/~rossb/code/oscpack\n"
"\n"
"Copyright (c) 2004-2005 Ross Bencina <rossb@audiomulch.com>\n"
"\n"
"A simple C++ library for packing and unpacking OSC packets.\n"
"\n"
"\n"
"Oscpack is simply a set of C++ classes for packing and unpacking OSC packets. \n"
"Oscpack includes a minimal set of UDP networking classes for windows and posix \n"
"which are sufficient for writing many OSC applications and servers, but you are \n"
"encouraged to use another networking framework if it better suits your needs. \n"
"Oscpack is not an OSC application framework, it doesn't include infrastructure for \n"
"constructing or routing OSC namespaces, just classes for easily constructing, \n"
"sending, receiving and parsing OSC packets. The library should also be easy to use \n"
"for other transport methods (eg serial).\n"
"\n"
"The key goals of the oscpack library are:\n"
"\n"
"\t- to be a simple and complete implementation of OSC\n"
"\t- to be portable to a wide variety of platforms\n"
"\t- to allow easy development of robust OSC applications \n"
"\t\t(for example it should be impossible to crash a server\n"
"\t\t by sending it malformed packets, and difficult to \n"
"\t\t create malformed packets.)\n"
"\n"
"Here's a summary of the key files:\n"
"\n"
"osc/OscReceivedElements -- classes for parsing a packet\n"
"osc/OscPrintRecievedElements -- iostream << operators for printing packet elements\n"
"osc/OscOutboundPacket -- a class for packing messages into a packet\n"
"osc/OscPacketListener -- base class for listening to OSC packets on a UdpSocket\n"
"tests/OscUnitTests -- unit test program for the OSC modules\n"
"tests/OscSendTests -- examples of how to send messages\n"
"tests/OscReceiveTest -- example of how to receive the messages sent by OSCSendTests\n"
"examples/OscDump -- a program that prints received OSC packets\n"
"\n"
"\n"
"\n"
"Building\n"
"--------\n"
"\n"
"In general the idea is that you will embed this source code in your projects as you \n"
"see fit. The Makefile has an install rule for building a shared library and \n"
"installing headers in usr/local.\n"
"\n"
"The Makefile works for Linux and MaxOS X except that if you are on a big endian \n"
"machine such as PowerPC Macintosh you need to edit the line which sets the \n"
"endianness to OSC_HOST_BIG_ENDIAN (see the makefile comment for details) or it won't \n"
"work. If you want to build and install liboscpack as a library on OS X you also need \n"
"to edit the $(LIBFILENAME) rule by commenting out the Linux case and uncommenting \n"
"the OS X case since OS X uses different gcc flags for shared libraries.\n"
"\n"
"On Windows there is a batch file for doing a simple test build with MinGW gcc called \n"
"make.MinGW32.bat. This will build the test executables and oscdump in ./bin and run \n"
"the unit tests.\n"
"\n"
"--\n"
"\n"
"\n"
"If you fix anything or write a set of TCP send/recieve classes \n"
"please consider sending me a patch. Thanks :)\n"
"\n"
"For more information about Open Sound Control, see:\n"
"http://www.cnmat.berkeley.edu/OpenSoundControl/\n"
"\n"
"\n"
"Thanks to Till Bovermann for helping with POSIX networking code and\n"
"Mac compatibility, and to Martin Kaltenbrunner and the rest of the\n"
"reacTable team for giving me a reason to finish this library. Thanks\n"
"to Merlijn Blaauw for reviewing the interfaces. Thanks to Xavier Oliver\n"
"for additional help with Linux builds and POSIX implementation details.\n"
"\n"
"Portions developed at the Music Technology Group, Audiovisual Institute, \n"
"University Pompeu Fabra, Barcelona, during my stay as a visiting\n"
"researcher, November 2004 - September 2005. \n"
"\n"
"See the file LICENSE for information about distributing and using this code.\n"
"\n"
"\n";

const char* README = (const char*) temp_7bfbf171;

//================== TODO ==================
static const unsigned char temp_795718c9[] =
"TODO:\n"
"\n"
"    - consider adding the local endpoint name to PacketListener::PacketReceived() params\n"
"\n"
"    - consider adding ListenerThread class to support old seperate thread listener functionality, something like:\n"
"\n"
"        class UdpSocketListenerThread{\n"
"        public:\n"
"            UdpSocketListenerThread( UdpSocket& socket, Listener *listener );\n"
"            UdpSocketListenerThread( UdpSocketReceiveMultiplexer *mux );\n"
"            ~UdpSocketListenerThread();\n"
"\n"
"            void Run();\n"
"            void Stop();\n"
"        };\n"
"\n"
"    - provide some kind of automatic endianness configuration (hopefully there\n"
"        are gcc symbols for this)\n"
"\n"
"    - work out a way to make the parsing classes totally safe. at a minimum this\n"
"    means adding functions to test for invalid float/doublevalues,\n"
"    making sure the iterators never pass the end of the message, ...\n"
"        (passing end of message can happen if:\n"
"            - too many args in type tags\n"
"                a. typetags overflow message size\n"
"                b. args fulfilling typetags overflow message size\n"
"            - strings too long or not terminated correctly\n"
"            - blobs too long or not terminated correctly\n"
"\n"
"        if the message was fully checked during construction, the end() iterator\n"
"        could be moved back until only arguments which fit withing size() may\n"
"        be interated (this could be none). A flag could be set to indicate that\n"
"        something was wrong.\n"
"\n"
"    - other packet badness could include:\n"
"        - time tags too far into the future (the scheduler should deal with\n"
"            that i guess).\n"
"        - message address patterns which aren't correctly terminated\n"
"\n"
"    - improve the ability to parse messages without tags (SC uses methods which\n"
"            get the data and advance the iterator in one step.)\n"
"        - Check* could be modified to do this - ie if typetags are not present\n"
"            it could check that reading the field won't escape the message size\n"
"            and return the data, or return false if some consistency\n"
"            constraint is violated.\n"
"        (or alternately drop support for messages without type tags)\n"
"        \n"
"\n"
"    - add a method to discard an inprogress message if it gets half\n"
"        constructed and the buffer is full in OutboundPacket\n"
"\n"
"    - write a stress testing app which can send garbage packets to try to flush out other bugs in the parsing code.\n"
"\n"
"\n"
"\n";

const char* TODO = (const char*) temp_795718c9;

//================== locked.png ==================
static const unsigned char temp_e59d653b[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,8,3,0,0,0,40,45,15,83,0,0,0,4,103,65,77,65,0,0,177,143,11,252,97,5,0,0,3,0,80,76,84,69,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,118,236,99,0,0,1,0,116,82,78,83,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,0,83,247,7,
37,0,0,0,9,112,72,89,115,0,0,14,194,0,0,14,194,1,21,40,74,128,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,80,97,105,110,116,46,78,69,84,32,118,51,46,53,46,56,55,59,128,93,0,0,0,53,73,68,65,84,40,83,99,248,143,6,24,136,21,96,4,2,136,90,136,
22,16,31,42,2,22,0,115,160,34,80,1,136,48,92,11,132,137,34,128,48,4,102,6,220,84,170,9,32,123,16,195,183,0,242,166,183,145,139,159,97,29,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* locked_png = (const char*) temp_e59d653b;

//================== silkfont ==================
static const unsigned char temp_f5d0699a[] =
{ 120,156,237,91,201,146,28,53,16,173,43,69,16,193,133,187,217,49,182,217,205,106,170,216,215,134,6,204,98,22,227,49,34,109,108,119,55,48,102,223,130,8,110,21,92,230,23,248,5,238,227,51,7,98,190,128,249,13,130,27,165,146,148,202,84,74,85,234,113,207,116,
119,128,79,25,93,42,229,254,114,169,241,201,11,147,75,151,213,38,192,172,40,138,63,206,159,174,15,21,103,90,234,80,241,215,70,89,207,224,198,98,180,181,83,205,166,187,27,101,181,187,209,108,79,52,113,195,63,71,175,180,68,227,9,253,72,217,67,229,182,253,
201,18,237,109,63,119,175,105,66,193,77,69,251,86,173,111,108,170,246,110,115,35,18,236,234,238,198,186,123,166,165,64,194,30,42,235,238,16,220,92,252,114,120,179,118,50,150,78,34,75,48,30,106,74,36,178,242,227,141,154,232,100,236,216,58,177,59,38,19,
45,51,18,230,144,62,110,185,148,53,18,76,90,53,213,199,187,103,163,173,177,39,186,67,250,145,147,9,141,219,80,162,52,130,107,194,177,51,30,168,9,81,26,118,230,144,97,215,108,91,118,72,148,134,93,119,8,110,41,244,143,51,47,27,179,23,83,215,152,66,202,
230,140,90,123,163,74,95,178,187,141,108,44,22,80,57,175,9,114,115,138,160,153,43,119,83,229,131,138,59,76,58,179,85,211,57,101,220,198,201,173,24,39,36,4,172,145,118,60,65,189,219,144,200,67,239,58,33,73,120,198,227,196,123,151,199,119,32,164,61,228,
35,78,122,208,164,206,109,75,119,28,51,28,13,170,169,136,216,137,136,106,39,83,235,66,116,170,51,248,196,25,32,211,151,183,35,46,13,163,8,220,97,192,140,102,108,104,55,199,43,173,8,51,9,17,59,52,9,38,35,28,182,108,9,183,210,19,1,210,185,215,56,118,90,
196,216,177,108,5,156,82,33,27,203,246,78,15,135,28,104,48,169,72,4,103,225,83,14,170,228,129,24,186,139,225,185,140,146,97,28,71,15,136,130,224,67,9,142,176,202,16,75,116,60,156,72,60,153,249,241,12,246,146,193,81,86,50,251,171,225,180,48,255,164,71,
171,137,254,93,19,10,142,97,201,244,200,239,171,49,79,105,184,171,141,186,166,101,111,95,15,226,184,99,225,152,42,184,155,220,60,84,103,167,194,85,19,225,206,116,136,82,15,195,61,22,200,72,229,12,19,210,179,205,74,72,226,198,48,33,93,113,193,96,23,245,
166,188,226,226,184,5,139,123,73,127,146,200,90,230,125,121,225,96,134,222,231,213,79,152,72,34,44,105,60,132,208,62,195,41,178,251,166,68,128,14,53,109,188,182,17,103,221,31,58,235,64,216,250,96,71,244,17,238,227,101,51,209,83,40,120,128,152,219,73,
194,177,58,9,7,161,3,100,3,22,11,136,208,145,166,10,29,143,139,17,94,104,237,129,22,178,81,200,140,30,84,248,171,182,208,131,40,90,19,193,210,72,170,101,176,109,210,153,209,248,174,141,165,152,40,126,121,182,81,240,16,105,140,88,247,21,70,88,44,48,13,
176,37,226,209,57,52,93,242,233,33,120,120,143,185,210,143,126,11,207,149,52,68,18,235,244,205,80,195,165,30,30,33,166,200,96,155,209,135,102,217,43,72,215,232,120,22,68,80,108,166,82,240,40,171,163,97,209,237,88,184,18,171,92,177,13,162,171,227,161,
31,153,202,252,152,156,165,251,58,165,12,71,100,213,248,19,182,160,245,20,43,58,72,197,70,34,150,155,108,162,137,24,18,99,50,104,95,105,175,212,144,46,46,1,56,216,245,194,227,172,253,73,140,24,177,145,76,68,82,105,107,124,229,111,28,110,209,178,76,146,
5,33,89,38,113,176,68,131,59,232,68,20,212,203,170,201,193,144,67,11,167,69,117,120,130,140,217,49,119,237,109,182,228,150,96,6,148,200,151,128,254,72,13,205,153,127,224,73,107,237,235,127,191,206,200,168,137,238,240,223,215,252,234,137,238,173,139,199,
175,173,145,232,30,253,246,231,49,79,116,175,235,71,72,116,66,155,123,44,97,194,78,83,157,210,200,3,111,212,132,233,41,158,10,55,2,105,237,201,24,151,234,131,155,100,81,26,136,12,9,233,180,220,147,190,131,7,29,60,77,196,31,240,217,188,240,47,84,35,43,
165,172,200,36,10,36,235,22,60,19,235,228,7,58,168,30,99,41,238,200,152,34,58,251,159,229,128,120,48,190,122,142,96,230,92,170,230,200,99,144,227,249,249,246,91,139,26,11,81,36,6,240,177,213,9,243,253,11,61,190,39,120,149,216,32,176,86,53,177,137,136,
152,232,69,210,67,244,48,53,135,95,34,85,66,50,141,88,33,179,61,28,232,252,94,38,211,4,19,63,26,95,161,169,196,102,40,115,211,158,84,146,20,254,188,37,58,254,36,102,111,159,168,48,226,107,130,220,108,114,195,224,43,100,51,53,228,71,95,36,81,53,109,96,
36,230,216,10,44,172,77,129,87,247,73,129,129,33,102,113,10,140,87,120,7,245,218,94,203,74,54,214,102,133,11,188,190,186,54,242,55,177,224,210,44,221,233,74,193,27,171,97,200,5,226,206,201,248,238,48,222,77,164,235,39,51,228,224,170,37,127,195,48,56,
179,194,155,4,56,105,0,244,141,39,222,73,44,163,141,69,222,218,151,42,23,29,193,104,149,123,155,0,32,170,24,175,105,106,158,217,187,36,99,117,80,11,89,144,160,147,68,143,162,166,236,39,137,183,198,17,239,240,207,230,41,5,242,99,51,107,60,228,11,26,41,
219,46,194,60,189,41,40,253,198,3,167,164,2,7,51,197,103,36,124,82,57,153,121,11,216,7,228,165,39,254,132,95,239,92,241,245,31,23,224,93,242,245,112,80,203,44,5,48,6,226,214,245,171,224,65,45,115,140,10,239,197,230,148,220,9,136,180,145,40,100,152,132,
209,21,114,186,125,69,182,10,222,199,15,194,97,53,26,5,95,148,98,211,162,72,42,5,31,204,243,183,52,66,32,137,51,4,31,153,215,88,121,80,112,154,124,79,31,248,216,213,135,238,78,105,5,31,242,125,220,208,246,57,246,135,59,236,181,188,145,1,206,240,121,179,
114,50,86,94,33,159,24,27,243,126,205,103,105,48,17,169,162,224,236,202,174,149,62,90,239,181,146,90,247,181,210,199,75,89,43,193,50,214,74,231,246,127,173,116,126,173,214,74,159,44,99,173,116,97,158,181,210,197,229,172,149,46,253,23,214,74,147,171,91,
43,77,215,125,173,52,91,247,181,210,167,171,187,50,129,207,86,99,27,2,159,175,174,141,242,214,74,155,171,97,200,5,226,206,229,117,95,43,125,177,232,181,210,151,203,89,43,125,181,238,107,165,175,215,125,173,244,205,255,107,165,197,175,149,190,93,247,181,
210,119,171,187,86,250,62,156,219,210,102,139,23,216,56,118,37,130,218,163,153,199,119,79,4,221,208,15,252,111,225,43,123,102,236,64,108,140,14,105,171,234,143,228,15,213,18,203,19,137,229,163,45,255,31,55,146,243,150,15,45,130,28,3,43,43,154,156,240,
147,28,33,9,110,4,35,36,113,104,88,39,185,180,233,56,32,153,204,234,92,164,247,129,86,139,226,95,133,6,119,89,0,0 };

const char* silkfont = (const char*) temp_f5d0699a;

//================== unlocked.png ==================
static const unsigned char temp_8b77a314[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,16,0,0,0,16,8,3,0,0,0,40,45,15,83,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,0,4,103,65,77,65,0,0,177,143,11,252,97,5,0,0,3,0,80,76,84,69,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,103,118,236,99,0,0,1,
0,116,82,78,83,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
255,255,255,255,255,255,255,0,83,247,7,37,0,0,0,9,112,72,89,115,0,0,14,195,0,0,14,195,1,199,111,168,100,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,80,97,105,110,116,46,78,69,84,32,118,51,46,53,46,56,55,59,128,93,0,0,0,50,73,68,65,84,40,83,
99,96,68,3,12,36,8,252,7,2,160,114,184,22,16,31,36,2,19,0,115,64,4,66,0,100,58,118,1,136,102,176,129,32,192,0,210,72,125,1,66,190,5,0,94,211,72,113,27,249,245,159,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* unlocked_png = (const char*) temp_8b77a314;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x56d6eea3:  numBytes = 3356; return CHANGES;
        case 0x34bc1021:  numBytes = 1375; return LICENSE;
        case 0x8fd84dae:  numBytes = 3487; return README;
        case 0x00276046:  numBytes = 2392; return TODO;
        case 0x52fe7f74:  numBytes = 1232; return locked_png;
        case 0x1af4b824:  numBytes = 1649; return silkfont;
        case 0xd173e2cd:  numBytes = 1242; return unlocked_png;
        default: break;
    }

    numBytes = 0;
    return 0;
}

}
