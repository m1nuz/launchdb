# launchdb

[![Build Status](https://travis-ci.org/m1nuz/launchdb.svg?branch=master)](https://travis-ci.org/m1nuz/launchdb)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000?style=flat-square)](https://github.com/m1nuz/launchdb/blob/master/LICENSE)

Database creation tool

## How to Build
```sh
cd build
cmake ..
cmake --build .
```

## Usage
> launch-db [options] DB_PATH 

DB_PATH Path to database

Options:
- -g Generator postgres, mysql, maria. Default: postgres
- -v Version
- -h Display help
### Example usage for Postgres
```sh
createdb estoredb
launch-db examples/estore.json | psql -d estoredb
```
