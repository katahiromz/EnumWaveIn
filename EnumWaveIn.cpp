#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <mmsystem.h>
#include <strsafe.h>
#include <vector>
#include <cstdio>
#include <cassert>
#pragma comment(lib, "winmm.lib")

LPTSTR LoadStringDx(INT nID)
{
    static UINT s_index = 0;
    const UINT cchBuffMax = 1024;
    static TCHAR s_sz[2][cchBuffMax];

    TCHAR *pszBuff = s_sz[s_index];
    s_index = (s_index + 1) % _countof(s_sz);
    pszBuff[0] = 0;
    if (!::LoadString(NULL, nID, pszBuff, cchBuffMax))
        assert(0);
    return pszBuff;
}

struct WAVE_FORMAT_INFO
{
    DWORD flags;
    DWORD samples;
    WORD bits;
    WORD channels;
};
static const WAVE_FORMAT_INFO s_wave_formats[] =
{
    { WAVE_FORMAT_1M08, 11025, 8, 1 },
    { WAVE_FORMAT_1S08, 11025, 8, 2 },
    { WAVE_FORMAT_1M16, 11025, 16, 1 },
    { WAVE_FORMAT_1S16, 11025, 16, 2 },
    { WAVE_FORMAT_2M08, 22050, 8, 1 },
    { WAVE_FORMAT_2S08, 22050, 8, 2 },
    { WAVE_FORMAT_2M16, 22050, 16, 1 },
    { WAVE_FORMAT_2S16, 22050, 16, 2 },
    { WAVE_FORMAT_4M08, 44100, 8, 1 },
    { WAVE_FORMAT_4S08, 44100, 8, 2 },
    { WAVE_FORMAT_4M16, 44100, 16, 1 },
    { WAVE_FORMAT_4S16, 44100, 16, 2 },
    { WAVE_FORMAT_44M08, 44100, 8, 1 },
    { WAVE_FORMAT_44S08, 44100, 8, 2 },
    { WAVE_FORMAT_44M16, 44100, 16, 1 },
    { WAVE_FORMAT_44S16, 44100, 16, 2 },
    { WAVE_FORMAT_48M08, 48000, 8, 1 },
    { WAVE_FORMAT_48S08, 48000, 8, 2 },
    { WAVE_FORMAT_48M16, 48000, 16, 1 },
    { WAVE_FORMAT_48S16, 48000, 16, 2 },
    { WAVE_FORMAT_96M08, 96000, 8, 1 },
    { WAVE_FORMAT_96S08, 96000, 8, 2 },
    { WAVE_FORMAT_96M16, 96000, 16, 1 },
    { WAVE_FORMAT_96S16, 96000, 16, 2 },
};

std::vector<WAVE_FORMAT_INFO> g_formats;

void OnChooseItem(HWND hwnd, INT iDev)
{
    WAVEINCAPS wic;
    if (::waveInGetDevCaps(iDev, &wic, sizeof(wic)) != MMSYSERR_NOERROR)
    {
        return;
    }

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    ComboBox_ResetContent(hCmb2);
    g_formats.clear();

    for (int i = 0; i < ARRAYSIZE(s_wave_formats); ++i)
    {
        const WAVE_FORMAT_INFO& item = s_wave_formats[i];
        if (item.flags & wic.dwFormats)
        {
            TCHAR szFormat[128];
            StringCbPrintf(szFormat, sizeof(szFormat), LoadStringDx(100),
                           item.samples, item.bits, item.channels);
            ComboBox_AddString(hCmb2, szFormat);
            g_formats.push_back(item);
        }
    }

    ComboBox_SetCurSel(hCmb2, 0);
}

BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
    UINT nNumDevs = ::waveInGetNumDevs();

    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    ComboBox_AddString(hCmb1, LoadStringDx(101));

    WAVEINCAPS wic;
    for (UINT i = 0; i < nNumDevs; ++i)
    {
        if (::waveInGetDevCaps(i, &wic, sizeof(wic)) == MMSYSERR_NOERROR)
        {
            ComboBox_AddString(hCmb1, wic.szPname);
        }
        else
        {
            break;
        }
    }

    ComboBox_SetCurSel(hCmb1, 0);
    OnChooseItem(hwnd, WAVE_MAPPER);

    return TRUE;
}

void OnOK(HWND hwnd)
{
    HWND hCmb1 = GetDlgItem(hwnd, cmb1);
    INT iDev = ComboBox_GetCurSel(hCmb1) - 1;

    HWND hCmb2 = GetDlgItem(hwnd, cmb2);
    SIZE_T i = ComboBox_GetCurSel(hCmb2);
    if (i < g_formats.size())
    {
        const WAVE_FORMAT_INFO& item = g_formats[i];

        TCHAR szFormat[128];
        StringCbPrintf(szFormat, sizeof(szFormat), LoadStringDx(102),
                       iDev, item.samples, item.bits, item.channels);
        MessageBox(hwnd, szFormat, LoadStringDx(103), MB_ICONINFORMATION);
    }

    EndDialog(hwnd, IDOK);
}

void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
    switch (id)
    {
    case IDOK:
        OnOK(hwnd);
        break;
    case IDCANCEL:
        EndDialog(hwnd, id);
        break;
    case cmb1:
        if (codeNotify == CBN_SELCHANGE)
        {
            HWND hCmb1 = GetDlgItem(hwnd, cmb1);
            INT iDev = ComboBox_GetCurSel(hCmb1);
            OnChooseItem(hwnd, iDev - 1);
        }
        break;
    case cmb2:
        break;
    }
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
        HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
    }
    return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
    InitCommonControls();
    DialogBox(hInstance, MAKEINTRESOURCE(100), NULL, DialogProc);
    return 0;
}
