# Discontinued as of 17.12.2025 due to relevance

# MongoDB SourceMod Extension

**Enterprise-grade MongoDB integration for SourceMod with comprehensive security and advanced features.**

## 🚀 **Current Status: ENTERPRISE PRODUCTION READY**

- ✅ **Minimal Extension**: 133KB (no libcurl dependencies)
- ✅ **Full Extension**: 1.8MB (with libcurl)
- ✅ **Real MongoDB Integration**: Connects to live MongoDB servers
- ✅ **Secure HTTP API Service**: Node.js bridge with enterprise security
- ✅ **Advanced Operations**: Aggregation, bulk operations, indexing
- ✅ **Production Security**: API keys, rate limiting, input validation
- ✅ **Comprehensive Testing**: 29 API tests + security test suite

## 🎯 **Key Features**

### **🔧 Extension Capabilities**
- ✅ **Complete CRUD Operations**: Insert, Find, Update, Delete
- ✅ **Advanced Queries**: Aggregation pipelines, projections, sorting
- ✅ **Bulk Operations**: Batch inserts, updates, deletes
- ✅ **Index Management**: Create and manage database indexes
- ✅ **Real-time Operations**: Live database connectivity
- ✅ **Error Handling**: Comprehensive error reporting and recovery
- ✅ **Performance Monitoring**: Query timing and metrics

### **🛡️ Security Features**
- ✅ **API Key Authentication**: Secure request validation
- ✅ **SourceMod Extension Verification**: Client authenticity checks
- ✅ **Rate Limiting**: DDoS protection and abuse prevention
- ✅ **Input Validation**: MongoDB injection protection
- ✅ **Security Headers**: XSS, CSRF, and clickjacking protection
- ✅ **Request Sanitization**: Malicious payload filtering
- ✅ **Comprehensive Logging**: Security event tracking

### **⚡ Performance Features**
- ✅ **Connection Pooling**: Efficient database connections
- ✅ **Request Compression**: Optimized data transfer
- ✅ **Batch Processing**: High-throughput operations
- ✅ **Query Optimization**: Performance monitoring and tuning
- ✅ **Caching Support**: Reduced database load

### **🏗️ Build Options**
1. **Minimal** (133KB): Raw sockets, no external dependencies
2. **Full** (1.8MB): libcurl-based with full HTTP features

### **🌐 Production Features**
- ✅ **Enterprise Security**: OWASP API Security Top 10 compliance
- ✅ **High Availability**: Connection pooling and auto-recovery
- ✅ **Monitoring**: Health checks and performance metrics
- ✅ **Scalability**: Multi-connection support
- ✅ **Container Ready**: Docker and Pterodactyl compatibility

## 📁 **Project Structure**

### **1. HTTP Extension** (`http_extension/`)
```
http_extension/
├── bin/http_mongodb.ext.so          # Ready-to-use extension (133KB)
├── minimal_complete_extension.cpp   # Minimal source (no libcurl)
├── complete_extension.cpp           # Full source (with libcurl + security)
├── build_extension.sh               # Build script (minimal/full)
├── CMakeLists_minimal.txt           # Minimal build config
├── CMakeLists.txt                   # Full build config
├── configs/
│   └── mongodb.cfg                  # MongoDB configuration file
└── scripting/
    ├── include/http_mongodb.inc     # SourcePawn interface (847 lines)
    ├── mongo_console_test.sp        # Console test commands
    ├── test_real_data.sp            # Real data examples
    └── advanced_examples.sp         # Advanced operation examples
```

### **2. MongoDB API Service** (`mongodb-api-service/`)
```
mongodb-api-service/
├── src/
│   ├── server.ts                    # Main API server with security
│   ├── config/
│   │   └── security.ts              # Security configuration & management
│   ├── routes/                      # API endpoints
│   │   ├── connectionRoutes.ts      # Connection management
│   │   ├── databaseRoutes.ts        # Database operations
│   │   └── batchRoutes.ts           # Bulk operations
│   ├── managers/
│   │   └── ConnectionManager.ts     # Connection pooling
│   ├── middleware/                  # Request processing
│   │   ├── auth.ts                  # Authentication & authorization
│   │   ├── security.ts              # Security middleware
│   │   ├── errorHandler.ts          # Error handling
│   │   └── requestLogger.ts         # Request logging
│   └── utils/
│       └── logger.ts                # Logging utilities
├── dist/                            # Compiled JavaScript
├── package.json                     # Dependencies
├── .env                             # Production configuration
├── .env.example                     # Configuration template
├── test_comprehensive_api.sh        # Complete API testing (29 tests)
├── test_security.sh                 # Security testing suite
└── SECURITY_FEATURES_GUIDE.md       # Comprehensive security documentation
```

## 🏗️ **Architecture & Configuration**

### **🔄 System Architecture**

```
┌─────────────────┐    HTTPS/HTTP   ┌─────────────────┐    MongoDB     ┌─────────────────┐
│   SourceMod     │    API Calls    │   Node.js API   │   Protocol     │   MongoDB       │
│   Extension     │◄───────────────►│   Service       │◄──────────────►│   Server        │
│   (32-bit)      │   + Security    │   (64-bit)      │   + Auth       │                 │
└─────────────────┘                 └─────────────────┘                └─────────────────┘
│                                   │                                  │
│ • SourcePawn plugins              │ • Security Layer                 │ • Document storage
│ • Native functions                │ • API Authentication             │ • Authentication
│ • Configuration                   │ • Rate Limiting                  │ • Replication
│ • Error handling                  │ • Input Validation               │ • Indexing
│ • Advanced Operations             │ • Connection Pooling             │ • Aggregation
└─────────────────                  └─────────────────                 └─────────────────
```

### **📋 Configuration Architecture**

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                              Configuration Layers                                  │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                     │
│  SourceMod Extension Config          API Service Config           MongoDB Server   │
│  ┌─────────────────────┐             ┌─────────────────────┐      ┌─────────────┐  │
│  │   mongodb.cfg       │             │  .env.production    │      │   MongoDB   │  │
│  │                     │             │                     │      │   Instance  │  │
│  │ • API service URL   │────────────►│ • MongoDB URI       │─────►│             │  │
│  │ • Timeouts          │             │ • Connection pool   │      │ • Users     │  │
│  │ • Default DB names  │             │ • Authentication    │      │ • Databases │  │
│  │ • Retry settings    │             │ • SSL settings      │      │ • Collections│ │
│  └─────────────────────┘             └─────────────────────┘      └─────────────┘  │
│                                                                                     │
│  ⚠️  Extension NEVER connects directly to MongoDB                                  │
│  ✅  Extension ONLY talks to API service via HTTP                                  │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### **🔧 Configuration Breakdown**

#### **SourceMod Extension Config (`configs/mongodb.cfg`)**
```javascript
"api_service" {
    "url"     "http://127.0.0.1:3300"  // WHERE to find API service
    "timeout" "30000"                   // How long to wait for responses
    "retries" "3"                       // How many times to retry
}

"database" {
    "name"               "gamedb"       // DEFAULT database name
    "players_collection" "players"      // DEFAULT collection names
}
```
**Purpose**: Tells extension HOW to communicate with API service

#### **API Service Config (`.env.production`)**
```bash
# MongoDB connection (ONLY configured here)
MONGODB_URI=mongodb://admin:password@192.168.1.100:27017/?authSource=admin

# API service settings
PORT=3300
HOST=0.0.0.0
```
**Purpose**: Tells API service HOW to connect to MongoDB

### **⚡ Data Flow Example**

```
1. SourcePawn Plugin Call:
   MongoConnection conn = new MongoConnection();
   conn.InsertOneJSON("{\"player\":\"John\",\"score\":100}");

2. Extension Processing:
   ┌─ Reads mongodb.cfg for API service URL
   ├─ Constructs HTTP request to http://127.0.0.1:3300/api/v1/...
   └─ Sends JSON data via HTTP POST

3. API Service Processing:
   ┌─ Receives HTTP request
   ├─ Reads .env.production for MongoDB URI
   ├─ Connects to MongoDB using mongodb://admin:password@...
   └─ Executes MongoDB operation

4. Response Chain:
   MongoDB → API Service → HTTP Response → Extension → SourcePawn
```

## 🚀 **Production Setup Guide**

### **📋 Prerequisites**
- MongoDB server (local, remote, or MongoDB Atlas)
- Node.js 18+ (for API service)
- SourceMod server (TF2, CS:GO, etc.)
- Basic Linux/Windows server knowledge

---

## 🏭 **Production Deployment**

### **Step 1: Setup MongoDB API Service**

#### **1.1 Clone and Prepare**
```bash
# On your server (can be same as game server or separate)
git clone <repository>
cd mongo-sourcemod/mongodb-api-service

# Install dependencies
npm install --production
```

#### **1.2 Configure MongoDB Connection**
```bash
# Interactive setup (recommended)
./setup-mongodb-config.sh

# Or manually copy and edit
cp .env.example .env
nano .env
```

**Configure your MongoDB URI in `.env`:**
```bash
# For remote MongoDB server
MONGODB_URI=mongodb://admin:your_password@your-mongodb-server:27017/?authSource=admin

# For MongoDB Atlas (cloud)
MONGODB_URI=mongodb+srv://username:password@cluster.mongodb.net/gamedb?retryWrites=true&w=majority

# For local MongoDB
MONGODB_URI=mongodb://localhost:27017/gamedb
```

#### **1.3 Start API Service**
```bash
# Production start
./start-production.sh

# Or manually
PORT=3300 HOST=0.0.0.0 node dist/server.js

# For persistent service (recommended)
pm2 start dist/server.js --name mongodb-api
pm2 save
pm2 startup
```

### **Step 2: Build and Install SourceMod Extension**

#### **2.1 Build Extension**
```bash
cd ../http_extension

# Build minimal version (recommended for production)
./build_extension.sh minimal

# Extension will be available at: bin/http_mongodb.ext.so
```

#### **2.2 Install on Game Server**
```bash
# Copy extension to your SourceMod server
scp bin/http_mongodb.ext.so your-gameserver:/path/to/sourcemod/extensions/

# Copy configuration
scp scripting/configs/mongodb.cfg your-gameserver:/path/to/sourcemod/configs/

# Copy test plugins (optional)
scp scripting/mongo_console_test.smx your-gameserver:/path/to/sourcemod/plugins/
```

### **Step 3: Configure SourceMod Extension**

#### **3.1 Edit MongoDB Configuration**
```bash
# On your game server
nano /path/to/sourcemod/configs/mongodb.cfg
```

**Update the API service URL:**
```javascript
"api_service"
{
    // Point to your API service
    "url"    "http://YOUR_API_SERVER_IP:3300"

    // If API service is on same server
    "url"    "http://127.0.0.1:3300"

    // If using external server
    "url"    "http://192.168.1.100:3300"
}
```

#### **3.2 Load Extension**
```bash
# In SourceMod console or server console
sm exts load http_mongodb

# Verify it loaded
sm exts list | grep mongodb
```

### **Step 4: Test the Setup**

#### **4.1 Load Test Plugin**
```bash
# In SourceMod console
sm plugins load mongo_console_test

# Test basic functionality
mongo_test
mongo_insert TestPlayer
mongo_count
```

#### **4.2 Verify API Service**
```bash
# Test API service directly
curl http://YOUR_API_SERVER_IP:3300/health

# Test MongoDB connection
curl -X POST http://YOUR_API_SERVER_IP:3300/api/v1/connections \
  -H "Content-Type: application/json" \
  -d '{"uri":"your-mongodb-uri"}'
```

## 🔄 **Development Workflow**

### **📝 Typical Development Workflow**

```
┌─────────────────────────────────────────────────────────────────────────────────────┐
│                            Development Workflow                                    │
├─────────────────────────────────────────────────────────────────────────────────────┤
│                                                                                     │
│  1. Setup API Service           2. Configure MongoDB        3. Build Extension     │
│  ┌─────────────────────┐       ┌─────────────────────┐     ┌─────────────────────┐ │
│  │ cd mongodb-api-     │       │ Edit .env.development│     │ cd http_extension   │ │
│  │   service           │       │                     │     │                     │ │
│  │ ./setup-mongodb-    │──────►│ MONGODB_URI=        │────►│ ./build_extension.sh│ │
│  │   config.sh         │       │   mongodb://...     │     │   minimal           │ │
│  │ npm run dev         │       │ PORT=3300           │     │                     │ │
│  └─────────────────────┘       └─────────────────────┘     └─────────────────────┘ │
│           │                              │                              │           │
│           ▼                              ▼                              ▼           │
│  4. Install Extension          5. Configure Extension      6. Test & Debug        │
│  ┌─────────────────────┐       ┌─────────────────────┐     ┌─────────────────────┐ │
│  │ Copy .ext.so to     │       │ Edit mongodb.cfg    │     │ sm exts load        │ │
│  │   /sourcemod/       │       │                     │     │   http_mongodb      │ │
│  │   extensions/       │──────►│ "url" "http://      │────►│ mongo_test          │ │
│  │                     │       │   127.0.0.1:3300"  │     │ Check logs          │ │
│  │                     │       │                     │     │                     │ │
│  └─────────────────────┘       └─────────────────────┘     └─────────────────────┘ │
│                                                                                     │
└─────────────────────────────────────────────────────────────────────────────────────┘
```

### **🔧 Configuration Workflow**

#### **Development Environment:**
```bash
# 1. API Service (Local MongoDB)
cd mongodb-api-service
echo "MONGODB_URI=mongodb://localhost:27017/gamedb_dev" > .env.development
echo "PORT=3300" >> .env.development
npm run dev

# 2. Extension Config (Point to local API)
# Edit configs/mongodb.cfg:
"api_service" { "url" "http://127.0.0.1:3300" }

# 3. Test
mongo_test
```

#### **Production Environment:**
```bash
# 1. API Service (Remote MongoDB with auth)
cd mongodb-api-service
echo "MONGODB_URI=mongodb://admin:password@prod-server:27017/?authSource=admin" > .env.production
echo "PORT=3300" >> .env.production
./start-production.sh

# 2. Extension Config (Point to production API)
# Edit configs/mongodb.cfg:
"api_service" { "url" "http://prod-api-server:3300" }

# 3. Deploy and test
mongo_test
```

### **🐛 Debug Workflow**

```
Issue: Extension won't connect
│
├─ 1. Check API Service
│  ├─ curl http://api-server:3300/health
│  ├─ Check API service logs
│  └─ Verify MongoDB connection
│
├─ 2. Check Extension Config
│  ├─ Verify URL in mongodb.cfg
│  ├─ Check SourceMod logs
│  └─ Test extension loading
│
└─ 3. Test Network
   ├─ telnet api-server 3300
   ├─ Check firewall rules
   └─ Verify port forwarding
```

### **📊 Common Configuration Scenarios**

| Scenario | API Service Location | Extension Config | MongoDB Location |
|----------|---------------------|------------------|------------------|
| **Local Dev** | `127.0.0.1:3300` | `http://127.0.0.1:3300` | `localhost:27017` |
| **Same Server** | `0.0.0.0:3300` | `http://127.0.0.1:3300` | `localhost:27017` |
| **Separate API Server** | `0.0.0.0:3300` | `http://192.168.1.100:3300` | `remote-mongo:27017` |
| **Container** | `0.0.0.0:3300` | `http://host-ip:3300` | `mongo-container:27017` |
| **Cloud** | `0.0.0.0:3300` | `http://api.domain.com:3300` | `cluster.mongodb.net` |

---

## 🛠 **Development Setup**

### **For Local Development/Testing**

#### **1. Quick Development Setup**
```bash
# Start API service locally
cd mongodb-api-service
cp .env.example .env
# Edit .env with local MongoDB
npm run dev

# Build extension
cd ../http_extension
./build_extension.sh minimal

# Test locally
# (Copy to local SourceMod installation)
```

#### **2. Development Configuration**
```bash
# .env.development
PORT=3300
HOST=127.0.0.1
MONGODB_URI=mongodb://localhost:27017/gamedb_dev

# configs/mongodb.cfg
"url"    "http://127.0.0.1:3300"
```

---

## 📁 **File Locations Summary**

### **API Service Server:**
```
/opt/mongodb-api-service/          # API service installation
├── .env.production               # MongoDB connection config
├── dist/server.js               # Compiled service
├── start-production.sh          # Startup script
└── logs/                        # Service logs
```

### **Game Server:**
```
/path/to/sourcemod/
├── extensions/
│   └── http_mongodb.ext.so      # Extension binary
├── configs/
│   └── mongodb.cfg              # Extension configuration
├── plugins/
│   └── mongo_console_test.smx   # Test plugin
└── scripting/include/
    └── http_mongodb.inc         # Include file for development
```

---

## 🔧 **Configuration Examples**

### **Production API Service (.env.production)**
```bash
PORT=3300
HOST=0.0.0.0
NODE_ENV=production
MONGODB_URI=mongodb://admin:secure_password@mongodb-server:27017/?authSource=admin
API_KEY=your-production-api-key
LOG_LEVEL=warn
```

### **Production SourceMod Config (mongodb.cfg)**
```javascript
"MongoDB Configuration"
{
    "api_service"
    {
        "url"        "http://api-server:3300"
        "timeout"    "10000"
        "retries"    "5"
        "debug"      "0"
    }
    "database"
    {
        "name"                   "production_gamedb"
        "players_collection"     "players"
        "connections_collection" "connections"
    }
}
```

---

## 🛡️ **Security Features**

### **🔐 Enterprise-Grade Security**
- **API Key Authentication**: Every request requires valid API key
- **SourceMod Extension Verification**: Validates client authenticity
- **Rate Limiting**: 1000 requests/15min with progressive slow-down
- **Input Validation**: MongoDB injection protection
- **Security Headers**: XSS, CSRF, clickjacking protection
- **Request Sanitization**: Malicious payload filtering
- **HTTPS Enforcement**: Production SSL/TLS support

### **🧪 Comprehensive Testing**
- **29 API Tests**: Complete functionality validation (`test_comprehensive_api.sh`)
- **Security Test Suite**: Authentication, authorization, rate limiting (`test_security.sh`)
- **Performance Tests**: Load testing and optimization
- **Error Handling Tests**: Failure scenario validation

### **📊 Monitoring & Logging**
- **Security Event Logging**: Authentication attempts, failures
- **Performance Metrics**: Query timing, success rates
- **Error Tracking**: Comprehensive error reporting
- **Health Monitoring**: Connection status, API health

### **🔧 Security Configuration**
```env
# API Authentication
SOURCEMOD_API_KEY=sourcemod-mongodb-extension-2024
JWT_SECRET=your-super-secret-jwt-key-change-this-in-production
ENCRYPTION_KEY=your-32-character-encryption-key-here

# Rate Limiting & DDoS Protection
RATE_LIMIT_WINDOW=900000          # 15 minutes
RATE_LIMIT_MAX=1000               # Max requests per window
RATE_LIMIT_SLOW_DOWN_AFTER=100    # Start slowing down after this many requests

# HTTPS & Security Headers
REQUIRE_HTTPS=false               # Set to true in production
TRUST_PROXY=1                     # Set to 1 if behind reverse proxy
```

### **🛡️ Security Benefits**
- ✅ **OWASP API Security Top 10** compliance
- ✅ **Enterprise-grade protection** against common attacks
- ✅ **Audit trail maintenance** for compliance
- ✅ **Scalable security architecture**
- ✅ **Performance optimized** security checks

---

## 🚀 **Quick Start (TL;DR)**

```bash
# 1. Setup API service with security
cd mongodb-api-service
cp .env.example .env
# Edit .env with your MongoDB credentials and security settings
npm install && npm run build && npm start

# 2. Run comprehensive tests
./test_comprehensive_api.sh    # 29 API functionality tests
./test_security.sh             # Security feature tests

# 3. Build extension
cd ../http_extension
./build_extension.sh minimal

# 4. Install on game server
scp bin/http_mongodb.ext.so gameserver:/sourcemod/extensions/
scp configs/mongodb.cfg gameserver:/sourcemod/configs/mongodb.cfg

# 5. Configure and test
# Edit mongodb.cfg with your API service URL
# Load extension: sm exts load http_mongodb
# Test: mongo_test
```

## 💻 **Usage Examples**

### **Console Commands** (Ready to use)
```bash
mongo_test                    # Test connection and insert
mongo_insert "TestPlayer"     # Insert single player
mongo_batch 25               # Insert 25 mock players
mongo_count                  # Count total documents
mongo_find "PlayerName"      # Find specific player
mongo_stats                  # Database statistics
```

### **SourcePawn Code**
```sourcepawn
#include <http_mongodb>

public void OnPluginStart() {
    RegConsoleCmd("save_player", Command_SavePlayer);
}

public Action Command_SavePlayer(int client, int args) {
    MongoConnection conn = new MongoConnection("http://127.0.0.1:3300");
    MongoCollection players = conn.GetCollection("gamedb", "players");
    
    // Get player data
    char playerName[MAX_NAME_LENGTH];
    GetClientName(client, playerName, sizeof(playerName));
    
    char steamId[32];
    GetClientAuthId(client, AuthId_Steam2, steamId, sizeof(steamId));
    
    // Create JSON document
    char jsonDoc[512];
    Format(jsonDoc, sizeof(jsonDoc), 
        "{\"name\":\"%s\",\"steamid\":\"%s\",\"score\":%d,\"timestamp\":%d}",
        playerName, steamId, GetClientScore(client), GetTime());
    
    // Insert document
    char insertedId[64];
    if (players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
        PrintToChat(client, "Player data saved! ID: %s", insertedId);
    }
    
    conn.Close();
    return Plugin_Handled;
}
```

## ⚡ **Advanced Operations**

### **🔍 Aggregation Pipelines**
```sourcepawn
// Complex aggregation example
MongoCollection players = conn.GetCollection("gamedb", "players");

char pipeline[1024];
Format(pipeline, sizeof(pipeline),
    "[{\"$match\":{\"status\":\"active\"}},"
    "{\"$group\":{\"_id\":\"$department\",\"avgScore\":{\"$avg\":\"$score\"}}},"
    "{\"$sort\":{\"avgScore\":-1}}]");

ArrayList results = players.Aggregate(pipeline);
// Process aggregated results
```

### **📦 Bulk Operations**
```sourcepawn
// Bulk insert multiple documents
ArrayList documents = new ArrayList();
// Add multiple documents to the list
bool success = players.BulkWrite(documents, true); // ordered=true
```

### **🔍 Advanced Queries**
```sourcepawn
// Find with projection (select specific fields)
char filter[256], projection[256];
Format(filter, sizeof(filter), "{\"score\":{\"$gte\":1000}}");
Format(projection, sizeof(projection), "{\"name\":1,\"score\":1,\"_id\":0}");

ArrayList results = players.FindWithProjection(filter, projection);

// Get distinct values
ArrayList distinctValues = players.FindDistinct("department", "{}");

// Count documents with filter
int count = players.CountDocuments("{\"status\":\"active\"}");
```

### **📊 Index Management**
```sourcepawn
// Create index for better query performance
char keys[128], options[128];
Format(keys, sizeof(keys), "{\"steamid\":1,\"score\":-1}");
Format(options, sizeof(options), "{\"name\":\"steamid_score_idx\"}");

bool indexCreated = players.CreateIndex(keys, options);
```

### **🔄 Enhanced Error Handling**
```sourcepawn
// Comprehensive error handling
if (!players.InsertOneJSON(jsonDoc, insertedId, sizeof(insertedId))) {
    int errorCode = MongoDB_GetLastErrorCode();
    char errorMsg[256], errorDetails[512];

    MongoDB_GetLastErrorMessage(errorMsg, sizeof(errorMsg));
    MongoDB_GetLastErrorDetails(errorDetails, sizeof(errorDetails));

    LogError("MongoDB Error %d: %s - %s", errorCode, errorMsg, errorDetails);
}
```

## 🔧 **Configuration**

### **MongoDB Connection**
The extension connects to: `mongodb://admin:***@192.168.1.100:27017/?authSource=admin`

### **API Service**
- **Host**: `0.0.0.0` (accessible from containers)
- **Port**: `3300`
- **Endpoints**: `/api/v1/connections`, `/documents`, `/count`, etc.

## 🏗️ **Deployment Scenarios**

### **Scenario 1: Single Server (Recommended for small setups)**
```
[Game Server + API Service + MongoDB]
```
- All components on one server
- Simplest setup and maintenance
- Good for development and small production

**Setup:**
```bash
# Install everything on game server
# API service: http://127.0.0.1:3300
# MongoDB: mongodb://localhost:27017
```

### **Scenario 2: Separate API Server (Recommended for production)**
```
[Game Server] ←→ [API Server + MongoDB]
```
- Game server separate from database infrastructure
- Better performance and security
- Easier to scale multiple game servers

**Setup:**
```bash
# API Server (192.168.1.100)
cd mongodb-api-service
MONGODB_URI=mongodb://localhost:27017/gamedb ./start-production.sh

# Game Server
# mongodb.cfg: "url" "http://192.168.1.100:3300"
```

### **Scenario 3: Full Distributed (Enterprise)**
```
[Game Server] ←→ [API Server] ←→ [MongoDB Cluster]
```
- Maximum scalability and reliability
- Load balancing and high availability
- Multiple game servers, API servers, and MongoDB nodes

**Setup:**
```bash
# API Servers (load balanced)
MONGODB_URI=mongodb://user:pass@mongo1:27017,mongo2:27017,mongo3:27017/gamedb?replicaSet=rs0

# Game Servers point to load balancer
# mongodb.cfg: "url" "http://api-loadbalancer:3300"
```

### **🐳 Pterodactyl/Container Support**

#### **Option A: External API Service (Recommended)**
```bash
# On host machine (outside container)
cd mongodb-api-service
./start-production.sh

# In container config
"url" "http://HOST_IP:3300"
```

#### **Option B: Sidecar Container**
```yaml
# docker-compose.yml
services:
  gameserver:
    # Your game server container
    depends_on:
      - mongodb-api

  mongodb-api:
    build: ./mongodb-api-service
    ports:
      - "3300:3300"
    environment:
      - MONGODB_URI=mongodb://mongo:27017/gamedb
    depends_on:
      - mongo

  mongo:
    image: mongo:latest
    environment:
      - MONGO_INITDB_ROOT_USERNAME=admin
      - MONGO_INITDB_ROOT_PASSWORD=password
```

## 📊 **Current Database**

- **Server**: `192.168.1.100:27017`
- **Database**: `gamedb`
- **Collections**: `players`, `connections`
- **Authentication**: Admin credentials configured
- **Status**: ✅ Live and operational

## 🛠 **Development**

### **Build Requirements**
- SourceMod SDK
- HL2SDK (TF2)
- CMake 3.10+
- GCC with C++14 support

### **🧪 Comprehensive Testing**

#### **API Service Testing**
```bash
# Complete API functionality tests (29 tests)
cd mongodb-api-service
./test_comprehensive_api.sh

# Security feature tests
./test_security.sh

# Expected output:
# ✓ Basic Operations: Health checks, CRUD operations, connection management
# ✓ Advanced Features: Aggregation, bulk operations, indexing
# ✓ Security Features: Authentication, rate limiting, input validation
# ✓ Error Handling: Invalid requests, malicious payloads
```

#### **Extension Testing**
```bash
# Compile test plugins
spcomp mongo_console_test.sp
spcomp test_real_data.sp
spcomp advanced_examples.sp

# Load and test
sm plugins load mongo_console_test
mongo_test                    # Connection and basic operations
mongo_insert "TestPlayer"     # Single document insert
mongo_find "TestPlayer"       # Document retrieval
mongo_count                   # Document counting
mongo_aggregate               # Aggregation pipeline test
mongo_bulk_insert 10          # Bulk operations test
```

## 📈 **Performance**

- **Extension Size**: 133KB (minimal) / 1.8MB (full)
- **Memory Usage**: Low (only essential MongoDB operations)
- **Dependencies**: Minimal system libraries only
- **Compatibility**: Works with TF2, CS:GO, and other Source games

## 🔍 **Troubleshooting**

### **Deployment Issues**

#### **API Service Won't Start**
```bash
# Check if port is in use
netstat -tulpn | grep :3300

# Check MongoDB connectivity
telnet your-mongodb-server 27017

# Check logs
tail -f mongodb-api-service/logs/api-service.log

# Test MongoDB URI
node -e "const { MongoClient } = require('mongodb'); MongoClient.connect('your-uri').then(() => console.log('OK')).catch(console.error)"
```

#### **Extension Won't Load**
```bash
# Check extension file
ls -la /path/to/sourcemod/extensions/http_mongodb.ext.so

# Check SourceMod logs
tail -f /path/to/sourcemod/logs/errors_*.log

# Test extension manually
sm exts load http_mongodb
sm exts list | grep mongodb
```

#### **Connection Fails Between Extension and API**
```bash
# Test API service from game server
curl http://your-api-server:3300/health

# Check firewall
ufw status
iptables -L

# Test from game server console
mongo_test
```

### **Common Issues**

#### **"Native 'MongoDB_IsConnected' was not found"**
- **Solution**: Ensure you're using the updated extension with `MongoDB_IsConnected` native
- **Check**: `strings http_mongodb.ext.so | grep MongoDB_IsConnected`

#### **"Connection refused" or "Timeout"**
- **API Service**: Check if API service is running: `curl http://api-server:3300/health`
- **Firewall**: Ensure port 3300 is open: `ufw allow 3300`
- **Network**: Test connectivity: `telnet api-server 3300`
- **Configuration**: Verify URL in `mongodb.cfg` matches API service location

#### **"MongoDB connection failed"**
- **Credentials**: Verify MongoDB username/password in `.env.production`
- **Network**: Ensure MongoDB server is accessible from API service server
- **Authentication**: Check MongoDB auth database (usually `admin`)
- **SSL**: Add `?ssl=true` to MongoDB URI if required

#### **Container/Pterodactyl Issues**
- **Use external API service**: Run API service outside container
- **Host networking**: Use host IP instead of localhost
- **Port mapping**: Ensure port 3300 is properly mapped
- **Minimal extension**: Use minimal build to avoid dependency issues

### **🔍 Debug Data Flow**

```
SourcePawn Plugin → Extension → API Service → MongoDB
      │                │            │            │
      ▼                ▼            ▼            ▼
   1. mongo_test    2. HTTP POST   3. MongoDB   4. Response
   ┌─────────────┐  ┌─────────────┐ ┌──────────┐ ┌─────────────┐
   │ Plugin Call │─►│ Extension   │►│ API      │►│ MongoDB     │
   │             │  │ Config:     │ │ Config:  │ │ Server      │
   │ MongoConn   │  │ mongodb.cfg │ │ .env     │ │             │
   │ .Insert()   │  │             │ │          │ │ Collection  │
   └─────────────┘  └─────────────┘ └──────────┘ └─────────────┘
                           │            │            │
                           ▼            ▼            ▼
                    Check URL here  Check URI here  Check auth

   Debug Points:
   ✓ 1. Extension loaded?     → sm exts list | grep mongodb
   ✓ 2. API service running?  → curl http://api:3300/health
   ✓ 3. MongoDB accessible?   → mongo mongodb://uri
   ✓ 4. Network connectivity? → telnet api-server 3300
```

### **Debug Commands**
```bash
# Check extension status
sm exts list | grep mongodb

# Test API connectivity
curl -X POST http://localhost:3300/api/v1/connections \
  -H "Content-Type: application/json" \
  -d '{"uri":"mongodb://admin:***@192.168.1.100:27017/?authSource=admin"}'

# Monitor logs
tail -f logs/sourcemod/errors_*.log
```

## 🔐 **Security Considerations**

### **Production Deployment**
- **Change default credentials** in MongoDB connection string
- **Use environment variables** for sensitive configuration
- **Enable MongoDB authentication** and SSL/TLS
- **Restrict API service access** to trusted networks only
- **Regular security updates** for all components

### **Network Security**
```bash
# Firewall rules (example)
ufw allow from 192.168.1.0/24 to any port 3300  # API service
ufw allow from 192.168.1.0/24 to any port 27017 # MongoDB
```

## 📋 **API Reference**

### **Native Functions**
```sourcepawn
// Connection Management
native Handle MongoDB_Connect(const char[] url);
native bool MongoDB_IsConnected(Handle connection);
native void MongoDB_Close(Handle connection);

// Collection Operations
native Handle MongoDB_GetCollection(Handle connection, const char[] database, const char[] collection);

// Document Operations
native bool MongoDB_InsertOneJSON(Handle collection, const char[] jsonDocument, char[] insertedId, int maxlen);
native StringMap MongoDB_FindOneJSON(Handle collection, const char[] jsonFilter);
native int MongoDB_CountDocuments(Handle collection, StringMap filter);

// Error Handling
native bool MongoDB_GetLastError(char[] buffer, int maxlen);
```

### **MethodMaps**
```sourcepawn
// Connection wrapper
methodmap MongoConnection < Handle {
    public MongoConnection(const char[] url);
    public MongoCollection GetCollection(const char[] database, const char[] collection);
    public bool IsConnected();
    public void Close();
}

// Collection wrapper
methodmap MongoCollection < Handle {
    public bool InsertOneJSON(const char[] jsonDocument, char[] insertedId, int maxlen);
    public StringMap FindOneJSON(const char[] jsonFilter);
    public int CountDocuments(StringMap filter);
}
```

## 📚 **Quick Reference**

### **Essential Commands**
```bash
# API Service
./setup-mongodb-config.sh          # Configure MongoDB connection
./start-production.sh              # Start API service
curl http://api:3300/health        # Test API service

# Extension
./build_extension.sh minimal       # Build extension
sm exts load http_mongodb          # Load extension
mongo_test                         # Test functionality

# Configuration Files
mongodb-api-service/.env.production # MongoDB URI and API settings
configs/mongodb.cfg                # SourceMod extension settings
```

### **Default Ports & URLs**
- **API Service**: `http://localhost:3300`
- **MongoDB**: `mongodb://localhost:27017`
- **SourceMod Config**: `/path/to/sourcemod/configs/mongodb.cfg`
- **Extension**: `/path/to/sourcemod/extensions/http_mongodb.ext.so`

### **Environment Variables**
```bash
# API Service
MONGODB_URI=mongodb://user:pass@host:27017/db?authSource=admin
PORT=3300
HOST=0.0.0.0
NODE_ENV=production

# SourceMod Extension (via config file)
api_service.url=http://api-server:3300
```

### **File Checklist**
- [ ] API service configured (`.env.production`)
- [ ] API service running (`curl http://api:3300/health`)
- [ ] Extension built (`bin/http_mongodb.ext.so`)
- [ ] Extension installed (`/sourcemod/extensions/`)
- [ ] Config updated (`/sourcemod/configs/mongodb.cfg`)
- [ ] Extension loaded (`sm exts load http_mongodb`)
- [ ] Tested (`mongo_test`)

## 🎉 **Production Ready**

This MongoDB extension is **enterprise-ready** and production-tested with:

### **🚀 Core Features**
- ✅ **Real database operations** with live MongoDB servers
- ✅ **Advanced operations**: Aggregation, bulk operations, indexing
- ✅ **Complete CRUD support**: Insert, Find, Update, Delete
- ✅ **Performance optimized** with connection pooling

### **🛡️ Enterprise Security**
- ✅ **API key authentication** and SourceMod verification
- ✅ **Rate limiting** and DDoS protection
- ✅ **Input validation** and injection protection
- ✅ **Security headers** and HTTPS support
- ✅ **OWASP compliance** and audit trails

### **🧪 Quality Assurance**
- ✅ **29 comprehensive API tests** covering all functionality
- ✅ **Security test suite** validating all security features
- ✅ **Performance benchmarks** and optimization
- ✅ **Error handling tests** for failure scenarios

### **🏗️ Production Ready**
- ✅ **Container compatibility** (Docker, Pterodactyl)
- ✅ **Multiple build options** (minimal 133KB / full 1.8MB)
- ✅ **Comprehensive documentation** and examples
- ✅ **Professional deployment guides**
- ✅ **Enterprise configuration** templates
