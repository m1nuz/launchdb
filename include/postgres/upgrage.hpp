#pragma once

#include <context.hpp>

namespace postgres {
    int upgrade(const db::context_diff &ctx);
}
