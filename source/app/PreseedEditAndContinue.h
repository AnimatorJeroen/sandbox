#pragma once

#ifdef _DEBUG

namespace PreseedEditAndContinue {
    // Preseeds common template instantiations for Edit and Continue
    // 
    // This file uses EXPLICIT TEMPLATE INSTANTIATION for GLM functions
    // to force the compiler to generate these templates in a dedicated
    // compilation unit with optimizations disabled.
    //
    // This significantly improves Edit and Continue because:
    // 1. GLM template code is already generated in the binary
    // 2. No inlining means code can be hot-swapped more reliably
    // 3. Changes to code using these templates are more likely to succeed
    //
    // Call once at application startup in DEBUG builds
    void Preseed();
}

#endif // _DEBUG
