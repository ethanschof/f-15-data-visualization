//
// Gives some added capabilities to the usual iostreams such as prompted printout lines
//
//////////////////////////////////////////////////////////////////////
#ifndef _OUTSTREAM_H
#define _OUTSTREAM_H

#include <iostream>
#include <string>

typedef std::ios_base& (__cdecl * TIoManip)(std::ios_base&); // for hex, dec a.s.o from iomanip
typedef std::ostream& (__cdecl * TIosManip)(std::ostream&); // endl, ends a.s.o from iostream

class OutStream
{
public:
	OutStream(std::ostream& stream, bool bActivated = true);	
	OutStream(std::ostream& stream, const char* szPrompt, bool bActivated = true);

	void Activate(bool bActivate = true) {bActive = bActivate;}

	template<class T> OutStream& operator <<(const T& val);

	void PrintF(const char* szFormat, ...);

	OutStream& operator <<(TIoManip iomanip);
	OutStream& operator <<(TIosManip iomanip);

	void SetPrompt(const char* szPrompt) {sPrompt = szPrompt;}
	void ActivateTimePrompt(bool b = true) {bTimePrompt = b;}

protected:
	void NL();

	std::ostream& os;
	std::string sPrompt;
	bool bNewLine;
	bool bTimePrompt;
	bool bActive;
}; // class OutStream

inline OutStream::OutStream(std::ostream& stream, bool bActivated)
:os(stream)
{
	bNewLine = true;
	bTimePrompt = false;
	bActive = bActivated;
}

inline OutStream::OutStream(std::ostream& stream, const char* szPrompt, bool bActivated)
:os(stream)
{
	bNewLine = true;
	bTimePrompt = false;
	bActive = bActivated;
	OutStream::sPrompt = szPrompt;
}

template<class T> inline OutStream& OutStream::operator <<(const T& val)
{
	if (bActive)
	{
		NL();
		os << val;
	}
	return *this;
}

// for hex, dec a.s.o from iomanip
inline OutStream& OutStream::operator <<(TIoManip iomanip)
{
	if (bActive)
	{
		NL();
		os << iomanip;
	}
	return *this;
}

// for endl, ends a.s.o from iOutStream
inline OutStream& OutStream::operator <<(TIosManip iomanip)
{
	if (bActive)
	{
		NL();
		if (iomanip == std::endl)
			bNewLine = true;
		os << iomanip;
	}
	return *this;
}


#endif