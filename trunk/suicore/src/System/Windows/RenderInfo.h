// 华勤科技版权所有 2008-2022
// 
// 文件名：uihwndobject.h
// 功  能：封装windows的句柄。
// 
// 作  者：MPF开发组
// 时  间：2010-07-02
// 
// ============================================================================

#ifndef _UIRENDERINFO_H_
#define _UIRENDERINFO_H_

#include <System/Types/Interface.h>
#include <System/Tools/Array.h>

#include <System/Graphics/Bitmap.h>
#include <System/Windows/RenderTarget.h>
#include <System/Windows/FrameworkElement.h>

#include <System/Input/HwndMouseFilter.h>
#include <System/Input/HwndKeyboardFilter.h>

#include <System/Tools/SSE.h>

namespace suic
{

class Win32Bitmap
{
public:

    Win32Bitmap()
    {
        hbmp = NULL;
        autodel = true;
    }

    ~Win32Bitmap()
    {
        Clear();
    }

    void Clear()
    {
        if (hbmp && autodel)
        {
            ::DeleteObject(hbmp);
            hbmp = NULL;
        }
        bmp.Clear();
    }

    void CreateBmp(HDC hdc, int w, int h)
    {
        Clear();

        autodel = true;
        hbmp = CreateBitmap(hdc, &bmp, w, h);
    }

    static HBITMAP CreateBitmap(HDC hdc, Bitmap* pBmp, int w, int h)
    {
        BITMAPINFO bmi = {0};
        HBITMAP hBmp = NULL;

        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = w;
        bmi.bmiHeader.biHeight = -h;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biSizeImage = 4 * h * w;
        Byte* rq = NULL;

        hBmp = ::CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, (LPVOID*)&rq, NULL, 0);

        pBmp->SetConfig(w, h, 32);
        pBmp->SetPixels(rq, (Handle)(LONG_PTR)hBmp, false);
        CSSE::SetColorKey(rq, bmi.bmiHeader.biSizeImage, 0);

        return hBmp;
    }

    Bitmap* GetBitmap()
    {
        return &bmp;
    }

    HBITMAP GetHBitmap()
    {
        return hbmp;
    }

    void SetHBitmap(HBITMAP bmp, bool bDel)
    {
        if (hbmp && autodel)
        {
            ::DeleteObject(hbmp);
        }

        hbmp = bmp;
        autodel = bDel;
    }

private:

    Bitmap bmp;
    HBITMAP hbmp;
    bool autodel;
};

class RenderInfo
{
public:

    RenderInfo();
    ~RenderInfo();

    bool OnFilterMessage(Element* rootElement, MessageParam* mp);
    bool OnHookFilterMessage(Element* rootElement, MessageParam* mp, bool& interrupt);

    Win32Bitmap* GetDib();
 
    bool DrawDebugLine() const;
    void SetDrawDebugLine(bool val);

    bool IsNeedRender();
    void SetNeedRender(bool val);
    HwndObject* GetHwndObject();
    HWND GetHostHwnd();

    void InitHwndObject(HwndObject* obj);

    void RegisterHook(Object* obj, MessageHook hook);
    void RemmoveHook(Object* obj, MessageHook hook);

    void AddClip(Rect rect);
    bool PopClip(Rect& rect);
    void CombineClips();
    int GetClipCount() const;

    bool BeginRender();
    void EndRender();

    bool GetClipRender(Rect& clip);
    void SetClipRender(Rect rect, bool bUnion);

    void SetSizeToContent(int val)
    {
        if (NULL != _hwndObj)
        {
            _hwndObj->SetSizeToContent(val);
        }
    }

    void AddArrangeElement(Element* elem)
    {
        _arrangs.Add(WeakRef(elem));
    }

public:

    HwndMouseFilter _mouseFilter;
    HwndKeyboardFilter _keyboardFilter;

    HookInfo _hooks;
    Win32Bitmap _dib;

    Rect _clipRender;

    bool _drawDebugLine;
    bool _needRender;
    bool _isRendering;

    StringAutoDic<Object> Context;
    Array<WeakRef> _arrangs;

protected:

    HwndObject* _hwndObj;
    InvManager _clipRect;
};

inline HwndObject* RenderInfo::GetHwndObject() 
{ 
    return _hwndObj; 
}

inline HWND RenderInfo::GetHostHwnd()
{
    if (NULL == _hwndObj)
    {
        return NULL;
    }
    else
    {
        return HANDLETOHWND(_hwndObj->GetHandle());
    }
}

inline Win32Bitmap* RenderInfo::GetDib()
{
    return &_dib;
}

inline bool RenderInfo::DrawDebugLine() const
{
    return _drawDebugLine;
}

inline void RenderInfo::SetDrawDebugLine(bool val)
{
    _drawDebugLine = val;
}

inline bool RenderInfo::IsNeedRender()
{
    return _needRender;
}

inline void RenderInfo::SetNeedRender(bool val)
{
    _needRender = val;
}

inline void RenderInfo::AddClip(Rect rect)
{
    _clipRect.AddClip(rect);
    /*if (_hwndObj)
    {
        _hwndObj->AddInvalidateRect(rect);
    }*/
}

inline bool RenderInfo::PopClip(Rect& rect)
{
    return _clipRect.PopClip(rect);
}

inline void RenderInfo::CombineClips()
{
    _clipRect.CombineClips();
}

inline int RenderInfo::GetClipCount() const
{
    return _clipRect.GetClipCount();
}

inline void RenderInfo::SetClipRender(Rect rect, bool bUnion)
{
    if (bUnion)
    {
        _clipRender.Union(&rect);
    }
    else
    {
        _clipRender = rect;
    }
}

inline bool RenderInfo::GetClipRender(Rect& clip)
{
    if (_clipRender.Empty())
    {
        return false;
    }
    else
    {
        clip = _clipRender;
        return true;
    }
}

inline bool RenderInfo::BeginRender()
{
    KillTimer(GetHostHwnd(), 1);

    if (_clipRender.Empty() || _isRendering)
    {
        return false;
    }
    else
    {
        _isRendering = true;
        return true;
    }
}

inline void RenderInfo::EndRender()
{
    _isRendering = false;
    _clipRender.SetEmpty();

    if (GetClipCount() > 0)
    {
        if (PopClip(_clipRender))
        {
            ::SetTimer(GetHostHwnd(), 1, 0, NULL);
        }
    }
}

class SysRenderSource : public VisualHost
{
public:

    RTTIOfClass(SysRenderSource)

    SysRenderSource(HwndParam hp);
    SysRenderSource();
    virtual ~SysRenderSource();

    Handle GetHandle();

    bool Initialize(Element* rootElement);

    void SetHwnd(Handle hwnd);
    FrameworkElement* GetRootElement();

    void StopMouseTrack();
    void StartMouseTrack();

    void Close(bool bAsync);

    Point GetScreenPoint();

    HwndObject* GetHwndObject();
    Win32Bitmap* GetDib();
};

inline HwndObject* SysRenderSource::GetHwndObject()
{
    RenderInfo* pInfo(GetRenderInfo());
    return pInfo->GetHwndObject();
}

inline Win32Bitmap* SysRenderSource::GetDib()
{
    RenderInfo* pInfo(GetRenderInfo());
    return pInfo->GetDib();
}

}

#endif
