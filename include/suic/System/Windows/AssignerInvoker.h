// 华勤科技版权所有 2008-2022
// 
// 文件名：uiassignerinvoker.h
// 功  能：定义核心库的事件处理。
// 
// 作  者：MPF开发组
// 时  间：2010-07-02
// 
// ============================================================================

#ifndef _UIASSIGNERINVOKER_H_
#define _UIASSIGNERINVOKER_H_

#include <System/Windows/EventHandler.h>

namespace suic
{

enum InvokerPriority
{
    // 优先级最高
    eSend = -1,
    // 界面呈现
    eRender = 0,
    eNormal = 1,
    eInvalid = 2,
    eInput = 3,
    eInactive = 4,
    eBackgound = 5,
};

enum AssignerInvokerStatus
{
    eCompleted = 1,
    eAborted = 2,
};

class Assigner;
typedef shared<Assigner> AssignerPtr;

class AssignerInvoker;
typedef shared<AssignerInvoker> AssignerInvokerPtr;

class SUICORE_API AssignerInvoker : public Object
{
public:

    EventHandler Aborted; 
    EventHandler Completed;

    AssignerInvoker();
    ~AssignerInvoker();

    Assigner* GetAssigner();
    InvokerPriority GetPriority();
    void SetPriority(InvokerPriority value);

    AssignerInvokerStatus GetStatus();
    
    bool Abort();
    AssignerInvokerStatus Wait(); 
    AssignerInvokerStatus Wait(Int32 timeout); 

    virtual void OnComplete();
    virtual void Invoke() = 0;

protected:

    Handle _signal;
    InvokerPriority _priority;
    AssignerInvokerStatus _status;

    friend class Assigner;
};

typedef delegate<void()> InvokerOne;

class SUICORE_API AssignerOneInvoker : public AssignerInvoker
{
public:

    AssignerOneInvoker(InvokerOne val)
    {
        _invoke = val;
    }

    void Invoke()
    {
        if (_invoke)
        {
            _invoke();
        }
    }

protected:

    InvokerOne _invoke;
};

};

#endif

