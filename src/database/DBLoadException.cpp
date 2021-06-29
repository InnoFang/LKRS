//
// Created by InnoFang on 2021/6/29.
//

#include "database/DBLoadException.hpp"

DBLoadException::DBLoadException(std::string db_name) {
    this->message = "[DBLoadException] cannot load database from " + db_name;
}

DBLoadException::~DBLoadException() {

}

const char *DBLoadException::what() const noexcept {
    return message.c_str();
}

