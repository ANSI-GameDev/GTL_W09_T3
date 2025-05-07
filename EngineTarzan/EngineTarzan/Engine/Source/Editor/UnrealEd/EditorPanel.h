#pragma once

#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT        0
#define    FEATHER_FONT        1

#endif // !__ICON_FONT_INDEX__

//~ Windows.h
#define _TCHAR_DEFINED  // TCHAR 재정의 에러 때문
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class UEditorPanel
{
public:
    virtual ~UEditorPanel() = default;
    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;
};
