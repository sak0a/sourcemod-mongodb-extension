/**
 * MongoDB Comprehensive Test Plugin
 * Combines all MongoDB test functionality into a single plugin
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "MongoDB Comprehensive Test",
    author = "SourceMod Team",
    description = "Comprehensive MongoDB testing plugin with all features",
    version = "1.0.0",
    url = "http://www.sourcemod.net/"
};

public void OnPluginStart() {
    // Configuration Commands
    RegServerCmd("mongo_config", Command_MongoConfig, "Test configuration loading");
    RegServerCmd("mongo_set_url", Command_SetURL, "Set API URL at runtime");
    RegServerCmd("mongo_get_config", Command_GetConfig, "Show current configuration");
    
    // Basic Commands
    RegServerCmd("mongo_test", Command_MongoTest, "Basic MongoDB connection test");
    RegServerCmd("mongo_insert", Command_MongoInsert, "Insert mock player data");
    RegServerCmd("mongo_batch", Command_MongoBatch, "Insert multiple mock players");
    RegServerCmd("mongo_find", Command_MongoFind, "Find player by name");
    RegServerCmd("mongo_count", Command_MongoCount, "Count documents");
    RegServerCmd("mongo_stats", Command_MongoStats, "Show collection statistics");
    
    // Advanced Commands
    RegServerCmd("mongo_aggregation", Command_TestAggregation, "Test aggregation pipeline");
    RegServerCmd("mongo_bulk_ops", Command_TestBulkOperations, "Test bulk operations");
    RegServerCmd("mongo_query_test", Command_TestQueries, "Test enhanced query operations");
    RegServerCmd("mongo_performance", Command_TestPerformance, "Test performance monitoring");
    RegServerCmd("mongo_error_test", Command_TestErrorHandling, "Test error handling");
    
    // Real Data Commands
    RegServerCmd("mongo_real_test", Command_RealDataTest, "Test with real player data");
    RegConsoleCmd("sm_mongo_test", Command_MongoTestChat, "Test MongoDB (chat command)");
    RegConsoleCmd("sm_mongo_insert", Command_MongoInsertChat, "Insert player data (chat command)");
    
    PrintToServer("=== MongoDB Comprehensive Test Plugin Loaded ===");
    PrintToServer("Available commands:");
    PrintToServer("Configuration: mongo_config, mongo_set_url, mongo_get_config");
    PrintToServer("Basic: mongo_test, mongo_insert, mongo_batch, mongo_find, mongo_count, mongo_stats");
    PrintToServer("Advanced: mongo_aggregation, mongo_bulk_ops, mongo_query_test, mongo_performance, mongo_error_test");
    PrintToServer("Real Data: mongo_real_test, sm_mongo_test, sm_mongo_insert");
}

// Configuration Management Functions

public Action Command_MongoConfig(int args) {
    PrintToServer("=== MongoDB Configuration Test ===");
    
    // Load configuration
    char configPath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, configPath, sizeof(configPath), "configs/mongodb.cfg");
    
    if (MongoDB_LoadConfig(configPath)) {
        PrintToServer("✅ Configuration loaded successfully");
        
        // Display loaded configuration
        char apiUrl[256];
        if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
            PrintToServer("API URL: %s", apiUrl);
        }
        
        int timeout = MongoDB_GetTimeout();
        PrintToServer("Timeout: %d seconds", timeout);
    } else {
        PrintToServer("❌ Failed to load configuration from: %s", configPath);
        PrintToServer("Using default settings");
    }
    
    return Plugin_Handled;
}

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

public Action Command_GetConfig(int args) {
    PrintToServer("=== Current MongoDB Configuration ===");
    
    char apiUrl[256];
    if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
        PrintToServer("API URL: %s", apiUrl);
    }
    
    int timeout = MongoDB_GetTimeout();
    PrintToServer("Timeout: %d seconds", timeout);
    
    return Plugin_Handled;
}

// Basic Test Functions

public Action Command_MongoTest(int args) {
    PrintToServer("=== MongoDB Basic Connection Test ===");
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    PrintToServer("✅ Connection successful!");
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    int count = players.CountDocuments(null);
    
    PrintToServer("📊 Collection 'gamedb.players' has %d documents", count);
    
    // Insert a test document
    char jsonDoc[512];
    Format(jsonDoc, sizeof(jsonDoc), "{\"name\":\"TestPlayer_%d\",\"steamid\":\"STEAM_1:0:%d\",\"score\":%d,\"timestamp\":%d,\"test\":true}", GetRandomInt(1000, 9999), GetRandomInt(100000, 999999), GetRandomInt(0, 5000), GetTime());
    
    char insertedId[64];
    if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
        PrintToServer("✅ Test document inserted! ID: %s", insertedId);
    } else {
        PrintToServer("❌ Failed to insert test document");
    }
    
    conn.Close();
    PrintToServer("=== Test completed ===");
    
    return Plugin_Handled;
}

public Action Command_MongoInsert(int args) {
    char playerName[64];
    if (args >= 1) {
        GetCmdArg(1, playerName, sizeof(playerName));
    } else {
        Format(playerName, sizeof(playerName), "Player_%d", GetRandomInt(1000, 9999));
    }
    
    PrintToServer("=== Inserting Mock Player: %s ===", playerName);
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Create mock player data
    StringMap doc = new StringMap();
    doc.SetString("name", playerName);
    doc.SetString("steamid", "STEAM_1:0:123456");
    doc.SetValue("score", GetRandomInt(0, 5000));
    doc.SetValue("kills", GetRandomInt(0, 100));
    doc.SetValue("deaths", GetRandomInt(0, 100));
    doc.SetValue("timestamp", GetTime());
    doc.SetString("server", "Test Server");
    doc.SetValue("playtime", GetRandomInt(60, 7200));
    
    char insertedId[64];
    if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
        PrintToServer("✅ Player '%s' inserted! ID: %s", playerName, insertedId);
    } else {
        PrintToServer("❌ Failed to insert player '%s'", playerName);
    }
    
    delete doc;
    conn.Close();
    
    return Plugin_Handled;
}

public Action Command_MongoBatch(int args) {
    int count = 10;
    if (args >= 1) {
        count = GetCmdArgInt(1);
        if (count <= 0 || count > 100) {
            count = 10;
        }
    }
    
    PrintToServer("=== Batch Insert: %d Players ===", count);
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    int successCount = 0;
    for (int i = 0; i < count; i++) {
        char playerName[64];
        Format(playerName, sizeof(playerName), "BatchPlayer_%d_%d", GetTime(), i);
        
        char jsonDoc[512];
        Format(jsonDoc, sizeof(jsonDoc), "{\"name\":\"%s\",\"steamid\":\"STEAM_1:0:%d\",\"score\":%d,\"batch\":true,\"timestamp\":%d}", playerName, GetRandomInt(100000, 999999), GetRandomInt(0, 5000), GetTime());
        
        char insertedId[64];
        if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
            successCount++;
        }
    }
    
    PrintToServer("✅ Batch insert completed: %d/%d successful", successCount, count);
    
    conn.Close();
    return Plugin_Handled;
}

public Action Command_MongoFind(int args) {
    if (args < 1) {
        PrintToServer("Usage: mongo_find <name>");
        return Plugin_Handled;
    }
    
    char searchName[64];
    GetCmdArg(1, searchName, sizeof(searchName));
    
    PrintToServer("=== Finding Player: %s ===", searchName);
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Create JSON filter
    char jsonFilter[256];
    Format(jsonFilter, sizeof(jsonFilter), "{\"name\":\"%s\"}", searchName);
    
    StringMap result = players.FindOneJSON(jsonFilter);
    if (result != null) {
        PrintToServer("✅ Player found!");
        // Note: In a full implementation, we'd extract and display the player data
        delete result;
    } else {
        PrintToServer("❌ Player not found");
    }
    
    conn.Close();
    return Plugin_Handled;
}

public Action Command_MongoCount(int args) {
    PrintToServer("=== Document Count ===");
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    int count = players.CountDocuments(null);
    
    PrintToServer("📊 Total documents in 'gamedb.players': %d", count);
    
    conn.Close();
    return Plugin_Handled;
}

public Action Command_MongoStats(int args) {
    PrintToServer("=== Collection Statistics ===");
    
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Get total count
    int totalCount = players.CountDocuments(null);
    PrintToServer("📊 Total documents: %d", totalCount);
    
    // Count test documents
    char testFilter[128];
    Format(testFilter, sizeof(testFilter), "{\"test\":true}");
    StringMap testResult = players.FindOneJSON(testFilter);
    if (testResult != null) {
        PrintToServer("🧪 Test documents found");
        delete testResult;
    }
    
    // Count batch documents
    char batchFilter[128];
    Format(batchFilter, sizeof(batchFilter), "{\"batch\":true}");
    StringMap batchResult = players.FindOneJSON(batchFilter);
    if (batchResult != null) {
        PrintToServer("📦 Batch documents found");
        delete batchResult;
    }
    
    conn.Close();
    return Plugin_Handled;
}

// Advanced Test Functions

public Action Command_TestAggregation(int args) {
    PrintToServer("=== MongoDB Aggregation Test ===");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create aggregation pipeline (simplified)
    ArrayList pipeline = new ArrayList(ByteCountToCells(512));
    pipeline.PushString("{\"$match\":{\"score\":{\"$gte\":1000}}}");
    pipeline.PushString("{\"$group\":{\"_id\":null,\"avgScore\":{\"$avg\":\"$score\"},\"count\":{\"$sum\":1}}}");
    pipeline.PushString("{\"$sort\":{\"avgScore\":-1}}");

    ArrayList results = players.Aggregate(pipeline);
    if (results != null) {
        PrintToServer("✅ Aggregation completed with %d results", results.Length);
        delete results;
    } else {
        PrintToServer("❌ Aggregation failed");
    }

    delete pipeline;
    conn.Close();
    return Plugin_Handled;
}

public Action Command_TestBulkOperations(int args) {
    PrintToServer("=== MongoDB Bulk Operations Test ===");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create bulk operations (simplified)
    ArrayList bulkOps = new ArrayList(ByteCountToCells(1024));

    // Add multiple insert operations
    for (int i = 0; i < 5; i++) {
        char name[64];
        Format(name, sizeof(name), "BulkPlayer_%d", i);

        char opJson[512];
        Format(opJson, sizeof(opJson), "{\"insertOne\":{\"document\":{\"name\":\"%s\",\"score\":%d,\"timestamp\":%d,\"type\":\"bulk_test\"}}}", name, GetRandomInt(100, 1000), GetTime());
        bulkOps.PushString(opJson);
    }

    // Execute bulk operations
    if (players.BulkWrite(bulkOps, true)) {
        PrintToServer("✅ Bulk operations completed successfully");
    } else {
        PrintToServer("❌ Bulk operations failed");
    }

    delete bulkOps;
    conn.Close();
    return Plugin_Handled;
}

public Action Command_TestQueries(int args) {
    PrintToServer("=== MongoDB Enhanced Query Test ===");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Test basic query with JSON filter (simplified)
    char queryFilter[256];
    Format(queryFilter, sizeof(queryFilter), "{\"score\":{\"$gte\":500}}");

    StringMap result = players.FindOneJSON(queryFilter);
    if (result != null) {
        PrintToServer("✅ Enhanced query found result");
        delete result;
    } else {
        PrintToServer("❌ Enhanced query failed");
    }

    // Test another query
    char logicalFilter[256];
    Format(logicalFilter, sizeof(logicalFilter), "{\"$or\":[{\"score\":{\"$gte\":1000}},{\"kills\":{\"$gte\":50}}]}");

    StringMap logicalResult = players.FindOneJSON(logicalFilter);
    if (logicalResult != null) {
        PrintToServer("✅ Logical query found result");
        delete logicalResult;
    } else {
        PrintToServer("❌ Logical query failed");
    }

    conn.Close();
    return Plugin_Handled;
}

public Action Command_TestPerformance(int args) {
    PrintToServer("=== MongoDB Performance Test ===");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Test multiple operations and measure time
    int startTime = GetTime();

    // Perform multiple inserts
    for (int i = 0; i < 10; i++) {
        char jsonDoc[256];
        Format(jsonDoc, sizeof(jsonDoc), "{\"name\":\"PerfTest_%d\",\"score\":%d,\"timestamp\":%d}", i, GetRandomInt(0, 1000), GetTime());

        char insertedId[64];
        players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId));
    }

    // Perform multiple finds
    for (int i = 0; i < 5; i++) {
        char filter[128];
        Format(filter, sizeof(filter), "{\"score\":{\"$gte\":%d}}", GetRandomInt(0, 500));
        StringMap result = players.FindOneJSON(filter);
        if (result != null) {
            delete result;
        }
    }

    int endTime = GetTime();
    PrintToServer("✅ Performance test completed in %d seconds", endTime - startTime);

    conn.Close();
    return Plugin_Handled;
}

public Action Command_TestErrorHandling(int args) {
    PrintToServer("=== MongoDB Error Handling Test ===");

    // Test with invalid URL
    MongoConnection badConn = new MongoConnection("http://invalid-url:9999");
    if (!badConn.IsConnected()) {
        PrintToServer("✅ Correctly handled invalid connection");
    } else {
        PrintToServer("❌ Should have failed with invalid URL");
    }
    badConn.Close();

    // Test with valid connection but invalid operations
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (conn.IsConnected()) {
        MongoCollection players = conn.GetCollection("gamedb", "players");

        // Test invalid JSON
        char insertedId[64];
        if (!players.InsertOneJSON("{invalid json}", insertedId, sizeof(insertedId))) {
            PrintToServer("✅ Correctly handled invalid JSON");
        } else {
            PrintToServer("❌ Should have failed with invalid JSON");
        }

        // Test invalid filter
        StringMap result = players.FindOneJSON("{invalid: filter}");
        if (result == null) {
            PrintToServer("✅ Correctly handled invalid filter");
        } else {
            PrintToServer("❌ Should have failed with invalid filter");
            delete result;
        }
    }

    conn.Close();
    return Plugin_Handled;
}

// Real Data Test Functions

public Action Command_RealDataTest(int args) {
    PrintToServer("=== MongoDB Real Data Test ===");

    // Load configuration first
    char configPath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, configPath, sizeof(configPath), "configs/mongodb.cfg");
    MongoDB_LoadConfig(configPath);

    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Insert real-looking player data
    for (int i = 1; i <= MaxClients; i++) {
        if (IsClientConnected(i) && !IsFakeClient(i)) {
            char playerName[MAX_NAME_LENGTH];
            GetClientName(i, playerName, sizeof(playerName));

            StringMap doc = new StringMap();
            doc.SetString("name", playerName);
            doc.SetString("steamid", "STEAM_1:0:123456"); // Mock SteamID
            doc.SetValue("userid", GetClientUserId(i));
            doc.SetValue("score", GetRandomInt(0, 5000));
            doc.SetValue("timestamp", GetTime());
            doc.SetString("server", "Test Server");
            doc.SetValue("connected", 1);

            char insertedId[64];
            if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
                PrintToServer("✅ Real player '%s' data inserted", playerName);
            }

            delete doc;
        }
    }

    conn.Close();
    return Plugin_Handled;
}

// Chat Commands for In-Game Testing

public Action Command_MongoTestChat(int client, int args) {
    if (client == 0) {
        ReplyToCommand(client, "This command can only be used in-game");
        return Plugin_Handled;
    }

    PrintToChat(client, "[MongoDB] Testing connection...");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] ❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");
    int count = players.CountDocuments(null);

    PrintToChat(client, "[MongoDB] ✅ Connected! Found %d documents", count);

    conn.Close();
    return Plugin_Handled;
}

public Action Command_MongoInsertChat(int client, int args) {
    if (client == 0) {
        ReplyToCommand(client, "This command can only be used in-game");
        return Plugin_Handled;
    }

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] ❌ Connection failed");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create a test document with player info
    StringMap doc = new StringMap();
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));

    doc.SetString("name", playerName);
    doc.SetValue("steamid", GetSteamAccountID(client));
    doc.SetValue("score", GetRandomInt(1, 1000));
    doc.SetValue("timestamp", GetTime());
    doc.SetString("method", "chat_command");

    char insertedId[64];
    if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
        PrintToChat(client, "[MongoDB] ✅ Your data was inserted! ID: %s", insertedId);
    } else {
        PrintToChat(client, "[MongoDB] ❌ Failed to insert your data");
    }

    delete doc;
    conn.Close();
    return Plugin_Handled;
}
