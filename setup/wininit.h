/*
 *  KernelEx
 *  Copyright (C) 2007-2008, Xeno86
 *
 *  This file is part of KernelEx source code.
 *
 *  KernelEx is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation; version 2 of the License.
 *
 *  KernelEx is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

/** @file wininit.h
 * Class WININIT.
 */

#ifndef __WININIT_H
#define __WININIT_H

#include "sstring.hpp"
#include <queue>

using namespace std;

/** @class WININIT
 * Maintains wininit.ini entries.
 */
class WININIT
{
	/** @struct _2files
	 * Helper struct holding two filenames.
	 */
	struct _2files
	{
		_2files(const sstring& d, const sstring& s) : dest(d), src(s) {}
		sstring dest;
		sstring src;
	};

	/** Stack holding wininit.ini entries. */
	std::queue<_2files> que;
	
	/** Holds path to wininit.ini file. */
	char InitFile[MAX_PATH];

public:

	/** Constructor. */
	WININIT()
	{
		if (GetWindowsDirectory(InitFile, MAX_PATH) > 3) strcat(InitFile, "\\");
		strcat(InitFile, "WININIT.INI");
	}

	/** Destructor. */
	~WININIT()
	{
		clear();
	}

	/** Deletes all files not saved by save() method. @see save */
	void clear()
	{
		if (!que.empty()) DBGPRINTF(("\nRemoving temporary files ...\n"));
		while (!que.empty())
		{
			DeleteFile(que.front().src);
			DBGPRINTF(("... %s\n", que.front().src));
			que.pop();
		}
	}

	/** Checks if wininit.ini file exists.
	 * @return true if exists, false otherwise.
	 */
	bool exists()
	{
		return (GetFileAttributes(InitFile) != 0xffffffff);
	}

	/** Pushes new source-destination pair onto the stack.
	 * @param dest Destination file name (or 'NUL' for delete).
	 * @param src Source file name.
	 */
	void add(const sstring& dest, const sstring& src)
	{
		_2files t(dest, src);
		que.push(t);
	}

	/** Saves all data from stack to wininit.ini file. */
	bool save()
	{
		char buf[32767];
		int buf_len;

		if (que.empty())
			return true;

		buf_len = GetPrivateProfileSection("rename", buf, sizeof(buf), "wininit.ini");

		DBGPRINTF(("\nSaving WININIT.INI entries ...\n"));

		while (!que.empty())
		{
			char line[256];
			int line_len;
			char short_dest[MAX_PATH];
			char short_src[MAX_PATH];

			if (!GetShortPathName(que.front().dest, short_dest, sizeof(short_dest)))
				//if fail, assume destination doesn't exit
				//fixme: may not be short path name or be invalid path name
				strcpy(short_dest, que.front().dest);
			if (!GetShortPathName(que.front().src, short_src, sizeof(short_src)))
				strcpy(short_src, que.front().src);
			
			strcpy(line, short_dest);
			strcat(line, "=");
			strcat(line, short_src);
			strcat(line, "\r\n");
			line_len = strlen(line);

			if (buf_len + line_len + 2 <= sizeof(buf))
			{
				DBGPRINTF(("... %s => %s\n", (const char*) que.front().src, 
						(const char*) que.front().dest));
				memcpy(&buf[buf_len], line, line_len);
				buf_len += line_len;
			}
			else  //buffer overflow
				return false;

			que.pop();
		}
		buf[buf_len] = 0;
		buf[buf_len + 1] = 0;

		if (WritePrivateProfileSection("rename", buf, "wininit.ini"))
			return true;
		return false;
	}
} Wininit;

#endif // __WININIT_H
