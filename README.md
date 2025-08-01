# MongoDB SourceMod Extension

A lightweight SourceMod extension for MongoDB integration using HTTP API communication.

## ✅ **Current Status: PRODUCTION READY**

- ✅ **Minimal Extension**: 133KB (no libcurl dependencies)
- ✅ **Full Extension**: 1.8MB (with libcurl)
- ✅ **Real MongoDB Integration**: Connects to live MongoDB servers
- ✅ **HTTP API Service**: Node.js bridge service
- ✅ **Console Commands**: Ready-to-use test commands
- ✅ **JSON Document Support**: Real data insertion and retrieval

## 🎯 **Key Features**

### **Extension Capabilities**
- ✅ Connect to MongoDB via HTTP API
- ✅ Insert documents with `InsertOneJSON()`
- ✅ Find documents with `FindOneJSON()`
- ✅ Count documents with `CountDocuments()`
- ✅ Real-time database operations
- ✅ Comprehensive error logging

### **Two Build Options**
1. **Minimal** (133KB): Raw sockets, no external dependencies
2. **Full** (1.8MB): libcurl-based with full HTTP features

### **Production Features**
- ✅ Real MongoDB server connection (`37.114.54.74:27017`)
- ✅ Authentication support (admin credentials)
- ✅ JSON document handling
- ✅ Console test commands
- ✅ Pterodactyl container compatibility

## 📁 **Project Structure**

### **1. HTTP Extension** (`http_extension/`)
```
http_extension/
├── bin/http_mongodb.ext.so          # Ready-to-use extension (133KB)
├── minimal_complete_extension.cpp   # Minimal source (no libcurl)
├── complete_extension.cpp           # Full source (with libcurl)
├── build_extension.sh               # Build script (minimal/full)
├── CMakeLists_minimal.txt           # Minimal build config
├── CMakeLists.txt                   # Full build config
└── scripting/
    ├── include/http_mongodb.inc     # SourcePawn interface
    ├── mongo_console_test.sp        # Console test commands
    └── test_real_data.sp            # Real data examples
```

### **2. MongoDB API Service** (`mongodb-api-service/`)
```
mongodb-api-service/
├── dist/server.js                   # Compiled Node.js service
├── src/server.ts                    # TypeScript source
└── package.json                     # Dependencies
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
cp .env.example .env.production
nano .env.production
```

**Configure your MongoDB URI in `.env.production`:**
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

---

## 🛠 **Development Setup**

### **For Local Development/Testing**

#### **1. Quick Development Setup**
```bash
# Start API service locally
cd mongodb-api-service
cp .env.example .env.development
# Edit .env.development with local MongoDB
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

## 🚀 **Quick Start (TL;DR)**

```bash
# 1. Setup API service
cd mongodb-api-service
./setup-mongodb-config.sh
./start-production.sh

# 2. Build extension
cd ../http_extension
./build_extension.sh minimal

# 3. Install on game server
scp bin/http_mongodb.ext.so gameserver:/sourcemod/extensions/
scp scripting/configs/mongodb.cfg gameserver:/sourcemod/configs/

# 4. Configure and test
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

## 🔧 **Configuration**

### **MongoDB Connection**
The extension connects to: `mongodb://admin:***@37.114.54.74:27017/?authSource=admin`

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

- **Server**: `37.114.54.74:27017`
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

### **Testing**
```bash
# Compile test plugins
spcomp mongo_console_test.sp
spcomp test_real_data.sp

# Load and test
sm plugins load mongo_console_test
mongo_test
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

### **Debug Commands**
```bash
# Check extension status
sm exts list | grep mongodb

# Test API connectivity
curl -X POST http://localhost:3300/api/v1/connections \
  -H "Content-Type: application/json" \
  -d '{"uri":"mongodb://admin:***@37.114.54.74:27017/?authSource=admin"}'

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

This MongoDB extension is **fully functional** and ready for production use with:
- Real database operations
- Comprehensive error handling
- Container compatibility
- Multiple build options
- Extensive testing commands
- Security considerations
- Complete API documentation
- Detailed deployment guides
- Multiple deployment scenarios
- Production-grade configuration
