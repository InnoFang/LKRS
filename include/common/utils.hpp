/*
 * @FileName   : macros.hpp 
 * @CreateAt   : 2021/11/4
 * @Author     : Inno Fang
 * @Email      : innofang@yeah.net
 * @Description: 
 */

#ifndef RETRIEVE_SYSTEM_UTILS_HPP
#define RETRIEVE_SYSTEM_UTILS_HPP

#define TIMEIT( CODE, RECORD ) do { \
            auto start_time = std::chrono::high_resolution_clock::now(); \
            CODE;  \
            auto stop_time = std::chrono::high_resolution_clock::now(); \
            std::chrono::duration<double, std::milli> used_time = stop_time - start_time; \
            (RECORD) = used_time.count(); \
        } while(0);

#endif //RETRIEVE_SYSTEM_UTILS_HPP
