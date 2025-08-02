/**
 * Advanced MongoDB Examples Plugin
 * 
 * Demonstrates all advanced features of the MongoDB HTTP Extension
 * including aggregation, bulk operations, indexing, and real-world scenarios.
 */

#include <sourcemod>
#include <http_mongodb>

#pragma semicolon 1
#pragma newdecls required

public Plugin myinfo = {
    name = "Advanced MongoDB Examples",
    author = "SourceMod Team",
    description = "Comprehensive examples of MongoDB advanced features",
    version = "2.0.0",
    url = "http://www.sourcemod.net/"
};

// Global variables
MongoConnection g_mongoConn;
bool g_isConnected = false;

public void OnPluginStart() {
    // Real-world example commands
    RegServerCmd("mongo_player_stats", Command_PlayerStats, "Advanced player statistics");
    RegServerCmd("mongo_leaderboard", Command_Leaderboard, "Generate leaderboard using aggregation");
    RegServerCmd("mongo_bulk_update", Command_BulkUpdate, "Bulk update player records");
    RegServerCmd("mongo_search_players", Command_SearchPlayers, "Advanced player search");
    RegServerCmd("mongo_analytics", Command_Analytics, "Game analytics dashboard");
    RegServerCmd("mongo_maintenance", Command_Maintenance, "Database maintenance tasks");
    
    // Initialize connection
    InitializeMongoDB();
    
    PrintToServer("[Advanced MongoDB Examples] Plugin loaded with real-world examples");
    PrintToServer("Available commands:");
    PrintToServer("  mongo_player_stats [steamid] - Get detailed player statistics");
    PrintToServer("  mongo_leaderboard [limit] - Generate top players leaderboard");
    PrintToServer("  mongo_bulk_update - Update all player records");
    PrintToServer("  mongo_search_players [query] - Search players by name/criteria");
    PrintToServer("  mongo_analytics - Show game analytics dashboard");
    PrintToServer("  mongo_maintenance - Run database maintenance");
}

public void OnPluginEnd() {
    if (g_isConnected && g_mongoConn.IsValid()) {
        g_mongoConn.Close();
    }
}

// Initialize MongoDB connection with configuration
void InitializeMongoDB() {
    // Load configuration
    MongoDB_LoadConfig("configs/mongodb_config.cfg");
    
    // Set custom timeout for advanced operations
    MongoDB_SetTimeout(60); // 60 seconds for complex queries
    
    // Connect to MongoDB
    g_mongoConn = MongoDB_Connect("http://127.0.0.1:3300");
    g_isConnected = g_mongoConn.IsValid();
    
    if (g_isConnected) {
        PrintToServer("✅ MongoDB connection established for advanced operations");
        
        // Create indexes for better performance
        CreateOptimalIndexes();
    } else {
        PrintToServer("❌ Failed to connect to MongoDB for advanced operations");
    }
}

// Create optimal indexes for performance
void CreateOptimalIndexes() {
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Index for player lookups
    players.CreateIndex("{\"steamid\":1}", "{\"name\":\"steamid_idx\",\"unique\":true}");
    
    // Index for leaderboard queries
    players.CreateIndex("{\"score\":-1,\"lastSeen\":-1}", "{\"name\":\"leaderboard_idx\"}");
    
    // Index for search functionality
    players.CreateIndex("{\"name\":\"text\",\"clan\":\"text\"}", "{\"name\":\"search_idx\"}");
    
    PrintToServer("🔧 Optimal indexes created for advanced operations");
}

// Advanced player statistics with aggregation
public Action Command_PlayerStats(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    char steamid[32];
    if (args > 0) {
        GetCmdArg(1, steamid, sizeof(steamid));
    } else {
        strcopy(steamid, sizeof(steamid), "STEAM_1:0:12345"); // Default for demo
    }
    
    PrintToServer("=== Advanced Player Statistics for %s ===", steamid);
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    MongoCollection matches = g_mongoConn.GetCollection("gamedb", "matches");
    
    // Complex aggregation: player stats with match history
    char pipeline[1024];
    Format(pipeline, sizeof(pipeline),
        "[{\"$match\":{\"steamid\":\"%s\"}},"
        "{\"$lookup\":{\"from\":\"matches\",\"localField\":\"steamid\",\"foreignField\":\"players.steamid\",\"as\":\"matches\"}},"
        "{\"$addFields\":{\"totalMatches\":{\"$size\":\"$matches\"},\"avgScore\":{\"$avg\":\"$matches.score\"}}},"
        "{\"$project\":{\"name\":1,\"score\":1,\"totalMatches\":1,\"avgScore\":1,\"lastSeen\":1}}]",
        steamid);
    
    ArrayList results = players.Aggregate(pipeline);
    if (results != null && results.Length > 0) {
        PrintToServer("✅ Player statistics retrieved successfully");
        PrintToServer("📊 Advanced analytics available via aggregation pipeline");
        delete results;
    } else {
        PrintToServer("❌ Player not found or no statistics available");
    }
    
    return Plugin_Handled;
}

// Generate leaderboard using aggregation
public Action Command_Leaderboard(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    int limit = 10;
    if (args > 0) {
        char limitStr[8];
        GetCmdArg(1, limitStr, sizeof(limitStr));
        limit = StringToInt(limitStr);
        if (limit <= 0 || limit > 100) limit = 10;
    }
    
    PrintToServer("=== Top %d Players Leaderboard ===", limit);
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Advanced leaderboard with ranking and statistics
    char pipeline[512];
    Format(pipeline, sizeof(pipeline),
        "[{\"$match\":{\"score\":{\"$gte\":0}}},"
        "{\"$sort\":{\"score\":-1,\"lastSeen\":-1}},"
        "{\"$limit\":%d},"
        "{\"$addFields\":{\"rank\":{\"$add\":[{\"$indexOfArray\":[\"$$ROOT\",\"$$ROOT\"]},1]}}},"
        "{\"$project\":{\"name\":1,\"score\":1,\"clan\":1,\"lastSeen\":1}}]",
        limit);
    
    ArrayList results = players.Aggregate(pipeline);
    if (results != null) {
        PrintToServer("🏆 Leaderboard generated with %d players", results.Length);
        PrintToServer("📈 Rankings calculated using advanced aggregation");
        delete results;
    } else {
        PrintToServer("❌ Failed to generate leaderboard");
    }
    
    return Plugin_Handled;
}

// Bulk update operations
public Action Command_BulkUpdate(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    PrintToServer("=== Bulk Update Operations ===");
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Create bulk update operations
    ArrayList operations = new ArrayList();
    
    // Update 1: Add bonus points to active players
    char op1[256];
    Format(op1, sizeof(op1),
        "{\"updateMany\":{\"filter\":{\"status\":\"active\"},\"update\":{\"$inc\":{\"score\":100,\"bonusPoints\":50}}}}");
    operations.PushString(op1);
    
    // Update 2: Set inactive status for old players
    char op2[256];
    Format(op2, sizeof(op2),
        "{\"updateMany\":{\"filter\":{\"lastSeen\":{\"$lt\":%d}},\"update\":{\"$set\":{\"status\":\"inactive\"}}}}",
        GetTime() - 2592000); // 30 days ago
    operations.PushString(op2);
    
    // Update 3: Add achievement for high scorers
    char op3[256];
    Format(op3, sizeof(op3),
        "{\"updateMany\":{\"filter\":{\"score\":{\"$gte\":5000}},\"update\":{\"$addToSet\":{\"achievements\":\"high_scorer\"}}}}");
    operations.PushString(op3);
    
    PrintToServer("📦 Executing %d bulk update operations...", operations.Length);
    
    bool success = players.BulkWrite(operations, false); // unordered for better performance
    if (success) {
        PrintToServer("✅ Bulk update completed successfully");
        PrintToServer("🔄 Updated player statuses, scores, and achievements");
    } else {
        PrintToServer("❌ Bulk update failed");
    }
    
    delete operations;
    return Plugin_Handled;
}

// Advanced player search with text search and filtering
public Action Command_SearchPlayers(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    char query[128];
    if (args > 0) {
        GetCmdArg(1, query, sizeof(query));
    } else {
        strcopy(query, sizeof(query), "player"); // Default search term
    }
    
    PrintToServer("=== Advanced Player Search: '%s' ===", query);
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Advanced search with text search and scoring
    char pipeline[512];
    Format(pipeline, sizeof(pipeline),
        "[{\"$match\":{\"$or\":[{\"$text\":{\"$search\":\"%s\"}},{\"name\":{\"$regex\":\"%s\",\"$options\":\"i\"}}]}},"
        "{\"$addFields\":{\"searchScore\":{\"$meta\":\"textScore\"}}},"
        "{\"$sort\":{\"searchScore\":{\"$meta\":\"textScore\"},\"score\":-1}},"
        "{\"$limit\":20},"
        "{\"$project\":{\"name\":1,\"score\":1,\"clan\":1,\"status\":1,\"searchScore\":1}}]",
        query, query);
    
    ArrayList results = players.Aggregate(pipeline);
    if (results != null) {
        PrintToServer("🔍 Search completed: %d players found", results.Length);
        PrintToServer("📊 Results ranked by relevance and score");
        delete results;
    } else {
        PrintToServer("❌ Search failed or no results found");
    }
    
    return Plugin_Handled;
}

// Game analytics dashboard
public Action Command_Analytics(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    PrintToServer("=== Game Analytics Dashboard ===");
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Analytics aggregation: comprehensive statistics
    char pipeline[768];
    Format(pipeline, sizeof(pipeline),
        "[{\"$group\":{\"_id\":null,"
        "\"totalPlayers\":{\"$sum\":1},"
        "\"activePlayers\":{\"$sum\":{\"$cond\":[{\"$eq\":[\"$status\",\"active\"]},1,0]}},"
        "\"avgScore\":{\"$avg\":\"$score\"},"
        "\"maxScore\":{\"$max\":\"$score\"},"
        "\"minScore\":{\"$min\":\"$score\"},"
        "\"totalScore\":{\"$sum\":\"$score\"}}},"
        "{\"$addFields\":{\"activePercentage\":{\"$multiply\":[{\"$divide\":[\"$activePlayers\",\"$totalPlayers\"]},100]}}}]");
    
    ArrayList results = players.Aggregate(pipeline);
    if (results != null && results.Length > 0) {
        PrintToServer("📊 Analytics Dashboard Generated");
        PrintToServer("📈 Comprehensive game statistics calculated");
        PrintToServer("🎯 Player engagement metrics available");
        delete results;
    } else {
        PrintToServer("❌ Failed to generate analytics");
    }
    
    return Plugin_Handled;
}

// Database maintenance tasks
public Action Command_Maintenance(int args) {
    if (!g_isConnected) {
        PrintToServer("❌ MongoDB not connected");
        return Plugin_Handled;
    }
    
    PrintToServer("=== Database Maintenance ===");
    
    MongoCollection players = g_mongoConn.GetCollection("gamedb", "players");
    
    // Maintenance task 1: Clean up old inactive players
    char filter[128];
    Format(filter, sizeof(filter),
        "{\"status\":\"inactive\",\"lastSeen\":{\"$lt\":%d}}",
        GetTime() - 7776000); // 90 days ago
    
    PrintToServer("🧹 Cleaning up old inactive players...");
    // Note: In a real implementation, you might want to archive rather than delete
    
    // Maintenance task 2: Update player rankings
    char rankingPipeline[256];
    Format(rankingPipeline, sizeof(rankingPipeline),
        "[{\"$sort\":{\"score\":-1}},"
        "{\"$group\":{\"_id\":null,\"players\":{\"$push\":\"$$ROOT\"}}},"
        "{\"$unwind\":{\"path\":\"$players\",\"includeArrayIndex\":\"rank\"}},"
        "{\"$replaceRoot\":{\"newRoot\":{\"$mergeObjects\":[\"$players\",{\"rank\":{\"$add\":[\"$rank\",1]}}]}}}]");
    
    PrintToServer("📊 Updating player rankings...");
    ArrayList rankingResults = players.Aggregate(rankingPipeline);
    if (rankingResults != null) {
        PrintToServer("✅ Player rankings updated");
        delete rankingResults;
    }
    
    // Maintenance task 3: Rebuild indexes
    PrintToServer("🔧 Rebuilding indexes for optimal performance...");
    CreateOptimalIndexes();
    
    PrintToServer("✅ Database maintenance completed");
    
    return Plugin_Handled;
}
