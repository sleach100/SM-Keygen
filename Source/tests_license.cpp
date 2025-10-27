#if defined(RUN_LICENSE_TESTS)
#include "license.h"
#include <cassert>
#include <iostream>

int main()
{
    const std::string first = "Ada";
    const std::string last = "Lovelace";
    const std::string email = "ada@example.com";

    const std::string license = license::makeLicense(first, last, email);
    assert(license::verifyLicense(license, first, last, email));
    assert(! license::verifyLicense(license, "A", last, email));
    assert(! license::verifyLicense("INVALID", first, last, email));

    std::cout << "All license tests passed\n";
    return 0;
}
#endif
