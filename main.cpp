#include <iostream>

#ifdef _WIN32

#include <windows.h>

#elif defined(__linux__)
#include <dlfcn.h>
#endif

class FnResolver {
public:

#ifdef _WIN32
    HMODULE standardLibHandle;
#elif defined(__linux__)
    void* standardLibHandle;
#endif

    int loadLibrary() {
#ifdef _WIN32
        // Load the standard library dynamically
        standardLibHandle = LoadLibrary(TEXT("msvcrt.dll"));
        if (!standardLibHandle) {
            std::cerr << "Failed to load standard library: " << GetLastError() << std::endl;
            return 1;
        }
#elif defined(__linux__)
        standardLibHandle = dlopen(nullptr, RTLD_LAZY);
        if (!standardLibHandle) {
            std::cerr << "Failed to load standard library: " << dlerror() << std::endl;
            return 1;
        }
#endif
        return 0;
    }

#ifdef _WIN32

    FARPROC get_ptr_to(const char *fnName) {
        FARPROC printfPtr = GetProcAddress(standardLibHandle, fnName);
        if (!printfPtr) {
            std::cerr << "Failed to find function '" << fnName << "': " << GetLastError() << std::endl;
            return nullptr;
        }
        return printfPtr;
    }

#elif defined(__linux__)
    void* get_ptr_to(const char* fnName) {
       void* printfPtr = dlsym(standardLibHandle, fnName);
        if (!printfPtr) {
            std::cerr << "Failed to find function '" << fnName << "': " << dlerror() << std::endl;
            return nullptr;
        }
        return printfPtr;
    }
#endif

    void unloadLibrary() {
#ifdef _WIN32
        FreeLibrary(standardLibHandle);
#elif defined(__linux__)
        dlclose(standardLibHandle);
#endif
    }

    ~FnResolver() {
        unloadLibrary();
    }

};

int main() {
    FnResolver resolver{};
    int result = resolver.loadLibrary();
    if (result != 0) {
        return result;
    }
    auto printfPtr = resolver.get_ptr_to("printf");
    typedef int (*PrintfFuncPtr)(const char*, ...);
    auto printfFunc = reinterpret_cast<PrintfFuncPtr>(printfPtr);
    printfFunc("Hello, world!\n");
    return 0;
}
