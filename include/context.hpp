#pragma once

#include <vector>
#include <set>
#include <string>

namespace db {
    using std::string;
    using std::vector;
    using std::set;

    enum class column_fields {
        type,
        default_value,
        size,
        precision,
        scale,
        primary_key,
        unique_key,
        not_null,
        rename
    };

    typedef struct column_value_type {
        column_value_type() = default;
        column_value_type(const string &_type_name,
                          const size_t _size,
                          const string &_default_value,
                          const bool _primary_key = false,
                          const bool _unique_key = false,
                          const bool _not_null = false)
            : type_name{_type_name},
              default_value{_default_value},
              size{_size},
              precision{0},
              scale{0},
              primary_key{_primary_key},
              unique_key{_unique_key},
              not_null{_not_null} {

        }        

        string type_name;
        string default_value;
        size_t size = 0;
        size_t precision = 0;
        size_t scale{0};
        bool primary_key = false;
        bool unique_key = false;
        bool not_null = false;
    } column_value_t;

    inline bool operator==(const column_value_type &lhs, const column_value_type &rhs) noexcept {
        return lhs.type_name == rhs.type_name &&
                lhs.size == rhs.size &&
                lhs.precision == rhs.precision &&
                lhs.scale == rhs.scale &&
                lhs.default_value == rhs.default_value &&
                lhs.primary_key == rhs.primary_key &&
                lhs.unique_key == rhs.unique_key &&
                lhs.not_null == rhs.not_null;
    }

    typedef struct column_type final : column_value_type  {
        column_type() = default;
        column_type(const string &_name,
                    const string &_renamed,
                    const string &_comment,
                    const column_value_type &_value_type)
            : column_value_type{_value_type}, column_name{_name}, column_renamed{_renamed}, comment{_comment} {

        }

        column_type& operator =(const column_type &) = default;

        string column_name;
        string column_renamed;
        string comment;
    } column_t;

    inline auto get_diff(const column_type &u, const column_type &v) noexcept {
        set<column_fields> diffs;

        if (u.type_name != v.type_name)
            diffs.insert(column_fields::type);

        if (u.default_value != v.default_value)
            diffs.insert(column_fields::default_value);

        if (u.size != v.size)
            diffs.insert(column_fields::size);

        if (u.precision != v.precision)
            diffs.insert(column_fields::precision);

        if (u.scale != v.scale)
            diffs.insert(column_fields::scale);

        if (u.primary_key != v.primary_key)
            diffs.insert(column_fields::primary_key);

        if (u.unique_key != v.unique_key)
            diffs.insert(column_fields::unique_key);

        if (u.not_null != v.not_null)
            diffs.insert(column_fields::not_null);

        if (u.column_renamed == v.column_name)
            diffs.insert(column_fields::rename);

        return diffs;
    }

    typedef set<string> primary_key_t;

    typedef struct foreign_key_type final {
        string ref_table;
        string ref_schema;
        vector<string> columns;
        vector<string> ref_columns;
    } foreign_key_t;

    typedef vector<foreign_key_t> foreign_keys_t;

    typedef struct table_type final {
        table_type() = default;
        table_type(const string &_schema,
                   const string &_name,
                   const string &_comment,
                   const vector<column_t> &_columns,
                   const primary_key_t &_pk,
                   const vector<foreign_key_t> &_fks)
            : schema_name{_schema},
              table_name{_name},
              comment{_comment},
              columns{_columns},
              primary_key{_pk},
              foreign_keys{_fks} {

        }

        table_type(const string &_schema, const string &_name, const string &_comment)
            : schema_name{_schema}, table_name{_name}, comment{_comment} {

        }

        //table_type& operator =(const table_type &) = default;

        string schema_name;
        string table_name;
        string comment;
        vector<column_t> columns;
        primary_key_t primary_key;
        vector<foreign_key_t> foreign_keys;
    } table_t;

    typedef string schema_t;

    typedef struct index_type final {
        string schema_name;
        string table_name;
        string name;
        string column_name;
        //bool if_exists;
    } index_t;

    typedef struct user_type final {
        string name;
        string password;
        //bool if_exists;
    } user_t;

    typedef struct privilege_type final {
        string user;
        string name;
    } privilege_t;

    struct context final {
        string owner;

        set<schema_t> schemas;
        vector<table_t> tables;
        vector<index_t> indices;
        vector<user_t> users;
        vector<privilege_t> privileges;
    };

    struct config {
        bool make_transaction = true;
    };

    struct context_diff {
        typedef struct column_type final : column_value_t {
            column_type() = default;
            column_type(const string &_schema, const string &_table_name, const string &_name, const string &_renamed, const column_value_type &_value_type)
                : column_value_t{_value_type}, schema_name{_schema}, table_name{_table_name}, column_name{_name}, column_renamed{_renamed} {

            }

            string schema_name;
            string table_name;
            string column_name;
            string column_renamed;
            string comment;
        } column_t;

        enum status_type {
            status_added,
            status_removed,
            status_changed
        };

        struct empty_t {
        };

        template <typename T, typename V, typename C = empty_t>
        struct modification_t {
            modification_t() = default;
            modification_t(const T &t, const V &v) : modification_t(t, v, C{}) {

            }

            modification_t(const T &t, const V &v, const C &c) : status{t}, value{v}, changes{c} {

            }

            T status;
            V value;
            C changes;
        };

        vector<modification_t<status_type, table_t>> tables;
        vector<modification_t<status_type, column_t, set<column_fields>>> columns;
        vector<modification_t<status_type, index_t>> indices;
    };
}
