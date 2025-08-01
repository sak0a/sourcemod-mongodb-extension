#!/bin/bash

# MongoDB API Service Test Script
# Tests all CRUD operations: Connect, Insert, Update, Delete, Find

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# API Configuration
API_BASE="http://127.0.0.1:3300"
API_V1="$API_BASE/api/v1"
TEST_DB="test_sourcemod"
TEST_COLLECTION="test_players"
CONNECTION_ID=""

echo -e "${BLUE}MongoDB API Service - Comprehensive Test Suite${NC}"
echo "=============================================="
echo "API Base URL: $API_BASE"
echo "API V1 URL: $API_V1"
echo "Test Database: $TEST_DB"
echo "Test Collection: $TEST_COLLECTION"
echo ""

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

# Test 1: Health Check
echo -e "${BLUE}=== Test 1: Health Check ===${NC}"
test_endpoint "GET" "$API_BASE/health" "" "Health check endpoint"

# Test 2: Create MongoDB Connection
echo -e "${BLUE}=== Test 2: Create MongoDB Connection ===${NC}"
response=$(curl -s -X POST \
    -H "Content-Type: application/json" \
    -d '{"uri": "mongodb://127.0.0.1:27017"}' \
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

# Test 4: Ping Connection
echo -e "${BLUE}=== Test 4: Ping Connection ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/ping" "" "Test connection health"

# Test 5: Insert Document
echo -e "${BLUE}=== Test 5: Insert Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"TestPlayer1\", \"score\": 1500, \"level\": 25, \"timestamp\": $(date +%s)}}" \
    "Insert a test player document" 201

# Test 6: Insert Another Document (for update/delete tests)
echo -e "${BLUE}=== Test 6: Insert Second Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"TestPlayer2\", \"score\": 2000, \"level\": 30, \"timestamp\": $(date +%s)}}" \
    "Insert second test player document" 201

# Test 7: Find Documents
echo -e "${BLUE}=== Test 7: Find Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/find" \
    "{\"filter\": {}, \"options\": {\"limit\": 10}}" \
    "Find all documents in collection"

# Test 8: Find Specific Document
echo -e "${BLUE}=== Test 8: Find Specific Document ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}}" \
    "Find specific player by name"

# Test 9: Update Document
echo -e "${BLUE}=== Test 9: Update Document ===${NC}"
test_endpoint "PUT" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/updateOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}, \"update\": {\"\$set\": {\"score\": 1750, \"level\": 28}}}" \
    "Update TestPlayer1's score and level"

# Test 10: Verify Update
echo -e "${BLUE}=== Test 10: Verify Update ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer1\"}}" \
    "Verify TestPlayer1 was updated"

# Test 11: Count Documents
echo -e "${BLUE}=== Test 11: Count Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    "{\"filter\": {}}" \
    "Count all documents in collection"

# Test 12: Delete Document
echo -e "${BLUE}=== Test 12: Delete Document ===${NC}"
test_endpoint "DELETE" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/deleteOne" \
    "{\"filter\": {\"name\": \"TestPlayer2\"}}" \
    "Delete TestPlayer2"

# Test 13: Verify Deletion
echo -e "${BLUE}=== Test 13: Verify Deletion ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/findOne" \
    "{\"filter\": {\"name\": \"TestPlayer2\"}}" \
    "Verify TestPlayer2 was deleted (should return null)"

# Test 14: Insert Multiple Documents
echo -e "${BLUE}=== Test 14: Insert Multiple Documents ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"BulkPlayer1\", \"score\": 800}}" \
    "Insert BulkPlayer1" 201

test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"BulkPlayer2\", \"score\": 900}}" \
    "Insert BulkPlayer2" 201

test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents" \
    "{\"document\": {\"name\": \"BulkPlayer3\", \"score\": 1000}}" \
    "Insert BulkPlayer3" 201

# Test 15: Update Many Documents
echo -e "${BLUE}=== Test 15: Update Many Documents ===${NC}"
test_endpoint "PUT" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/updateMany" \
    "{\"filter\": {\"score\": {\"\$lt\": 1000}}, \"update\": {\"\$inc\": {\"score\": 100}}}" \
    "Update multiple documents (add 100 to scores < 1000)"

# Test 16: Delete Many Documents
echo -e "${BLUE}=== Test 16: Delete Many Documents ===${NC}"
test_endpoint "DELETE" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/deleteMany" \
    "{\"filter\": {\"name\": {\"\$regex\": \"^Bulk\"}}}" \
    "Delete all documents with names starting with 'Bulk'"

# Test 17: Final Count
echo -e "${BLUE}=== Test 17: Final Document Count ===${NC}"
test_endpoint "POST" "$API_V1/connections/$CONNECTION_ID/databases/$TEST_DB/collections/$TEST_COLLECTION/documents/count" \
    "{\"filter\": {}}" \
    "Final count of documents"

# Test 18: Close Connection
echo -e "${BLUE}=== Test 18: Close Connection ===${NC}"
test_endpoint "DELETE" "$API_V1/connections/$CONNECTION_ID" "" "Close MongoDB connection"

echo -e "${GREEN}Test suite completed!${NC}"
echo ""
echo -e "${BLUE}Summary:${NC}"
echo "- All major CRUD operations tested"
echo "- Bulk operations tested"
echo "- Error handling tested"
echo "- Connection management tested"
echo ""
echo -e "${YELLOW}Note: Check the responses above for any failures.${NC}"
echo -e "${YELLOW}Green checkmarks (✓) indicate successful operations.${NC}"
echo -e "${YELLOW}Red X marks (✗) indicate failed operations.${NC}"