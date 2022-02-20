#ifndef RT_ASSERT_H_
#define RT_ASSERT_H_

#include <stdexcept>
#include <string>
#include <cassert>
#include <iostream>

/**
 * Assertion for release-enabled assertions
 */
class ReleaseAssertException : public std::runtime_error
{
public:
    ReleaseAssertException(const std::string &msg = "") : std::runtime_error(msg) {}
};

#define rt_assert(b)                                                              \
    if (!(b))                                                                     \
    {                                                                             \
        std::cout << "rt_assert at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw ReleaseAssertException();                                           \
    }

#define rt_assert_eq(ex, ac)                                                                                                                                                          \
    if (!((ex) == (ac)))                                                                                                                                                              \
    {                                                                                                                                                                                 \
        std::cout << "rt_assert_eq: expected (" << (ex) << ", 0x" << std::hex << (ex) << std::dec << ") got (" << (ac) << ", 0x" << std::hex << (ac) << std::dec << ")" << std::endl; \
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;                                                                                                                        \
        throw ReleaseAssertException();                                                                                                                                               \
    }

#endif /*ASSERT_HELPERS_H_*/