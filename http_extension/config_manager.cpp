/**
 * MongoDB Extension Configuration Manager Implementation
 */

#include "config_manager.h"
#include <IKeyValues.h>
#include <IFileSystem.h>
#include <smsdk_ext.h>

ConfigManager::ConfigManager()
    : m_apiServiceURL("http://127.0.0.1:3300")
    , m_apiKey("sourcemod-mongodb-extension-2024")
    , m_timeout(30000)
    , m_retries(3)
    , m_debug(false)
    , m_defaultDatabase("sourcemod")
    , m_playersCollection("players")
    , m_connectionsCollection("servers")
    , m_maxConnections(5)
    , m_idleTimeout(300)
{
}

ConfigManager::~ConfigManager()
{
}

bool ConfigManager::LoadConfig(const char* configPath)
{
    IKeyValues* kv = new KeyValues("MongoDB Configuration");
    if (!kv)
    {
        m_lastError = "Failed to create KeyValues object";
        return false;
    }
    
    // Try to load the config file
    if (!kv->LoadFromFile(filesystem, configPath))
    {
        m_lastError = "Failed to load config file: ";
        m_lastError += configPath;
        kv->deleteThis();
        return false;
    }
    
    // Load API service settings
    IKeyValues* apiService = kv->FindKey("api");
    if (apiService)
    {
        m_apiServiceURL = GetStringValue(apiService, "url", "http://127.0.0.1:3300");
        m_apiKey = GetStringValue(apiService, "api_key", "sourcemod-mongodb-extension-2024");
        m_timeout = GetIntValue(apiService, "timeout", 30) * 1000; // Convert seconds to milliseconds
        m_retries = GetIntValue(apiService, "max_retries", 3);
        m_debug = GetBoolValue(apiService, "debug_mode", false);
    }

    // Load database settings
    IKeyValues* database = kv->FindKey("database");
    if (database)
    {
        m_defaultDatabase = GetStringValue(database, "default_db", "sourcemod");
        m_playersCollection = GetStringValue(database, "players_collection", "players");
        m_connectionsCollection = GetStringValue(database, "servers_collection", "servers");
    }

    // Load connection pool settings
    IKeyValues* connections = kv->FindKey("connections");
    if (connections)
    {
        m_maxConnections = GetIntValue(connections, "pool_size", 5);
        m_idleTimeout = GetIntValue(connections, "keep_alive", 300);
    }

    // Load development settings (for debug mode)
    IKeyValues* development = kv->FindKey("development");
    if (development)
    {
        m_debug = GetBoolValue(development, "debug_mode", false);
    }

    kv->deleteThis();
    return true;
}

std::string ConfigManager::GetStringValue(IKeyValues* kv, const char* key, const char* defaultValue)
{
    const char* value = kv->GetString(key, defaultValue);
    return value ? std::string(value) : std::string(defaultValue);
}

int ConfigManager::GetIntValue(IKeyValues* kv, const char* key, int defaultValue)
{
    return kv->GetInt(key, defaultValue);
}

bool ConfigManager::GetBoolValue(IKeyValues* kv, const char* key, bool defaultValue)
{
    return kv->GetInt(key, defaultValue ? 1 : 0) != 0;
}
