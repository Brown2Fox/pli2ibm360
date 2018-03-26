//
// Created by Antti on 26-Mar-18.
//

#ifndef PROJECT_UTILS_HPP
#define PROJECT_UTILS_HPP

#include <cassert>
#include <cerrno>
#include <cmath>

#define clean_errno() (errno == 0 ? "None" : strerror(errno))
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define assertf(A, M, ...) if(!(A)) {log_error(M, ##__VA_ARGS__); assert(A); }



#define with(x,xx)  { auto& (xx) = (x);
#define end_with }

constexpr int64_t ipow(int64_t base, int exp, int64_t result = 1) {
    return exp < 1 ? result : ipow(base*base, exp/2, (exp % 2) ? result*base : result);
}



#endif //PROJECT_UTILS_HPP
