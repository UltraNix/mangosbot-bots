/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PerformanceMonitor_H
#define _PerformanceMonitor_H

#include "Common.h"
#include <mutex>
#include <chrono>
#include <ctime>

struct PerformanceData
{
    uint32 minTime;
    uint32 maxTime;
    uint32 totalTime;
    uint32 count;
    std::mutex lock;
};

enum PerformanceMetric
{
    PERF_MON_TRIGGER,
    PERF_MON_VALUE,
    PERF_MON_ACTION,
    PERF_MON_RNDBOT,
    PERF_MON_TOTAL
};

class PerformanceMonitorOperation
{
    public:
        PerformanceMonitorOperation(PerformanceData* data);
        void finish();

    private:
        PerformanceData* data;
        std::chrono::milliseconds started;
};

class PerformanceMonitor
{
    public:
        PerformanceMonitor() { };
        virtual ~PerformanceMonitor() { };
        static PerformanceMonitor* instance()
        {
            static PerformanceMonitor instance;
            return &instance;
        }

	public:
        PerformanceMonitorOperation* start(PerformanceMetric metric, std::string name);
        void PrintStats();
        void Reset();

	private:
        std::map<PerformanceMetric, std::map<std::string, PerformanceData*> > data;
        std::mutex lock;
};


#define sPerformanceMonitor PerformanceMonitor::instance()

#endif
