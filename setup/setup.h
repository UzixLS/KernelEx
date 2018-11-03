/*
 *  KernelEx
 *  Copyright (C) 2008-2010, Xeno86
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

#ifndef __SETUP_H
#define __SETUP_H

#include "sstring.hpp"

void ShowError(UINT id, ...);

class Setup
{
public:
	Setup(char* _backup_file);
	int get_signature_ver();
	void install();
	void set_reboot_flag();
	void register_verify();
	void reboot();

private:
	sstring backup_file;
	HMODULE h_kernel32;
	int version;
	bool is_winme;
	bool upgrade;

	bool detect_old_version();
	void detect_downgrade();
	void kill_process(const char* name);
	sstring get_temp_file_name();
};

#endif
