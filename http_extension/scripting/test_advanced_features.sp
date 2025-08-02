/**
 * Test Advanced MongoDB Features
 * Demonstrates aggregation, bulk operations, error handling, and performance monitoring
 */

#include <sourcemod>
#include <http_mongodb>

public Plugin myinfo = {
    name = "Test Advanced MongoDB Features",
    author = "MongoDB Extension",
    description = "Test advanced MongoDB features including aggregation and bulk operations",
    version = "1.0.0",
    url = ""
};

public void OnPluginStart() {
    RegConsoleCmd("sm_test_aggregation", Command_TestAggregation, "Test MongoDB aggregation pipeline");
    RegConsoleCmd("sm_test_bulk_ops", Command_TestBulkOperations, "Test MongoDB bulk operations");
    RegConsoleCmd("sm_test_performance", Command_TestPerformance, "Test performance monitoring");
    RegConsoleCmd("sm_test_error_handling", Command_TestErrorHandling, "Test enhanced error handling");
    RegConsoleCmd("sm_test_projection", Command_TestProjection, "Test find with projection");
    RegConsoleCmd("sm_test_distinct", Command_TestDistinct, "Test find distinct values");
    RegConsoleCmd("sm_mongo_status", Command_MongoStatus, "Show MongoDB connection and performance status");
}

public Action Command_TestAggregation(int client, int args) {
    PrintToServer("=== Testing MongoDB Aggregation Pipeline ===");
    
    // Connect to MongoDB
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Build aggregation pipeline
    MongoAggregation pipeline = new MongoAggregation();
    
    // Stage 1: Match active players
    MongoQuery activeFilter = new MongoQuery();
    activeFilter.EqualsString("status", "active");
    pipeline.Match(activeFilter);
    
    // Stage 2: Group by role and count
    MongoDocument groupSpec = new MongoDocument();
    groupSpec.SetString("count", "{\"$sum\":1}");
    groupSpec.SetString("avgScore", "{\"$avg\":\"$score\"}");
    pipeline.Group("$role", groupSpec);
    
    // Stage 3: Sort by count descending
    MongoDocument sortSpec = new MongoDocument();
    sortSpec.SetValue("count", -1);
    pipeline.Sort(sortSpec);
    
    // Stage 4: Limit to top 5
    pipeline.Limit(5);
    
    PrintToServer("Running aggregation pipeline with %d stages", pipeline.Length);
    
    // Execute aggregation
    ArrayList results = players.Aggregate(pipeline);
    
    if (results != null) {
        PrintToServer("Aggregation completed successfully with %d results", results.Length);
        delete results;
    } else {
        PrintToServer("Aggregation failed");
        
        // Check for errors
        if (MongoError.HasError()) {
            char errorMsg[512];
            MongoError.GetFormatted(errorMsg, sizeof(errorMsg));
            PrintToServer("Error: %s", errorMsg);
        }
    }
    
    // Cleanup
    delete activeFilter;
    delete groupSpec;
    delete sortSpec;
    delete pipeline;
    conn.Close();
    
    PrintToServer("=== Aggregation Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Aggregation test completed. Check server console.");
    
    return Plugin_Handled;
}

public Action Command_TestBulkOperations(int client, int args) {
    PrintToServer("=== Testing MongoDB Bulk Operations ===");
    
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Build bulk operations
    MongoBulkOps bulkOps = new MongoBulkOps();
    
    // Insert new players
    MongoDocument newPlayer1 = new MongoDocument();
    newPlayer1.SetStringValue("name", "BulkPlayer1");
    newPlayer1.SetStringValue("status", "active");
    newPlayer1.SetStringValue("role", "player");
    bulkOps.InsertOne(newPlayer1);
    
    MongoDocument newPlayer2 = new MongoDocument();
    newPlayer2.SetStringValue("name", "BulkPlayer2");
    newPlayer2.SetStringValue("status", "active");
    newPlayer2.SetStringValue("role", "moderator");
    bulkOps.InsertOne(newPlayer2);
    
    // Update existing players
    MongoQuery updateFilter = new MongoQuery();
    updateFilter.EqualsString("status", "inactive");
    
    MongoDocument updateDoc = new MongoDocument();
    updateDoc.SetString("$set", "{\"status\":\"archived\"}");
    bulkOps.UpdateMany(updateFilter, updateDoc);
    
    // Delete test players
    MongoQuery deleteFilter = new MongoQuery();
    deleteFilter.Regex("name", "^TestPlayer");
    bulkOps.DeleteMany(deleteFilter);
    
    PrintToServer("Executing bulk operations with %d operations", bulkOps.Length);
    
    // Execute bulk write
    bool success = players.BulkWrite(bulkOps, true); // ordered = true
    
    if (success) {
        PrintToServer("Bulk operations completed successfully");
    } else {
        PrintToServer("Bulk operations failed");
        
        if (MongoError.HasError()) {
            char errorMsg[512];
            MongoError.GetFormatted(errorMsg, sizeof(errorMsg));
            PrintToServer("Error: %s", errorMsg);
        }
    }
    
    // Cleanup
    delete newPlayer1;
    delete newPlayer2;
    delete updateFilter;
    delete updateDoc;
    delete deleteFilter;
    delete bulkOps;
    conn.Close();
    
    PrintToServer("=== Bulk Operations Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Bulk operations test completed. Check server console.");
    
    return Plugin_Handled;
}

public Action Command_TestPerformance(int client, int args) {
    PrintToServer("=== Testing Performance Monitoring ===");
    
    // Reset metrics for clean test
    MongoPerformance.Reset();
    
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Perform several operations to generate metrics
    for (int i = 0; i < 5; i++) {
        MongoQuery query = new MongoQuery();
        query.EqualsString("status", "active");
        
        ArrayList results = players.Find(query, null);
        if (results != null) {
            delete results;
        }
        
        delete query;
    }
    
    // Test connection health
    bool connectionHealthy = MongoDB_TestConnection(conn);
    PrintToServer("Connection health check: %s", connectionHealthy ? "HEALTHY" : "UNHEALTHY");
    
    // Get performance summary
    char perfSummary[512];
    MongoPerformance.GetSummary(perfSummary, sizeof(perfSummary));
    PrintToServer("Performance Summary: %s", perfSummary);
    
    // Get detailed metrics
    PrintToServer("Detailed Metrics:");
    PrintToServer("  Total Operations: %d", MongoPerformance.GetTotalOperations());
    PrintToServer("  Successful: %d", MongoPerformance.GetSuccessfulOperations());
    PrintToServer("  Failed: %d", MongoPerformance.GetFailedOperations());
    PrintToServer("  Success Rate: %d%%", MongoPerformance.GetSuccessRate());
    PrintToServer("  Average Time: %d.%02d seconds", 
                 MongoPerformance.GetAverageExecutionTime() / 100,
                 MongoPerformance.GetAverageExecutionTime() % 100);
    
    conn.Close();
    
    PrintToServer("=== Performance Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Performance test completed. Check server console.");
    
    return Plugin_Handled;
}

public Action Command_TestErrorHandling(int client, int args) {
    PrintToServer("=== Testing Enhanced Error Handling ===");
    
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }
    
    // Try to access an invalid collection to trigger an error
    MongoCollection invalidCollection = conn.GetCollection("nonexistent_db", "nonexistent_collection");
    
    MongoQuery query = new MongoQuery();
    query.EqualsString("test", "value");
    
    ArrayList results = invalidCollection.Find(query, null);
    
    if (results == null) {
        PrintToServer("Operation failed as expected");
        
        // Check error details
        if (MongoError.HasError()) {
            PrintToServer("Error Code: %d", MongoError.GetCode());
            
            char message[256];
            MongoError.GetMessage(message, sizeof(message));
            PrintToServer("Error Message: %s", message);
            
            char details[512];
            MongoError.GetDetails(details, sizeof(details));
            PrintToServer("Error Details: %s", details);
            
            PrintToServer("Error Timestamp: %d", MongoError.GetTimestamp());
            
            char formatted[512];
            MongoError.GetFormatted(formatted, sizeof(formatted));
            PrintToServer("Formatted Error: %s", formatted);
        } else {
            PrintToServer("No error information available");
        }
    } else {
        PrintToServer("Operation unexpectedly succeeded");
        delete results;
    }
    
    delete query;
    conn.Close();
    
    PrintToServer("=== Error Handling Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Error handling test completed. Check server console.");

    return Plugin_Handled;
}

public Action Command_TestProjection(int client, int args) {
    PrintToServer("=== Testing Find with Projection ===");

    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Create filter
    MongoQuery filter = new MongoQuery();
    filter.EqualsString("status", "active");

    // Create projection (only return specific fields)
    MongoDocument projection = new MongoDocument();
    projection.SetValue("name", 1);        // Include name
    projection.SetValue("score", 1);       // Include score
    projection.SetValue("_id", 0);         // Exclude _id

    // Create options
    MongoDocument options = new MongoDocument();
    options.SetValue("limit", 10);

    PrintToServer("Finding active players with projection (name, score only)");

    ArrayList results = players.FindWithProjection(filter, projection, options);

    if (results != null) {
        PrintToServer("Found %d players with projection", results.Length);
        delete results;
    } else {
        PrintToServer("Projection query failed");

        if (MongoError.HasError()) {
            char errorMsg[512];
            MongoError.GetFormatted(errorMsg, sizeof(errorMsg));
            PrintToServer("Error: %s", errorMsg);
        }
    }

    delete filter;
    delete projection;
    delete options;
    conn.Close();

    PrintToServer("=== Projection Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Projection test completed. Check server console.");

    return Plugin_Handled;
}

public Action Command_TestDistinct(int client, int args) {
    PrintToServer("=== Testing Find Distinct Values ===");

    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) ReplyToCommand(client, "Failed to connect to MongoDB");
        return Plugin_Handled;
    }

    MongoCollection players = conn.GetCollection("gamedb", "players");

    // Find distinct roles
    PrintToServer("Finding distinct player roles");

    ArrayList distinctRoles = players.FindDistinct("role", null);

    if (distinctRoles != null) {
        PrintToServer("Found %d distinct roles", distinctRoles.Length);
        delete distinctRoles;
    } else {
        PrintToServer("Distinct query failed");

        if (MongoError.HasError()) {
            char errorMsg[512];
            MongoError.GetFormatted(errorMsg, sizeof(errorMsg));
            PrintToServer("Error: %s", errorMsg);
        }
    }

    // Find distinct statuses for active players
    MongoQuery activeFilter = new MongoQuery();
    activeFilter.EqualsString("status", "active");

    PrintToServer("Finding distinct statuses for active players");

    ArrayList distinctStatuses = players.FindDistinct("status", activeFilter);

    if (distinctStatuses != null) {
        PrintToServer("Found %d distinct statuses", distinctStatuses.Length);
        delete distinctStatuses;
    } else {
        PrintToServer("Filtered distinct query failed");
    }

    delete activeFilter;
    conn.Close();

    PrintToServer("=== Distinct Test Complete ===");
    if (client > 0) ReplyToCommand(client, "Distinct test completed. Check server console.");

    return Plugin_Handled;
}

public Action Command_MongoStatus(int client, int args) {
    PrintToServer("=== MongoDB Status Report ===");

    // Performance metrics
    char perfSummary[512];
    MongoPerformance.GetSummary(perfSummary, sizeof(perfSummary));
    PrintToServer("Performance: %s", perfSummary);

    // Error status
    if (MongoError.HasError()) {
        char errorMsg[512];
        MongoError.GetFormatted(errorMsg, sizeof(errorMsg));
        PrintToServer("Last Error: %s", errorMsg);
    } else {
        PrintToServer("No recent errors");
    }

    // Test connection
    MongoConnection conn = MongoConnection.FromConfig();
    if (conn.IsConnected()) {
        bool healthy = MongoDB_TestConnection(conn);
        PrintToServer("Connection Status: %s", healthy ? "HEALTHY" : "UNHEALTHY");
        conn.Close();
    } else {
        PrintToServer("Connection Status: FAILED TO CONNECT");
    }

    PrintToServer("=== Status Report Complete ===");

    if (client > 0) {
        ReplyToCommand(client, "MongoDB status report completed. Check server console for details.");
    }

    return Plugin_Handled;
}

public void OnPluginEnd() {
    PrintToServer("Advanced MongoDB features test plugin unloaded");

    // Show final performance summary
    char perfSummary[512];
    MongoPerformance.GetSummary(perfSummary, sizeof(perfSummary));
    PrintToServer("Final Performance Summary: %s", perfSummary);
}
