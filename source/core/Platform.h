#pragma once

namespace Core
{
    /// <summary>
    /// Platform configuration and compile-time constants
    /// </summary>
    namespace Platform
    {
#ifdef PLATFORM_WASM
        constexpr bool IsWasm = true;
#else
        constexpr bool IsWasm = false;
#endif
    }
}
