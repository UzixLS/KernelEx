/*
 *  KernelEx
 *  Copyright (C) 2010, Tihiy
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

#include <windows.h>
#include "unifwd.h"

//AddForm - not supported
FORWARD_TO_UNICOWS(AddJobW);
FORWARD_TO_UNICOWS(AddMonitorW);
FORWARD_TO_UNICOWS(AddPortW);
//AddPrinterConnection - not supported
FORWARD_TO_UNICOWS(AddPrinterDriverW);
FORWARD_TO_UNICOWS(AddPrinterW);
FORWARD_TO_UNICOWS(AddPrintProcessorW);
FORWARD_TO_UNICOWS(AddPrintProvidorW); //lol providor
FORWARD_TO_UNICOWS(AdvancedDocumentPropertiesW);
FORWARD_TO_UNICOWS(ConfigurePortW);
//DeleteForm - not supported
FORWARD_TO_UNICOWS(DeleteMonitorW);
FORWARD_TO_UNICOWS(DeletePortW);
//DeletePrinterConnection - not supported
FORWARD_TO_UNICOWS(DeletePrinterDriverW);
FORWARD_TO_UNICOWS(DeletePrintProcessorW);
FORWARD_TO_UNICOWS(DeletePrintProvidorW);
FORWARD_TO_UNICOWS(DeviceCapabilitiesW);
FORWARD_TO_UNICOWS(DocumentPropertiesW);
//EnumForms - not supported
//EnumJobsW - not implemented
//EnumMonitorsW - not implemented
//EnumPortsW - not implemented
//EnumPrinterDriversW - not implemented
//EnumPrintersW - not implemented
//EnumPrintProcessorDatatypesW - not implemented
//EnumPrintProcessorsW - not implemented
//GetForm - not supported
//GetJobW - not implemented
//GetPrinterDataW - not implemented
FORWARD_TO_UNICOWS(GetPrinterDriverDirectoryW);
//GetPrinterDriverW - not implemented
//GetPrinterW - not implemented
FORWARD_TO_UNICOWS(GetPrintProcessorDirectoryW);
FORWARD_TO_UNICOWS(OpenPrinterW);
FORWARD_TO_UNICOWS(ResetPrinterW);
//SetForm - not supported
FORWARD_TO_UNICOWS(SetJobW);
//SetPrinterDataW - not implemented
FORWARD_TO_UNICOWS(SetPrinterDataW); //lol lazy bastards
FORWARD_TO_UNICOWS(SetPrinterW);
FORWARD_TO_UNICOWS(StartDocPrinterW);
