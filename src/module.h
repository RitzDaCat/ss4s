#pragma once

#include "ss4s/modapi.h"
#include "ss4s/module.h"

bool SS4S_ModuleOpen(const char *name, SS4S_Module *module, const SS4S_LibraryContext *context);

SS4S_VideoDiagnosticsFunction *SS4S_ModuleGetVideoDiagnostics(const char *name);
