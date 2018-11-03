/*
 *  KernelEx
 *  Copyright (C) 2009, Xeno86
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

#include <stdio.h>
#include <windows.h>
#include "kexcoresdk.h"
#include "debugproto.h"

int menu()
{
	printf("\n------------------------------------------------------------\n");
	printf("KernelEx control options:\n");
	printf("1. dump configurations\n");
	printf("2. dump imtes\n");
	printf("3. dump application settings\n");
	printf("4. flush application settings\n");
	printf("0. exit\n");
	printf("\nyour choice: ");
	int ch;
	scanf("%d", &ch);
	printf("\n");
	return ch;
}

int main()
{
	printf("KernelEx control tool by Xeno86\n");
	printf("init return %d\n", kexInit());

	int ch;
	while ((ch = menu()) != 0)
	{
		switch(ch) 
		{
		case 1:
			kexDbgDumpConfigurations();
			break;
		case 2:
			kexDbgDumpImtes();
			break;
		case 3:
			kexDbgDumpAppSettings();
			break;
		case 4:
			kexFlushAppSettings();
			break;
		default:
			printf("Invalid option!\n");
		}
	}

	printf("uninit return %d\n", kexUninit());

	return 0;
}
