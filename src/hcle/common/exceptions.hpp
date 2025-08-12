#pragma once
#include <stdexcept>

namespace hcle
{
    namespace common
    {

        // A custom exception to be thrown when the user closes the display window.
        class WindowClosedException : public std::runtime_error
        {
        public:
            WindowClosedException() : std::runtime_error("User closed the display window.") {}
        };

    } // namespace common
} // namespace hcle