
// header gaurd
#ifndef _RAZOR_ENGINE_SCHEDULER_H
#define _RAZOR_ENGINE_SCHEDULER_H

// headers
#include "Engine.h"

// scheduler types
struct SCHEDULE_TASK
{
    STRING                      Name;
    VOID_FUNCTION_POINTER_LPV   TargetCallack;
    DOUBLE                      DesiredTimeMS;   
    VECTOR<METADATA>            MetaData;
    NOVA::THREAD*               Thread;

    SCHEDULE_TASK(STRING N, VOID_FUNCTION_POINTER_LPV P, DOUBLE T, VECTOR<METADATA> M, NOVA::THREAD* TR = NULL): Name(N), TargetCallack(P), DesiredTimeMS(T), MetaData(M), Thread(TR)
    {
        NOCODE;
    }
};

// this is a helper class
class RAZOR::SCHEDULER
{
    public:
    RAZOR::ENGINECORE *ParentEngine;
    VOID_FUNCTION_POINTER DrawCallback;
    VOID_FUNCTION_POINTER LogicCallback;
    VECTOR<SCHEDULE_TASK> ScheduleList;
    NOVA::MUTEX_FAST ScheduleLock;

    // computed
    DOUBLE MinimumLogicalDifference;	// = 1000.0/Timer.LogicalFramesDesired;
    DOUBLE MinimumDrawingDifference;	// = 1000.0/Timer.DrawingFramesDesired;

    // constructor
    SCHEDULER(RAZOR::ENGINECORE *MyParent);
    ~SCHEDULER();

    UINT32 Initialize(RAZOR::FUNCTION_LINKS InputLinks, RAZOR::WINDOW_PARAMS WindowParams);
    void ScheduleTask(SCHEDULE_TASK &Callback);

    void StopAll();
    void StartAll();
};

#endif