/**
 * MongoDB Configuration Example
 * Demonstrates how to use configurable MongoDB connections
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "MongoDB Config Example",
    author = "SourceMod Team",
    description = "Example of configurable MongoDB connections",
    version = "1.0.0",
    url = "http://www.sourcemod.net/"
};

public void OnPluginStart() {
    // Load MongoDB configuration
    char configPath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, configPath, sizeof(configPath), "configs/mongodb.cfg");
    
    if (MongoDB_LoadConfig(configPath)) {
        LogMessage("MongoDB configuration loaded successfully");
        
        // Display loaded configuration
        char apiUrl[256];
        if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
            LogMessage("API URL: %s", apiUrl);
        }
        
        int timeout = MongoDB_GetTimeout();
        LogMessage("Timeout: %d ms", timeout);
    } else {
        LogError("Failed to load MongoDB configuration from: %s", configPath);
        LogError("Using default settings");
    }
    
    // Register commands
    RegServerCmd("mongo_config_test", Command_ConfigTest, "Test MongoDB with config");
    RegServerCmd("mongo_set_url", Command_SetURL, "Set API URL at runtime");
    RegServerCmd("mongo_set_mongodb_uri", Command_SetMongoURI, "Set MongoDB URI for API service");
    RegServerCmd("mongo_get_config", Command_GetConfig, "Show current config");
}

// Test connection using configuration
public Action Command_ConfigTest(int args) {
    PrintToServer("=== MongoDB Configuration Test ===");
    
    // Method 1: Use configured connection
    MongoConnection conn = MongoConnection.FromConfig();
    
    if (conn.IsConnected()) {
        PrintToServer("✅ Connected using configuration");
        
        MongoCollection players = conn.GetCollection("gamedb", "players");
        
        // Insert test document
        char jsonDoc[512];
        Format(jsonDoc, sizeof(jsonDoc), 
            "{\"name\":\"ConfigTest\",\"timestamp\":%d,\"method\":\"config\"}", 
            GetTime());
        
        char insertedId[64];
        if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
            PrintToServer("✅ Document inserted: %s", insertedId);
        } else {
            PrintToServer("❌ Failed to insert document");
        }
        
        // Count documents
        int count = players.CountDocuments(null);
        PrintToServer("📊 Total documents: %d", count);
        
    } else {
        PrintToServer("❌ Connection failed");
    }
    
    conn.Close();
    PrintToServer("=== Test completed ===");
    
    return Plugin_Handled;
}

// Set API URL at runtime
public Action Command_SetURL(int args) {
    if (args < 1) {
        PrintToServer("Usage: mongo_set_url <url>");
        PrintToServer("Example: mongo_set_url http://192.168.1.100:3300");
        return Plugin_Handled;
    }
    
    char newUrl[256];
    GetCmdArg(1, newUrl, sizeof(newUrl));
    
    if (MongoDB_SetAPIURL(newUrl)) {
        PrintToServer("✅ API URL updated to: %s", newUrl);
        
        // Test new URL
        MongoConnection conn = MongoConnection.FromConfig();
        if (conn.IsConnected()) {
            PrintToServer("✅ Connection test successful");
        } else {
            PrintToServer("❌ Connection test failed");
        }
        conn.Close();
        
    } else {
        PrintToServer("❌ Failed to set API URL");
    }
    
    return Plugin_Handled;
}

// Show current configuration
public Action Command_GetConfig(int args) {
    PrintToServer("=== Current MongoDB Configuration ===");
    
    char apiUrl[256];
    if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
        PrintToServer("API URL: %s", apiUrl);
    }
    
    int timeout = MongoDB_GetTimeout();
    PrintToServer("Timeout: %d ms", timeout);
    
    return Plugin_Handled;
}

// Set MongoDB URI for the API service
public Action Command_SetMongoURI(int args) {
    if (args < 1) {
        PrintToServer("Usage: mongo_set_mongodb_uri <mongodb_uri>");
        PrintToServer("Examples:");
        PrintToServer("  mongo_set_mongodb_uri mongodb://admin:pass@localhost:27017/?authSource=admin");
        PrintToServer("  mongo_set_mongodb_uri mongodb://user:pass@cluster.mongodb.net/gamedb");
        return Plugin_Handled;
    }

    char mongoUri[512];
    GetCmdArg(1, mongoUri, sizeof(mongoUri));

    PrintToServer("Setting MongoDB URI for API service...");
    PrintToServer("Note: This requires the API service to support dynamic URI configuration");
    PrintToServer("MongoDB URI: %s", mongoUri);

    // In a real implementation, you would send this to the API service
    // via a special endpoint like POST /api/v1/config/mongodb-uri
    // For now, we'll just show how it would work

    char apiUrl[256];
    if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
        PrintToServer("API Service: %s", apiUrl);
        PrintToServer("You would send the MongoDB URI to: %s/api/v1/config/mongodb-uri", apiUrl);
    }

    return Plugin_Handled;
}

// Example of different connection methods
public void ExampleConnectionMethods() {
    // Method 1: Use configuration file settings
    MongoConnection configConn = MongoConnection.FromConfig();
    
    // Method 2: Override with specific URL
    MongoConnection customConn = new MongoConnection("http://custom-server:3300");
    
    // Method 3: Runtime URL change
    MongoDB_SetAPIURL("http://new-server:3300");
    MongoConnection runtimeConn = MongoConnection.FromConfig();
    
    // Clean up
    configConn.Close();
    customConn.Close();
    runtimeConn.Close();
}
