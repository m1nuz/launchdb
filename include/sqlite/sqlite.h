#pragma once

#include <memory>
#include <cstring>
#include <functional>

#include <format.hpp>
#include <variant.hpp>

#include <sqlite3.h>

namespace sqlite {
    using std::shared_ptr;
    using std::string;
    using std::vector;
    using std::size_t;
    using namespace mpark;

    class connection final {
    public:
        connection() = default;

        static connection create(const string &connect_info) {
            return {connect_info};
        }

        bool is_ok() const {
            return ret_code == SQLITE_OK;
        }

        operator bool() {
            return is_ok();
        }

        string error_message() const {
            return string{sqlite3_errstr(ret_code)};
        }

        friend class query;

    private:
        connection(const string &connect_info) {
            sqlite3 *db = nullptr;
            ret_code = sqlite3_open(connect_info.c_str(), &db);

            _conn = {db, [&] (auto *p) {
                         sqlite3_close(p);
                     }};
        }

        int ret_code;
        shared_ptr<sqlite3> _conn;
    };

    class query final {
    public:

        class result {
        public:
            struct null_type {
                null_type() = default;
            };

            typedef vector<uint8_t> blob_type;
            typedef variant<null_type, int64_t, double, string, vector<uint8_t>> value_type;
            typedef vector<value_type> row_values;

            class row {
            public:
                friend class result;

                row(const result &res, const size_t r) noexcept
                    : _result{res}
                    , _row{r} {
                }

                const result& _result;
                size_t _row;
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

            //private:
                const_iterator(const row& r) noexcept
                    : row{r} {
                }
            };


            result() = default;
            result(sqlite3_stmt *res, const int err_code = SQLITE_OK)
                : _res{res, [&] (auto *p) { sqlite3_finalize(p); }}
                , _column_count{static_cast<size_t>(sqlite3_column_count(res))}
                , _err_code{err_code} {
            }

            bool is_ok() const {
                return _err_code == SQLITE_OK;
            }

            operator bool() {
                return is_ok();
            }

            string error_message() const {
                return string{sqlite3_errstr(_err_code)};
            }

            const_iterator begin() const {
                return row{*this, 0};
            }

            const_iterator end() const {
                return row{*this, size()};
            }

            size_t size() const noexcept {
                return _rows.size();
            }

            bool empty() const noexcept{
                return _rows.empty();
            }

            row at(const size_t r) const {
                return row{*this, r};
            }

            row operator[](const size_t r) const {
                return row{*this, r};
            }

            bool is_null(const size_t _row, const size_t _column) const {
                if (get_if<null_type>(&_rows[_row][_column]))
                    return true;

                return false;
            }

            friend class query;

        //private:
            shared_ptr<sqlite3_stmt> _res;
            size_t _column_count;
            vector<row_values> _rows;
            int _err_code;
        };

        query() = default;
        query(const connection &conn) : _conn{conn._conn} {

        }

        template< typename... Args >
        static result execute(const connection &conn, const string &text, Args&&... args ) {
            query q{conn};
            const auto t = xfmt::format(text, std::forward<Args>(args) ...);

            return q.execute(t);
        }

        result execute(const std::string &text) const {
            sqlite3_stmt *stmt = nullptr;
            auto ret = sqlite3_prepare_v2(_conn.get(), text.c_str(), text.size(), &stmt, NULL);
            if (ret != SQLITE_OK)
                return {nullptr, ret};

            vector<result::row_values> rows;
            result res{stmt};

            while ((ret = sqlite3_step(stmt)) == SQLITE_ROW) {
                const auto num_cols = sqlite3_column_count(stmt);

                result::row_values values;

                for (int i = 0; i < num_cols; i++)
                {
                    switch (sqlite3_column_type(stmt, i))
                    {
                    case SQLITE_INTEGER:
                        values.push_back(int64_t{sqlite3_column_int64(stmt, i)});
                        break;
                    case SQLITE_FLOAT:
                        values.push_back(double{sqlite3_column_double(stmt, i)});
                        break;
                    case SQLITE_BLOB:
                    {
                        result::blob_type blob;
                        blob.resize(sqlite3_column_bytes(stmt, i));
                        memcpy(&blob[0], sqlite3_column_blob(stmt, i), blob.size());
                        break;
                    }
                    case SQLITE_NULL:
                        values.push_back(result::null_type{});
                        break;
                    case SQLITE3_TEXT:
                        values.push_back(string{reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))});
                        break;
                    default:
                        break;
                    }

                }

                rows.emplace_back(values);
            }

            if (ret != SQLITE_DONE)
                return {stmt, ret};

            res._rows = rows;

            return res;
        }

    private:
        shared_ptr<sqlite3> _conn;
    };

    //template <typename T> T get(const query::result::row &r, const size_t column);

    template<typename T> inline T get(const query::result::row &r, const size_t column) {
        return get<T>(r._result._rows[r._row][column]);
    }

    template<typename T> inline auto get_if(const query::result::row &r, const size_t column) {
        return get_if<T>(&r._result._rows[r._row][column]);
    }
}
