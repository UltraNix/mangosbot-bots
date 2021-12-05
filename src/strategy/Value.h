/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_VALUE_H
#define _PLAYERBOT_VALUE_H

#include "AiObject.h"
#include "ObjectGuid.h"

class PlayerbotAI;
class Unit;

struct CreatureData;

class UntypedValue : public AiNamedObject
{
    public:
        UntypedValue(PlayerbotAI* botAI, std::string const& name) : AiNamedObject(botAI, name) { }
        virtual void Update() { }
        virtual void Reset() { }
        virtual std::string const& Format() { return "?"; }
        virtual std::string const& Save() { return "?"; }
        virtual bool Load(std::string const& value) { return false; }
};

template<class T>
class Value
{
    public:
        virtual T Get() = 0;
        virtual T LazyGet() = 0;
        virtual void Reset() {}
        virtual void Set(T value) = 0;
        operator T() { return Get(); }
};

template<class T>
class CalculatedValue : public UntypedValue, public Value<T>
{
	public:
        CalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", uint32 checkInterval = 1) : UntypedValue(botAI, name),
            checkInterval(checkInterval), lastCheckTime(0) { }

        virtual ~CalculatedValue() { }

        T Get() override;
        T LazyGet() override
        {
            if (!lastCheckTime)
                return Get();

            return value;
        }

        void Set(T val) override { value = val; }
        void Update() override { }
        virtual void Reset() { lastCheckTime = 0; }

    protected:
        virtual T Calculate() = 0;

    protected:
		uint32 checkInterval;
		time_t lastCheckTime;
        T value;
};

template <class T> class SingleCalculatedValue : public CalculatedValue<T>
{
public:
        SingleCalculatedValue(PlayerbotAI* ai, string name = "value") : CalculatedValue(ai, name)
        {
            Reset();
        }

        T Get() override;
};

template<class T>
class MemoryCalculatedValue : public CalculatedValue<T>
{
    public:
        MemoryCalculatedValue(PlayerbotAI* ai, string name = "value", int32 checkInterval = 1) : CalculatedValue<T>(ai, name,checkInterval)
        {
            lastChangeTime = time(0);
        }

        virtual bool EqualToLast(T value) = 0;
        virtual bool CanCheckChange()
        {
            return time(0) - lastChangeTime < minChangeInterval || EqualToLast(value);
        }

        virtual bool UpdateChange()
        {
            if (CanCheckChange())
                return false;

            lastChangeTime = time(0);
            lastValue = value;
            return true;
        }

        void Set(T value) override
        {
            CalculatedValue<T>::Set(value);
            UpdateChange();
        }

        T Get() override
        {
            value = CalculatedValue<T>::Get();
            UpdateChange();
            return value;
        }

        T LazyGet() override
        {
            return value;
        }

        time_t LastChangeOn()
        {
            Get();
            UpdateChange();
            return lastChangeTime;
        }

        uint32 LastChangeDelay() { return time(0) - LastChangeOn(); }

        void Reset() override
        {
            CalculatedValue::Reset();
            lastChangeTime = time(0);
        }

    protected:
        T lastValue;
        uint32 minChangeInterval = 0;
        time_t lastChangeTime;
};

template<class T>
class LogCalculatedValue : public MemoryCalculatedValue<T>
{
    public:
        LogCalculatedValue(PlayerbotAI* ai, string name = "value", int32 checkInterval = 1) : MemoryCalculatedValue<T>(ai, name, checkInterval) {}

        bool UpdateChange() override
        {
            if (MemoryCalculatedValue::UpdateChange())
                return false;

            valueLog.push_back(make_pair(value, time(0)));

            if (valueLog.size() > logLength)
                valueLog.pop_front();
            return true;
        }

        list<pair<T, time_t>> ValueLog() { return valueLog; }

        void Reset() override
        {
            MemoryCalculatedValue::Reset();
            valueLog.clear();
        }

    protected:
        list<pair<T, time_t>> valueLog;
        uint8 logLength = 10;
    };

class Uint8CalculatedValue : public CalculatedValue<uint8>
{
    public:
        Uint8CalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", uint32 checkInterval = 1) :
            CalculatedValue<uint8>(botAI, name, checkInterval) { }

        std::string const& Format() override
        {
            std::ostringstream out;
            out << Calculate();
            return out.str();
        }
};

class Uint32CalculatedValue : public CalculatedValue<uint32>
{
    public:
        Uint32CalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int checkInterval = 1) :
            CalculatedValue<uint32>(botAI, name, checkInterval) { }

        std::string const& Format() override
        {
            std::ostringstream out;
            out << Calculate();
            return out.str();
        }
};

class FloatCalculatedValue : public CalculatedValue<float>
{
    public:
        FloatCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int checkInterval = 1) :
            CalculatedValue<float>(botAI, name, checkInterval) { }

        std::string const& Format() override
        {
            std::ostringstream out;
            out << Calculate();
            return out.str();
        }
};

class BoolCalculatedValue : public CalculatedValue<bool>
{
    public:
        BoolCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int checkInterval = 1) :
            CalculatedValue<bool>(botAI, name, checkInterval) { }

        std::string const& Format() override
        {
            return Calculate() ? "true" : "false";
        }
};

class UnitCalculatedValue : public CalculatedValue<Unit*>
{
    public:
        UnitCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1);

        std::string const& Format() override;
};

class CDPairCalculatedValue : public CalculatedValue<CreatureData const*>
{
public:
    CDPairCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1) : CalculatedValue<CreatureData const*>(botAI, name, checkInterval)
    {
        lastCheckTime = time(nullptr) - checkInterval / 2;
    }

    std::string const& Format() override;
};

class CDPairListCalculatedValue : public CalculatedValue<std::vector<CreatureData const*>>
{
public:
    CDPairListCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1) : CalculatedValue<std::vector<CreatureData const*>>(botAI, name, checkInterval)
    {
        lastCheckTime = time(nullptr) - checkInterval / 2;
    }

    std::string const& Format() override;
};

class ObjectGuidCalculatedValue : public CalculatedValue<ObjectGuid>
{
public:
    ObjectGuidCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1) : CalculatedValue<ObjectGuid>(botAI, name, checkInterval)
    {
        lastCheckTime = time(nullptr) - checkInterval / 2;
    }

    std::string const& Format() override;
};

class ObjectGuidListCalculatedValue : public CalculatedValue<GuidVector>
{
    public:
        ObjectGuidListCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", int32 checkInterval = 1) : CalculatedValue<GuidVector>(botAI, name, checkInterval)
        {
            lastCheckTime = time(nullptr) - checkInterval / 2;
        }

        std::string const& Format() override
        {
            std::ostringstream out;
            out << "{";

            GuidVector guids = Calculate();
            for (GuidVector::iterator i = guids.begin(); i != guids.end(); ++i)
            {
                ObjectGuid guid = *i;
                out << guid.GetRawValue() << ",";
            }
            out << "}";

            return out.str();
        }
};

template<class T>
class ManualSetValue : public UntypedValue, public Value<T>
{
    public:
        ManualSetValue(PlayerbotAI* botAI, T defaultValue, std::string const& name = "value") :
            UntypedValue(botAI, name), value(defaultValue), defaultValue(defaultValue) { }

        virtual ~ManualSetValue() { }

        T Get() override { return value; }
        void Set(T val) override { value = val; }
        void Update() override { }
        void Reset() override
        {
            value = defaultValue;
        }

    protected:
        T value;
        T defaultValue;
};

class UnitManualSetValue : public ManualSetValue<Unit*>
{
    public:
        UnitManualSetValue(PlayerbotAI* botAI, Unit* defaultValue, std::string const& name = "value") :
            ManualSetValue<Unit*>(botAI, defaultValue, name) { }

        std::string const& Format() override;
};

#endif
