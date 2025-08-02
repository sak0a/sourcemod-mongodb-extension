/**
 * Test Enhanced StringMap Integration
 * Demonstrates the new helper natives for StringMap operations
 */

#include <sourcemod>
#include <http_mongodb>

public Plugin myinfo = {
    name = "Test Enhanced StringMap",
    author = "MongoDB Extension",
    description = "Test the enhanced StringMap helper natives",
    version = "1.0.0",
    url = ""
};

public void OnPluginStart() {
    RegConsoleCmd("sm_test_stringmap", Command_TestStringMap, "Test StringMap helper natives");
    RegConsoleCmd("sm_test_mongodb_find", Command_TestMongoDBFind, "Test MongoDB FindOne with enhanced StringMap");
}

public Action Command_TestStringMap(int client, int args) {
    PrintToServer("=== Testing Enhanced StringMap Helper Natives ===");
    
    // Test 1: Create empty StringMap
    Handle testMap = StringMap_CreateEmpty();
    PrintToServer("Created empty StringMap handle: %d", testMap);
    
    // Test 2: Set string values
    StringMap_SetString(testMap, "name", "John Doe");
    StringMap_SetString(testMap, "email", "john@example.com");
    StringMap_SetString(testMap, "age", "30");
    StringMap_SetString(testMap, "score", "95.5");
    PrintToServer("Set multiple key-value pairs");
    
    // Test 3: Get string values
    char buffer[256];
    
    if (StringMap_GetString(testMap, "name", buffer, sizeof(buffer))) {
        PrintToServer("Retrieved name: %s", buffer);
    }
    
    if (StringMap_GetString(testMap, "email", buffer, sizeof(buffer))) {
        PrintToServer("Retrieved email: %s", buffer);
    }
    
    if (StringMap_GetString(testMap, "age", buffer, sizeof(buffer))) {
        PrintToServer("Retrieved age: %s", buffer);
    }
    
    if (StringMap_GetString(testMap, "score", buffer, sizeof(buffer))) {
        PrintToServer("Retrieved score: %s", buffer);
    }
    
    // Test 4: Try to get non-existent key
    if (!StringMap_GetString(testMap, "nonexistent", buffer, sizeof(buffer))) {
        PrintToServer("Correctly failed to retrieve non-existent key");
    }
    
    // Test 5: Convert to JSON
    char jsonBuffer[1024];
    if (JSON_StringMapToString(view_as<StringMap>(testMap), jsonBuffer, sizeof(jsonBuffer))) {
        PrintToServer("StringMap as JSON: %s", jsonBuffer);
    }
    
    // Test 6: Create MongoDocument from handle
    MongoDocument doc = MongoDocument.FromHandle(testMap);
    char docJson[1024];
    if (doc.ToString(docJson, sizeof(docJson))) {
        PrintToServer("MongoDocument JSON: %s", docJson);
    }
    
    // Test 7: Use enhanced MongoDocument methods
    MongoDocument newDoc = new MongoDocument();
    newDoc.SetStringValue("username", "testuser");
    newDoc.SetStringValue("password", "secret123");
    newDoc.SetStringValue("level", "admin");
    
    char username[64];
    if (newDoc.GetStringValue("username", username, sizeof(username))) {
        PrintToServer("MongoDocument username: %s", username);
    }
    
    char newDocJson[1024];
    if (newDoc.ToString(newDocJson, sizeof(newDocJson))) {
        PrintToServer("New MongoDocument JSON: %s", newDocJson);
    }
    
    PrintToServer("=== StringMap Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "StringMap test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}

public Action Command_TestMongoDBFind(int client, int args) {
    PrintToServer("=== Testing MongoDB FindOne with Enhanced StringMap ===");
    
    // Connect to MongoDB (using configured URL)
    MongoConnection conn = MongoConnection.FromConfig();
    if (!conn.IsConnected()) {
        PrintToServer("Failed to connect to MongoDB");
        if (client > 0) {
            ReplyToCommand(client, "Failed to connect to MongoDB");
        }
        return Plugin_Handled;
    }
    
    PrintToServer("Connected to MongoDB");
    
    // Get collection
    MongoCollection collection = conn.GetCollection("testdb", "users");
    
    // Create a filter using enhanced StringMap
    MongoDocument filter = new MongoDocument();
    filter.SetStringValue("username", "testuser");
    
    PrintToServer("Created filter for username: testuser");
    
    // Find document
    StringMap result = collection.FindOne(filter);
    
    if (result != null) {
        PrintToServer("Found document!");
        
        // Use enhanced StringMap helpers to extract data
        char username[64], email[128], userId[32];
        
        if (StringMap_GetString(result, "username", username, sizeof(username))) {
            PrintToServer("Found username: %s", username);
        }
        
        if (StringMap_GetString(result, "email", email, sizeof(email))) {
            PrintToServer("Found email: %s", email);
        }
        
        if (StringMap_GetString(result, "_id", userId, sizeof(userId))) {
            PrintToServer("Found user ID: %s", userId);
        }
        
        // Convert result to MongoDocument for easier manipulation
        MongoDocument resultDoc = MongoDocument.FromHandle(result);
        char fullJson[1024];
        if (resultDoc.ToString(fullJson, sizeof(fullJson))) {
            PrintToServer("Full document JSON: %s", fullJson);
        }
        
        // Test enhanced MongoDocument methods
        char testValue[64];
        if (resultDoc.GetStringValue("username", testValue, sizeof(testValue))) {
            PrintToServer("Enhanced method retrieved username: %s", testValue);
        }
        
    } else {
        PrintToServer("No document found with username: testuser");
    }
    
    // Clean up
    delete filter;
    conn.Close();
    
    PrintToServer("=== MongoDB FindOne Test Complete ===");
    
    if (client > 0) {
        ReplyToCommand(client, "MongoDB FindOne test completed. Check server console for results.");
    }
    
    return Plugin_Handled;
}

public void OnPluginEnd() {
    PrintToServer("Enhanced StringMap test plugin unloaded");
}
