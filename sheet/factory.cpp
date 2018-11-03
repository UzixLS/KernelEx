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

#include <windows.h>
#include "factory.h"
#include "server.h"
#include "sheet.h"
#include "KexLinkage.h"

Factory::Factory() 
{
	m_RefCount = 0;
}


STDMETHODIMP_(ULONG) Factory::AddRef() 
{
	g_LockCount++;
	return ++m_RefCount;
}


STDMETHODIMP_(ULONG) Factory::Release() 
{
	g_LockCount--;
	return --m_RefCount;
}


STDMETHODIMP Factory::QueryInterface(REFIID riid,LPVOID *ppv)
{
	if (riid==IID_IUnknown)
		*ppv = static_cast<IUnknown*>(this);
	else if (riid==IID_IClassFactory)
		*ppv = static_cast<IClassFactory*>(this);
	else
	{
		*ppv = NULL;
		return E_NOINTERFACE;
	}

	AddRef();
	return S_OK;
}


STDMETHODIMP Factory::LockServer(BOOL bLock)
{
	if (bLock==TRUE)
		g_LockCount++;
	else
		g_LockCount--;
	return S_OK;
}


STDMETHODIMP Factory::CreateInstance(IUnknown *pUnkOuter,REFIID riid, LPVOID *ppv)
{
	*ppv = NULL;
	
	if (pUnkOuter != NULL)
		return CLASS_E_NOAGGREGATION;

	if (!KexLinkage::instance.IsReady())
		return E_ACCESSDENIED;
	
	KexShlExt* pShlExt = new KexShlExt;
	if (pShlExt == NULL)
		return E_OUTOFMEMORY;
	
	HRESULT hr = pShlExt->QueryInterface(riid,ppv);
	if (FAILED(hr))
	{
		delete pShlExt;
		return E_NOINTERFACE;
	}
	return S_OK;
}
