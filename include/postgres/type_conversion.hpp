#pragma once

#include <string>
#include <context.hpp>

namespace postgres {
    ///
    /// \brief Convert complex type to postgres type
    /// \param[in] c Input type
    /// \return Postgres type
    ///
    std::string to_type(const db::column_value_type &c) noexcept;

    ///
    /// \brief Get type from postgres type
    /// \param[in] t Type name
    /// \param[in] size Size of type
    /// \param[in] def Default value
    /// \param[in] _primary_key True if primary key
    /// \param[in] _unique_key True if unique key or part of that
    /// \param[in] _not_null True if can't be null
    /// \return Complex type
    ///
    db::column_value_type from_type(const std::string &t, const size_t size, const std::string &def, const bool _primary_key = false, const bool _unique_key = false, const bool _not_null = false) noexcept;
}
