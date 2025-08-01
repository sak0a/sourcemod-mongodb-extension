/**
 * Simple HTTP MongoDB Extension Header
 * Minimal version for testing HTTP API integration
 */

#ifndef _SIMPLE_EXTENSION_H_
#define _SIMPLE_EXTENSION_H_

#include "smsdk_ext.h"
#include <string>

/**
 * @brief Simple HTTP MongoDB Extension
 * 
 * This is a minimal SourceMod extension that demonstrates HTTP-based
 * MongoDB integration without complex data structure handling.
 */
class HTTPMongoDBExtension : public SDKExtension
{
public:
    /**
     * @brief Called when the extension is loaded.
     */
    virtual bool SDK_OnLoad(char *error, size_t maxlen, bool late);
    
    /**
     * @brief Called when the extension is unloaded.
     */
    virtual void SDK_OnUnload();
    
    /**
     * @brief Called when the extension is paused.
     */
    virtual void SDK_OnPauseChange(bool paused) {}
    
    /**
     * @brief Called when all plugins are loaded.
     */
    virtual void SDK_OnAllLoaded() {}
    
    /**
     * @brief Called when the extension is about to be unloaded.
     */
    virtual void SDK_OnUnload() {}
    
    /**
     * @brief Called when a plugin is loaded.
     */
    virtual void OnPluginLoaded(IPlugin *plugin) {}
    
    /**
     * @brief Called when a plugin is unloaded.
     */
    virtual void OnPluginUnloaded(IPlugin *plugin) {}
    
public:
#if defined SMEXT_CONF_METAMOD
    /**
     * @brief Called when Metamod is attached, before the extension version is called.
     */
    virtual bool SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlen, bool late);
    
    /**
     * @brief Called when Metamod is detaching, after the extension version is called.
     */
    virtual bool SDK_OnMetamodUnload(char *error, size_t maxlen);
    
    /**
     * @brief Called when Metamod's pause state is changing.
     */
    virtual bool SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlen);
#endif
};

extern HTTPMongoDBExtension g_HTTPMongoDBExtension;

// Native function declarations
cell_t MongoDB_Connect(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Insert(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Find(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Update(IPluginContext *pContext, const cell_t *params);
cell_t MongoDB_Delete(IPluginContext *pContext, const cell_t *params);

#endif // _SIMPLE_EXTENSION_H_
