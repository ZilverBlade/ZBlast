#include "error.h"
#include "util.h"

#include <iomanip>

namespace zblast {
    void Error::PrettyPrint(std::ostream& out, const std::string& inputFile, EErrorFormat format) const {
        const auto& def = k_ErrorDefs[code];
        const char* sev = k_ErrorSevStrings[def.severity];

        switch (format) {
        case EErrorFormat_MSVC:
            out << inputFile << '(' << line << ',' << column << "): "
                << sev << ' ' << def.code << ": " << message;
            break;
        case EErrorFormat_GNU:
            out << inputFile << ':' << line << ':' << column << ": "
                << sev << ": " << def.code << ": " << message;
            break;
        }
    }
} // namespace zblast
