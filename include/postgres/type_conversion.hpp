#pragma once

#include <string>
#include <context.hpp>

namespace postgres {
    std::string to_type(const db::column_value_type &c);
    db::column_value_type from_type(const std::string &t, const std::string def, const bool _primary_key = false, const bool _unique_key = false, const bool _not_null = false);
}
