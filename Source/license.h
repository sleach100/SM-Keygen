#pragma once

#include <string>

namespace license
{
    std::string normalizeField(const std::string& s);
    std::string makePayload(const std::string& first,
                            const std::string& last,
                            const std::string& email,
                            const std::string& version,
                            const std::string& yyyymmdd);
    std::string utcDateYYYYMMDD();
    std::string makeLicense(const std::string& first,
                            const std::string& last,
                            const std::string& email);
    bool verifyLicense(const std::string& license,
                       const std::string& first,
                       const std::string& last,
                       const std::string& email);
}
