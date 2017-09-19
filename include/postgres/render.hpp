#pragma once

#include <context.hpp>

namespace postgres {
    int render(const db::context &ctx, const db::config &cfg);
}
