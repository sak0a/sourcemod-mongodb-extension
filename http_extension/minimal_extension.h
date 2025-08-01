/**
 * Minimal HTTP MongoDB Extension Header
 */

#ifndef _MINIMAL_EXTENSION_H_
#define _MINIMAL_EXTENSION_H_

#include "smsdk_ext.h"

class HTTPMongoDBExtension : public SDKExtension
{
public:
    // SDKExtension methods
    virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
    virtual void SDK_OnUnload();
    virtual void SDK_OnAllLoaded();
    virtual void SDK_OnPauseChange(bool paused) {}
    virtual bool QueryRunning(char *error, size_t maxlength) { return true; }

    // IExtensionInterface methods
    virtual bool OnExtensionLoad(IExtension *me, IShareSys *sys, char *error, size_t maxlength, bool late);
    virtual void OnExtensionUnload();
    virtual void OnExtensionsAllLoaded();
    virtual bool IsMetamodExtension() { return false; }
    virtual void OnExtensionPauseChange(bool state) {}
};

extern HTTPMongoDBExtension g_HTTPMongoDBExtension;

// Native function declarations
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Insert(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Update(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Delete(IPluginContext *pContext, const cell_t *params);

#endif // _MINIMAL_EXTENSION_H_
