#pragma once

#include <memory>
#include <cstring>

#if defined(_WIN32) || defined(_WIN64)
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#endif

#include <format.hpp>

#include <libpq-fe.h>

namespace postgres {
    using std::shared_ptr;
    using std::string;
    using std::size_t;

    class connection final {
    public:
        connection() = default;

        static connection create(const string &connect_info) {
            return {connect_info};
        }

        bool is_ok() const {
            return PQstatus(_conn.get()) == CONNECTION_OK;
        }

        operator bool() {
            return is_ok();
        }

        string error_message() const {
            const char* message = PQerrorMessage(_conn.get());
            const size_t size = strlen(message);
            return string(message, size - 1);
        }

        friend class query;

    private:
        connection(const string &connect_info) : _conn{PQconnectdb(connect_info.c_str()), [&] (auto *p) { PQfinish(p); }} {

        }

        shared_ptr<PGconn> _conn;
    };

    class query final {
    public:

        class result {
        public:

            class row {
            public:
                friend class result;

                row(const result &res, const size_t r, const size_t offset, const size_t columns) noexcept
                    : _result{res}
                    , _row{r}
                    , _offset{offset}
                    , _columns{columns} {
                }

                const result& _result;
                size_t _row;
                const size_t _offset;
                const size_t _columns;
            };

            class const_iterator : row {
            public:
                friend class result;

                friend bool operator !=(const const_iterator &lhs, const const_iterator &rhs) noexcept {
                    return lhs._row != rhs._row;
                }

                const_iterator& operator++() noexcept {
                    ++_row;
                    return *this;
                }

                const row& operator*() const noexcept {
                    return *this;
                }

            private:
                const_iterator(const row& r) noexcept
                    : row{r} {
                }                
            };

            result() = default;
            result(PGresult *res)
                : _res{res, [&] (auto *p) { PQclear(p); }}
                , _columns{static_cast<size_t>(PQnfields(res))}
                , _rows{static_cast<size_t>(PQntuples(res))} {

            }

            bool is_ok() const {
                return PQresultStatus(_res.get()) == PGRES_TUPLES_OK;
            }

            operator bool() {
                return is_ok();
            }

            string error_message() const {
                const char* message = PQresultErrorMessage(_res.get());
                const size_t size = strlen(message);
                return string(message, size - 1);
            }

            const_iterator begin() const {
                return row{*this, 0, 0, _columns};
            }

            const_iterator end() const {
                return row{*this, size(), 0, _columns};
            }

            size_t size() const noexcept {
                return _rows;
            }

            bool empty() const noexcept {
                return _rows != 0;
            }

            row at(const size_t r) const {
                return row(*this, r, 0, _columns);
            }

            row operator[](const size_t r) const {
                return row(*this, r, 0, _columns);
            }

            bool is_null(const size_t _row, const size_t _column) const {
                return PQgetisnull(_res.get(), _row, _column) == 1;
            }

            const char* get(const size_t _row, const size_t _column) const {
                return PQgetvalue(_res.get(), _row, _column);
            }

        private:
            shared_ptr<PGresult> _res;
            const size_t _columns;
            const size_t _rows;
        };

        static result execute(const connection &conn, const std::string &text) {
            query q{conn};
            return q.execute(text);
        }

        template< typename... Args >
        static result execute(const connection &conn, const std::string &text, Args&&... args ) {
            query q{conn};
            const auto t = xfmt::format(text, std::forward<Args>(args) ...);

            return q.execute(t);
        }

        query() = default;
        query(const connection &conn) : _conn{conn._conn} {

        }

        result execute(const std::string &text) const {
            return {PQexec(_conn.get(), text.c_str())};
        }

    private:
        shared_ptr<PGconn> _conn;
    };

    template <typename T> T get(const query::result::row &r, const size_t column);

    template<> inline string get<string>(const query::result::row &r, const size_t column) {
        return {r._result.get(r._row, column)};
    }

    template<> inline bool get<bool>(const query::result::row &r, const size_t column) {
        return strcasecmp(r._result.get(r._row, column), "true") == 0
                || strcasecmp(r._result.get(r._row, column), "yes");
    }
}
