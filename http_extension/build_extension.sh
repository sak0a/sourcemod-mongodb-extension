#!/bin/bash

echo "=== MongoDB Extension Build Script ==="
echo ""

# Set environment variables
export SOURCEMOD_PATH="/root/sourcemod-workspace/sourcemod"
export HL2SDK_PATH="/root/sourcemod-workspace/hl2sdk-tf2"

# Check if we want minimal build
BUILD_TYPE="full"
if [ "$1" = "minimal" ]; then
    BUILD_TYPE="minimal"
    echo "Building MINIMAL version (no libcurl, ~130KB)..."
else
    echo "Building FULL version (with libcurl, ~1.8MB)..."
fi

echo "SourceMod Path: $SOURCEMOD_PATH"
echo "HL2SDK Path: $HL2SDK_PATH"
echo ""

# Check dependencies
if [ ! -d "$SOURCEMOD_PATH" ]; then
    echo "❌ ERROR: SourceMod SDK not found at $SOURCEMOD_PATH"
    exit 1
fi

if [ ! -d "$HL2SDK_PATH" ]; then
    echo "❌ ERROR: HL2SDK not found at $HL2SDK_PATH"
    exit 1
fi

# Ensure required libraries exist
mkdir -p "$HL2SDK_PATH/lib/linux"
if [ ! -f "$HL2SDK_PATH/lib/linux/tier1_i486.a" ]; then
    echo "📦 Copying required libraries from CS:GO SDK..."
    cp /root/sourcemod-workspace/hl2sdk-csgo/lib/linux/tier1_i486.a "$HL2SDK_PATH/lib/linux/" 2>/dev/null || echo "⚠️  Warning: tier1_i486.a not found"
    cp /root/sourcemod-workspace/hl2sdk-csgo/lib/linux/mathlib_i486.a "$HL2SDK_PATH/lib/linux/" 2>/dev/null || echo "⚠️  Warning: mathlib_i486.a not found"
fi

# Create build directory
BUILD_DIR="build"
if [ "$BUILD_TYPE" = "minimal" ]; then
    BUILD_DIR="build_minimal"
fi

echo "🔨 Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf CMakeCache.txt CMakeFiles/ bin/

# Configure build
echo "⚙️  Configuring build..."
if [ "$BUILD_TYPE" = "minimal" ]; then
    cp ../CMakeLists_minimal.txt ./CMakeLists.txt
    echo "Using minimal CMakeLists.txt (no libcurl)"
    cmake .
else
    echo "Using full CMakeLists.txt (with libcurl)"
    cmake ..
fi

if [ $? -ne 0 ]; then
    echo "❌ Configuration failed!"
    exit 1
fi

# Build
echo "🔨 Building extension..."
make

if [ $? -ne 0 ]; then
    echo "❌ Build failed!"
    exit 1
fi

# Copy to bin directory
echo "📦 Installing extension..."
mkdir -p ../bin

if [ "$BUILD_TYPE" = "minimal" ]; then
    cp bin/libhttp_mongodb_minimal.ext.so ../bin/http_mongodb.ext.so
    FINAL_SIZE=$(ls -lh ../bin/http_mongodb.ext.so | awk '{print $5}')
    echo "✅ MINIMAL build complete! Size: $FINAL_SIZE"
else
    cp bin/libhttp_mongodb.ext.so ../bin/http_mongodb.ext.so
    FINAL_SIZE=$(ls -lh ../bin/http_mongodb.ext.so | awk '{print $5}')
    echo "✅ FULL build complete! Size: $FINAL_SIZE"
fi

echo ""
echo "📁 Extension available at: bin/http_mongodb.ext.so"
echo ""
echo "🚀 Usage:"
echo "  1. Copy to your server: scp bin/http_mongodb.ext.so your-server:/path/to/extensions/"
echo "  2. Load extension: sm exts load http_mongodb"
echo "  3. Test: mongo_test"
echo ""

# Show dependencies
echo "🔍 Dependencies:"
ldd ../bin/http_mongodb.ext.so 2>/dev/null | head -10 || echo "Static build - minimal dependencies"

# Compile SourcePawn plugins if spcomp is available
echo ""
echo "🔧 Compiling SourcePawn plugins..."

if command -v spcomp >/dev/null 2>&1; then
    cd scripting

    echo "Compiling mongo_console_test.sp..."
    spcomp mongo_console_test.sp -o ../bin/mongo_console_test.smx

    echo "Compiling test_real_data.sp..."
    spcomp test_real_data.sp -o ../bin/test_real_data.smx

    echo "Compiling advanced_mongodb_examples.sp..."
    spcomp advanced_mongodb_examples.sp -o ../bin/advanced_mongodb_examples.smx

    cd ..
    echo "✅ SourcePawn plugins compiled successfully!"
    echo "📁 Plugins available in bin/ directory"
else
    echo "⚠️ spcomp not found - SourcePawn plugins not compiled"
    echo "   Copy .sp files to your SourceMod scripting directory and compile manually"
fi

echo ""
echo "📦 Complete package contents:"
echo "  📄 http_mongodb.ext.so - Main extension"
echo "  📄 http_mongodb.inc - Include file"
echo "  📄 mongodb_config_example.cfg - Configuration template"
echo "  📄 *.smx - Compiled plugins (if spcomp available)"
echo "  📄 *.sp - Source plugins"

echo ""
echo "Build completed successfully! 🎉"
