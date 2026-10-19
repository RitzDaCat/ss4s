#include "module.h"
#include "module_impl.h"

#include <windows.h>
#include <string.h>

SS4S_VideoDiagnosticsFunction *SS4S_ModuleGetVideoDiagnostics(const char *name) {
    if (name == NULL || name[0] == '\0') { return NULL; }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), name);
    HMODULE lib = LoadLibraryA(tmp);
    if (lib == NULL) { return NULL; }
    ModuleFunctionName(tmp, sizeof(tmp), "VideoDiagnosticsV1", name);
    SS4S_VideoDiagnosticsFunction *fn = (SS4S_VideoDiagnosticsFunction *) (void *) GetProcAddress(lib, tmp);
    FreeLibrary(lib);
    return fn;
}

bool SS4S_ModuleOpen(const char *name, SS4S_Module *module, const SS4S_LibraryContext *context) {
    if (name == NULL || name[0] == '\0') {
        return false;
    }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), name);
    HMODULE lib = LoadLibraryA(tmp);
    if (lib == NULL) {
        return false;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Open", name);
    SS4S_ModuleOpenFunction *fn = (SS4S_ModuleOpenFunction *) (void *) GetProcAddress(lib, tmp);
    if (fn == NULL) {
        context->Log(SS4S_LogLevelError, "Module", "Module `%s` is not valid!", name);
        FreeLibrary(lib);
        return false;
    }
    memset(module, 0, sizeof(SS4S_Module));
    return fn(module, context);
}

SS4S_ModuleCheckFlag SS4S_ModuleCheck(const char *id, SS4S_ModuleCheckFlag flags) {
    if (id == NULL || id[0] == '\0') {
        return 0;
    }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), id);
    HMODULE lib = LoadLibraryA(tmp);
    if (lib == NULL) {
        return 0;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Check", id);
    SS4S_ModuleCheckFlag result = flags;
    SS4S_ModuleCheckFunction *fn = (SS4S_ModuleCheckFunction *) (void *) GetProcAddress(lib, tmp);
    if (fn != NULL) {
        result = fn(flags);
    }
    FreeLibrary(lib);
    return result;
}
