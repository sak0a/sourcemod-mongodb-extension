/**
 * Simple test script for HTTP MongoDB Extension
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "HTTP MongoDB Test",
    author = "SourceMod Team",
    description = "Test script for HTTP MongoDB Extension",
    version = "1.0.0",
    url = "http://www.sourcemod.net/"
};

public void OnPluginStart() {
    RegConsoleCmd("sm_mongo_test", Command_MongoTest, "Test MongoDB operations");
    RegConsoleCmd("sm_mongo_connect", Command_MongoConnect, "Test MongoDB connection");
    RegConsoleCmd("sm_mongo_insert", Command_MongoInsert, "Test MongoDB insert");
    RegConsoleCmd("sm_mongo_find", Command_MongoFind, "Test MongoDB find");
}

public Action Command_MongoTest(int client, int args) {
    PrintToChat(client, "[MongoDB] Starting comprehensive test...");

    // Test connection using the new interface
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");

    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] Failed to connect to database");
        return Plugin_Handled;
    }

    PrintToChat(client, "[MongoDB] Connected successfully!");

    // Get collection
    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create a document
    MongoDocument doc = new MongoDocument();
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));

    doc.SetString("name", playerName);
    doc.SetValue("score", GetRandomInt(1, 1000));
    doc.SetDate("timestamp", GetTime());

    // Insert document
    char insertedId[64];
    if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
        PrintToChat(client, "[MongoDB] Document inserted! ID: %s", insertedId);
    } else {
        PrintToChat(client, "[MongoDB] Failed to insert document");
    }

    // Find document
    MongoDocument filter = new MongoDocument();
    filter.SetString("name", playerName);

    MongoDocument result = view_as<MongoDocument>(players.FindOne(filter));
    if (result != null) {
        PrintToChat(client, "[MongoDB] Found player document!");
        delete result;
    } else {
        PrintToChat(client, "[MongoDB] Player not found");
    }

    // Count documents
    int count = players.CountDocuments(null);
    PrintToChat(client, "[MongoDB] Total documents in collection: %d", count);

    // Cleanup
    delete doc;
    delete filter;
    conn.Close();

    return Plugin_Handled;
}

public Action Command_MongoConnect(int client, int args) {
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");

    if (conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] Connection successful!");
        conn.Close();
    } else {
        PrintToChat(client, "[MongoDB] Connection failed");
    }

    return Plugin_Handled;
}

public Action Command_MongoInsert(int client, int args) {
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] Failed to connect");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create a test document with player info
    MongoDocument doc = new MongoDocument();
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));

    doc.SetString("name", playerName);
    doc.SetValue("steamid", GetSteamAccountID(client));
    doc.SetValue("score", GetRandomInt(1, 1000));
    doc.SetDate("timestamp", GetTime());

    char insertedId[64];
    if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
        PrintToChat(client, "[MongoDB] Player data inserted! ID: %s", insertedId);
    } else {
        PrintToChat(client, "[MongoDB] Failed to insert player data");
    }

    delete doc;
    conn.Close();
    return Plugin_Handled;
}

public Action Command_MongoFind(int client, int args) {
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    if (!conn.IsConnected()) {
        PrintToChat(client, "[MongoDB] Failed to connect");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Find all players (null filter = find all)
    int count = players.CountDocuments(null);
    PrintToChat(client, "[MongoDB] Found %d documents in players collection", count);

    conn.Close();
    return Plugin_Handled;
}
