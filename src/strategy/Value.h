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
        virtual void Set(T value) = 0;
        operator T() { return Get(); }
};

template <class T>
class SingleCalculatedValue : public UntypedValue, public Value<T>
{
    public:
        SingleCalculatedValue(PlayerbotAI* botAI, std::string const& name = "value") : UntypedValue(botAI, name)
        {
            Reset();
        }

        virtual ~SingleCalculatedValue() { }

        T Get() override
        {
            if (!calculated)
            {
                value = Calculate();
                calculated = true;
            }

            return value;
        }

        void Set(T val) override
        {
            value = val;
        }

        void Update() override { }

        void Reset() override
        {
            calculated = false;
        }

    protected:
        virtual T Calculate() = 0;
        T value;
        bool calculated;
};

template<class T>
class CalculatedValue : public UntypedValue, public Value<T>
{
	public:
        CalculatedValue(PlayerbotAI* botAI, std::string const& name = "value", uint32 checkInterval = 1) : UntypedValue(botAI, name),
            checkInterval(checkInterval), lastCheckTime(time(nullptr) - rand() % checkInterval) { }

        virtual ~CalculatedValue() { }

        T Get() override;
        void Set(T val) { value = val; }
        void Update() override { }

    protected:
        virtual T Calculate() = 0;

    protected:
		uint32 checkInterval;
		time_t lastCheckTime;
        T value;
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
