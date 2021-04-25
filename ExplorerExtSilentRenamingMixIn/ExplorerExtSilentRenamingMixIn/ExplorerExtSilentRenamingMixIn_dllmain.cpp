// ExplorerExtSilentRenaming 
// Hook to suppress the confirmation when changing extensions in Windows Explorer.
//
// Copyright 2021 Gu Zhengxiong <rectigu@gmail.com>
//
// This file is part of ExplorerExtSilentRenaming.
//
// ExplorerExtSilentRenaming is free software:
// you can redistribute it and/or modify it
// under the terms of the GNU General Public License
// as published by the Free Software Foundation,
// either version 3 of the License,
// or (at your option) any later version.
//
// ExplorerExtSilentRenaming is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ExplorerExtSilentRenaming.
// If not, see <https://www.gnu.org/licenses/>.


#include <stdio.h>

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <tchar.h>
#include <CommCtrl.h>
//#pragma comment(lib, "Comctl32.lib") // Don't depend on comctl32.dll for the time being. We load it on our own.

#include <detours.h>


TCHAR gTitle[MAX_PATH];
TCHAR gOrigText[MAX_PATH];
TCHAR* gpFormattedText = NULL;


HRESULT(WINAPI* OldTaskDialogIndirect)(_In_ const TASKDIALOGCONFIG* pTaskConfig, _Out_opt_ int* pnButton, _Out_opt_ int* pnRadioButton, _Out_opt_ BOOL* pfVerificationFlagChecked);


__declspec(dllexport) HRESULT WINAPI NewTaskDialogIndirect(_In_ const TASKDIALOGCONFIG* pTaskConfig, _Out_opt_ int* pnButton, _Out_opt_ int* pnRadioButton, _Out_opt_ BOOL* pfVerificationFlagChecked);


class Hook {
public:
    Hook()
        : Hooked(false)
        , TgtDll(NULL)
    {
        TCHAR ExePath[MAX_PATH];
        GetModuleFileName(GetModuleHandle(NULL), ExePath, MAX_PATH);
        auto PathLow = _tcslwr(ExePath);
        if (_tcsstr(PathLow, TEXT("explorer.exe")))
        {
            // Use LoadLibrary since we don't depend on comctl32.dll via import tables.
            this->TgtDll = LoadLibrary(TEXT("comctl32.dll"));
            // E.g., C:\Windows\System32\en-US\shell32.dll.mui, 
            auto StrDll = GetModuleHandle(TEXT("shell32.dll"));
            //   4148, 	"Rename"
            LoadString(StrDll, 0x1034, gTitle, sizeof gTitle);
            //   4112, 	"If you change a file name extension, the file might become unusable.\n\nAre you sure you want to change it?"
            LoadString(StrDll, 0x1010, gOrigText, sizeof gOrigText);
            // Formatting is required as SHMessageBox does so.
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING, gOrigText, 0, 0, reinterpret_cast<TCHAR*>(&gpFormattedText), 0, 0);

            OldTaskDialogIndirect = reinterpret_cast<decltype(TaskDialogIndirect)*>(GetProcAddress(this->TgtDll, "TaskDialogIndirect"));
            DetourTransactionBegin();
            DetourAttach(&reinterpret_cast<PVOID&>(OldTaskDialogIndirect), NewTaskDialogIndirect);
            if (DetourTransactionCommit() == NO_ERROR)
            {
                this->Hooked = true;
            }
            else
            {
            }
        }
    }

    ~Hook()
    {
        if (this->Hooked)
        {
            DetourTransactionBegin();
            DetourDetach(&reinterpret_cast<PVOID&>(OldTaskDialogIndirect), NewTaskDialogIndirect);
            if (DetourTransactionCommit() == NO_ERROR)
            {
            }
            else
            {
            }
        }

        if (this->TgtDll)
        {
            FreeLibrary(this->TgtDll);
        }

        if (gpFormattedText)
        {
            LocalFree(gpFormattedText);
        }
    }

private:
    bool Hooked;
    HMODULE TgtDll;
} gHook;


BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


__declspec(dllexport) HRESULT WINAPI NewTaskDialogIndirect(const TASKDIALOGCONFIG* pTaskConfig, int* pnButton, int* pnRadioButton, BOOL* pfVerificationFlagChecked)
{
    if (!IS_INTRESOURCE(pTaskConfig->pszWindowTitle) && !IS_INTRESOURCE(pTaskConfig->pszContent))
    {
        auto TitleEq = !wcscmp(pTaskConfig->pszWindowTitle, gTitle);
        auto TextEq = !wcscmp(pTaskConfig->pszContent, gpFormattedText);
        if (TitleEq && TextEq)
        {
            *pnButton = IDYES;
            return S_OK;
        }
    }

    return OldTaskDialogIndirect(pTaskConfig, pnButton, pnRadioButton, pfVerificationFlagChecked);
}
