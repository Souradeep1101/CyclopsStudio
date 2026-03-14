#pragma once

#include <string>
#include "Cyclops/Core/Base.h"

namespace Cyclops {
    class CYCLOPS_API FileDialogs
    {
    public:
        // Returns empty string if cancelled
        static std::string OpenFile(const char* filter);
        static std::string SaveFile(const char* filter);

        // [NEW] For your Genga Test
        static std::string OpenFolder();
    };
}