/**
 * HTTP MongoDB Extension Include File
 *
 * This extension provides MongoDB database operations through an HTTP API layer,
 * designed specifically for SourceMod plugins. It offers a comprehensive interface
 * for database operations including CRUD operations, aggregation, indexing, and
 * advanced query capabilities.
 *
 * Architecture:
 * - SourceMod Plugin -> HTTP MongoDB Extension -> HTTP API Service -> MongoDB
 * - Supports both direct native calls and object-oriented methodmap interfaces
 * - Includes performance monitoring, error handling, and configuration management
 *
 * @author SourceMod Team
 * @version 1.0.0
 * @url https://github.com/sourcemod/mongodb-extension
 */

#if defined _http_mongodb_included
 #endinput
#endif
#define _http_mongodb_included

//=============================================================================
// CONFIGURATION MANAGEMENT NATIVES
//=============================================================================

/**
 * Loads MongoDB configuration from a KeyValues file.
 *
 * The configuration file should contain sections for:
 * - api: API service settings (url, api_key, timeout, retries)
 * - database: Default database and collection names
 * - connections: Connection pool settings
 * - development: Debug and development options
 *
 * @param configPath    Path to the configuration file (relative to sourcemod directory)
 * @return              True if configuration was loaded successfully, false otherwise
 *
 * @example
 * char configPath[PLATFORM_MAX_PATH];
 * BuildPath(Path_SM, configPath, sizeof(configPath), "configs/mongodb.cfg");
 * if (MongoDB_LoadConfig(configPath)) {
 *     LogMessage("MongoDB configuration loaded successfully");
 * }
 */
native bool MongoDB_LoadConfig(const char[] configPath);

/**
 * Sets the MongoDB HTTP API service URL at runtime.
 *
 * This overrides the URL specified in the configuration file and affects
 * all subsequent connection attempts.
 *
 * @param url           The HTTP API service URL (e.g., "http://127.0.0.1:3300")
 * @return              True if URL was set successfully, false otherwise
 *
 * @example
 * MongoDB_SetAPIURL("http://192.168.1.100:3300");
 */
native bool MongoDB_SetAPIURL(const char[] url);

/**
 * Retrieves the current MongoDB HTTP API service URL.
 *
 * @param buffer        Buffer to store the URL string
 * @param maxlen        Maximum length of the buffer
 * @return              True if URL was retrieved successfully, false otherwise
 *
 * @example
 * char apiUrl[256];
 * if (MongoDB_GetAPIURL(apiUrl, sizeof(apiUrl))) {
 *     LogMessage("Current API URL: %s", apiUrl);
 * }
 */
native bool MongoDB_GetAPIURL(char[] buffer, int maxlen);

/**
 * Sets the request timeout for MongoDB operations.
 *
 * @param timeout       Timeout in seconds (1-300, recommended: 30-60)
 * @return              True if timeout was set successfully, false if invalid
 *
 * @example
 * MongoDB_SetTimeout(60); // Set 60 second timeout for complex operations
 */
native bool MongoDB_SetTimeout(int timeout);

/**
 * Gets the current request timeout for MongoDB operations.
 *
 * @return              Current timeout in seconds
 *
 * @example
 * int currentTimeout = MongoDB_GetTimeout();
 * LogMessage("Current timeout: %d seconds", currentTimeout);
 */
native int MongoDB_GetTimeout();

//=============================================================================
// CONNECTION MANAGEMENT NATIVES
//=============================================================================

/**
 * Creates a connection to the MongoDB HTTP API service.
 *
 * @param url           HTTP API service URL. If empty string, uses configured URL
 * @return              Handle to the connection, or INVALID_HANDLE on failure
 *
 * @note The connection handle must be closed with MongoDB_Close() when done
 * @note Use MongoDB_IsConnected() to verify connection status
 *
 * @example
 * // Connect using configured URL
 * Handle conn = MongoDB_Connect("");
 *
 * // Connect using specific URL
 * Handle conn = MongoDB_Connect("http://192.168.1.100:3300");
 *
 * if (conn != INVALID_HANDLE && MongoDB_IsConnected(conn)) {
 *     // Connection successful
 * }
 */
native Handle MongoDB_Connect(const char[] url = "");

/**
 * Gets a collection handle from an existing connection.
 *
 * @param connection    Valid connection handle from MongoDB_Connect()
 * @param database      Name of the database
 * @param collection    Name of the collection within the database
 * @return              Handle to the collection, or INVALID_HANDLE on failure
 *
 * @note Collection handles are automatically managed and don't need explicit closing
 * @note The parent connection must remain valid while using collection handles
 *
 * @example
 * Handle conn = MongoDB_Connect("");
 * Handle players = MongoDB_GetCollection(conn, "gamedb", "players");
 * Handle servers = MongoDB_GetCollection(conn, "gamedb", "servers");
 */
native Handle MongoDB_GetCollection(Handle connection, const char[] database, const char[] collection);

/**
 * Checks if a connection is still active and valid.
 *
 * @param connection    Connection handle to test
 * @return              True if connection is active, false otherwise
 *
 * @example
 * if (!MongoDB_IsConnected(conn)) {
 *     LogError("MongoDB connection lost, attempting to reconnect...");
 *     MongoDB_Close(conn);
 *     conn = MongoDB_Connect("");
 * }
 */
native bool MongoDB_IsConnected(Handle connection);

/**
 * Closes a MongoDB connection and releases associated resources.
 *
 * @param connection    Connection handle to close
 *
 * @note Always call this when done with a connection to prevent resource leaks
 * @note All collection handles from this connection become invalid after closing
 *
 * @example
 * Handle conn = MongoDB_Connect("");
 * // ... perform operations ...
 * MongoDB_Close(conn); // Always close when done
 */
native void MongoDB_Close(Handle connection);

//=============================================================================
// DOCUMENT OPERATIONS NATIVES
//=============================================================================

/**
 * Inserts a single document into a collection using a StringMap.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param document      StringMap containing the document data
 * @param insertedId    Buffer to store the inserted document's ObjectId
 * @param maxlen        Maximum length of the insertedId buffer
 * @return              True if insertion was successful, false otherwise
 *
 * @example
 * StringMap doc = new StringMap();
 * doc.SetString("name", "John Doe");
 * doc.SetValue("score", 1500);
 * doc.SetValue("timestamp", GetTime());
 *
 * char insertedId[64];
 * if (MongoDB_InsertOne(players, doc, insertedId, sizeof(insertedId))) {
 *     LogMessage("Document inserted with ID: %s", insertedId);
 * }
 * delete doc;
 */
native bool MongoDB_InsertOne(Handle collection, StringMap document, char[] insertedId, int maxlen);

/**
 * Inserts a single document into a collection using a JSON string.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param jsonDocument  JSON string representing the document
 * @param insertedId    Buffer to store the inserted document's ObjectId
 * @param maxlen        Maximum length of the insertedId buffer
 * @return              True if insertion was successful, false otherwise
 *
 * @example
 * char jsonDoc[512];
 * Format(jsonDoc, sizeof(jsonDoc),
 *     "{\"name\":\"%s\",\"score\":%d,\"timestamp\":%d}",
 *     playerName, score, GetTime());
 *
 * char insertedId[64];
 * if (MongoDB_InsertOneJSON(players, jsonDoc, insertedId, sizeof(insertedId))) {
 *     LogMessage("Document inserted with ID: %s", insertedId);
 * }
 */
native bool MongoDB_InsertOneJSON(Handle collection, const char[] jsonDocument, char[] insertedId, int maxlen);

/**
 * Inserts multiple documents into a collection in a single operation.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param documents     ArrayList containing StringMap documents to insert
 * @param insertedIds   ArrayList to store the inserted document ObjectIds (optional, can be null)
 * @return              True if all insertions were successful, false otherwise
 *
 * @note This is more efficient than multiple InsertOne calls for bulk operations
 * @note If insertedIds is provided, it will be populated with the ObjectIds in order
 *
 * @example
 * ArrayList docs = new ArrayList();
 * for (int i = 0; i < 10; i++) {
 *     StringMap doc = new StringMap();
 *     doc.SetString("name", "Player" + i);
 *     doc.SetValue("score", GetRandomInt(0, 1000));
 *     docs.Push(doc);
 * }
 *
 * ArrayList insertedIds = new ArrayList(ByteCountToCells(64));
 * if (MongoDB_InsertMany(players, docs, insertedIds)) {
 *     LogMessage("Inserted %d documents", docs.Length);
 * }
 */
native bool MongoDB_InsertMany(Handle collection, ArrayList documents, ArrayList insertedIds);

/**
 * Finds and returns the first document matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria (null for no filter)
 * @return              StringMap containing the found document, or null if not found
 *
 * @note The returned StringMap must be deleted when no longer needed
 * @note Returns null if no document matches the filter
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetString("name", "John Doe");
 *
 * StringMap result = MongoDB_FindOne(players, filter);
 * if (result != null) {
 *     char name[64];
 *     int score;
 *     result.GetString("name", name, sizeof(name));
 *     result.GetValue("score", score);
 *     LogMessage("Found player: %s with score %d", name, score);
 *     delete result;
 * }
 * delete filter;
 */
native StringMap MongoDB_FindOne(Handle collection, StringMap filter);

/**
 * Finds and returns the first document matching the JSON filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param jsonFilter    JSON string containing the search criteria
 * @return              StringMap containing the found document, or null if not found
 *
 * @note The returned StringMap must be deleted when no longer needed
 * @note Use "{}" for an empty filter to find any document
 *
 * @example
 * char filter[256];
 * Format(filter, sizeof(filter), "{\"score\":{\"$gte\":%d}}", minScore);
 *
 * StringMap result = MongoDB_FindOneJSON(players, filter);
 * if (result != null) {
 *     // Process the found document
 *     delete result;
 * }
 */
native StringMap MongoDB_FindOneJSON(Handle collection, const char[] jsonFilter);

/**
 * Finds multiple documents matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria (null for no filter)
 * @param options       StringMap containing query options like limit, skip, sort (optional)
 * @return              ArrayList containing StringMap documents, or null if error
 *
 * @note The returned ArrayList and all contained StringMaps must be deleted
 * @note Options can include: limit, skip, sort fields
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetValue("score", 1000); // Find players with score >= 1000
 *
 * StringMap options = new StringMap();
 * options.SetValue("limit", 10);   // Limit to 10 results
 * options.SetValue("skip", 0);     // Skip 0 documents
 *
 * ArrayList results = MongoDB_Find(players, filter, options);
 * if (results != null) {
 *     for (int i = 0; i < results.Length; i++) {
 *         StringMap doc = results.Get(i);
 *         // Process document
 *         delete doc;
 *     }
 *     delete results;
 * }
 */
native ArrayList MongoDB_Find(Handle collection, StringMap filter, StringMap options);

/**
 * Updates the first document matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria
 * @param update        StringMap containing the update operations
 * @return              True if update was successful, false otherwise
 *
 * @note Use MongoDB update operators like $set, $inc, $push in the update document
 * @note Only updates the first matching document
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetString("name", "John Doe");
 *
 * StringMap update = new StringMap();
 * update.SetValue("score", 2000);        // $set operation
 * update.SetValue("lastSeen", GetTime()); // $set operation
 *
 * if (MongoDB_UpdateOne(players, filter, update)) {
 *     LogMessage("Player updated successfully");
 * }
 * delete filter;
 * delete update;
 */
native bool MongoDB_UpdateOne(Handle collection, StringMap filter, StringMap update);

/**
 * Updates all documents matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria
 * @param update        StringMap containing the update operations
 * @return              True if update was successful, false otherwise
 *
 * @note Use MongoDB update operators like $set, $inc, $push in the update document
 * @note Updates all matching documents, not just the first one
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetValue("active", 1); // Find all active players
 *
 * StringMap update = new StringMap();
 * update.SetValue("lastUpdate", GetTime()); // Update timestamp for all
 *
 * if (MongoDB_UpdateMany(players, filter, update)) {
 *     LogMessage("All active players updated");
 * }
 */
native bool MongoDB_UpdateMany(Handle collection, StringMap filter, StringMap update);

/**
 * Deletes the first document matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria
 * @return              True if deletion was successful, false otherwise
 *
 * @note Only deletes the first matching document
 * @note Returns true even if no documents were found to delete
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetString("name", "John Doe");
 *
 * if (MongoDB_DeleteOne(players, filter)) {
 *     LogMessage("Player deleted successfully");
 * }
 * delete filter;
 */
native bool MongoDB_DeleteOne(Handle collection, StringMap filter);

/**
 * Deletes all documents matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria
 * @return              True if deletion was successful, false otherwise
 *
 * @note Deletes all matching documents, use with caution
 * @note Returns true even if no documents were found to delete
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetValue("score", 0); // Delete all players with 0 score
 *
 * if (MongoDB_DeleteMany(players, filter)) {
 *     LogMessage("Inactive players deleted");
 * }
 * delete filter;
 */
native bool MongoDB_DeleteMany(Handle collection, StringMap filter);

/**
 * Counts the number of documents matching the filter criteria.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing the search criteria (null for total count)
 * @return              Number of matching documents, or -1 on error
 *
 * @example
 * // Count all documents
 * int totalPlayers = MongoDB_CountDocuments(players, null);
 *
 * // Count documents matching criteria
 * StringMap filter = new StringMap();
 * filter.SetValue("active", 1);
 * int activePlayers = MongoDB_CountDocuments(players, filter);
 * delete filter;
 *
 * LogMessage("Total: %d, Active: %d", totalPlayers, activePlayers);
 */
native int MongoDB_CountDocuments(Handle collection, StringMap filter);

//=============================================================================
// INDEX OPERATIONS NATIVES
//=============================================================================

/**
 * Creates an index on the specified fields to improve query performance.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param keys          StringMap containing field names and sort order (1 for ascending, -1 for descending)
 * @param options       StringMap containing index options like unique, sparse, etc. (optional)
 * @return              True if index creation was successful, false otherwise
 *
 * @note Indexes significantly improve query performance for large collections
 * @note Common options: unique (boolean), sparse (boolean), name (string)
 *
 * @example
 * // Create single field index
 * StringMap keys = new StringMap();
 * keys.SetValue("name", 1); // Ascending index on name field
 *
 * StringMap options = new StringMap();
 * options.SetValue("unique", 1); // Ensure unique names
 *
 * if (MongoDB_CreateIndex(players, keys, options)) {
 *     LogMessage("Index created successfully");
 * }
 * delete keys;
 * delete options;
 */
native bool MongoDB_CreateIndex(Handle collection, StringMap keys, StringMap options);

/**
 * Drops (removes) an existing index from the collection.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param indexName     Name of the index to drop
 * @return              True if index was dropped successfully, false otherwise
 *
 * @note Be careful when dropping indexes as it may impact query performance
 * @note Cannot drop the default _id index
 *
 * @example
 * if (MongoDB_DropIndex(players, "name_1")) {
 *     LogMessage("Index dropped successfully");
 * }
 */
native bool MongoDB_DropIndex(Handle collection, const char[] indexName);

//=============================================================================
// ADVANCED OPERATIONS NATIVES
//=============================================================================

/**
 * Executes an aggregation pipeline for advanced data processing.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param pipeline      ArrayList containing aggregation stage JSON strings
 * @return              ArrayList containing result documents, or null on error
 *
 * @note Aggregation pipelines allow complex data transformations and analysis
 * @note Common stages: $match, $group, $sort, $project, $limit, $skip
 * @note The returned ArrayList and all contained StringMaps must be deleted
 *
 * @example
 * ArrayList pipeline = new ArrayList(ByteCountToCells(512));
 * pipeline.PushString("{\"$match\":{\"score\":{\"$gte\":1000}}}");
 * pipeline.PushString("{\"$group\":{\"_id\":null,\"avgScore\":{\"$avg\":\"$score\"}}}");
 * pipeline.PushString("{\"$sort\":{\"avgScore\":-1}}");
 *
 * ArrayList results = MongoDB_Aggregate(players, pipeline);
 * if (results != null) {
 *     // Process aggregation results
 *     delete results;
 * }
 * delete pipeline;
 */
native ArrayList MongoDB_Aggregate(Handle collection, ArrayList pipeline);

/**
 * Finds documents with field projection to limit returned data.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param filter        StringMap containing search criteria (null for no filter)
 * @param projection    StringMap specifying which fields to include/exclude
 * @param options       StringMap containing query options (optional)
 * @return              ArrayList containing projected documents, or null on error
 *
 * @note Projection reduces network traffic by returning only needed fields
 * @note Use 1 to include a field, 0 to exclude (cannot mix include/exclude except for _id)
 * @note The returned ArrayList and all contained StringMaps must be deleted
 *
 * @example
 * StringMap filter = new StringMap();
 * filter.SetValue("active", 1);
 *
 * StringMap projection = new StringMap();
 * projection.SetValue("name", 1);     // Include name
 * projection.SetValue("score", 1);    // Include score
 * projection.SetValue("_id", 0);      // Exclude _id
 *
 * ArrayList results = MongoDB_FindWithProjection(players, filter, projection, null);
 * if (results != null) {
 *     // Process projected results
 *     delete results;
 * }
 */
native ArrayList MongoDB_FindWithProjection(Handle collection, StringMap filter, StringMap projection, StringMap options);

/**
 * Executes multiple write operations in a single batch for efficiency.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param operations    ArrayList containing bulk operation JSON strings
 * @param ordered       Whether operations should be executed in order (stops on first error if true)
 * @return              True if bulk write was successful, false otherwise
 *
 * @note Bulk operations are more efficient than individual operations
 * @note Operation types: insertOne, updateOne, updateMany, deleteOne, deleteMany, replaceOne
 * @note Ordered execution stops on first error, unordered continues despite errors
 *
 * @example
 * ArrayList operations = new ArrayList(ByteCountToCells(1024));
 * operations.PushString("{\"insertOne\":{\"document\":{\"name\":\"Player1\",\"score\":100}}}");
 * operations.PushString("{\"updateOne\":{\"filter\":{\"name\":\"Player2\"},\"update\":{\"$set\":{\"score\":200}}}}");
 * operations.PushString("{\"deleteOne\":{\"filter\":{\"name\":\"Player3\"}}}");
 *
 * if (MongoDB_BulkWrite(players, operations, true)) {
 *     LogMessage("Bulk operations completed successfully");
 * }
 * delete operations;
 */
native bool MongoDB_BulkWrite(Handle collection, ArrayList operations, bool ordered);

/**
 * Finds distinct values for a specified field across the collection.
 *
 * @param collection    Collection handle from MongoDB_GetCollection()
 * @param field         Name of the field to find distinct values for
 * @param filter        StringMap containing search criteria (null for no filter)
 * @return              ArrayList containing distinct values, or null on error
 *
 * @note Useful for finding unique values like player names, server IPs, etc.
 * @note The returned ArrayList must be deleted when no longer needed
 *
 * @example
 * // Find all unique player names
 * ArrayList distinctNames = MongoDB_FindDistinct(players, "name", null);
 * if (distinctNames != null) {
 *     LogMessage("Found %d unique player names", distinctNames.Length);
 *     for (int i = 0; i < distinctNames.Length; i++) {
 *         char name[64];
 *         distinctNames.GetString(i, name, sizeof(name));
 *         LogMessage("Player: %s", name);
 *     }
 *     delete distinctNames;
 * }
 */
native ArrayList MongoDB_FindDistinct(Handle collection, const char[] field, StringMap filter);

//=============================================================================
// ERROR HANDLING NATIVES
//=============================================================================

/**
 * Gets the error code from the last MongoDB operation.
 *
 * @return              Error code (0 = no error, >0 = error occurred)
 *
 * @note Error codes follow standard HTTP status codes and MongoDB error codes
 * @note Common codes: 0 (success), 400 (bad request), 404 (not found), 500 (server error)
 *
 * @example
 * if (MongoDB_GetLastErrorCode() != 0) {
 *     char errorMsg[256];
 *     MongoDB_GetLastErrorMessage(errorMsg, sizeof(errorMsg));
 *     LogError("MongoDB operation failed: %s", errorMsg);
 * }
 */
native int MongoDB_GetLastErrorCode();

/**
 * Gets the error message from the last MongoDB operation.
 *
 * @param buffer        Buffer to store the error message
 * @param maxlen        Maximum length of the buffer
 * @return              True if error message was retrieved, false if no error
 *
 * @example
 * char errorMsg[256];
 * if (MongoDB_GetLastErrorMessage(errorMsg, sizeof(errorMsg))) {
 *     LogError("Last MongoDB error: %s", errorMsg);
 * }
 */
native bool MongoDB_GetLastErrorMessage(char[] buffer, int maxlen);

/**
 * Gets detailed error information from the last MongoDB operation.
 *
 * @param buffer        Buffer to store the detailed error information
 * @param maxlen        Maximum length of the buffer
 * @return              True if error details were retrieved, false if no error
 *
 * @note Provides more technical details than the basic error message
 * @note May include stack traces, operation details, and troubleshooting info
 *
 * @example
 * char errorDetails[512];
 * if (MongoDB_GetLastErrorDetails(errorDetails, sizeof(errorDetails))) {
 *     LogError("MongoDB error details: %s", errorDetails);
 * }
 */
native bool MongoDB_GetLastErrorDetails(char[] buffer, int maxlen);

/**
 * Gets the timestamp when the last error occurred.
 *
 * @return              Unix timestamp of the last error, or 0 if no error
 *
 * @example
 * int errorTime = MongoDB_GetLastErrorTimestamp();
 * if (errorTime > 0) {
 *     LogError("Last error occurred at timestamp: %d", errorTime);
 * }
 */
native int MongoDB_GetLastErrorTimestamp();

//=============================================================================
// PERFORMANCE MONITORING NATIVES
//=============================================================================

/**
 * Gets the total number of MongoDB operations performed since startup or last reset.
 *
 * @return              Total number of operations (includes both successful and failed)
 *
 * @example
 * int totalOps = MongoDB_GetTotalOperations();
 * LogMessage("Total MongoDB operations: %d", totalOps);
 */
native int MongoDB_GetTotalOperations();

/**
 * Gets the number of successful MongoDB operations.
 *
 * @return              Number of successful operations
 *
 * @example
 * int successOps = MongoDB_GetSuccessfulOperations();
 * int totalOps = MongoDB_GetTotalOperations();
 * LogMessage("Successful operations: %d/%d", successOps, totalOps);
 */
native int MongoDB_GetSuccessfulOperations();

/**
 * Gets the number of failed MongoDB operations.
 *
 * @return              Number of failed operations
 *
 * @example
 * int failedOps = MongoDB_GetFailedOperations();
 * if (failedOps > 0) {
 *     LogWarning("MongoDB has %d failed operations", failedOps);
 * }
 */
native int MongoDB_GetFailedOperations();

/**
 * Gets the average execution time for MongoDB operations.
 *
 * @return              Average execution time in centiseconds (1/100th of a second)
 *
 * @note Centiseconds provide better precision than milliseconds for SourceMod
 * @note Divide by 100 to get seconds, or use modulo 100 for fractional part
 *
 * @example
 * int avgTime = MongoDB_GetAverageExecutionTime();
 * LogMessage("Average execution time: %d.%02d seconds", avgTime / 100, avgTime % 100);
 */
native int MongoDB_GetAverageExecutionTime();

/**
 * Gets the success rate of MongoDB operations as a percentage.
 *
 * @return              Success rate as percentage (0-100)
 *
 * @example
 * int successRate = MongoDB_GetSuccessRate();
 * if (successRate < 95) {
 *     LogWarning("MongoDB success rate is low: %d%%", successRate);
 * }
 */
native int MongoDB_GetSuccessRate();

/**
 * Resets all performance monitoring metrics to zero.
 *
 * @return              True if metrics were reset successfully
 *
 * @note Useful for monitoring performance over specific time periods
 *
 * @example
 * MongoDB_ResetPerformanceMetrics();
 * LogMessage("Performance metrics reset");
 */
native bool MongoDB_ResetPerformanceMetrics();

//=============================================================================
// CONNECTION TESTING NATIVES
//=============================================================================

/**
 * Tests if a connection is still active and responsive.
 *
 * @param connection    Connection handle to test
 * @return              True if connection is active and responsive, false otherwise
 *
 * @note This performs an actual network test, unlike MongoDB_IsConnected which checks handle validity
 * @note Use this for health checks and connection monitoring
 *
 * @example
 * if (!MongoDB_TestConnection(conn)) {
 *     LogWarning("MongoDB connection is unresponsive, reconnecting...");
 *     MongoDB_Close(conn);
 *     conn = MongoDB_Connect("");
 * }
 */
native bool MongoDB_TestConnection(Handle connection);

//=============================================================================
// LEGACY AND UTILITY NATIVES
//=============================================================================

/**
 * Gets the last error message (deprecated - use MongoDB_GetLastErrorMessage instead).
 *
 * @param buffer        Buffer to store the error message
 * @param maxlen        Maximum length of the buffer
 * @return              True if error message was retrieved
 *
 * @deprecated Use MongoDB_GetLastErrorMessage() for better error handling
 */
native bool MongoDB_GetLastError(char[] buffer, int maxlen);

/**
 * Converts a StringMap to a JSON string representation.
 *
 * @param map           StringMap to convert
 * @param buffer        Buffer to store the JSON string
 * @param maxlen        Maximum length of the buffer
 * @return              True if conversion was successful
 *
 * @example
 * StringMap data = new StringMap();
 * data.SetString("name", "John");
 * data.SetValue("score", 100);
 *
 * char jsonStr[512];
 * if (JSON_StringMapToString(data, jsonStr, sizeof(jsonStr))) {
 *     LogMessage("JSON: %s", jsonStr);
 * }
 * delete data;
 */
native bool JSON_StringMapToString(StringMap map, char[] buffer, int maxlen);

/**
 * Parses a JSON string into a StringMap.
 *
 * @param map           StringMap to populate with parsed data
 * @param jsonStr       JSON string to parse
 * @return              True if parsing was successful
 *
 * @example
 * StringMap data = new StringMap();
 * if (JSON_StringFromString(data, "{\"name\":\"John\",\"score\":100}")) {
 *     char name[64];
 *     int score;
 *     data.GetString("name", name, sizeof(name));
 *     data.GetValue("score", score);
 * }
 * delete data;
 */
native bool JSON_StringFromString(StringMap map, const char[] jsonStr);

/**
 * Converts an ArrayList to a JSON array string.
 *
 * @param array         ArrayList to convert
 * @param buffer        Buffer to store the JSON array string
 * @param maxlen        Maximum length of the buffer
 * @return              True if conversion was successful
 *
 * @example
 * ArrayList names = new ArrayList(ByteCountToCells(64));
 * names.PushString("John");
 * names.PushString("Jane");
 *
 * char jsonArray[256];
 * if (JSON_ArrayListToString(names, jsonArray, sizeof(jsonArray))) {
 *     LogMessage("JSON Array: %s", jsonArray);
 * }
 * delete names;
 */
native bool JSON_ArrayListToString(ArrayList array, char[] buffer, int maxlen);

/**
 * Parses a JSON array string into an ArrayList.
 *
 * @param array         ArrayList to populate with parsed data
 * @param jsonStr       JSON array string to parse
 * @return              True if parsing was successful
 *
 * @example
 * ArrayList names = new ArrayList(ByteCountToCells(64));
 * if (JSON_ArrayFromString(names, "[\"John\",\"Jane\",\"Bob\"]")) {
 *     for (int i = 0; i < names.Length; i++) {
 *         char name[64];
 *         names.GetString(i, name, sizeof(name));
 *         LogMessage("Name: %s", name);
 *     }
 * }
 * delete names;
 */
native bool JSON_ArrayFromString(ArrayList array, const char[] jsonStr);

/**
 * Enhanced StringMap string setter with better error handling.
 *
 * @param map           StringMap handle
 * @param key           Key name
 * @param value         String value to set
 * @return              True if value was set successfully
 *
 * @example
 * Handle map = StringMap_CreateEmpty();
 * StringMap_SetString(map, "playerName", "John Doe");
 */
native bool StringMap_SetString(Handle map, const char[] key, const char[] value);

/**
 * Enhanced StringMap string getter with better error handling.
 *
 * @param map           StringMap handle
 * @param key           Key name
 * @param buffer        Buffer to store the retrieved value
 * @param maxlen        Maximum length of the buffer
 * @return              True if value was retrieved successfully
 *
 * @example
 * char playerName[64];
 * if (StringMap_GetString(map, "playerName", playerName, sizeof(playerName))) {
 *     LogMessage("Player: %s", playerName);
 * }
 */
native bool StringMap_GetString(Handle map, const char[] key, char[] buffer, int maxlen);

/**
 * Creates an empty StringMap with enhanced functionality.
 *
 * @return              Handle to the new StringMap
 *
 * @note The returned StringMap must be deleted when no longer needed
 *
 * @example
 * Handle map = StringMap_CreateEmpty();
 * StringMap_SetString(map, "key", "value");
 * // ... use the map ...
 * delete map;
 */
native Handle StringMap_CreateEmpty();

//=============================================================================
// METHODMAP INTERFACES
//=============================================================================

/**
 * MongoDB Connection - Object-oriented wrapper for connection management.
 *
 * This methodmap provides a convenient object-oriented interface for managing
 * MongoDB connections. It wraps the native connection functions and provides
 * automatic resource management.
 *
 * @example Basic Usage:
 * MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
 * if (conn.IsConnected()) {
 *     MongoCollection players = conn.GetCollection("gamedb", "players");
 *     // ... perform operations ...
 * }
 * conn.Close();
 *
 * @example Using Configuration:
 * MongoDB_LoadConfig("configs/mongodb.cfg");
 * MongoConnection conn = MongoConnection.FromConfig();
 * // ... use connection ...
 * conn.Close();
 */
methodmap MongoConnection < Handle {
    /**
     * Creates a new MongoDB connection using an explicit API URL.
     *
     * @param url           HTTP API service URL (e.g., "http://127.0.0.1:3300")
     *
     * @example
     * MongoConnection conn = new MongoConnection("http://192.168.1.100:3300");
     * if (conn.IsConnected()) {
     *     LogMessage("Connected to MongoDB API service");
     * }
     */
    public MongoConnection(const char[] url) {
        return view_as<MongoConnection>(MongoDB_Connect(url));
    }

    /**
     * Creates a new MongoDB connection using the configured API URL.
     *
     * @return              MongoConnection instance using configured settings
     *
     * @note Requires MongoDB_LoadConfig() to be called first
     *
     * @example
     * MongoDB_LoadConfig("configs/mongodb.cfg");
     * MongoConnection conn = MongoConnection.FromConfig();
     */
    public static MongoConnection FromConfig() {
        return view_as<MongoConnection>(MongoDB_Connect(""));
    }

    /**
     * Gets a collection handle from this connection.
     *
     * @param database      Name of the database
     * @param collection    Name of the collection within the database
     * @return              MongoCollection instance for the specified collection
     *
     * @example
     * MongoCollection players = conn.GetCollection("gamedb", "players");
     * MongoCollection servers = conn.GetCollection("gamedb", "servers");
     */
    public MongoCollection GetCollection(const char[] database, const char[] collection) {
        return view_as<MongoCollection>(MongoDB_GetCollection(this, database, collection));
    }

    /**
     * Checks if this connection is still active and valid.
     *
     * @return              True if connection is active, false otherwise
     *
     * @example
     * if (!conn.IsConnected()) {
     *     LogError("Connection lost, attempting to reconnect...");
     *     conn.Close();
     *     conn = MongoConnection.FromConfig();
     * }
     */
    public bool IsConnected() {
        return MongoDB_IsConnected(this);
    }

    /**
     * Closes this connection and releases associated resources.
     *
     * @note Always call this when done with a connection to prevent resource leaks
     * @note All collection handles from this connection become invalid after closing
     *
     * @example
     * MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
     * // ... perform operations ...
     * conn.Close(); // Always close when done
     */
    public void Close() {
        MongoDB_Close(this);
    }
}

/**
 * MongoDB Collection - Object-oriented wrapper for collection operations.
 *
 * This methodmap provides a convenient interface for performing CRUD operations
 * and advanced database operations on MongoDB collections. It wraps the native
 * collection functions with a clean, easy-to-use API.
 *
 * @example Basic CRUD Operations:
 * MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
 * MongoCollection players = conn.GetCollection("gamedb", "players");
 *
 * // Insert
 * char insertedId[64];
 * players.InsertOneJSON("{\"name\":\"John\",\"score\":100}", insertedId, sizeof(insertedId));
 *
 * // Find
 * StringMap result = players.FindOneJSON("{\"name\":\"John\"}");
 * if (result != null) {
 *     delete result;
 * }
 *
 * // Update
 * StringMap filter = new StringMap();
 * filter.SetString("name", "John");
 * StringMap update = new StringMap();
 * update.SetValue("score", 200);
 * players.UpdateOne(filter, update);
 *
 * // Count
 * int totalPlayers = players.CountDocuments(null);
 *
 * conn.Close();
 */
methodmap MongoCollection < Handle {
    /**
     * Inserts a single document using a StringMap.
     *
     * @param document      StringMap containing the document data
     * @param insertedId    Buffer to store the inserted document's ObjectId
     * @param maxlen        Maximum length of the insertedId buffer
     * @return              True if insertion was successful, false otherwise
     *
     * @example
     * StringMap doc = new StringMap();
     * doc.SetString("name", "John Doe");
     * doc.SetValue("score", 1500);
     *
     * char insertedId[64];
     * if (players.InsertOne(doc, insertedId, sizeof(insertedId))) {
     *     LogMessage("Document inserted with ID: %s", insertedId);
     * }
     * delete doc;
     */
    public bool InsertOne(StringMap document, char[] insertedId, int maxlen) {
        return MongoDB_InsertOne(this, document, insertedId, maxlen);
    }

    /**
     * Inserts a single document using a JSON string.
     *
     * @param jsonDocument  JSON string representing the document
     * @param insertedId    Buffer to store the inserted document's ObjectId
     * @param maxlen        Maximum length of the insertedId buffer
     * @return              True if insertion was successful, false otherwise
     *
     * @example
     * char jsonDoc[256];
     * Format(jsonDoc, sizeof(jsonDoc), "{\"name\":\"%s\",\"score\":%d}", playerName, score);
     *
     * char insertedId[64];
     * if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
     *     LogMessage("Player data inserted: %s", insertedId);
     * }
     */
    public bool InsertOneJSON(const char[] jsonDocument, char[] insertedId, int maxlen) {
        return MongoDB_InsertOneJSON(this, jsonDocument, insertedId, maxlen);
    }

    /**
     * Inserts multiple documents in a single batch operation.
     *
     * @param documents     ArrayList containing StringMap documents to insert
     * @param insertedIds   ArrayList to store the inserted document ObjectIds (optional)
     * @return              True if all insertions were successful, false otherwise
     *
     * @example
     * ArrayList docs = new ArrayList();
     * for (int i = 0; i < 10; i++) {
     *     StringMap doc = new StringMap();
     *     doc.SetString("name", "Player" + i);
     *     docs.Push(doc);
     * }
     *
     * if (players.InsertMany(docs, null)) {
     *     LogMessage("Batch insert completed successfully");
     * }
     */
    public bool InsertMany(ArrayList documents, ArrayList insertedIds) {
        return MongoDB_InsertMany(this, documents, insertedIds);
    }

    /**
     * Finds and returns the first document matching the filter.
     *
     * @param filter        StringMap containing search criteria (null for any document)
     * @return              StringMap containing the found document, or null if not found
     *
     * @note The returned StringMap must be deleted when no longer needed
     *
     * @example
     * StringMap filter = new StringMap();
     * filter.SetString("name", "John Doe");
     *
     * StringMap result = players.FindOne(filter);
     * if (result != null) {
     *     // Process the found document
     *     delete result;
     * }
     * delete filter;
     */
    public StringMap FindOne(StringMap filter = null) {
        return view_as<StringMap>(MongoDB_FindOne(this, filter));
    }

    /**
     * Finds and returns the first document matching the JSON filter.
     *
     * @param jsonFilter    JSON string containing search criteria
     * @return              StringMap containing the found document, or null if not found
     *
     * @note The returned StringMap must be deleted when no longer needed
     *
     * @example
     * StringMap result = players.FindOneJSON("{\"score\":{\"$gte\":1000}}");
     * if (result != null) {
     *     char name[64];
     *     result.GetString("name", name, sizeof(name));
     *     LogMessage("High-score player: %s", name);
     *     delete result;
     * }
     */
    public StringMap FindOneJSON(const char[] jsonFilter) {
        return view_as<StringMap>(MongoDB_FindOneJSON(this, jsonFilter));
    }

    /**
     * Finds multiple documents matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria (null for all documents)
     * @param options       StringMap containing query options like limit, skip, sort
     * @return              ArrayList containing StringMap documents, or null on error
     *
     * @note The returned ArrayList and all contained StringMaps must be deleted
     *
     * @example
     * StringMap options = new StringMap();
     * options.SetValue("limit", 10);
     *
     * ArrayList results = players.Find(null, options);
     * if (results != null) {
     *     for (int i = 0; i < results.Length; i++) {
     *         StringMap doc = results.Get(i);
     *         // Process document
     *         delete doc;
     *     }
     *     delete results;
     * }
     */
    public ArrayList Find(StringMap filter = null, StringMap options = null) {
        return view_as<ArrayList>(MongoDB_Find(this, filter, options));
    }

    /**
     * Updates the first document matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria
     * @param update        StringMap containing update operations
     * @return              True if update was successful, false otherwise
     *
     * @example
     * StringMap filter = new StringMap();
     * filter.SetString("name", "John Doe");
     *
     * StringMap update = new StringMap();
     * update.SetValue("score", 2000);
     * update.SetValue("lastSeen", GetTime());
     *
     * if (players.UpdateOne(filter, update)) {
     *     LogMessage("Player updated successfully");
     * }
     * delete filter;
     * delete update;
     */
    public bool UpdateOne(StringMap filter, StringMap update) {
        return MongoDB_UpdateOne(this, filter, update);
    }

    /**
     * Updates all documents matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria
     * @param update        StringMap containing update operations
     * @return              True if update was successful, false otherwise
     *
     * @example
     * StringMap filter = new StringMap();
     * filter.SetValue("active", 1);
     *
     * StringMap update = new StringMap();
     * update.SetValue("lastUpdate", GetTime());
     *
     * if (players.UpdateMany(filter, update)) {
     *     LogMessage("All active players updated");
     * }
     */
    public bool UpdateMany(StringMap filter, StringMap update) {
        return MongoDB_UpdateMany(this, filter, update);
    }

    /**
     * Deletes the first document matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria
     * @return              True if deletion was successful, false otherwise
     *
     * @example
     * StringMap filter = new StringMap();
     * filter.SetString("name", "John Doe");
     *
     * if (players.DeleteOne(filter)) {
     *     LogMessage("Player deleted successfully");
     * }
     * delete filter;
     */
    public bool DeleteOne(StringMap filter) {
        return MongoDB_DeleteOne(this, filter);
    }

    /**
     * Deletes all documents matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria
     * @return              True if deletion was successful, false otherwise
     *
     * @note Use with caution as this deletes all matching documents
     *
     * @example
     * StringMap filter = new StringMap();
     * filter.SetValue("score", 0);
     *
     * if (players.DeleteMany(filter)) {
     *     LogMessage("Inactive players deleted");
     * }
     */
    public bool DeleteMany(StringMap filter) {
        return MongoDB_DeleteMany(this, filter);
    }

    /**
     * Counts the number of documents matching the filter criteria.
     *
     * @param filter        StringMap containing search criteria (null for total count)
     * @return              Number of matching documents, or -1 on error
     *
     * @example
     * int totalPlayers = players.CountDocuments(null);
     *
     * StringMap filter = new StringMap();
     * filter.SetValue("active", 1);
     * int activePlayers = players.CountDocuments(filter);
     * delete filter;
     *
     * LogMessage("Players: %d total, %d active", totalPlayers, activePlayers);
     */
    public int CountDocuments(StringMap filter = null) {
        return MongoDB_CountDocuments(this, filter);
    }

    /**
     * Creates an index on specified fields to improve query performance.
     *
     * @param keys          StringMap containing field names and sort order
     * @param options       StringMap containing index options (optional)
     * @return              True if index creation was successful, false otherwise
     *
     * @example
     * StringMap keys = new StringMap();
     * keys.SetValue("name", 1);  // Ascending index on name
     *
     * StringMap options = new StringMap();
     * options.SetValue("unique", 1);  // Unique constraint
     *
     * if (players.CreateIndex(keys, options)) {
     *     LogMessage("Index created successfully");
     * }
     */
    public bool CreateIndex(StringMap keys, StringMap options = null) {
        return MongoDB_CreateIndex(this, keys, options);
    }

    /**
     * Drops (removes) an existing index from the collection.
     *
     * @param indexName     Name of the index to drop
     * @return              True if index was dropped successfully, false otherwise
     *
     * @example
     * if (players.DropIndex("name_1")) {
     *     LogMessage("Index dropped successfully");
     * }
     */
    public bool DropIndex(const char[] indexName) {
        return MongoDB_DropIndex(this, indexName);
    }

    /**
     * Executes an aggregation pipeline for advanced data processing.
     *
     * @param pipeline      ArrayList containing aggregation stage JSON strings
     * @return              ArrayList containing result documents, or null on error
     *
     * @note The returned ArrayList and all contained StringMaps must be deleted
     *
     * @example
     * ArrayList pipeline = new ArrayList(ByteCountToCells(512));
     * pipeline.PushString("{\"$match\":{\"score\":{\"$gte\":1000}}}");
     * pipeline.PushString("{\"$group\":{\"_id\":null,\"avgScore\":{\"$avg\":\"$score\"}}}");
     *
     * ArrayList results = players.Aggregate(pipeline);
     * if (results != null) {
     *     // Process aggregation results
     *     delete results;
     * }
     * delete pipeline;
     */
    public ArrayList Aggregate(ArrayList pipeline) {
        return view_as<ArrayList>(MongoDB_Aggregate(this, pipeline));
    }

    /**
     * Finds documents with field projection to limit returned data.
     *
     * @param filter        StringMap containing search criteria (optional)
     * @param projection    StringMap specifying which fields to include/exclude
     * @param options       StringMap containing query options (optional)
     * @return              ArrayList containing projected documents, or null on error
     *
     * @note The returned ArrayList and all contained StringMaps must be deleted
     *
     * @example
     * StringMap projection = new StringMap();
     * projection.SetValue("name", 1);    // Include name
     * projection.SetValue("score", 1);   // Include score
     * projection.SetValue("_id", 0);     // Exclude _id
     *
     * ArrayList results = players.FindWithProjection(null, projection, null);
     * if (results != null) {
     *     // Process projected results
     *     delete results;
     * }
     */
    public ArrayList FindWithProjection(StringMap filter = null, StringMap projection = null, StringMap options = null) {
        return view_as<ArrayList>(MongoDB_FindWithProjection(this, filter, projection, options));
    }

    /**
     * Executes multiple write operations in a single batch.
     *
     * @param operations    ArrayList containing bulk operation JSON strings
     * @param ordered       Whether operations should be executed in order
     * @return              True if bulk write was successful, false otherwise
     *
     * @example
     * ArrayList operations = new ArrayList(ByteCountToCells(1024));
     * operations.PushString("{\"insertOne\":{\"document\":{\"name\":\"Player1\"}}}");
     * operations.PushString("{\"updateOne\":{\"filter\":{\"name\":\"Player2\"},\"update\":{\"$set\":{\"score\":200}}}}");
     *
     * if (players.BulkWrite(operations, true)) {
     *     LogMessage("Bulk operations completed");
     * }
     * delete operations;
     */
    public bool BulkWrite(ArrayList operations, bool ordered = true) {
        return MongoDB_BulkWrite(this, operations, ordered);
    }

    /**
     * Finds distinct values for a specified field across the collection.
     *
     * @param field         Name of the field to find distinct values for
     * @param filter        StringMap containing search criteria (optional)
     * @return              ArrayList containing distinct values, or null on error
     *
     * @note The returned ArrayList must be deleted when no longer needed
     *
     * @example
     * ArrayList distinctNames = players.FindDistinct("name", null);
     * if (distinctNames != null) {
     *     LogMessage("Found %d unique player names", distinctNames.Length);
     *     delete distinctNames;
     * }
     */
    public ArrayList FindDistinct(const char[] field, StringMap filter = null) {
        return view_as<ArrayList>(MongoDB_FindDistinct(this, field, filter));
    }
}

/**
 * MongoDB Document - Enhanced StringMap with MongoDB-specific functionality
 */
methodmap MongoDocument < StringMap {
    public MongoDocument() {
        return view_as<MongoDocument>(StringMap_CreateEmpty());
    }

    // Create from existing StringMap handle (useful for FindOne results)
    public static MongoDocument FromHandle(Handle handle) {
        return view_as<MongoDocument>(handle);
    }
    
    // Set ObjectId
    public void SetObjectId(const char[] key, const char[] objectId) {
        this.SetString(key, objectId);
    }
    
    // Get ObjectId
    public bool GetObjectId(const char[] key, char[] buffer, int maxlen) {
        return this.GetString(key, buffer, maxlen);
    }
    
    // Set Date (timestamp)
    public void SetDate(const char[] key, int timestamp) {
        this.SetValue(key, timestamp);
    }
    
    // Get Date
    public int GetDate(const char[] key) {
        int value;
        this.GetValue(key, value);
        return value;
    }
    
    // Set nested document (stores as JSON string)
    public void SetDocument(const char[] key, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(key, jsonStr);
    }
    
    // Get nested document (parses from JSON string)
    public MongoDocument GetDocument(const char[] key) {
        char jsonStr[4096];
        if (this.GetString(key, jsonStr, sizeof(jsonStr))) {
            return MongoDocument.FromString(jsonStr);
        }
        return null;
    }
    
    // Set array (stores as JSON string)
    public void SetArray(const char[] key, MongoDocumentArray array) {
        char jsonStr[4096];
        array.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(key, jsonStr);
    }
    
    // Convert to JSON string
    public bool ToString(char[] buffer, int maxlen) {
        return JSON_StringMapToString(this, buffer, maxlen);
    }
    
    // Create from JSON string
    public static MongoDocument FromString(const char[] jsonStr) {
        MongoDocument doc = new MongoDocument();
        JSON_StringFromString(doc, jsonStr);
        return doc;
    }

    // Enhanced string operations using helper natives
    public bool SetStringValue(const char[] key, const char[] value) {
        return StringMap_SetString(this, key, value);
    }

    public bool GetStringValue(const char[] key, char[] buffer, int maxlen) {
        return StringMap_GetString(this, key, buffer, maxlen);
    }
}

/**
 * MongoDB Document Array - Enhanced ArrayList for MongoDB documents
 */
methodmap MongoDocumentArray < ArrayList {
    public MongoDocumentArray() {
        return view_as<MongoDocumentArray>(new ArrayList());
    }
    
    // Push a document (as JSON string)
    public void PushDocument(MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.PushString(jsonStr);
    }
    
    // Get a document by index
    public MongoDocument GetDocument(int index) {
        char jsonStr[4096];
        this.GetString(index, jsonStr, sizeof(jsonStr));
        return MongoDocument.FromString(jsonStr);
    }
    
    // Set a document at index
    public void SetDocument(int index, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(index, jsonStr);
    }
    
    // Insert document at specific position (simplified)
    public void InsertDocument(int index, MongoDocument document) {
        char jsonStr[4096];
        document.ToString(jsonStr, sizeof(jsonStr));
        this.PushString(jsonStr);
    }
    
    // Remove document at index
    public void RemoveDocument(int index) {
        this.Erase(index);
    }
    
    // Find document by field value
    public int FindDocument(const char[] field, const char[] value) {
        for (int i = 0; i < this.Length; i++) {
            MongoDocument doc = this.GetDocument(i);
            char docValue[256];
            if (doc.GetString(field, docValue, sizeof(docValue))) {
                if (StrEqual(docValue, value)) {
                    delete doc;
                    return i;
                }
            }
            delete doc;
        }
        return -1;
    }
    
    // Convert to JSON string
    public void ToString(char[] buffer, int maxlen) {
        JSON_ArrayListToString(this, buffer, maxlen);
    }
    
    // Create from JSON string
    public static MongoDocumentArray FromString(const char[] jsonStr) {
        MongoDocumentArray arr = new MongoDocumentArray();
        JSON_ArrayFromString(arr, jsonStr);
        return arr;
    }
}

/**
 * MongoDB Result Set - Simple result container
 */
methodmap MongoResultSet < ArrayList {
    public MongoResultSet() {
        return view_as<MongoResultSet>(new ArrayList());
    }
}

/**
 * MongoDB Find Options - Simple options container
 */
methodmap MongoFindOptions < StringMap {
    public MongoFindOptions() {
        return view_as<MongoFindOptions>(new StringMap());
    }

    public void SetLimit(int limit) {
        this.SetValue("limit", limit);
    }

    public void SetSkip(int skip) {
        this.SetValue("skip", skip);
    }
    
    // Simplified sort methods
    public void SortAscending(const char[] field) {
        char sortStr[256];
        Format(sortStr, sizeof(sortStr), "{\"%s\":1}", field);
        this.SetString("sort", sortStr);
    }

    public void SortDescending(const char[] field) {
        char sortStr[256];
        Format(sortStr, sizeof(sortStr), "{\"%s\":-1}", field);
        this.SetString("sort", sortStr);
    }
}

/**
 * MongoDB Query Builder - Enhanced query construction
 */
methodmap MongoQuery < MongoDocument {
    public MongoQuery() {
        return view_as<MongoQuery>(new MongoDocument());
    }

    // Basic comparison methods (enhanced)
    public MongoQuery Where(const char[] field, const char[] op, any value) {
        if (StrEqual(op, "=") || StrEqual(op, "==") || StrEqual(op, "eq")) {
            this.SetValue(field, value);
        } else {
            // Create proper MongoDB operator structure: {"field": {"$op": value}}
            char fieldValue[256];
            char opKey[32];
            Format(opKey, sizeof(opKey), "$%s", op);
            Format(fieldValue, sizeof(fieldValue), "{\"%s\":%d}", opKey, value);
            this.SetString(field, fieldValue);
        }
        return this;
    }

    // String-based where clause for complex values
    public MongoQuery WhereString(const char[] field, const char[] op, const char[] value) {
        if (StrEqual(op, "=") || StrEqual(op, "==") || StrEqual(op, "eq")) {
            this.SetString(field, value);
        } else {
            // Create proper MongoDB operator structure: {"field": {"$op": "value"}}
            char fieldValue[512];
            char opKey[32];
            Format(opKey, sizeof(opKey), "$%s", op);
            Format(fieldValue, sizeof(fieldValue), "{\"%s\":\"%s\"}", opKey, value);
            this.SetString(field, fieldValue);
        }
        return this;
    }

    // Convenience methods for common operations (enhanced)
    public MongoQuery Equals(const char[] field, any value) {
        this.SetValue(field, value);
        return this;
    }

    public MongoQuery EqualsString(const char[] field, const char[] value) {
        this.SetString(field, value);
        return this;
    }

    public MongoQuery NotEquals(const char[] field, any value) {
        return this.Where(field, "ne", value);
    }

    public MongoQuery NotEqualsString(const char[] field, const char[] value) {
        return this.WhereString(field, "ne", value);
    }

    public MongoQuery GreaterThan(const char[] field, any value) {
        return this.Where(field, "gt", value);
    }

    public MongoQuery GreaterThanString(const char[] field, const char[] value) {
        return this.WhereString(field, "gt", value);
    }

    public MongoQuery GreaterThanOrEqual(const char[] field, any value) {
        return this.Where(field, "gte", value);
    }

    public MongoQuery GreaterThanOrEqualString(const char[] field, const char[] value) {
        return this.WhereString(field, "gte", value);
    }

    public MongoQuery LessThan(const char[] field, any value) {
        return this.Where(field, "lt", value);
    }

    public MongoQuery LessThanString(const char[] field, const char[] value) {
        return this.WhereString(field, "lt", value);
    }

    public MongoQuery LessThanOrEqual(const char[] field, any value) {
        return this.Where(field, "lte", value);
    }

    public MongoQuery LessThanOrEqualString(const char[] field, const char[] value) {
        return this.WhereString(field, "lte", value);
    }

    // Array operations (simplified)
    public MongoQuery In(const char[] field, MongoDocumentArray values) {
        char fieldName[128];
        Format(fieldName, sizeof(fieldName), "%s_$in", field);
        char jsonStr[4096];
        values.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(fieldName, jsonStr);
        return this;
    }

    public MongoQuery NotIn(const char[] field, MongoDocumentArray values) {
        char fieldName[128];
        Format(fieldName, sizeof(fieldName), "%s_$nin", field);
        char jsonStr[4096];
        values.ToString(jsonStr, sizeof(jsonStr));
        this.SetString(fieldName, jsonStr);
        return this;
    }

    // Text search
    public MongoQuery TextSearch(const char[] searchText) {
        char textQuery[512];
        Format(textQuery, sizeof(textQuery), "{\"$search\":\"%s\"}", searchText);
        this.SetString("$text", textQuery);
        return this;
    }

    // Logical operations (full implementations)
    public MongoQuery And(MongoQuery other) {
        // Create $and array with both queries
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        // Create new query with $and operator
        MongoQuery andQuery = new MongoQuery();
        char andValue[4096];
        Format(andValue, sizeof(andValue), "[%s,%s]", thisJson, otherJson);
        andQuery.SetString("$and", andValue);

        return andQuery;
    }

    public MongoQuery Or(MongoQuery other) {
        // Create $or array with both queries
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        // Create new query with $or operator
        MongoQuery orQuery = new MongoQuery();
        char orValue[4096];
        Format(orValue, sizeof(orValue), "[%s,%s]", thisJson, otherJson);
        orQuery.SetString("$or", orValue);

        return orQuery;
    }

    public MongoQuery Not(MongoQuery condition) {
        // Wrap the condition with $not operator
        char conditionJson[2048];
        condition.ToString(conditionJson, sizeof(conditionJson));

        // Create new query with $not operator
        MongoQuery notQuery = new MongoQuery();
        notQuery.SetString("$not", conditionJson);

        return notQuery;
    }

    // Additional logical operations
    public MongoQuery Nor(MongoQuery other) {
        // Create $nor array (neither this nor other)
        char thisJson[2048], otherJson[2048];
        this.ToString(thisJson, sizeof(thisJson));
        other.ToString(otherJson, sizeof(otherJson));

        MongoQuery norQuery = new MongoQuery();
        char norValue[4096];
        Format(norValue, sizeof(norValue), "[%s,%s]", thisJson, otherJson);
        norQuery.SetString("$nor", norValue);

        return norQuery;
    }

    // Exists check
    public MongoQuery Exists(const char[] field, bool exists = true) {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$exists", field);
        this.SetValue(fieldKey, exists ? 1 : 0);
        return this;
    }

    // Type check
    public MongoQuery Type(const char[] field, int bsonType) {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$type", field);
        this.SetValue(fieldKey, bsonType);
        return this;
    }

    // Regular expression
    public MongoQuery Regex(const char[] field, const char[] pattern, const char[] options = "") {
        char fieldKey[128];
        Format(fieldKey, sizeof(fieldKey), "%s.$regex", field);
        this.SetString(fieldKey, pattern);

        if (strlen(options) > 0) {
            char optionsKey[128];
            Format(optionsKey, sizeof(optionsKey), "%s.$options", field);
            this.SetString(optionsKey, options);
        }
        return this;
    }
}

/**
 * MongoDB Aggregation Pipeline Builder
 */
methodmap MongoAggregation < ArrayList {
    public MongoAggregation() {
        return view_as<MongoAggregation>(new ArrayList(ByteCountToCells(512)));
    }

    // Add a $match stage
    public MongoAggregation Match(MongoQuery query) {
        char queryJson[1024];
        query.ToString(queryJson, sizeof(queryJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$match\":%s}", queryJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $group stage
    public MongoAggregation Group(const char[] idField, MongoDocument groupSpec) {
        char groupJson[1024];
        groupSpec.ToString(groupJson, sizeof(groupJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$group\":{\"_id\":\"%s\",%s}}", idField, groupJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $sort stage
    public MongoAggregation Sort(MongoDocument sortSpec) {
        char sortJson[512];
        sortSpec.ToString(sortJson, sizeof(sortJson));

        char stageJson[512];
        Format(stageJson, sizeof(stageJson), "{\"$sort\":%s}", sortJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a $limit stage
    public MongoAggregation Limit(int count) {
        char stageJson[128];
        Format(stageJson, sizeof(stageJson), "{\"$limit\":%d}", count);
        this.PushString(stageJson);
        return this;
    }

    // Add a $skip stage
    public MongoAggregation Skip(int count) {
        char stageJson[128];
        Format(stageJson, sizeof(stageJson), "{\"$skip\":%d}", count);
        this.PushString(stageJson);
        return this;
    }

    // Add a $project stage
    public MongoAggregation Project(MongoDocument projection) {
        char projJson[1024];
        projection.ToString(projJson, sizeof(projJson));

        char stageJson[1024];
        Format(stageJson, sizeof(stageJson), "{\"$project\":%s}", projJson);
        this.PushString(stageJson);
        return this;
    }

    // Add a custom stage
    public MongoAggregation AddStage(const char[] stageJson) {
        this.PushString(stageJson);
        return this;
    }
}

/**
 * MongoDB Bulk Operations Builder
 */
methodmap MongoBulkOps < ArrayList {
    public MongoBulkOps() {
        return view_as<MongoBulkOps>(new ArrayList(ByteCountToCells(1024)));
    }

    // Add an insertOne operation
    public MongoBulkOps InsertOne(MongoDocument document) {
        char docJson[1024];
        document.ToString(docJson, sizeof(docJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"insertOne\":{\"document\":%s}}", docJson);
        this.PushString(opJson);
        return this;
    }

    // Add an updateOne operation
    public MongoBulkOps UpdateOne(MongoQuery filter, MongoDocument update, bool upsert = false) {
        char filterJson[512], updateJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        update.ToString(updateJson, sizeof(updateJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"updateOne\":{\"filter\":%s,\"update\":%s,\"upsert\":%s}}",
               filterJson, updateJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }

    // Add an updateMany operation
    public MongoBulkOps UpdateMany(MongoQuery filter, MongoDocument update, bool upsert = false) {
        char filterJson[512], updateJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        update.ToString(updateJson, sizeof(updateJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"updateMany\":{\"filter\":%s,\"update\":%s,\"upsert\":%s}}",
               filterJson, updateJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }

    // Add a deleteOne operation
    public MongoBulkOps DeleteOne(MongoQuery filter) {
        char filterJson[512];
        filter.ToString(filterJson, sizeof(filterJson));

        char opJson[512];
        Format(opJson, sizeof(opJson), "{\"deleteOne\":{\"filter\":%s}}", filterJson);
        this.PushString(opJson);
        return this;
    }

    // Add a deleteMany operation
    public MongoBulkOps DeleteMany(MongoQuery filter) {
        char filterJson[512];
        filter.ToString(filterJson, sizeof(filterJson));

        char opJson[512];
        Format(opJson, sizeof(opJson), "{\"deleteMany\":{\"filter\":%s}}", filterJson);
        this.PushString(opJson);
        return this;
    }

    // Add a replaceOne operation
    public MongoBulkOps ReplaceOne(MongoQuery filter, MongoDocument replacement, bool upsert = false) {
        char filterJson[512], replaceJson[512];
        filter.ToString(filterJson, sizeof(filterJson));
        replacement.ToString(replaceJson, sizeof(replaceJson));

        char opJson[1024];
        Format(opJson, sizeof(opJson), "{\"replaceOne\":{\"filter\":%s,\"replacement\":%s,\"upsert\":%s}}",
               filterJson, replaceJson, upsert ? "true" : "false");
        this.PushString(opJson);
        return this;
    }
}

/**
 * MongoDB Performance Monitor
 */
methodmap MongoPerformance {
    // Get total operations count
    public static int GetTotalOperations() {
        return MongoDB_GetTotalOperations();
    }

    // Get successful operations count
    public static int GetSuccessfulOperations() {
        return MongoDB_GetSuccessfulOperations();
    }

    // Get failed operations count
    public static int GetFailedOperations() {
        return MongoDB_GetFailedOperations();
    }

    // Get average execution time in centiseconds
    public static int GetAverageExecutionTime() {
        return MongoDB_GetAverageExecutionTime();
    }

    // Get success rate as percentage (0-100)
    public static int GetSuccessRate() {
        return MongoDB_GetSuccessRate();
    }

    // Reset all performance metrics
    public static void Reset() {
        MongoDB_ResetPerformanceMetrics();
    }

    // Get performance summary
    public static void GetSummary(char[] buffer, int maxlen) {
        int total = MongoDB_GetTotalOperations();
        int successful = MongoDB_GetSuccessfulOperations();
        int failed = MongoDB_GetFailedOperations();
        int avgTime = MongoDB_GetAverageExecutionTime();
        int successRate = MongoDB_GetSuccessRate();

        Format(buffer, maxlen, "Operations: %d (Success: %d, Failed: %d), Success Rate: %d%%, Avg Time: %d.%02ds",
               total, successful, failed, successRate, avgTime / 100, avgTime % 100);
    }
}

/**
 * MongoDB Error Handler
 */
methodmap MongoError {
    // Get last error code
    public static int GetCode() {
        return MongoDB_GetLastErrorCode();
    }

    // Get last error message
    public static void GetMessage(char[] buffer, int maxlen) {
        MongoDB_GetLastErrorMessage(buffer, maxlen);
    }

    // Get last error details
    public static void GetDetails(char[] buffer, int maxlen) {
        MongoDB_GetLastErrorDetails(buffer, maxlen);
    }

    // Get last error timestamp
    public static int GetTimestamp() {
        return MongoDB_GetLastErrorTimestamp();
    }

    // Check if there's an active error
    public static bool HasError() {
        return MongoDB_GetLastErrorCode() != 0;
    }

    // Get formatted error string
    public static void GetFormatted(char[] buffer, int maxlen) {
        int code = MongoDB_GetLastErrorCode();
        if (code == 0) {
            strcopy(buffer, maxlen, "No error");
            return;
        }

        char message[256], details[512];
        MongoDB_GetLastErrorMessage(message, sizeof(message));
        MongoDB_GetLastErrorDetails(details, sizeof(details));

        if (strlen(details) > 0) {
            Format(buffer, maxlen, "Error %d: %s - %s", code, message, details);
        } else {
            Format(buffer, maxlen, "Error %d: %s", code, message);
        }
    }
}

/**
 * Simple callback types for asynchronous operations
 */
typedef MongoCallback = function void (bool success, any data);

//=============================================================================
// USAGE EXAMPLES AND BEST PRACTICES
//=============================================================================

/**
 * QUICK START GUIDE:
 *
 * 1. Load Configuration:
 *    MongoDB_LoadConfig("configs/mongodb.cfg");
 *
 * 2. Create Connection:
 *    MongoConnection conn = MongoConnection.FromConfig();
 *
 * 3. Get Collection:
 *    MongoCollection players = conn.GetCollection("gamedb", "players");
 *
 * 4. Perform Operations:
 *    // Insert
 *    char insertedId[64];
 *    players.InsertOneJSON("{\"name\":\"John\",\"score\":100}", insertedId, sizeof(insertedId));
 *
 *    // Find
 *    StringMap result = players.FindOneJSON("{\"name\":\"John\"}");
 *    if (result != null) delete result;
 *
 *    // Update
 *    StringMap filter = new StringMap();
 *    filter.SetString("name", "John");
 *    StringMap update = new StringMap();
 *    update.SetValue("score", 200);
 *    players.UpdateOne(filter, update);
 *    delete filter; delete update;
 *
 * 5. Clean Up:
 *    conn.Close();
 *
 * PERFORMANCE TIPS:
 * - Use batch operations (InsertMany, BulkWrite) for multiple documents
 * - Create indexes on frequently queried fields
 * - Use projection to limit returned data
 * - Monitor performance with MongoPerformance methodmap
 *
 * ERROR HANDLING:
 * - Always check return values for critical operations
 * - Use MongoError methodmap for detailed error information
 * - Implement retry logic for network-related failures
 *
 * MEMORY MANAGEMENT:
 * - Always delete StringMap and ArrayList objects when done
 * - Close connections to prevent resource leaks
 * - Use try-finally patterns for guaranteed cleanup
 *
 * SECURITY CONSIDERATIONS:
 * - Configure API keys in the configuration file
 * - Use SSL/TLS for production deployments
 * - Validate and sanitize user input before database operations
 * - Implement proper authentication and authorization
 */
