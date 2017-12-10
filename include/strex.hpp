#pragma once

#include <sstream>
#include <vector>
#include <locale>
#include <algorithm>

namespace strex {
    template <typename T>
    inline T from_string(const std::string& str)
    {
        std::stringstream buff(str);
        T value;
        buff >> value;
        return value;
    }

    constexpr unsigned int hash(const char* str, int h = 0)
    {
        return !str[h] ? 5381 : (hash(str, h + 1) * 33) ^ str[h];
    }

    inline unsigned int hash(const std::string& str)
    {
        return hash(str.c_str());
    }

    inline std::vector<std::string> split(const std::string& text, const std::string& delims) {
        std::vector<std::string> tokens;

        auto start = text.find_first_not_of(delims);
        auto end = std::string::size_type{0};

        while ((end = text.find_first_of(delims, start)) != std::string::npos) {
            tokens.push_back(text.substr(start, end - start));
            start = text.find_first_not_of(delims, end);
        }

        if (start != std::string::npos)
            tokens.push_back(text.substr(start));

        return tokens;
    }


    template <typename C>
    inline std::string join(const C& strs, const std::string& delimiter)
    {
        std::stringstream buff;

        size_t s = 1;
        for (const std::string& str : strs)
        {
            buff << str;
            if (s != strs.size())
            {
                buff << delimiter;
            }
            s++;
        }

        return buff.str();
    }

    inline std::string tolower(const std::string& str, const std::locale& loc = std::locale("C")) {
        std::string result(str.size(), '\0');

        std::transform(str.begin(), str.end(), result.begin(), [&] (char c) {
            return std::tolower(c, loc);
        });

        return result;
    }

    inline std::string toupper(const std::string& str, const std::locale& loc = std::locale("C")) {
        std::string result(str.size(), '\0');

        std::transform(str.begin(), str.end(), result.begin(), [&] (char c) {
            return std::toupper(c, loc);
        });

        return result;
    }
}
