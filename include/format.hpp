#pragma once

#include <vector>
#include <string>
#include <sstream>

namespace xfmt {
    using std::string;
    using std::vector;


    inline string to_string(const int x) {
        return std::to_string(x);
    }

    inline string to_string(const unsigned int x) {
        return std::to_string(x);
    }

    inline string to_string(const long x) {
        return std::to_string(x);
    }

    inline string to_string(const unsigned long x) {
        return std::to_string(x);
    }

    inline string to_string(const long long x) {
        return std::to_string(x);
    }

    inline string to_string(const unsigned long long x) {
        return std::to_string(x);
    }

    inline string to_string(const float x) {
        return std::to_string(x);
    }

    inline string to_string(const double x) {
        return std::to_string(x);
    }

    inline string to_string(const long double x) {
        return std::to_string(x);
    }

    inline string to_string(const char *x) {
        return string{x};
    }

    inline string to_string(const std::string &x) noexcept {
        return x;
    }

    template <typename T> inline string to_string(const vector<T> &v) {
        constexpr const char split_symbols[] = ", ";

        string res;
        res.reserve(v.size() * sizeof (T) + sizeof split_symbols * v.size());

        for (const auto &x : v) {
            if (!res.empty())
                res += split_symbols;

            res += to_string(x);
        }

        return res;
    }

    using std::stringstream;

    template <typename T> inline string to_string(const T &t) {
        stringstream ss;
        ss << t;
        return ss.str();
    }

    inline string format_impl(const string &fmt, const std::vector<string> &strs) {
        constexpr const char format_symbol = '%';
        constexpr const size_t max_digits = 10;

        string res;
        string buf;
        bool arg = false;
        for (size_t i = 0; i <= fmt.size(); ++i) {
            const bool last = i == fmt.size();
            const char ch = fmt[i];
            if (arg) {
                if (ch >= '0' && ch <= '9') {
                    buf += ch;
                } else {
                    int num = 0;
                    if (!buf.empty() && buf.length() < max_digits)
                        num = atoi(buf.c_str());
                    if (num >= 1 && num <= static_cast<int>(strs.size()))
                        res += strs[num - 1];
                    else
                        res += format_symbol + buf;
                    buf.clear();

                    if (ch != format_symbol) {
                        if (!last)
                            res += ch;
                        arg = false;
                    }
                }
            } else {
                if (ch == format_symbol) {
                    arg = true;
                } else {
                    if (!last)
                        res += ch;
                }
            }
        }

        return res;
    }

    template<typename Arg, typename ... Args> inline string format_impl(const string& fmt, std::vector<string>& strs, Arg&& arg, Args&& ... args) {
        strs.push_back(to_string(std::forward<Arg>(arg)));
        return format_impl(fmt, strs, std::forward<Args>(args) ...);
    }

    inline string format(const std::string& fmt) {
        return fmt;
    }

    template<typename Arg, typename ... Args> inline string format(const string& fmt, Arg&& arg, Args&& ... args) {
        constexpr int n = sizeof...(Args) + 1;
        vector<string> strs;
        strs.reserve(n);
        return format_impl(fmt, strs, std::forward<Arg>(arg), std::forward<Args>(args) ...);
    }
}
