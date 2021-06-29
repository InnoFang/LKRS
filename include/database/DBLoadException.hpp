//
// Created by InnoFang on 2021/6/29.
//

#ifndef RETRIEVE_SYSTEM_DBLOADEXCEPTION_HPP
#define RETRIEVE_SYSTEM_DBLOADEXCEPTION_HPP

#include <exception>
#include <string>

class DBLoadException : public std::exception {
public:
    DBLoadException(std::string db_name);
    ~DBLoadException();
private:
    std::string message;
};


#endif //RETRIEVE_SYSTEM_DBLOADEXCEPTION_HPP
