#include "stdafx.h"
#include "outstream.h"
#include <iomanip>
#include <sstream>
#include <cstdio>
#include <cstdarg>
#include <ctime>

using namespace std;

// check if newline happened
void OutStream::NL()
{
	if (bNewLine)
	{
		if (bTimePrompt)
		{
			stringstream ss;
			time_t t;
			time(&t);
			struct tm* pLT = localtime(&t);
			ss << setw(2) << setfill('0') << pLT->tm_hour << ":";
			ss << pLT->tm_min << ":" << pLT->tm_sec;
			if (sPrompt.empty())
				os << ss.str() << ": ";
			else
				os << ss.str() << " " << sPrompt;
		}
		else
			os << sPrompt;

		bNewLine = false;
	}
}

void OutStream::PrintF(const char* szFormat, ...)
{
	char szBuf[500];

	va_list arglist;
	va_start(arglist, szFormat );
	vsprintf(szBuf, szFormat, arglist);
	va_end(arglist);

	int i = 0;
	while (szBuf[i] != 0)
	{
		NL();
		int j = i;
		while (szBuf[j] != 0)
		{
			if (szBuf[j] =='\n')
			{
				bNewLine = true;
				szBuf[j++] = 0;
				break;
			}
			++j;
		}
		
		os << &szBuf[i];
		if (bNewLine) os << endl;
		i = j;
	}

} // OutStream::PrintF
