# Ubuntu 22.04 with Node.js 20 LTS
FROM ubuntu:22.04

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Install Node.js 20 from NodeSource
RUN apt-get update && apt-get install -y \
    curl \
    ca-certificates \
    gnupg \
    && curl -fsSL https://deb.nodesource.com/setup_20.x | bash - \
    && apt-get install -y nodejs

# Install C++ build tools and dependencies
RUN apt-get install -y \
    build-essential \
    cmake \
    g++ \
    python3 \
    python3-pip \
    git \
    pkg-config

# Install OpenGL and graphics libraries
RUN apt-get install -y \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    mesa-utils \
    libglfw3-dev \
    libglew-dev \
    libglm-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev

# Install X11 libraries for GUI
RUN apt-get install -y \
    x11-apps \
    libgtk-3-0 \
    libgbm1 \
    libnss3 \
    libasound2

# Clean up
RUN apt-get clean && rm -rf /var/lib/apt/lists/*

# Verify installations
RUN node --version && \
    npm --version && \
    g++ --version && \
    cmake --version

# Set working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]
