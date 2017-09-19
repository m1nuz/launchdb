# launchdb

[![Build Status](https://travis-ci.org/m1nuz/launchdb.svg?branch=master)](https://travis-ci.org/m1nuz/launchdb)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg?maxAge=2592000?style=flat-square)](https://github.com/m1nuz/launchdb/blob/master/LICENSE)

Database creation tool

## Features

### Input formats
- [x] JSON
- [ ] YAML

### Postgres
- [x] Schema creation
- [x] Tables
- [x] Pimary keys
- [x] Foreign keys
- [x] Indices
- [x] Comments
- [x] Ownership
- [x] Users
- [x] Privileges

### Sqlite
- [ ] Schema creation
- [ ] Tables
- [ ] Pimary keys
- [ ] Foreign keys
- [ ] Indices
- [ ] Comments


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

## Usage
> create-db [options] DB_PATH

DB_PATH Path to database

Options:
- -g Generator postgres, mysql, maria. Default: postgres
- -v Version
- -h Display help

### Example usage for Postgres
```sh
createdb estoredb
create-db examples/estore.json | psql -d estoredb
```
