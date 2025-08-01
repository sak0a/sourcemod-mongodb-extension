#!/bin/bash

# MongoDB API Service - Production Startup Script

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

print_status "Starting MongoDB API Service in Production Mode..."

# Check if .env.production exists
if [ ! -f ".env.production" ]; then
    print_error ".env.production file not found!"
    print_error "Please copy .env.example to .env.production and configure it"
    exit 1
fi

# Load production environment
export $(cat .env.production | grep -v '^#' | xargs)

# Validate required environment variables
if [ -z "$MONGODB_URI" ]; then
    print_error "MONGODB_URI is not set in .env.production"
    print_error "Please set your MongoDB connection string"
    exit 1
fi

if [ -z "$PORT" ]; then
    print_warning "PORT not set, using default 3300"
    export PORT=3300
fi

if [ -z "$HOST" ]; then
    print_warning "HOST not set, using default 0.0.0.0"
    export HOST=0.0.0.0
fi

print_status "Configuration:"
print_status "  Port: $PORT"
print_status "  Host: $HOST"
print_status "  MongoDB: ${MONGODB_URI%%@*}@***" # Hide credentials in log
print_status "  Environment: $NODE_ENV"

# Create logs directory if it doesn't exist
mkdir -p logs

# Start the service
print_status "Starting service..."
node dist/server.js
