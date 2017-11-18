## How to Build
```sh
cd build
cmake ..
cmake --build .
```
### Making deb package:
```sh
cpack .
```

## Install
### From sources
```sh
sudo make install
```

### deb
```sh
sudo dpkg -i launchdb-0.1.0.deb
```

## Uninstall
### From sources
```sh
sudo make uninstall
```
### deb
```sh
sudo dpkg -r launchdb
```

## JSON Schema
### Supported types
* int
* float
* decimal
* str
* text
* bytes
* datetime
* date
* time
* bool
* uuid
* json

### Column
Structure:
```
root
  │
  ├ owner
  │
  ├ privileges
  │
  ├ users
  │    │
  │    └ user
  │         │ 
  │         ├ name
  │         └ password
  │
  └ tables
       │
       └ table
        │
        ├ name
        ├ schema
        ├ comment
        ├ columns
        │     │
        │     └ column
        │            │
        │            ├ name
        │            ├ type
        │            ├ size
        │            ├ not_null
        │            ├ unique
        │            ├ comment
        │            ├ default
        │            └ primary_key
        │
        └ foreign_keys
```

## JSON ER

### create-db
> create-db [options] DB_PATH
DB_PATH Path to database

Options:
- -g Generator postgres, mysql, maria. Default: postgres
- -v Version
- -h Display help

### Example
```sh
createdb estoredb
create-db examples/estore.json | psql -d estoredb
```
#### Usage
### migrate-db
### grab-db
