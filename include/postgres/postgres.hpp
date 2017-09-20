#pragma once

#include <memory>

#include <postgresql/libpq-fe.h>

namespace postgres {
    using std::shared_ptr;
    using std::string;
    using std::size_t;

    class connection final {
    public:
        connection() = default;

        static connection create(const std::string &connect_info) {
            return {connect_info};
        }

        bool is_ok() const {
            return PQstatus(_conn.get()) == CONNECTION_OK;
        }

        operator bool() {
            return is_ok();
        }

        string error_message() const
        {
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
            protected:
                friend class result;

                const result& _result;
                size_t _row;
                const size_t _offset;
                const size_t _columns;

                row(const result &res, const size_t r, const size_t offset, const size_t columns)
                    : _result{res}
                    , _row{r}
                    , _offset{offset}
                    , _columns{columns} {
                }
            };

            class const_iterator : row {
            public:
                friend class result;

                friend bool operator !=(const const_iterator &lhs, const const_iterator &rhs) {
                    return lhs._row != rhs._row;
                }

            private:
                const_iterator(const row& r)
                    : row{r} {
                }

                const row& operator*() const {
                    return *this;
                }
            };

            result() = default;
            result(PGresult *res) : _res{res, [&] (auto *p) { PQclear(p); }}
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

            size_t size() const {
                return _rows;
            }

            bool empty() const {
                return _rows != 0;
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

        query() = default;
        query(const connection &conn) : _conn{conn._conn} {

        }

        result execute(const std::string &text) const {
            return {PQexec(_conn.get(), text.c_str())};
        }

    private:
        shared_ptr<PGconn> _conn;
    };
}
