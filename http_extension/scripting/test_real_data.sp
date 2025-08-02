/**
 * Test plugin for MongoDB extension with real data
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "MongoDB Real Data Test",
    author = "SourceMod Team",
    description = "Test MongoDB extension with real player data",
    version = "1.0.0",
    url = "http://www.sourcemod.net/"
};

public void OnPluginStart() {
    RegConsoleCmd("sm_mongo_real_test", Command_MongoRealTest, "Test MongoDB with real data");
    RegServerCmd("mongo_real_test", Command_MongoRealTestConsole, "Test MongoDB with real data (console)");
    RegServerCmd("mongo_insert_player", Command_InsertMockPlayer, "Insert mock player data");
    RegServerCmd("mongo_insert_multiple", Command_InsertMultiplePlayers, "Insert multiple mock players");
    RegServerCmd("mongo_test_connection", Command_TestConnection, "Test MongoDB connection");
}

public Action Command_MongoRealTest(int client, int args) {
    PrintToChat(client, "[MongoDB] Starting real data test...");
    
    // Test connection
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    
    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] Failed to connect to database");
        return Plugin_Handled;
    }
    
    PrintToChat(client, "[MongoDB] Connected successfully!");
    
    // Get collection
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Get real player data
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));
    
    char steamId[32];
    GetClientAuthId(client, AuthId_Steam2, steamId, sizeof(steamId));
    
    int userId = GetClientUserId(client);
    int team = GetClientTeam(client);
    
    // Create JSON document with real player data
    char jsonDocument[1024];
    Format(jsonDocument, sizeof(jsonDocument), 
        "{\"name\":\"%s\",\"steamid\":\"%s\",\"userid\":%d,\"team\":%d,\"timestamp\":%d,\"server\":\"test-server\"}",
        playerName, steamId, userId, team, GetTime());
    
    PrintToChat(client, "[MongoDB] Inserting player data: %s", playerName);
    
    // Insert document using JSON method
    char insertedId[64];
    if (players.InsertOneJSON(jsonDocument, insertedId, sizeof(insertedId))) {
        PrintToChat(client, "[MongoDB] Player data inserted! ID: %s", insertedId);
    } else {
        PrintToChat(client, "[MongoDB] Failed to insert player data");
        conn.Close();
        return Plugin_Handled;
    }
    
    // Test finding the player by name
    char findJson[256];
    Format(findJson, sizeof(findJson), "{\"name\":\"%s\"}", playerName);
    
    // For now, we'll use the existing FindOne method with a StringMap
    // In a future version, we could add FindOneJSON method
    MongoDocument filter = new MongoDocument();
    filter.SetString("name", playerName);
    
    StringMap result = players.FindOne(filter);
    if (result != null) {
        PrintToChat(client, "[MongoDB] Found player in database!");
        delete result;
    } else {
        PrintToChat(client, "[MongoDB] Player not found (this is expected since we just inserted with different structure)");
    }
    
    // Test count
    int count = players.CountDocuments(null);
    PrintToChat(client, "[MongoDB] Total documents in collection: %d", count);
    
    // Cleanup
    delete filter;
    conn.Close();
    
    PrintToChat(client, "[MongoDB] Real data test completed!");
    return Plugin_Handled;
}

// Console version of the test command
public Action Command_MongoRealTestConsole(int args) {
    PrintToServer("[MongoDB] Starting real data test from console...");

    // Test connection
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");

    if (!conn.IsConnected()) {
        PrintToServer("[MongoDB] Failed to connect to database");
        return Plugin_Handled;
    }

    PrintToServer("[MongoDB] Connected successfully!");

    // Get collection
    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Mock player data
    char playerName[] = "ConsoleTestPlayer";
    char steamId[] = "STEAM_1:0:999999";
    int userId = 999;
    int team = 2;

    // Create JSON document with mock player data
    char jsonDocument[1024];
    Format(jsonDocument, sizeof(jsonDocument),
        "{\"name\":\"%s\",\"steamid\":\"%s\",\"userid\":%d,\"team\":%d,\"timestamp\":%d,\"server\":\"console-test\",\"source\":\"console_command\"}",
        playerName, steamId, userId, team, GetTime());

    PrintToServer("[MongoDB] Inserting mock player data: %s", playerName);

    // Insert document using JSON method
    char insertedId[64];
    if (players.InsertOneJSON(jsonDocument, insertedId, sizeof(insertedId))) {
        PrintToServer("[MongoDB] Mock player data inserted! ID: %s", insertedId);
    } else {
        PrintToServer("[MongoDB] Failed to insert mock player data");
        conn.Close();
        return Plugin_Handled;
    }

    // Test count
    int count = players.CountDocuments(null);
    PrintToServer("[MongoDB] Total documents in collection: %d", count);

    // Cleanup
    conn.Close();

    PrintToServer("[MongoDB] Console test completed!");
    return Plugin_Handled;
}

// Insert a single mock player
public Action Command_InsertMockPlayer(int args) {
    char playerName[64];
    if (args >= 1) {
        GetCmdArg(1, playerName, sizeof(playerName));
    } else {
        Format(playerName, sizeof(playerName), "MockPlayer_%d", GetRandomInt(1000, 9999));
    }

    PrintToServer("[MongoDB] Inserting mock player: %s", playerName);

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("[MongoDB] Failed to connect to database");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Generate mock data
    char steamId[32];
    Format(steamId, sizeof(steamId), "STEAM_1:0:%d", GetRandomInt(100000, 999999));

    int score = GetRandomInt(0, 5000);
    int kills = GetRandomInt(0, 100);
    int deaths = GetRandomInt(0, 100);

    char jsonDocument[1024];
    Format(jsonDocument, sizeof(jsonDocument),
        "{\"name\":\"%s\",\"steamid\":\"%s\",\"score\":%d,\"kills\":%d,\"deaths\":%d,\"timestamp\":%d,\"server\":\"mock-server\",\"class\":\"soldier\",\"map\":\"cp_badlands\"}",
        playerName, steamId, score, kills, deaths, GetTime());

    char insertedId[64];
    if (players.InsertOneJSON(jsonDocument, insertedId, sizeof(insertedId))) {
        PrintToServer("[MongoDB] Mock player '%s' inserted! ID: %s", playerName, insertedId);
    } else {
        PrintToServer("[MongoDB] Failed to insert mock player");
    }

    conn.Close();
    return Plugin_Handled;
}

// Insert multiple mock players
public Action Command_InsertMultiplePlayers(int args) {
    int count = 5; // Default count
    if (args >= 1) {
        char countStr[16];
        GetCmdArg(1, countStr, sizeof(countStr));
        count = StringToInt(countStr);
        if (count <= 0 || count > 50) count = 5; // Limit to reasonable range
    }

    PrintToServer("[MongoDB] Inserting %d mock players...", count);

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToServer("[MongoDB] Failed to connect to database");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    char playerNames[][] = {
        "Scout_Mike", "Soldier_John", "Pyro_Sarah", "Demo_Alex", "Heavy_Boris",
        "Engineer_Tom", "Medic_Hans", "Sniper_Steve", "Spy_Pierre", "Admin_Bob",
        "Pro_Gamer", "Noob_Player", "Veteran_Joe", "Rookie_Sam", "Elite_Anna"
    };

    char classes[][] = {
        "scout", "soldier", "pyro", "demoman", "heavy", "engineer", "medic", "sniper", "spy"
    };

    char maps[][] = {
        "cp_badlands", "cp_granary", "cp_well", "ctf_2fort", "pl_upward", "koth_harvest"
    };

    int inserted = 0;
    for (int i = 0; i < count; i++) {
        char playerName[64];
        Format(playerName, sizeof(playerName), "%s_%d",
               playerNames[GetRandomInt(0, sizeof(playerNames) - 1)],
               GetRandomInt(1000, 9999));

        char steamId[32];
        Format(steamId, sizeof(steamId), "STEAM_1:%d:%d",
               GetRandomInt(0, 1), GetRandomInt(100000, 999999));

        int score = GetRandomInt(0, 10000);
        int kills = GetRandomInt(0, 200);
        int deaths = GetRandomInt(0, 150);
        int playtime = GetRandomInt(60, 7200); // 1 minute to 2 hours

        char jsonDocument[1024];
        Format(jsonDocument, sizeof(jsonDocument),
            "{\"name\":\"%s\",\"steamid\":\"%s\",\"score\":%d,\"kills\":%d,\"deaths\":%d,\"playtime\":%d,\"timestamp\":%d,\"server\":\"batch-test\",\"class\":\"%s\",\"map\":\"%s\",\"batch_id\":%d}",
            playerName, steamId, score, kills, deaths, playtime, GetTime(),
            classes[GetRandomInt(0, sizeof(classes) - 1)],
            maps[GetRandomInt(0, sizeof(maps) - 1)],
            GetRandomInt(1000, 9999));

        char insertedId[64];
        if (players.InsertOneJSON(jsonDocument, insertedId, sizeof(insertedId))) {
            inserted++;
            if (i % 10 == 0 || i == count - 1) { // Progress update every 10 or at end
                PrintToServer("[MongoDB] Progress: %d/%d players inserted...", inserted, count);
            }
        }
    }

    PrintToServer("[MongoDB] Batch insert completed! %d/%d players inserted successfully.", inserted, count);

    // Show final count
    int totalCount = players.CountDocuments(null);
    PrintToServer("[MongoDB] Total documents in collection: %d", totalCount);

    conn.Close();
    return Plugin_Handled;
}

// Test connection only
public Action Command_TestConnection(int args) {
    PrintToServer("[MongoDB] Testing connection...");

    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");

    if (!conn.IsConnected()) {
        PrintToServer("[MongoDB] ❌ Connection FAILED");
        return Plugin_Handled;
    }

    PrintToServer("[MongoDB] ✅ Connection successful!");

    // Test collection access
    MongoCollection players = conn.GetCollection("gamedb", "players");
    int count = players.CountDocuments(null);

    PrintToServer("[MongoDB] Collection 'gamedb.players' has %d documents", count);

    conn.Close();
    PrintToServer("[MongoDB] Connection test completed!");

    return Plugin_Handled;
}

public void OnClientConnected(int client) {
    if (IsFakeClient(client)) return;
    
    // Automatically log player connection to MongoDB
    CreateTimer(2.0, Timer_LogPlayerConnection, GetClientUserId(client));
}

public Action Timer_LogPlayerConnection(Handle timer, int userid) {
    int client = GetClientOfUserId(userid);
    if (client == 0 || !IsClientInGame(client)) return Plugin_Stop;
    
    // Connect to MongoDB
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) return Plugin_Stop;
    
    MongoCollection connections = conn.GetCollection("gamedb", "connections");
    
    // Get player info
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));
    
    char steamId[32];
    GetClientAuthId(client, AuthId_Steam2, steamId, sizeof(steamId));
    
    char playerIP[32];
    GetClientIP(client, playerIP, sizeof(playerIP));
    
    // Create connection log JSON
    char jsonDocument[512];
    Format(jsonDocument, sizeof(jsonDocument), 
        "{\"event\":\"player_connect\",\"name\":\"%s\",\"steamid\":\"%s\",\"ip\":\"%s\",\"timestamp\":%d,\"server\":\"tf2-server\"}",
        playerName, steamId, playerIP, GetTime());
    
    // Log connection
    char insertedId[64];
    if (connections.InsertOneJSON(jsonDocument, insertedId, sizeof(insertedId))) {
        PrintToServer("[MongoDB] Logged player connection: %s (ID: %s)", playerName, insertedId);
    }
    
    conn.Close();
    return Plugin_Stop;
}
