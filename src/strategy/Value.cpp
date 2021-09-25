/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Value.h"
#include "PerformanceMonitor.h"
#include "Playerbot.h"

template<class T>
T CalculatedValue<T>::Get()
{
    time_t now = time(nullptr);
    if (!lastCheckTime || checkInterval < 2 || now - lastCheckTime >= checkInterval / 2)
    {
        lastCheckTime = now;

        PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_VALUE, getName());
        value = Calculate();
        if (pmo)
            pmo->finish();
    }

    return value;
}

UnitCalculatedValue::UnitCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1) : CalculatedValue<Unit*>(botAI, name, checkInterval)
{
    lastCheckTime = time(nullptr) - checkInterval / 2;
}

std::string const& UnitCalculatedValue::Format()
{
    Unit* unit = Calculate();
    return unit ? unit->GetName() : "<none>";
}

std::string const& UnitManualSetValue::Format()
{
    Unit* unit = Get();
    return unit ? unit->GetName() : "<none>";
}

std::string const& CDPairCalculatedValue::Format()
{
    CreatureData const* creatureData = Calculate();
    CreatureTemplate const* bmTemplate = sObjectMgr->GetCreatureTemplate(creatureData->id);
    return bmTemplate ? bmTemplate->Name : "<none>";
}

std::string const& CDPairListCalculatedValue::Format()
{
    std::ostringstream out; out << "{";
    std::vector<CreatureData const*> cdPairs = Calculate();
    for (CreatureData const* cdPair : cdPairs)
    {
        out << cdPair->id << ",";
    }

    out << "}";
    return out.str();
}

std::string const& ObjectGuidCalculatedValue::Format()
{
    ObjectGuid guid = Calculate();
    return guid ? std::to_string(guid.GetRawValue()) : "<none>";
}
