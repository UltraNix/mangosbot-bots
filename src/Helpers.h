/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "Common.h"

template<typename T>
std::map<uint32, T> filterList(std::vector<T> src, std::string const& filter)
{
    std::map<uint32, T> result;
    if (filter.empty() || filter == "*")
    {
        uint32 idx = 0;
        for (std::vector<T>::iterator i = src.begin(); i != src.end(); ++i)
            result[idx++] = *i;

        return result;
    }

    if (filter.find("-") != std::string::npos)
    {
        std::vector<std::string> ss = split(filter, '-');
        uint32 from = 0;
        uint32 to = src.size() - 1;

        if (!ss[0].empty())
            from = atoi(ss[0].c_str()) - 1;

        if (ss.size() > 1 && !ss[1].empty())
            to = atoi(ss[1].c_str()) - 1;

        if (from < 0)
            from = 0;

        if (from > src.size() - 1)
            from = src.size() - 1;

        if (to < 0)
            to = 0;

        if (to > src.size() - 1)
            to = src.size() - 1;

        for (uint32 i = from; i <= to; ++i)
            result[i] = src[i];

        return result;
    }

    std::vector<std::string> ss = split(filter, ',');
    for (std::vector<std::string>::iterator i = ss.begin(); i != ss.end(); ++i)
    {
        uint32 idx = atoi(i->c_str()) - 1;

        if (idx < 0)
            idx = 0;

        if (idx > src.size() - 1)
            idx = src.size() - 1;

        result[idx] = src[idx];
    }

    return result;
}
