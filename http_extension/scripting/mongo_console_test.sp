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
    RegServerCmd("mongo_test", Command_MongoTest, "Basic MongoDB test");
    RegServerCmd("mongo_insert", Command_MongoInsert, "Insert mock player data");
    RegServerCmd("mongo_batch", Command_MongoBatch, "Insert multiple mock players");
    RegServerCmd("mongo_find", Command_MongoFind, "Find player by name");
    RegServerCmd("mongo_count", Command_MongoCount, "Count documents");
    RegServerCmd("mongo_stats", Command_MongoStats, "Show collection statistics");
    
    PrintToServer("[MongoDB Console Test] Plugin loaded. Available commands:");
    PrintToServer("  mongo_test - Basic connection test");
    PrintToServer("  mongo_insert [name] - Insert mock player");
    PrintToServer("  mongo_batch [count] - Insert multiple players (default: 10)");
    PrintToServer("  mongo_find [name] - Find player by name");
    PrintToServer("  mongo_count - Count total documents");
    PrintToServer("  mongo_stats - Show collection statistics");
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
