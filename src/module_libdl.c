#include "module.h"
#include "module_impl.h"
#include "lib_logging.h"

#include <dlfcn.h>
#include <string.h>

SS4S_VideoDiagnosticsFunction *SS4S_ModuleGetVideoDiagnostics(const char *name) {
    if (name == NULL || name[0] == '\0') { return NULL; }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), name);
    void *lib = dlopen(tmp, RTLD_NOW);
    if (lib == NULL) { return NULL; }
    ModuleFunctionName(tmp, sizeof(tmp), "VideoDiagnosticsV1", name);
    SS4S_VideoDiagnosticsFunction *fn = (SS4S_VideoDiagnosticsFunction *) dlsym(lib, tmp);
    /* ModuleOpen retains its own reference for the driver's lifetime. */
    dlclose(lib);
    return fn;
}

bool SS4S_ModuleOpen(const char *name, SS4S_Module *module, const SS4S_LibraryContext *context) {
    if (name == NULL || name[0] == '\0') {
        return false;
    }
    char tmp[128];
    ModuleFileName(tmp, sizeof(tmp), name);
    void *lib = dlopen(tmp, RTLD_NOW);
    if (lib == NULL) {
        return false;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Open", name);
    SS4S_ModuleOpenFunction *fn = (SS4S_ModuleOpenFunction *) dlsym(lib, tmp);
    if (fn == NULL) {
        SS4S_Log(SS4S_LogLevelError, "Module", "Module `%s` is not valid!", name);
        dlclose(lib);
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
    void *lib = dlopen(tmp, RTLD_NOW);
    if (lib == NULL) {
        SS4S_Log(SS4S_LogLevelInfo, "Module", "Module `%s` can't be loaded: %s", id, dlerror());
        return 0;
    }
    ModuleFunctionName(tmp, sizeof(tmp), "Check", id);
    SS4S_ModuleCheckFlag result = flags;
    SS4S_ModuleCheckFunction *fn = (SS4S_ModuleCheckFunction *) dlsym(lib, tmp);
    if (fn != NULL) {
        result = fn(flags);
        SS4S_Log(SS4S_LogLevelInfo, "Module", "Module `%s` check result: %d", id, result);
    }
    dlclose(lib);
    return result;
}
