/**
 * vim: set ts=4 :
 * =============================================================================
 * SourceMod HTTP MongoDB Extension - Main Implementation
 * Copyright (C) 2024 AlliedModders LLC.  All rights reserved.
 * =============================================================================
 */

#include "extension.h"
#include "http_client.h"
#include "json_structures.h"
#include "mongodb_api.h"
#include "natives.h"

HTTPMongoDBExtension g_HTTPMongoDBExtension;
SMEXT_LINK(&g_HTTPMongoDBExtension);

bool HTTPMongoDBExtension::OnExtensionLoad(IExtension *me, IShareSys *sys, char *error, size_t maxlength, bool late) {
    // Store references
    myself = me;
    sharesys = sys;

    // Call the SDK load method
    return SDK_OnLoad(error, maxlength, late);
}

bool HTTPMongoDBExtension::SDK_OnLoad(char *error, size_t maxlength, bool late) {
    // Get API URL from environment or use default
    const char* apiUrl = getenv("MONGODB_API_URL");
    if (!apiUrl) {
        apiUrl = "http://127.0.0.1:3300"; // Default to our test service
    }
    
    // Initialize HTTP client
    m_pHTTPClient = std::make_unique<HTTPClient>(apiUrl);
    if (!m_pHTTPClient->Initialize()) {
        snprintf(error, maxlength, "Failed to initialize HTTP client: %s", 
                m_pHTTPClient->GetLastError().c_str());
        return false;
    }
    
    // Configure HTTP client
    m_pHTTPClient->SetTimeout(30000); // 30 seconds
    m_pHTTPClient->SetRetryCount(3);
    m_pHTTPClient->SetUserAgent("SourceMod-MongoDB-Extension/1.0");
    
    // Initialize JSON manager
    m_pJSONManager = std::make_unique<JSONStructureManager>();
    
    // Initialize MongoDB API layer
    m_pMongoAPI = std::make_unique<MongoDBAPILayer>(
        m_pHTTPClient.get(), m_pJSONManager.get());
    
    if (!m_pMongoAPI->Initialize()) {
        snprintf(error, maxlength, "Failed to initialize MongoDB API layer: %s",
                m_pMongoAPI->GetLastError().c_str());
        return false;
    }
    
    m_bInitialized = true;
    
    // Log successful initialization
    g_pSM->LogMessage(myself, "HTTP MongoDB Extension loaded successfully (API: %s)", apiUrl);
    
    return true;
}

void HTTPMongoDBExtension::SDK_OnAllLoaded() {
    // Register natives
    sharesys->AddNatives(myself, g_MongoDBNatives);
    sharesys->AddNatives(myself, g_JSONNatives);
    
    g_pSM->LogMessage(myself, "HTTP MongoDB Extension natives registered");
}

void HTTPMongoDBExtension::SDK_OnUnload() {
    if (m_bInitialized) {
        // Shutdown in reverse order
        if (m_pMongoAPI) {
            m_pMongoAPI->Shutdown();
        }
        
        if (m_pHTTPClient) {
            m_pHTTPClient->Shutdown();
        }
        
        m_bInitialized = false;
        
        g_pSM->LogMessage(myself, "HTTP MongoDB Extension unloaded");
    }
}

void HTTPMongoDBExtension::SDK_OnPauseChange(bool paused) {
    // Nothing special needed for pause/unpause
}

bool HTTPMongoDBExtension::QueryRunning(char *error, size_t maxlength) {
    if (!m_bInitialized) {
        snprintf(error, maxlength, "Extension not initialized");
        return false;
    }
    
    // Could add health check here
    return true;
}

#if defined SMEXT_CONF_METAMOD
bool HTTPMongoDBExtension::SDK_OnMetamodLoad(ISmmAPI *ismm, char *error, size_t maxlength, bool late) {
    return true;
}

bool HTTPMongoDBExtension::SDK_OnMetamodUnload(char *error, size_t maxlength) {
    return true;
}

bool HTTPMongoDBExtension::SDK_OnMetamodPauseChange(bool paused, char *error, size_t maxlength) {
    return true;
}
#endif
