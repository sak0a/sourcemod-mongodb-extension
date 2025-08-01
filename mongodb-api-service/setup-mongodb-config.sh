#!/bin/bash

# MongoDB Configuration Setup Script
# Helps configure MongoDB connection strings for different environments

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_question() {
    echo -e "${BLUE}[QUESTION]${NC} $1"
}

print_status "MongoDB API Service Configuration Setup"
echo "========================================"

# Ask for environment
echo ""
print_question "Which environment are you configuring?"
echo "1) Development (local)"
echo "2) Production (remote server)"
echo "3) MongoDB Atlas (cloud)"
echo "4) Custom"
read -p "Enter choice (1-4): " env_choice

# Ask for MongoDB details
echo ""
print_question "MongoDB Connection Details:"

case $env_choice in
    1)
        print_status "Configuring for Development (local MongoDB)"
        MONGODB_HOST="localhost"
        MONGODB_PORT="27017"
        MONGODB_USERNAME=""
        MONGODB_PASSWORD=""
        MONGODB_AUTH_DB=""
        MONGODB_DATABASE="gamedb"
        ;;
    2)
        print_status "Configuring for Production"
        read -p "MongoDB Host: " MONGODB_HOST
        read -p "MongoDB Port (default 27017): " MONGODB_PORT
        MONGODB_PORT=${MONGODB_PORT:-27017}
        read -p "Username: " MONGODB_USERNAME
        read -s -p "Password: " MONGODB_PASSWORD
        echo ""
        read -p "Auth Database (default admin): " MONGODB_AUTH_DB
        MONGODB_AUTH_DB=${MONGODB_AUTH_DB:-admin}
        read -p "Database Name (default gamedb): " MONGODB_DATABASE
        MONGODB_DATABASE=${MONGODB_DATABASE:-gamedb}
        ;;
    3)
        print_status "Configuring for MongoDB Atlas"
        read -p "Atlas Connection String (mongodb+srv://...): " ATLAS_URI
        MONGODB_URI=$ATLAS_URI
        ;;
    4)
        print_status "Custom Configuration"
        read -p "Full MongoDB URI: " MONGODB_URI
        ;;
esac

# Build MongoDB URI if not provided
if [ -z "$MONGODB_URI" ]; then
    if [ -n "$MONGODB_USERNAME" ] && [ -n "$MONGODB_PASSWORD" ]; then
        if [ -n "$MONGODB_AUTH_DB" ]; then
            MONGODB_URI="mongodb://${MONGODB_USERNAME}:${MONGODB_PASSWORD}@${MONGODB_HOST}:${MONGODB_PORT}/${MONGODB_DATABASE}?authSource=${MONGODB_AUTH_DB}"
        else
            MONGODB_URI="mongodb://${MONGODB_USERNAME}:${MONGODB_PASSWORD}@${MONGODB_HOST}:${MONGODB_PORT}/${MONGODB_DATABASE}"
        fi
    else
        MONGODB_URI="mongodb://${MONGODB_HOST}:${MONGODB_PORT}/${MONGODB_DATABASE}"
    fi
fi

# Ask for API service settings
echo ""
print_question "API Service Settings:"
read -p "Port (default 3300): " API_PORT
API_PORT=${API_PORT:-3300}

read -p "Host (default 0.0.0.0): " API_HOST
API_HOST=${API_HOST:-0.0.0.0}

# Generate API key
API_KEY=$(openssl rand -hex 32 2>/dev/null || echo "your-secret-api-key-$(date +%s)")

# Create environment file
ENV_FILE=".env.production"
if [ "$env_choice" = "1" ]; then
    ENV_FILE=".env.development"
fi

print_status "Creating $ENV_FILE..."

cat > $ENV_FILE << EOF
# MongoDB API Service Configuration
# Generated on $(date)

# Server Configuration
PORT=$API_PORT
HOST=$API_HOST
NODE_ENV=production

# CORS Configuration
CORS_ORIGIN=http://localhost:3000,http://127.0.0.1:3000

# Rate Limiting
RATE_LIMIT_WINDOW=900000
RATE_LIMIT_MAX=100

# MongoDB Connection
MONGODB_URI=$MONGODB_URI

# Connection Pool Settings
MONGODB_MAX_POOL_SIZE=10
MONGODB_MIN_POOL_SIZE=2
MONGODB_MAX_IDLE_TIME_MS=30000
MONGODB_SERVER_SELECTION_TIMEOUT_MS=5000
MONGODB_SOCKET_TIMEOUT_MS=45000

# API Security
API_KEY=$API_KEY

# Logging
LOG_LEVEL=info
EOF

print_status "Configuration saved to $ENV_FILE"
echo ""
print_status "Summary:"
echo "  MongoDB URI: ${MONGODB_URI%%@*}@***" # Hide credentials
echo "  API Service: http://$API_HOST:$API_PORT"
echo "  Environment File: $ENV_FILE"
echo ""
print_status "To start the service:"
echo "  ./start-production.sh"
echo ""
print_warning "Remember to:"
echo "  1. Update your SourceMod extension config to use: http://$API_HOST:$API_PORT"
echo "  2. Ensure MongoDB server is accessible from this machine"
echo "  3. Test the connection before deploying to production"
