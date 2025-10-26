# 🐳 Docker Setup - Zero Package Conflicts!

## Why Docker?

- ✅ **No system pollution** - Everything isolated
- ✅ **No package conflicts** - Clean environment
- ✅ **Reproducible** - Same setup on any machine
- ✅ **Easy cleanup** - Just delete container
- ✅ **Pre-configured** - Node 20, C++ tools, OpenGL all ready

---

## 📋 Prerequisites

Only need Docker installed:

```bash
# Check if Docker is installed
docker --version
docker compose version

# If not installed:
sudo apt install docker.io docker-compose-v2
sudo usermod -aG docker $USER
# Log out and back in for group changes
```

---

## 🚀 Quick Start (3 Commands)

### 1. Allow X11 Connections (for OpenGL window)

```bash
# Allow Docker to access your display (one-time setup)
xhost +local:docker
```

### 2. Build and Start Container

```bash
# From the C++ project directory
docker compose up -d --build
```

This will:
- Build Ubuntu image with Node 20
- Install all C++ dependencies (GLFW, GLEW, GLM, OpenGL)
- Mount your code as a volume
- Start container in background

### 3. Enter Container and Build

```bash
# Enter the container
docker compose exec electron-builder bash

# Now you're inside! Install npm dependencies
cd electron-demo
npm install

# Build the C++ addon
npm run build-native

# Run the Electron app
npm start
```

**That's it!** The Electron window and OpenGL canvas will appear on your host display.

---

## 📁 What Was Created

```
C++/
├── compose.yaml       # Docker Compose configuration
├── Dockerfile         # Container image definition
├── .dockerignore     # Files to exclude from image
└── DOCKER_SETUP.md   # This file
```

---

## 🎮 Usage

### Start Container

```bash
docker compose up -d
```

### Enter Container

```bash
docker compose exec electron-builder bash
```

### Inside Container - Build & Run

```bash
# Install dependencies (first time only)
cd electron-demo
npm install

# Build C++ addon
npm run build-native

# Run Electron app
npm start
```

### Stop Container

```bash
docker compose down
```

### Clean Everything (start fresh)

```bash
# Stop and remove container + volumes
docker compose down -v

# Remove image
docker rmi infinite-canvas-builder
```

---

## 🔧 Container Details

### Installed Software:

- **Node.js 20.x** (latest LTS)
- **npm 10.x**
- **g++ 11** (C++17 support)
- **CMake 3.22+**
- **OpenGL/Mesa**
- **GLFW 3.x**
- **GLEW 2.x**
- **GLM 0.9.9+**
- **X11 libraries** (for GUI)

### Volumes:

- **Source code**: Mounted at `/workspace`
- **node_modules**: Cached volume (faster rebuilds)
- **build/**: Cached volume (compiled artifacts)

### GPU Access:

- `/dev/dri` mounted for hardware acceleration
- X11 socket shared for display

---

## 🐛 Troubleshooting

### "cannot open display"

**Problem:** X11 not accessible

**Solution:**
```bash
# On host machine
xhost +local:docker

# Check DISPLAY is set
echo $DISPLAY  # Should show :0 or similar
```

### "Failed to initialize GLFW"

**Problem:** OpenGL not working in container

**Solution:**
```bash
# Test OpenGL in container
docker compose exec electron-builder glxinfo | grep "OpenGL"

# If fails, try software rendering
docker compose exec -e LIBGL_ALWAYS_SOFTWARE=1 electron-builder bash
```

### "Module not found: infinitecanvas.node"

**Problem:** C++ addon not built

**Solution:**
```bash
docker compose exec electron-builder bash
cd electron-demo
npm run build-native
```

### "Permission denied" on files

**Problem:** Container running as root

**Solution:**
```bash
# Fix ownership (on host)
sudo chown -R $USER:$USER .

# Or run as your user (add to compose.yaml):
# user: "${UID}:${GID}"
```

---

## 🎯 Development Workflow

### Recommended Flow:

```bash
# 1. Start container (once)
docker compose up -d

# 2. Enter container shell
docker compose exec electron-builder bash

# 3. Work inside container
cd electron-demo
npm run dev  # Runs build-native + start

# 4. Edit files on host (your IDE)
# Changes auto-sync via volume mount

# 5. Rebuild when needed
npm run build-native

# 6. When done, stop container
exit
docker compose down
```

### Quick Commands:

```bash
# Build without entering container
docker compose exec electron-builder bash -c "cd electron-demo && npm run build-native"

# Run Electron without entering
docker compose exec electron-builder bash -c "cd electron-demo && npm start"

# View logs
docker compose logs -f

# Restart container
docker compose restart
```

---

## 📊 Comparison: Docker vs Native

| Aspect | Docker ✅ | Native Installation ❌ |
|--------|-----------|----------------------|
| Setup time | 5 minutes | 30+ minutes |
| Package conflicts | None | High risk |
| System pollution | Zero | Many packages |
| Reproducible | 100% | Varies |
| Cleanup | One command | Manual |
| Multiple versions | Easy | Difficult |
| Share setup | Just `compose.yaml` | Long instructions |

---

## 🚀 Advanced: VS Code Integration

You can use VS Code's Remote-Containers extension:

1. Install "Dev Containers" extension
2. Open project folder
3. Click "Reopen in Container"
4. VS Code runs inside Docker!

Create `.devcontainer/devcontainer.json`:

```json
{
  "name": "Infinite Canvas Dev",
  "dockerComposeFile": "../compose.yaml",
  "service": "electron-builder",
  "workspaceFolder": "/workspace",
  "extensions": [
    "ms-vscode.cpptools",
    "ms-vscode.cmake-tools"
  ]
}
```

---

## 🎓 How It Works

### Architecture:

```
┌─────────────────────────────────────────┐
│  Host Machine (Your Ubuntu)            │
│                                         │
│  ┌───────────────────────────────────┐ │
│  │  Docker Container                 │ │
│  │  (Ubuntu 22.04 + Node 20)        │ │
│  │                                   │ │
│  │  ├─ Node.js 20                   │ │
│  │  ├─ C++ Build Tools               │ │
│  │  ├─ OpenGL Libraries              │ │
│  │  └─ Your Code (mounted)           │ │
│  │                                   │ │
│  │  Electron App ──┐                 │ │
│  │                 │                 │ │
│  └─────────────────┼─────────────────┘ │
│                    │                   │
│                    │ X11 Socket        │
│                    ▼                   │
│  ┌─────────────────────────────────┐  │
│  │  Host Display (:0)              │  │
│  │  ┌─────────────────────────┐    │  │
│  │  │ Electron UI             │    │  │
│  │  └─────────────────────────┘    │  │
│  │  ┌─────────────────────────┐    │  │
│  │  │ OpenGL Canvas Window    │    │  │
│  │  └─────────────────────────┘    │  │
│  └─────────────────────────────────┘  │
└─────────────────────────────────────────┘
```

### X11 Forwarding:

1. Container shares `/tmp/.X11-unix` socket
2. `DISPLAY` env var points to host display
3. `xhost +local:docker` grants permission
4. GUI windows appear on host screen

### Volume Mounts:

- **Source code**: Live sync, edit on host
- **node_modules**: Cached in container (fast)
- **build/**: Cached compiled files

---

## ✅ Success Checklist

You'll know it's working when:

1. ✅ `docker compose up -d` completes without errors
2. ✅ `docker compose exec electron-builder node --version` shows v20.x
3. ✅ `npm install` completes inside container
4. ✅ `npm run build-native` compiles successfully
5. ✅ `npm start` launches Electron on host display
6. ✅ OpenGL canvas window appears when clicked

---

## 🎉 Benefits Achieved

**What you get with Docker:**

- ✅ **Node.js 20** - No more v12 issues
- ✅ **Clean environment** - No package conflicts
- ✅ **Instant setup** - One `docker compose up`
- ✅ **Easy sharing** - Just share compose.yaml
- ✅ **Reproducible** - Same on any machine
- ✅ **Safe experimentation** - Can't break host
- ✅ **Easy cleanup** - `docker compose down -v`

**No more:**
- ❌ Version conflicts
- ❌ Missing dependencies
- ❌ System pollution
- ❌ Manual package management
- ❌ "Works on my machine" problems

---

## 🔗 Next Steps

1. **First time:**
   ```bash
   xhost +local:docker
   docker compose up -d --build
   docker compose exec electron-builder bash
   cd electron-demo && npm install && npm run build-native && npm start
   ```

2. **Subsequent times:**
   ```bash
   docker compose up -d
   docker compose exec electron-builder bash
   cd electron-demo && npm start
   ```

3. **When done:**
   ```bash
   docker compose down
   ```

**Your Electron + C++ app now runs in a perfect, isolated environment!** 🚀

---

## 📚 Resources

- [Docker Compose Docs](https://docs.docker.com/compose/)
- [Docker GUI Apps](https://docs.docker.com/desktop/features/wsl/#run-a-wsl-2-linux-gui-app)
- [VS Code Dev Containers](https://code.visualstudio.com/docs/devcontainers/containers)
