/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

class Qualified
{
    public:
        Qualified() { };
        Qualified(std::string const& qualifier) : qualifier(qualifier) { }
        Qualified(uint32 qualifier1)
        {
            Qualify(qualifier1);
        }

        virtual void Qualify(uint32 qual)
        {
            std::ostringstream out;
            out << qual;
            qualifier = out.str();
        }

        virtual void Qualify(std::string const& qual)
        {
            qualifier = qual;
        }

        std::string const& getQualifier() { return qualifier; }

    protected:
        std::string qualifier;
};

template <class T>
class NamedObjectFactory
{
    protected:
        typedef T*(*ActionCreator)();
        std::map<std::string, ActionCreator> creators;

    public:
        T* create(std::string const& name)
        {
            size_t found = name.find("::");
            std::string qualifier;
            if (found != std::string::npos)
            {
                qualifier = name.substr(found + 2);
                name = name.substr(0, found);
            }

            if (creators.find(name) == creators.end())
                return nullptr;

            ActionCreator creator = creators[name];
            if (!creator)
                return nullptr;

            T* object = (*creator)(botAI);
            Qualified* q = dynamic_cast<Qualified*>(object);
            if (q && found != std::string::npos)
                q->Qualify(qualifier);

            return object;
        }

        std::set<std::string> supports()
        {
            std::set<std::string> keys;
            for (typename std::map<std::string, ActionCreator>::iterator it = creators.begin(); it != creators.end(); it++)
                keys.insert(it->first);

            return std::move(keys);
        }
};

template <class T>
class NamedObjectContext : public NamedObjectFactory<T>
{
    public:
        NamedObjectContext(bool shared = false, bool supportsSiblings = false) :
            NamedObjectFactory<T>(), shared(shared), supportsSiblings(supportsSiblings) { }

        T* create(std::string const& name)
        {
            if (created.find(name) == created.end())
                return created[name] = NamedObjectFactory<T>::create(name);

            return created[name];
        }

        virtual ~NamedObjectContext()
        {
            Clear();
        }

        void Clear()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    delete i->second;
            }

            created.clear();
        }

        void Update()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Update();
            }
        }

        void Reset()
        {
            for (typename std::map<std::string, T*>::iterator i = created.begin(); i != created.end(); i++)
            {
                if (i->second)
                    i->second->Reset();
            }
        }

        bool IsShared() { return shared; }
        bool IsSupportsSiblings() { return supportsSiblings; }

        std::set<std::string> GetCreated()
        {
            std::set<std::string> keys;
            for (typename map<std::string, T*>::iterator it = created.begin(); it != created.end(); it++)
                keys.insert(it->first);

            return std::move(keys);
        }

    protected:
        std::map<std::string, T*> created;
        bool shared;
        bool supportsSiblings;
};

template <class T>
class NamedObjectContextList
{
    public:
        virtual ~NamedObjectContextList()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                NamedObjectContext<T>* context = *i;
                if (!context->IsShared())
                    delete context;
            }
        }

        void Add(NamedObjectContext<T>* context)
        {
            contexts.push_back(context);
        }

        T* GetObject(std::string const& name)
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (T* object = (*i)->create(name))
                    return object;
            }

            return nullptr;
        }

        void Update()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsShared())
                    (*i)->Update();
            }
        }

        void Reset()
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                (*i)->Reset();
            }
        }

        std::set<std::string> GetSiblings(std::string const& name)
        {
            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                if (!(*i)->IsSupportsSiblings())
                    continue;

                std::set<std::string> supported = (*i)->supports();
                std::set<std::string>::iterator found = supported.find(name);
                if (found == supported.end())
                    continue;

                supported.erase(found);
                return supported;
            }

            return std::move(std::set<std::string>());
        }

        std::set<std::string> supports()
        {
            std::set<std::string> std::result;

            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> supported = (*i)->supports();

                for (std::set<std::string>::iterator j = supported.begin(); j != supported.end(); j++)
                    result.insert(*j);
            }

            return std::move(result);
        }

        std::set<std::string> GetCreated()
        {
            std::et<std::string> result;

            for (typename std::vector<NamedObjectContext<T>*>::iterator i = contexts.begin(); i != contexts.end(); i++)
            {
                std::set<std::string> createdKeys = (*i)->GetCreated();

                for (std::set<std::string>::iterator j = createdKeys.begin(); j != createdKeys.end(); j++)
                    result.insert(*j);
            }

            return std::move(result);
        }

    private:
        std::vector<NamedObjectContext<T>*> contexts;
};

template <class T>
class NamedObjectFactoryList
{
    public:
        virtual ~NamedObjectFactoryList()
        {
            for (typename std::vector<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
                delete *i;
        }

        void Add(NamedObjectFactory<T>* context)
        {
            factories.push_front(context);
        }

        T* GetObject(std::string const& name)
        {
            for (typename std::vector<NamedObjectFactory<T>*>::iterator i = factories.begin(); i != factories.end(); i++)
            {
                if (T* object = (*i)->create(name))
                    return object;
            }

            return nullptr;
        }

    private:
        std::vector<NamedObjectFactory<T>*> factories;
};
