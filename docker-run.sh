#!/bin/bash
# Quick start script for Docker setup

set -e

echo "🐳 Infinite Canvas - Docker Setup"
echo "================================="
echo ""

# Check if Docker is installed
if ! command -v docker &> /dev/null; then
    echo "❌ Docker not found. Please install Docker first:"
    echo "   sudo apt install docker.io docker-compose-v2"
    exit 1
fi

# Check if user is in docker group
if ! groups | grep -q docker; then
    echo "⚠️  You're not in the docker group. Run:"
    echo "   sudo usermod -aG docker $USER"
    echo "   Then log out and back in."
    echo ""
    echo "Continuing with sudo..."
    DOCKER_CMD="sudo docker"
    COMPOSE_CMD="sudo docker compose"
else
    DOCKER_CMD="docker"
    COMPOSE_CMD="docker compose"
fi

# Allow X11 connections
echo "🖥️  Allowing X11 connections..."
xhost +local:docker > /dev/null 2>&1

# Build and start container
echo "🔨 Building Docker image (this may take a few minutes first time)..."
$COMPOSE_CMD up -d --build

echo ""
echo "✅ Container started!"
echo ""
echo "📝 Next steps:"
echo ""
echo "1. Enter the container:"
echo "   $COMPOSE_CMD exec electron-builder bash"
echo ""
echo "2. Inside container, build the project:"
echo "   cd electron-demo"
echo "   npm install"
echo "   npm run build-native"
echo "   npm start"
echo ""
echo "Or run everything at once:"
echo "   $COMPOSE_CMD exec electron-builder bash -c 'cd electron-demo && npm install && npm run build-native && npm start'"
echo ""
echo "🛑 To stop the container:"
echo "   $COMPOSE_CMD down"
echo ""
