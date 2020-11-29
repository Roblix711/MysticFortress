
// main header
#include "Scheduler.h"
#include <algorithm>

// constructor
RAZOR::SCHEDULER::SCHEDULER(RAZOR::ENGINECORE *MyParent) : ParentEngine(MyParent)
{
    NOCODE;
}

// stop all threads
RAZOR::SCHEDULER::~SCHEDULER()
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(ScheduleLock);

    // start all
    for(auto Schedule = ScheduleList.begin(); Schedule != ScheduleList.end(); ++Schedule)
    {
        delete Schedule->Thread;
    }
}

// proxy draw
static void StaticDrawFunction(VECTOR<METADATA> MetaData)
{
    // call the stored draw!
    RAZOR::SCHEDULER *Scheduler = static_cast<RAZOR::SCHEDULER*>(MetaData[0]);
    
    // capture bad stuff
    try                          {        if (Scheduler->DrawCallback != NULL) 
                                              Scheduler->ParentEngine->DirectX9->Render(Scheduler->DrawCallback);          } 
    catch(const EXCEPTION &e)    {        Scheduler->ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::SCHEDULER::Draw(): %s\n", e.what());             }
    catch(...)                   {        Scheduler->ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::SCHEDULER::Draw(): UNHANDLED EXCEPTION!\n");     }
}

// proxy logic 
static void StaticLogicFunction(VECTOR<METADATA> MetaData)
{
    // call the stored draw!
    RAZOR::SCHEDULER *Scheduler = static_cast<RAZOR::SCHEDULER*>(MetaData[0]);

    // capture bad stuff
    try                          {        if (Scheduler->LogicCallback != NULL) 
                                              Scheduler->LogicCallback();                                                  } 
    catch(const EXCEPTION &e)    {        Scheduler->ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::SCHEDULER::Logic(): %s\n", e.what());             }
    catch(...)                   {        Scheduler->ParentEngine->Console->Log(DEBUG_WARN, "RAZOR::SCHEDULER::Logic(): UNHANDLED EXCEPTION!\n");     }

}

// static task manager
static void StaticTask(NOVA::THREAD* Thread, VECTOR<METADATA> MetaData)
{
    // extract meta data
    RAZOR::SCHEDULER *Scheduler = static_cast<RAZOR::SCHEDULER*>(MetaData[0]);
    SCHEDULE_TASK *MetaDataCallback = static_cast<SCHEDULE_TASK*>(MetaData[1]);

    // use clock params
    NOVA::TIMER Clock;  Clock.Tick();
    DOUBLE CalculatedDifference = 0.0;
    DOUBLE LogicalDifference = 0.0;
    DOUBLE MinimumLogicalDifference = MetaDataCallback->DesiredTimeMS;

    // while loop to handle timing
    while (Thread->IsExiting() == false) //(FOREVER) // ParentEngine->bInit == true && ParentEngine->bSafeExit == false && ParentEngine->bForceExit == false
    {
        // Wait on the infinite loop
        Sleep(1);

        // add the calculated difference to the running total of logical and drawing difference
        CalculatedDifference = Clock.Tock(true);
        LogicalDifference += CalculatedDifference;
        LogicalDifference = std::min(MinimumLogicalDifference*10, LogicalDifference);
        Sleep(0);

        // for each block of MLD time difference, execute a logical function
        while (LogicalDifference >= MinimumLogicalDifference && Thread->IsExiting() == false)
        {
            // call the TickFunction associated
            if (MetaDataCallback->TargetCallack != NULL)
                MetaDataCallback->TargetCallack(MetaDataCallback->MetaData);

            // decrement logical difference since it was served
            LogicalDifference -= MinimumLogicalDifference;
        }
    }   

}

// starts the threads 
UINT32 RAZOR::SCHEDULER::Initialize(FUNCTION_LINKS InputLinks, WINDOW_PARAMS WindowParams)
{
    // transfer
    DrawCallback = InputLinks.DrawFunc;
    LogicCallback = InputLinks.TickFunc;

    // ScheduleTask
    SCHEDULE_TASK DrawTask = 
    {
        "DrawTask",
        StaticDrawFunction, 
        (1000.0 / WindowParams.DesiredFPS),
        {this}
    };

    // ScheduleTask
    SCHEDULE_TASK LogicTask =
    {
        "LogicTask",
        StaticLogicFunction,
        (1000.0 / WindowParams.DesiredLPS),
        {this}
    };

    // Add initial draw/logic tasks
    ScheduleList.reserve(8);
    ScheduleTask(DrawTask);
    ScheduleTask(LogicTask);

    return 0;
}

// schedules a timed task! 
void RAZOR::SCHEDULER::ScheduleTask(SCHEDULE_TASK &Callback)
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(ScheduleLock);

    // add it to the vector    
    ScheduleList.push_back( Callback );
    THREAD_CONFIG ThreadConfig = THREAD_CONFIG{StaticTask, 0, {this, &ScheduleList.back() }  };
    ScheduleList.back().Thread = new NOVA::THREAD( {&ThreadConfig} );

    // possible multithreading issue here: ScheduleList.back presents a reference, 
    // which can change if ScheduleList resizes
    // SO, instead, give the thread the index, and the object reference instead (update)

}

// stops internal tasks
void RAZOR::SCHEDULER::StopAll()
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(ScheduleLock);

    // start all
    for(auto Schedule = ScheduleList.begin(); Schedule != ScheduleList.end(); ++Schedule)
    {
        //ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::SCHEDULER::StopAll(): STOPPING TASK:[%s] @ %3.2lf\n", Schedule->Name.c_str(), 1000.0/Schedule->DesiredTimeMS);
        Schedule->Thread->Stop();
        Schedule->Thread->WaitForStop();
    }

}

// starts internal tasks
void RAZOR::SCHEDULER::StartAll()
{
    // threadsafe
    NOVA::SCOPE_LOCK AutoLock(ScheduleLock);

    // start all
    for(auto Schedule = ScheduleList.begin(); Schedule != ScheduleList.end(); ++Schedule)
    {
        //ParentEngine->Console->Log(DEBUG_INFO, "RAZOR::SCHEDULER::StartAll(): STARTING TASK:[%s] @ %3.2lf\n", Schedule->Name.c_str(), 1000.0/Schedule->DesiredTimeMS);
        Schedule->Thread->Start();
        Schedule->Thread->WaitForStart();
    }

}