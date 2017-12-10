#pragma once

#include <context.hpp>

namespace sqlite {
    int render(const db::context &ctx, const db::config &cfg);
    int upgrade(const db::context_diff &ctx);

    db::column_value_type from_type(const std::string &t, const size_t size, const std::string &def, const bool _primary_key = false, const bool _unique_key = false, const bool _not_null = false) noexcept;
}
