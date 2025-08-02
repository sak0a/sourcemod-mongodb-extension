/**
 * Test Enhanced MongoQuery Implementation
 * Demonstrates the logical operations and complex query building
 */

#include <sourcemod>
#include <http_mongodb>

public Plugin myinfo = {
    name = "Test Enhanced MongoQuery",
    author = "MongoDB Extension",
    description = "Test the enhanced MongoQuery logical operations",
    version = "1.0.0",
    url = ""
};

public void OnPluginStart() {
    RegConsoleCmd("sm_test_query_basic", Command_TestBasicQueries, "Test basic query operations");
    RegConsoleCmd("sm_test_query_logical", Command_TestLogicalOperations, "Test logical query operations");
    RegConsoleCmd("sm_test_query_advanced", Command_TestAdvancedQueries, "Test advanced query features");
    RegConsoleCmd("sm_test_query_real", Command_TestRealQueries, "Test queries with real MongoDB");
}

public Action Command_TestBasicQueries(int client, int args) {
    PrintToServer("=== Testing Basic MongoQuery Operations ===");
    
    // Test 1: Simple equality
    MongoQuery query1 = new MongoQuery();
    query1.EqualsString("username", "john_doe");
    
    char json1[512];
    query1.ToString(json1, sizeof(json1));
    PrintToServer("Simple equality query: %s", json1);
    
    // Test 2: Comparison operations
    MongoQuery query2 = new MongoQuery();
    query2.GreaterThan("score", 1000);
    
    char json2[512];
    query2.ToString(json2, sizeof(json2));
    PrintToServer("Greater than query: %s", json2);
    
    // Test 3: String comparison
    MongoQuery query3 = new MongoQuery();
    query3.GreaterThanOrEqualString("rank", "Gold");
    
    char json3[512];
    query3.ToString(json3, sizeof(json3));
    PrintToServer("String comparison query: %s", json3);
    
    // Test 4: Not equals
    MongoQuery query4 = new MongoQuery();
    query4.NotEqualsString("status", "banned");
    
    char json4[512];
    query4.ToString(json4, sizeof(json4));
    PrintToServer("Not equals query: %s", json4);
    
    // Test 5: Exists check
    MongoQuery query5 = new MongoQuery();
    query5.Exists("email", true);
    
    char json5[512];
    query5.ToString(json5, sizeof(json5));
    PrintToServer("Exists query: %s", json5);
    
    // Cleanup
    delete query1;
    delete query2;
    delete query3;
    delete query4;
    delete query5;
    
    PrintToServer("=== Basic Query Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "Basic query test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}

public Action Command_TestLogicalOperations(int client, int args) {
    PrintToServer("=== Testing Logical MongoQuery Operations ===");
    
    // Test 1: AND operation
    MongoQuery query1 = new MongoQuery();
    query1.EqualsString("status", "active");
    
    MongoQuery query2 = new MongoQuery();
    query2.GreaterThan("score", 500);
    
    MongoQuery andQuery = query1.And(query2);
    char andJson[1024];
    andQuery.ToString(andJson, sizeof(andJson));
    PrintToServer("AND query: %s", andJson);
    
    // Test 2: OR operation
    MongoQuery query3 = new MongoQuery();
    query3.EqualsString("rank", "Admin");
    
    MongoQuery query4 = new MongoQuery();
    query4.EqualsString("rank", "Moderator");
    
    MongoQuery orQuery = query3.Or(query4);
    char orJson[1024];
    orQuery.ToString(orJson, sizeof(orJson));
    PrintToServer("OR query: %s", orJson);
    
    // Test 3: NOT operation
    MongoQuery query5 = new MongoQuery();
    query5.EqualsString("status", "banned");
    
    MongoQuery notQuery = query5.Not(query5);
    char notJson[1024];
    notQuery.ToString(notJson, sizeof(notJson));
    PrintToServer("NOT query: %s", notJson);
    
    // Test 4: NOR operation
    MongoQuery query6 = new MongoQuery();
    query6.EqualsString("status", "banned");
    
    MongoQuery query7 = new MongoQuery();
    query7.EqualsString("status", "suspended");
    
    MongoQuery norQuery = query6.Nor(query7);
    char norJson[1024];
    norQuery.ToString(norJson, sizeof(norJson));
    PrintToServer("NOR query: %s", norJson);
    
    // Test 5: Complex nested query
    MongoQuery activeQuery = new MongoQuery();
    activeQuery.EqualsString("status", "active");
    
    MongoQuery highScoreQuery = new MongoQuery();
    highScoreQuery.GreaterThan("score", 1000);
    
    MongoQuery adminQuery = new MongoQuery();
    adminQuery.EqualsString("role", "admin");
    
    // (active AND high_score) OR admin
    MongoQuery activeAndHighScore = activeQuery.And(highScoreQuery);
    MongoQuery complexQuery = activeAndHighScore.Or(adminQuery);
    
    char complexJson[2048];
    complexQuery.ToString(complexJson, sizeof(complexJson));
    PrintToServer("Complex query: %s", complexJson);
    
    // Cleanup
    delete query1; delete query2; delete andQuery;
    delete query3; delete query4; delete orQuery;
    delete query5; delete notQuery;
    delete query6; delete query7; delete norQuery;
    delete activeQuery; delete highScoreQuery; delete adminQuery;
    delete activeAndHighScore; delete complexQuery;
    
    PrintToServer("=== Logical Operations Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "Logical operations test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}

public Action Command_TestAdvancedQueries(int client, int args) {
    PrintToServer("=== Testing Advanced MongoQuery Features ===");
    
    // Test 1: Regular expression
    MongoQuery regexQuery = new MongoQuery();
    regexQuery.Regex("username", "^admin_", "i");  // Case-insensitive, starts with "admin_"
    
    char regexJson[512];
    regexQuery.ToString(regexJson, sizeof(regexJson));
    PrintToServer("Regex query: %s", regexJson);
    
    // Test 2: Type checking
    MongoQuery typeQuery = new MongoQuery();
    typeQuery.Type("score", 16);  // BSON type 16 = 32-bit integer
    
    char typeJson[512];
    typeQuery.ToString(typeJson, sizeof(typeJson));
    PrintToServer("Type query: %s", typeJson);
    
    // Test 3: Text search
    MongoQuery textQuery = new MongoQuery();
    textQuery.TextSearch("game server admin");
    
    char textJson[512];
    textQuery.ToString(textJson, sizeof(textJson));
    PrintToServer("Text search query: %s", textJson);
    
    // Test 4: Range query (between values)
    MongoQuery rangeQuery = new MongoQuery();
    rangeQuery.GreaterThanOrEqual("score", 100);
    
    MongoQuery upperBound = new MongoQuery();
    upperBound.LessThanOrEqual("score", 1000);
    
    MongoQuery betweenQuery = rangeQuery.And(upperBound);
    char betweenJson[1024];
    betweenQuery.ToString(betweenJson, sizeof(betweenJson));
    PrintToServer("Range query (100 <= score <= 1000): %s", betweenJson);
    
    // Cleanup
    delete regexQuery;
    delete typeQuery;
    delete textQuery;
    delete rangeQuery;
    delete upperBound;
    delete betweenQuery;
    
    PrintToServer("=== Advanced Features Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "Advanced features test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}

public Action Command_TestRealQueries(int client, int args) {
    PrintToServer("=== Testing Real MongoDB Queries ===");
    
    // Connect to MongoDB
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) {
            ReplyToCommand(client, "Failed to connect to MongoDB");
        }
        return Plugin_Handled;
    }
    
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Test 1: Find active players with high scores
    MongoQuery activeQuery = new MongoQuery();
    activeQuery.EqualsString("status", "active");
    
    MongoQuery highScoreQuery = new MongoQuery();
    highScoreQuery.GreaterThan("score", 500);
    
    MongoQuery combinedQuery = activeQuery.And(highScoreQuery);
    
    char queryJson[1024];
    combinedQuery.ToString(queryJson, sizeof(queryJson));
    PrintToServer("Searching for active players with score > 500: %s", queryJson);
    
    ArrayList results = players.Find(combinedQuery, null);
    if (results != null) {
        PrintToServer("Found %d matching players", results.Length);
        delete results;
    } else {
        PrintToServer("No matching players found");
    }
    
    // Test 2: Find admins OR moderators
    MongoQuery adminQuery = new MongoQuery();
    adminQuery.EqualsString("role", "admin");
    
    MongoQuery modQuery = new MongoQuery();
    modQuery.EqualsString("role", "moderator");
    
    MongoQuery staffQuery = adminQuery.Or(modQuery);
    
    char staffJson[1024];
    staffQuery.ToString(staffJson, sizeof(staffJson));
    PrintToServer("Searching for staff members: %s", staffJson);
    
    ArrayList staffResults = players.Find(staffQuery, null);
    if (staffResults != null) {
        PrintToServer("Found %d staff members", staffResults.Length);
        delete staffResults;
    } else {
        PrintToServer("No staff members found");
    }
    
    // Cleanup
    delete activeQuery; delete highScoreQuery; delete combinedQuery;
    delete adminQuery; delete modQuery; delete staffQuery;
    conn.Close();
    
    PrintToServer("=== Real MongoDB Queries Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "Real MongoDB queries test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}
