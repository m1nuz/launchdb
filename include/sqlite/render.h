#pragma once

#include <context.hpp>

namespace sqlite {
    int render(const db::context &ctx, const db::config &cfg);
    int upgrade(const db::context_diff &ctx);
}
