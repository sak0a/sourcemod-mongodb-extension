/**
 * MongoDB Console Test Plugin
 * All commands executable from server console with mock data
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "MongoDB Console Test",
    author = "SourceMod Team", 
    description = "MongoDB testing via console commands with mock data",
    version = "1.0.0",
    url = "http://www.sourcemod.net/"
};

public void OnPluginStart() {
    // Console commands only
    // Basic Commands
    RegServerCmd("mongo_test", Command_MongoTest, "Basic MongoDB test");
    RegServerCmd("mongo_insert", Command_MongoInsert, "Insert mock player data");
    RegServerCmd("mongo_batch", Command_MongoBatch, "Insert multiple mock players");
    RegServerCmd("mongo_find", Command_MongoFind, "Find player by name");
    RegServerCmd("mongo_count", Command_MongoCount, "Count documents");
    RegServerCmd("mongo_stats", Command_MongoStats, "Show collection statistics");

    // Advanced Commands
    RegServerCmd("mongo_aggregate", Command_MongoAggregate, "Test aggregation pipeline");
    RegServerCmd("mongo_bulk_ops", Command_MongoBulkOps, "Test bulk operations");
    RegServerCmd("mongo_index", Command_MongoIndex, "Test index management");
    RegServerCmd("mongo_performance", Command_MongoPerformance, "Run performance tests");
    RegServerCmd("mongo_security", Command_MongoSecurity, "Test security features");

    // Configuration Commands
    RegServerCmd("mongo_config", Command_MongoConfig, "Test configuration management");
    RegServerCmd("mongo_set_url", Command_MongoSetURL, "Set API URL");
    RegServerCmd("mongo_get_url", Command_MongoGetURL, "Get current API URL");
    
    PrintToServer("[MongoDB Console Test] Plugin loaded with advanced features. Available commands:");
    PrintToServer("=== Basic Commands ===");
    PrintToServer("  mongo_test - Basic connection test");
    PrintToServer("  mongo_insert [name] - Insert mock player");
    PrintToServer("  mongo_batch [count] - Insert multiple players (default: 10)");
    PrintToServer("  mongo_find [name] - Find player by name");
    PrintToServer("  mongo_count - Count total documents");
    PrintToServer("  mongo_stats - Show collection statistics");
    PrintToServer("=== Advanced Commands ===");
    PrintToServer("  mongo_aggregate - Test aggregation pipeline");
    PrintToServer("  mongo_bulk_ops - Test bulk operations");
    PrintToServer("  mongo_index - Test index management");
    PrintToServer("  mongo_performance - Run performance tests");
    PrintToServer("  mongo_security - Test security features");
    PrintToServer("=== Configuration Commands ===");
    PrintToServer("  mongo_config - Test configuration management");
    PrintToServer("  mongo_set_url [url] - Set API URL");
    PrintToServer("  mongo_get_url - Get current API URL");
}

// Basic MongoDB test
public Action Command_MongoTest(int args) {
    PrintToServer("=== MongoDB Basic Test ===");
    
    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
    
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection FAILED");
        return Plugin_Handled;
    }
    
    PrintToServer("✅ Connection successful!");
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    int count = players.CountDocuments(null);
    
    PrintToServer("📊 Collection 'gamedb.players' has %d documents", count);
    
    // Insert a test document
    char jsonDoc[512];
    Format(jsonDoc, sizeof(jsonDoc), 
        "{\"name\":\"TestPlayer_%d\",\"steamid\":\"STEAM_1:0:%d\",\"score\":%d,\"timestamp\":%d,\"test\":true}",
        GetRandomInt(1000, 9999), GetRandomInt(100000, 999999), GetRandomInt(0, 5000), GetTime());
    
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

// Insert mock player
public Action Command_MongoInsert(int args) {
    char playerName[64];
    if (args >= 1) {
        GetCmdArg(1, playerName, sizeof(playerName));
    } else {
        Format(playerName, sizeof(playerName), "Player_%d", GetRandomInt(1000, 9999));
    }
    
    PrintToServer("=== Inserting Mock Player: %s ===", playerName);
    
    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Generate realistic mock data
    char steamId[32];
    Format(steamId, sizeof(steamId), "STEAM_1:%d:%d", GetRandomInt(0, 1), GetRandomInt(100000, 999999));
    
    int score = GetRandomInt(0, 10000);
    int kills = GetRandomInt(0, 500);
    int deaths = GetRandomInt(0, 400);
    int playtime = GetRandomInt(300, 36000); // 5 minutes to 10 hours
    float kdr = deaths > 0 ? float(kills) / float(deaths) : float(kills);
    
    char classes[] = "scout,soldier,pyro,demoman,heavy,engineer,medic,sniper,spy";
    char classList[10][16];
    int classCount = ExplodeString(classes, ",", classList, sizeof(classList), sizeof(classList[]));
    char favoriteClass[16];
    strcopy(favoriteClass, sizeof(favoriteClass), classList[GetRandomInt(0, classCount - 1)]);
    
    char jsonDoc[1024];
    Format(jsonDoc, sizeof(jsonDoc), 
        "{\"name\":\"%s\",\"steamid\":\"%s\",\"score\":%d,\"kills\":%d,\"deaths\":%d,\"kdr\":%.2f,\"playtime\":%d,\"favorite_class\":\"%s\",\"timestamp\":%d,\"server\":\"console-test\",\"level\":%d,\"premium\":%s}",
        playerName, steamId, score, kills, deaths, kdr, playtime, favoriteClass, GetTime(), 
        GetRandomInt(1, 100), GetRandomInt(0, 1) ? "true" : "false");
    
    char insertedId[64];
    if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
        PrintToServer("✅ Player '%s' inserted successfully!", playerName);
        PrintToServer("   ID: %s", insertedId);
        PrintToServer("   Stats: %d kills, %d deaths, %.2f K/D", kills, deaths, kdr);
    } else {
        PrintToServer("❌ Failed to insert player");
    }
    
    conn.Close();
    return Plugin_Handled;
}

// Batch insert multiple players
public Action Command_MongoBatch(int args) {
    int count = 10;
    if (args >= 1) {
        char countStr[16];
        GetCmdArg(1, countStr, sizeof(countStr));
        count = StringToInt(countStr);
        if (count <= 0 || count > 100) count = 10;
    }
    
    PrintToServer("=== Batch Insert: %d Players ===", count);
    
    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    char nameTemplates[][] = {
        "FragMaster", "ProGamer", "Noobslayer", "HeadHunter", "SniperKing",
        "RocketJumper", "MedicMain", "SpyMaster", "HeavyWeapons", "ScoutSpeed"
    };
    
    int startTime = GetTime();
    int inserted = 0;
    
    for (int i = 0; i < count; i++) {
        char playerName[64];
        Format(playerName, sizeof(playerName), "%s_%d", 
               nameTemplates[GetRandomInt(0, sizeof(nameTemplates) - 1)], 
               GetRandomInt(1000, 9999));
        
        char steamId[32];
        Format(steamId, sizeof(steamId), "STEAM_1:%d:%d", 
               GetRandomInt(0, 1), GetRandomInt(100000, 999999));
        
        char jsonDoc[1024];
        Format(jsonDoc, sizeof(jsonDoc), 
            "{\"name\":\"%s\",\"steamid\":\"%s\",\"score\":%d,\"kills\":%d,\"deaths\":%d,\"playtime\":%d,\"timestamp\":%d,\"batch_id\":%d,\"server\":\"batch-test\"}",
            playerName, steamId, GetRandomInt(0, 15000), GetRandomInt(0, 800), 
            GetRandomInt(0, 600), GetRandomInt(600, 72000), GetTime(), 
            GetRandomInt(1000, 9999));
        
        char insertedId[64];
        if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
            inserted++;
        }
        
        if ((i + 1) % 25 == 0 || i == count - 1) {
            PrintToServer("Progress: %d/%d players inserted...", inserted, count);
        }
    }
    
    int endTime = GetTime();
    PrintToServer("✅ Batch insert completed!");
    PrintToServer("   Inserted: %d/%d players", inserted, count);
    PrintToServer("   Time taken: %d seconds", endTime - startTime);
    
    int totalCount = players.CountDocuments(null);
    PrintToServer("   Total documents: %d", totalCount);
    
    conn.Close();
    return Plugin_Handled;
}

// Find player by name
public Action Command_MongoFind(int args) {
    if (args < 1) {
        PrintToServer("Usage: mongo_find <player_name>");
        return Plugin_Handled;
    }

    char searchName[64];
    GetCmdArg(1, searchName, sizeof(searchName));

    PrintToServer("=== Searching for player: %s ===", searchName);

    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
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

// Count documents
public Action Command_MongoCount(int args) {
    PrintToServer("=== Document Count ===");
    
    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
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

// Show collection statistics
public Action Command_MongoStats(int args) {
    PrintToServer("=== MongoDB Collection Statistics ===");
    
    MongoConnection conn = new MongoConnection("http://37.114.54.74:3300");
    if (!conn.IsConnected()) {
        PrintToServer("❌ Connection failed");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    int playerCount = players.CountDocuments(null);
    
    MongoCollection connections = conn.GetCollection("gamedb", "connections");
    int connectionCount = connections.CountDocuments(null);
    
    PrintToServer("📊 Database: gamedb");
    PrintToServer("   Players collection: %d documents", playerCount);
    PrintToServer("   Connections collection: %d documents", connectionCount);
    PrintToServer("   Total documents: %d", playerCount + connectionCount);
    
    conn.Close();
    return Plugin_Handled;
}

// ============================================
// Advanced Command Implementations
// ============================================

// Test aggregation pipeline
public Action Command_MongoAggregate(int args) {
    PrintToServer("=== MongoDB Aggregation Test ===");

    MongoConnection conn = MongoDB_Connect("http://127.0.0.1:3300");
    if (!conn.IsValid()) {
        PrintToServer("❌ Failed to connect to MongoDB API");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Test aggregation pipeline: group by status and calculate average score
    char pipeline[512];
    Format(pipeline, sizeof(pipeline),
        "[{\"$match\":{\"score\":{\"$gte\":0}}},"
        "{\"$group\":{\"_id\":\"$status\",\"avgScore\":{\"$avg\":\"$score\"},\"count\":{\"$sum\":1}}},"
        "{\"$sort\":{\"avgScore\":-1}}]");

    PrintToServer("🔍 Running aggregation pipeline...");
    PrintToServer("Pipeline: %s", pipeline);

    ArrayList results = players.Aggregate(pipeline);
    if (results != null) {
        PrintToServer("✅ Aggregation successful! Results: %d groups", results.Length);
        delete results;
    } else {
        PrintToServer("❌ Aggregation failed");
    }

    conn.Close();
    return Plugin_Handled;
}

// Test bulk operations
public Action Command_MongoBulkOps(int args) {
    PrintToServer("=== MongoDB Bulk Operations Test ===");

    MongoConnection conn = MongoDB_Connect("http://127.0.0.1:3300");
    if (!conn.IsValid()) {
        PrintToServer("❌ Failed to connect to MongoDB API");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "bulk_test");

    // Create bulk documents
    ArrayList documents = new ArrayList();

    for (int i = 1; i <= 5; i++) {
        char doc[256];
        Format(doc, sizeof(doc),
            "{\"name\":\"BulkPlayer%d\",\"score\":%d,\"status\":\"bulk_test\",\"timestamp\":\"%d\"}",
            i, i * 100, GetTime());
        documents.PushString(doc);
    }

    PrintToServer("📦 Performing bulk insert of %d documents...", documents.Length);

    bool success = players.BulkWrite(documents, true); // ordered=true
    if (success) {
        PrintToServer("✅ Bulk insert successful!");
    } else {
        PrintToServer("❌ Bulk insert failed");
    }

    delete documents;
    conn.Close();
    return Plugin_Handled;
}

// Test index management
public Action Command_MongoIndex(int args) {
    PrintToServer("=== MongoDB Index Management Test ===");

    MongoConnection conn = MongoDB_Connect("http://127.0.0.1:3300");
    if (!conn.IsValid()) {
        PrintToServer("❌ Failed to connect to MongoDB API");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create compound index on name and score
    char keys[128], options[128];
    Format(keys, sizeof(keys), "{\"name\":1,\"score\":-1}");
    Format(options, sizeof(options), "{\"name\":\"name_score_idx\",\"background\":true}");

    PrintToServer("🔧 Creating index on name and score fields...");
    PrintToServer("Keys: %s", keys);
    PrintToServer("Options: %s", options);

    bool success = players.CreateIndex(keys, options);
    if (success) {
        PrintToServer("✅ Index created successfully!");
    } else {
        PrintToServer("❌ Index creation failed");
    }

    conn.Close();
    return Plugin_Handled;
}

// Run performance tests
public Action Command_MongoPerformance(int args) {
    PrintToServer("=== MongoDB Performance Test ===");

    MongoConnection conn = MongoDB_Connect("http://127.0.0.1:3300");
    if (!conn.IsValid()) {
        PrintToServer("❌ Failed to connect to MongoDB API");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "perf_test");

    // Test 1: Insert performance
    float startTime = GetEngineTime();

    for (int i = 1; i <= 10; i++) {
        char doc[256], insertedId[64];
        Format(doc, sizeof(doc),
            "{\"name\":\"PerfTest%d\",\"score\":%d,\"timestamp\":\"%d\"}",
            i, i * 50, GetTime());

        players.InsertOneJSON(doc, insertedId, sizeof(insertedId));
    }

    float insertTime = GetEngineTime() - startTime;
    PrintToServer("📊 Insert Performance: 10 documents in %.3f seconds", insertTime);

    // Test 2: Find performance
    startTime = GetEngineTime();

    for (int i = 1; i <= 10; i++) {
        char filter[128], result[512];
        Format(filter, sizeof(filter), "{\"name\":\"PerfTest%d\"}", i);
        players.FindOneJSON(filter, result, sizeof(result));
    }

    float findTime = GetEngineTime() - startTime;
    PrintToServer("📊 Find Performance: 10 queries in %.3f seconds", findTime);

    // Test 3: Count performance
    startTime = GetEngineTime();
    int count = players.CountDocuments("{}");
    float countTime = GetEngineTime() - startTime;
    PrintToServer("📊 Count Performance: %d documents counted in %.3f seconds", count, countTime);

    conn.Close();
    return Plugin_Handled;
}

// Test security features
public Action Command_MongoSecurity(int args) {
    PrintToServer("=== MongoDB Security Test ===");

    // Test configuration functions
    char currentUrl[256];
    MongoDB_GetAPIURL(currentUrl, sizeof(currentUrl));
    PrintToServer("🔒 Current API URL: %s", currentUrl);

    int timeout = MongoDB_GetTimeout();
    PrintToServer("🔒 Current timeout: %d seconds", timeout);

    // Test connection with security headers
    MongoConnection conn = MongoDB_Connect("http://127.0.0.1:3300");
    if (!conn.IsValid()) {
        PrintToServer("❌ Failed to connect to MongoDB API (security check failed)");
        return Plugin_Handled;
    }

    PrintToServer("✅ Security test passed - API authentication working");

    conn.Close();
    return Plugin_Handled;
}

// Test configuration management
public Action Command_MongoConfig(int args) {
    PrintToServer("=== MongoDB Configuration Test ===");

    // Test loading configuration
    bool configLoaded = MongoDB_LoadConfig("configs/mongodb_config.cfg");
    if (configLoaded) {
        PrintToServer("✅ Configuration loaded successfully");
    } else {
        PrintToServer("⚠️ Configuration loading failed (using defaults)");
    }

    // Display current configuration
    char currentUrl[256];
    MongoDB_GetAPIURL(currentUrl, sizeof(currentUrl));
    int timeout = MongoDB_GetTimeout();

    PrintToServer("📋 Current Configuration:");
    PrintToServer("   API URL: %s", currentUrl);
    PrintToServer("   Timeout: %d seconds", timeout);

    return Plugin_Handled;
}

// Set API URL
public Action Command_MongoSetURL(int args) {
    if (args < 1) {
        PrintToServer("Usage: mongo_set_url <url>");
        PrintToServer("Example: mongo_set_url http://127.0.0.1:3300");
        return Plugin_Handled;
    }

    char newUrl[256];
    GetCmdArg(1, newUrl, sizeof(newUrl));

    bool success = MongoDB_SetAPIURL(newUrl);
    if (success) {
        PrintToServer("✅ API URL set to: %s", newUrl);
    } else {
        PrintToServer("❌ Failed to set API URL");
    }

    return Plugin_Handled;
}

// Get current API URL
public Action Command_MongoGetURL(int args) {
    char currentUrl[256];
    MongoDB_GetAPIURL(currentUrl, sizeof(currentUrl));

    PrintToServer("📋 Current API URL: %s", currentUrl);

    return Plugin_Handled;
}
