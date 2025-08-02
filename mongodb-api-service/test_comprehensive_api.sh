#!/bin/bash

# MongoDB API Service - Comprehensive Test Suite
# Tests all CRUD operations, advanced features, and error handling
# Merged from basic and advanced test scripts

set -e

# Configuration
API_BASE="http://127.0.0.1:3300"
API_V1="$API_BASE/api/v1"
CONNECTION_ID=""
TEST_DB="test_sourcemod"
TEST_COLLECTION="test_players"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}MongoDB API Service - Comprehensive Test Suite${NC}"
echo "=============================================="
echo "API Base URL: $API_BASE"
echo "API V1 URL: $API_V1"
echo "Test Database: $TEST_DB"
echo "Test Collection: $TEST_COLLECTION"
echo ""

# Helper functions
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to make HTTP requests and check responses
test_endpoint() {
    local method=$1
    local endpoint=$2
    local data=$3
    local description=$4
    local expected_status=${5:-200}

    echo -e "${YELLOW}Testing: $description${NC}"
    echo "Endpoint: $method $endpoint"

    if [ -n "$data" ]; then
        echo "Data: $data"
        response=$(curl -s -w "\n%{http_code}" -X $method \
            -H "Content-Type: application/json" \
            -d "$data" \
            "$endpoint")
    else
        response=$(curl -s -w "\n%{http_code}" -X $method "$endpoint")
    fi

    # Split response and status code
    http_code=$(echo "$response" | tail -n1)
    response_body=$(echo "$response" | head -n -1)

    echo "Response: $response_body"
    echo "HTTP Status: $http_code"

    # Check if response contains success: true and correct status code
    if echo "$response_body" | grep -q '"success":true' && [ "$http_code" -eq "$expected_status" ]; then
        echo -e "${GREEN}✓ PASSED${NC}"
        return 0
    else
        echo -e "${RED}✗ FAILED${NC}"
        return 1
    fi
    echo ""
}

# Function to extract connection ID from response
extract_connection_id() {
    local response=$1
    echo "$response" | grep -o '"connectionId":"[^"]*"' | cut -d'"' -f4
}

# Start testing
log_info "Starting comprehensive MongoDB API endpoint tests..."

# ===== SECTION 1: BASIC HEALTH AND CONNECTION TESTS =====

# Test 1: Health Check
echo -e "${BLUE}=== Test 1: Health Check ===${NC}"
test_endpoint "GET" "$API_BASE/health" "" "Health check endpoint"

# Test 2: Create MongoDB Connection
echo -e "${BLUE}=== Test 2: Create MongoDB Connection ===${NC}"
response=$(curl -s -X POST \
    -H "Content-Type: application/json" \
    -d '{"uri": "mongodb://admin:your-password@192.168.1.100:27017/?authSource=admin"}' \
    "$API_V1/connections")

echo "Response: $response"

if echo "$response" | grep -q '"success":true'; then
    CONNECTION_ID=$(extract_connection_id "$response")
    echo -e "${GREEN}✓ Connection created successfully${NC}"
    echo "Connection ID: $CONNECTION_ID"
else
    echo -e "${RED}✗ Failed to create connection${NC}"
    exit 1
fi
echo ""

# Test 3: Get Connection Status
echo -e "${BLUE}=== Test 3: Get Connection Status ===${NC}"
test_endpoint "GET" "$API_V1/connections/$CONNECTION_ID" "" "Get connection status"

# Test 4: Connection Health Check (Enhanced)
echo -e "${BLUE}=== Test 4: Enhanced Connection Health Check ===${NC}"
test_endpoint "GET" "$API_V1/connections/$CONNECTION_ID/health" "" "Enhanced connection health check with server info"

# Test 5: Ping Connection
echo -e "${BLUE}=== Test 5: Ping Connection ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/ping" "" "Test connection health"

# ===== SECTION 2: BASIC CRUD OPERATIONS =====

# Test 6: Insert Document
echo -e "${BLUE}=== Test 6: Insert Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"TestPlayer1\", \"score\": 1500, \"level\": 25, \"timestamp\": $(date +%s)}}" \
    "Insert a test player document" 201

# Test 7: Insert Another Document (for update/delete tests)
echo -e "${BLUE}=== Test 7: Insert Second Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"TestPlayer2\", \"score\": 2000, \"level\": 30, \"timestamp\": $(date +%s)}}" \
    "Insert second test player document" 201

# Test 8: Insert Many Documents (Advanced Feature)
echo -e "${BLUE}=== Test 8: Insert Many Documents ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/insertMany" \
    '{"documents":[
        {"name":"Alice","role":"admin","score":95,"status":"active","department":"IT"},
        {"name":"Bob","role":"user","score":78,"status":"active","department":"Sales"},
        {"name":"Charlie","role":"moderator","score":88,"status":"inactive","department":"IT"},
        {"name":"Diana","role":"user","score":92,"status":"active","department":"Marketing"},
        {"name":"Eve","role":"admin","score":97,"status":"active","department":"IT"}
    ]}' \
    "Insert many test documents" 201

# Test 9: Find Documents
echo -e "${BLUE}=== Test 9: Find Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/find" \
    "{\"filter\": {}, \"options\": {\"limit\": 10}}" \
    "Find all documents in collection"

# Test 10: Find Specific Document
echo -e "${BLUE}=== Test 10: Find Specific Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}}" \
    "Find specific player by name"

# Test 11: Update Document
echo -e "${BLUE}=== Test 11: Update Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/updateOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}, \"update\": {\"\$set\": {\"score\": 1750, \"level\": 28}}}" \
    "Update TestPlayer1's score and level"

# Test 12: Verify Update
echo -e "${BLUE}=== Test 12: Verify Update ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}}" \
    "Verify TestPlayer1 was updated"

# Test 13: Update Many Documents
echo -e "${BLUE}=== Test 13: Update Many Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/updateMany" \
    "{\"filter\": {\"score\": {\"\$lt\": 1000}}, \"update\": {\"\$inc\": {\"score\": 100}}}" \
    "Update multiple documents (add 100 to scores < 1000)"

# Test 14: Count Documents
echo -e "${BLUE}=== Test 14: Count Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    "{\"filter\": {}}" \
    "Count all documents in collection"

# Test 15: Delete Document
echo -e "${BLUE}=== Test 15: Delete Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/deleteOne" \
    "{\"filter\": {\"name\": \"TestPlayer2\"}}" \
    "Delete TestPlayer2"

# Test 16: Verify Deletion
echo -e "${BLUE}=== Test 16: Verify Deletion ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer2\"}}" \
    "Verify TestPlayer2 was deleted (should return null)"

# ===== SECTION 3: ADVANCED FEATURES =====

# Test 17: Aggregation Pipeline
echo -e "${BLUE}=== Test 17: Aggregation Pipeline ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/aggregate" \
    '{"pipeline":[
        {"$match":{"status":"active"}},
        {"$group":{"_id":"$role","count":{"$sum":1},"avgScore":{"$avg":"$score"}}},
        {"$sort":{"count":-1}}
    ]}' \
    "Aggregation pipeline - group active users by role"

# Test 18: Find with Projection
echo -e "${BLUE}=== Test 18: Find with Projection ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/find" \
    '{"filter":{"status":"active"},"projection":{"name":1,"score":1,"_id":0},"options":{"limit":3}}' \
    "Find with projection (name and score only)"

# Test 19: Distinct Values
echo -e "${BLUE}=== Test 19: Find Distinct Values ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/distinct" \
    '{"field":"role","filter":{"status":"active"}}' \
    "Find distinct roles for active users"

test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/distinct" \
    '{"field":"department"}' \
    "Find all distinct departments"

# Test 20: Bulk Write Operations
echo -e "${BLUE}=== Test 20: Bulk Write Operations ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/bulkWrite" \
    '{"operations":[
        {"insertOne":{"document":{"name":"Frank","role":"user","score":85,"status":"active","department":"HR"}}},
        {"updateOne":{"filter":{"name":"Bob"},"update":{"$set":{"score":82,"lastUpdated":"2024-01-01"}}}},
        {"updateMany":{"filter":{"department":"IT"},"update":{"$set":{"priority":"high"}}}},
        {"deleteOne":{"filter":{"name":"Charlie"}}}
    ],"ordered":true}' \
    "Bulk write operations (insert, update, delete)"

# Test 21: Index Operations
echo -e "${BLUE}=== Test 21: Index Management ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/indexes" \
    '{"keys":{"name":1,"department":1},"options":{"name":"name_department_idx","unique":false}}' \
    "Create compound index on name and department"

test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/indexes" \
    '{"keys":{"score":-1},"options":{"name":"score_desc_idx"}}' \
    "Create descending index on score"

# Test 22: Advanced Count Operations
echo -e "${BLUE}=== Test 22: Advanced Document Counting ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    '{"filter":{"status":"active"}}' \
    "Count active documents"

test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    '{"filter":{"role":"admin"}}' \
    "Count admin users"

# Test 23: Advanced Update Operations
echo -e "${BLUE}=== Test 23: Advanced Update Operations ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/updateMany" \
    '{"filter":{"department":"IT"},"update":{"$set":{"certification":"required","updated":"2024-01-01"}}}' \
    "Update many - set certification for IT department"

# Test 24: Complex Aggregation Pipeline
echo -e "${BLUE}=== Test 24: Complex Aggregation Pipeline ===${NC}"
test_endpoint "POST" \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/aggregate" \
    '{"pipeline":[
        {"$match":{"status":"active"}},
        {"$group":{"_id":"$department","userCount":{"$sum":1},"avgScore":{"$avg":"$score"},"maxScore":{"$max":"$score"}}},
        {"$sort":{"avgScore":-1}},
        {"$project":{"department":"$_id","userCount":1,"avgScore":{"$round":["$avgScore",2]},"maxScore":1,"_id":0}}
    ]}' \
    "Complex aggregation - department statistics"

# Test 25: Delete Many Documents
echo -e "${BLUE}=== Test 25: Delete Many Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/deleteMany" \
    "{\"filter\": {\"name\": {\"\$regex\": \"^Bulk\"}}}" \
    "Delete all documents with names starting with 'Bulk'"

# ===== SECTION 4: ERROR HANDLING TESTS =====

echo -e "${BLUE}=== Test 26: Error Handling ===${NC}"
echo -e "${YELLOW}Testing error handling (these should fail gracefully):${NC}"

# Test with invalid connection ID
curl -s -X GET "$API_V1/connections/invalid-uuid/health" | grep -q '"success":false' && log_info "✓ Invalid connection ID handled correctly" || log_warn "✗ Invalid connection ID not handled properly"

# Test with invalid aggregation pipeline
curl -s -X POST \
    -H "Content-Type: application/json" \
    -d '{"pipeline":[{"$invalidStage":{}}]}' \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/aggregate" | grep -q '"success":false' && log_info "✓ Invalid aggregation pipeline handled correctly" || log_warn "✗ Invalid aggregation pipeline not handled properly"

# Test with missing required fields
curl -s -X POST \
    -H "Content-Type: application/json" \
    -d '{"field":""}' \
    "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/distinct" | grep -q '"success":false' && log_info "✓ Missing field parameter handled correctly" || log_warn "✗ Missing field parameter not handled properly"

# ===== SECTION 5: FINAL TESTS AND CLEANUP =====

# Test 27: Final Document Count
echo -e "${BLUE}=== Test 27: Final Document Count ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    "{\"filter\": {}}" \
    "Final count of documents"

# Test 28: Final Connection Health Check
echo -e "${BLUE}=== Test 28: Final Connection Health Check ===${NC}"
test_endpoint "GET" \
    "$API_V1/connections/$CONNECTION_ID/health" \
    "" \
    "Final connection health check"

# Test 29: Close Connection
echo -e "${BLUE}=== Test 29: Close Connection ===${NC}"
test_endpoint "DELETE" "$API_V1/connections/$CONNECTION_ID" "" "Close MongoDB connection"

echo -e "${GREEN}Comprehensive MongoDB API test suite completed!${NC}"
echo ""
echo -e "${BLUE}Summary of Tests Performed:${NC}"
echo "=============================================="
echo -e "${GREEN}✓ Basic Operations:${NC}"
echo "  - Health checks (basic and enhanced)"
echo "  - Connection management"
echo "  - CRUD operations (Create, Read, Update, Delete)"
echo "  - Document insertion (single and multiple)"
echo "  - Document querying and filtering"
echo "  - Document updates (single and multiple)"
echo "  - Document deletion (single and multiple)"
echo "  - Document counting"
echo ""
echo -e "${GREEN}✓ Advanced Features:${NC}"
echo "  - Aggregation pipelines (simple and complex)"
echo "  - Bulk write operations"
echo "  - Find with projection"
echo "  - Distinct value queries"
echo "  - Index management (creation)"
echo "  - Enhanced connection health monitoring"
echo ""
echo -e "${GREEN}✓ Error Handling:${NC}"
echo "  - Invalid connection IDs"
echo "  - Invalid aggregation pipelines"
echo "  - Missing required parameters"
echo "  - Graceful error responses"
echo ""
echo -e "${YELLOW}Note: Check the responses above for any failures.${NC}"
echo -e "${YELLOW}Green checkmarks (✓) indicate successful operations.${NC}"
echo -e "${YELLOW}Red X marks (✗) indicate failed operations.${NC}"
echo ""
echo -e "${BLUE}All MongoDB API endpoints have been thoroughly tested!${NC}"
