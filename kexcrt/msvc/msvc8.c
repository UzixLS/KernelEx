#include <windows.h>

#ifndef STATUS_INVALID_CRUNTIME_PARAMETER
#define STATUS_INVALID_CRUNTIME_PARAMETER       0xC0000417
#endif

__declspec(noreturn) void _invoke_watson(
    const wchar_t *pszExpression,
    const wchar_t *pszFunction,
    const wchar_t *pszFile,
    unsigned int nLine,
    void* pReserved
    )
{
	MessageBox(NULL, "Invalid argument passed into a CRT function", "CRT error", MB_OK | MB_ICONERROR);
	RaiseException( STATUS_INVALID_CRUNTIME_PARAMETER, EXCEPTION_NONCONTINUABLE, 0, NULL );
}

void _invalid_parameter(
    const wchar_t *pszExpression,
    const wchar_t *pszFunction,
    const wchar_t *pszFile,
    unsigned int nLine,
    void* pReserved
    )
{
	_invoke_watson(pszExpression, pszFunction, pszFile, nLine, pReserved);
}

void _invalid_parameter_noinfo(void)
{
    _invalid_parameter(NULL, NULL, NULL, 0, 0);
}

