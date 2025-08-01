/**
 * MongoDB Extension Configuration Manager Implementation
 */

#include "config_manager.h"
#include <IKeyValues.h>
#include <IFileSystem.h>
#include <smsdk_ext.h>

ConfigManager::ConfigManager()
    : m_apiServiceURL("http://127.0.0.1:3300")
    , m_timeout(30000)
    , m_retries(3)
    , m_debug(false)
    , m_defaultDatabase("gamedb")
    , m_playersCollection("players")
    , m_connectionsCollection("connections")
    , m_maxConnections(10)
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
    IKeyValues* apiService = kv->FindKey("api_service");
    if (apiService)
    {
        m_apiServiceURL = GetStringValue(apiService, "url", "http://127.0.0.1:3300");
        m_timeout = GetIntValue(apiService, "timeout", 30000);
        m_retries = GetIntValue(apiService, "retries", 3);
        m_debug = GetBoolValue(apiService, "debug", false);
    }
    
    // Load database settings
    IKeyValues* database = kv->FindKey("database");
    if (database)
    {
        m_defaultDatabase = GetStringValue(database, "name", "gamedb");
        m_playersCollection = GetStringValue(database, "players_collection", "players");
        m_connectionsCollection = GetStringValue(database, "connections_collection", "connections");
    }
    
    // Load connection pool settings
    IKeyValues* pool = kv->FindKey("connection_pool");
    if (pool)
    {
        m_maxConnections = GetIntValue(pool, "max_connections", 10);
        m_idleTimeout = GetIntValue(pool, "idle_timeout", 300);
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
