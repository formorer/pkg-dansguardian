//Please refer to http://dansguardian.org/?page=copyright2
//for the license for this code.
//Written by Daniel Barron (daniel@//jadeb.com).
//For support go to http://groups.yahoo.com/group/dansguardian

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef __HPP_HTTPHeader
#define __HPP_HTTPHeader


// DEFINES

#define __DGHEADER_SENDALL 0
#define __DGHEADER_SENDFIRSTLINE 1
#define __DGHEADER_SENDREST 2


// INCLUDES

#include <deque>

#include "String.hpp"
//#include "DataBuffer.hpp"
#include "Socket.hpp"
#include "RegExp.hpp"


// DECLARATIONS

class HTTPHeader
{
public:
	std::deque<String> header;
	//DataBuffer postdata;
	unsigned int port;

	// reset header object for future use
	void reset();

	// network communication funcs

	void setTimeout(int t);
	void in(Socket *sock, bool allowpersistent = false, bool honour_reloadconfig = false);

	// send headers out over the given socket
	// "reconnect" flag gives permission to reconnect to the socket on write error
	// - this allows us to re-open the proxy connection on pconns if squid's end has
	// timed out but the client's end hasn't. not much use with NTLM, since squid
	// will throw a 407 and restart negotiation, but works well with basic & others.
	void out(Socket *peersock, Socket *sock, int sendflag, bool reconnect = false) throw(std::exception);

	// discard remainder of POST data
	void discard(Socket *sock);
	
	// header value and type checks

	// request type: GET, HEAD, POST etc.
	String requestType();
	int returnCode();
	// get content length - returns -1 if undetermined
	off_t contentLength();
	String getContentType();
	// check received content type against given content type
	bool isContentType(const String& t);
	// check HTTP message code to see if it's an auth required message
	bool authRequired();
	// Content-Disposition
	String disposition();
	String userAgent();
	// grab contents of X-Forwarded-For
	std::string getXForwardedForIP();
	// check HTTP message code to see if it's a redirect
	bool isRedirection();
	// see if content-type is something other than "identity"
	bool isCompressed();
	String contentEncoding();
	// grab the contents of Proxy-Authorization header
	// returns base64-decoding of the chunk of data after the auth type string
	std::string getAuthData();
	// grab raw contents of Proxy-Authorization header, without b64 decode
	std::string getRawAuthData();
	// check whether a connection is persistent
	bool isPersistent() { return ispersistent; };

	// detailed value/type checks

	bool malformedURL(const String& url);
	bool isPostUpload(Socket &peersock);
	String getAuthType();
	String url(bool withport = false);

	// header modifications

	void addXForwardedFor(const std::string &clientip);
	// strip content-encoding, and simultaneously set content-length to newlen
	void removeEncoding(int newlen);
	void setContentLength(int newlen);
	// regexp search and replace
	bool urlRegExp(int filtergroup);
	bool headerRegExp(int filtergroup);
	// make a connection persistent - or not
	void makePersistent(bool persist = true);

	// do URL decoding (%xx) on string
	// decode everything, or just numbers, letters and -
	String decode(const String &s, bool decodeAll = false);

	// Bypass URL & Cookie funcs
	
	// is this a temporary filter bypass URL?
	int isBypassURL(String *url, const char *magic, const char *clientip, bool *isvirusbypass);
	// is this a scan bypass URL? (download previously scanned file)
	bool isScanBypassURL(String *url, const char *magic, const char *clientip);
	// is this a temporary filter bypass cookie?
	bool isBypassCookie(String url, const char *magic, const char *clientip);
	// chop GBYPASS/GSPYBASS off URLs (must know it's there to begin with)
	void chopBypass(String url, bool infectionbypass);
	void chopScanBypass(String url);
	// add cookie to outgoing headers with given name & value
	void setCookie(const char *cookie, const char *domain, const char *value);
	
	HTTPHeader():dirty(true) { reset(); };

private:
	// timeout for socket operations
	int timeout;

	// header index pointers
	String *phost;
	String *pport;
	String *pcontentlength;
	String *pcontenttype;
	String *pproxyauthorization;
	String *pcontentdisposition;
	String *puseragent;
	String *pxforwardedfor;
	String *pcontentencoding;
	String *pproxyconnection;
	// cached result of url()
	std::string cachedurl;

	char postdata[15];
	off_t postdatalen;
	bool postdatachopped;
	bool ispostupload;

	bool ispersistent, waspersistent;

	bool dirty;

	// check & fix headers from servers that don't obey standards
	void checkheader(bool allowpersistent);

	// convert %xx back to original character
	String hexToChar(const String &n, bool all = false);
	// base64 decode an individual char
	int decode1b64(char c);
	// base64 decode a complete string
	std::string decodeb64(const String& line);

	// modify supplied accept-encoding header, adding "identity" and stripping unsupported compression types
	String modifyEncodings(String e);
	// modifies the URL in all relevant header lines after a regexp search and replace
	// setURL Code originally from from Ton Gorter 2004
	void setURL(String &url);

	// Generic search & replace code, called by urlRegExp and headerRegExp
	// urlRegExp Code originally from from Ton Gorter 2004
	bool regExp(String &line, std::deque<RegExp> &regexp_list, std::deque<String> &replacement_list);

	// grab cookies from headers
	String getCookie(const char *cookie);
};

#endif
