/**
 * Electron Main Process
 * Handles native canvas integration and file I/O
 */

const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');
const fs = require('fs').promises;
const fsSync = require('fs');

// Disable GPU acceleration for Docker compatibility
app.commandLine.appendSwitch('disable-gpu');
app.commandLine.appendSwitch('disable-gpu-compositing');
app.commandLine.appendSwitch('disable-software-rasterizer');

// Load C++ native addon
const canvas = require('../build/Release/infinitecanvas.node');

let mainWindow;

function createWindow() {
  mainWindow = new BrowserWindow({
    width: 1400,
    height: 900,
    webPreferences: {
      preload: path.join(__dirname, 'preload.js'),
      nodeIntegration: false,
      contextIsolation: true,
      sandbox: false  // Disable sandbox for Docker compatibility
    },
    backgroundColor: '#1a1a1a',
    title: 'Infinite Notes'
  });

  mainWindow.loadFile('index.html');
  
  // Initialize C++ canvas on startup
  try {
    const success = canvas.init();
    console.log('Canvas initialized:', success);
  } catch (error) {
    console.error('Failed to initialize canvas:', error);
  }
  
  // Open DevTools in development
  // mainWindow.webContents.openDevTools();
}

// ============================================================================
// IPC Handlers
// ============================================================================

/**
 * Open native OpenGL canvas window
 */
ipcMain.handle('open-canvas', async () => {
  try {
    const result = canvas.openWindow();
    return { success: true, message: result };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Check if canvas window is open
 */
ipcMain.handle('is-canvas-open', async () => {
  try {
    return canvas.isWindowOpen();
  } catch (error) {
    return false;
  }
});

/**
 * Save drawing with native file dialog
 */
ipcMain.handle('save-drawing', async () => {
  try {
    const result = await dialog.showSaveDialog(mainWindow, {
      title: 'Save Drawing',
      defaultPath: 'drawing.mm',
      filters: [
        { name: 'Mind Map Files', extensions: ['mm'] },
        { name: 'All Files', extensions: ['*'] }
      ]
    });
    
    if (result.canceled) {
      return { success: false, canceled: true };
    }
    
    const success = canvas.saveDrawing(result.filePath);
    return { 
      success, 
      filePath: result.filePath,
      message: success ? 'Drawing saved successfully' : 'Failed to save drawing'
    };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Load drawing with native file dialog
 */
ipcMain.handle('load-drawing', async () => {
  try {
    const result = await dialog.showOpenDialog(mainWindow, {
      title: 'Open Drawing',
      filters: [
        { name: 'Mind Map Files', extensions: ['mm'] },
        { name: 'All Files', extensions: ['*'] }
      ],
      properties: ['openFile']
    });
    
    if (result.canceled || result.filePaths.length === 0) {
      return { success: false, canceled: true };
    }
    
    const success = canvas.loadDrawing(result.filePaths[0]);
    return { 
      success, 
      filePath: result.filePaths[0],
      message: success ? 'Drawing loaded successfully' : 'Failed to load drawing'
    };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Clear canvas
 */
ipcMain.handle('clear-canvas', async () => {
  try {
    const success = canvas.clear();
    return { success, message: 'Canvas cleared' };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

// ============================================================================
// Vault & Filesystem Handlers
// ============================================================================

/**
 * Select folder for vault
 */
ipcMain.handle('select-folder', async () => {
  try {
    const result = await dialog.showOpenDialog(mainWindow, {
      title: 'Select Vault Folder',
      properties: ['openDirectory', 'createDirectory']
    });
    
    if (result.canceled || result.filePaths.length === 0) {
      return { success: false, canceled: true };
    }
    
    return { success: true, path: result.filePaths[0] };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Read directory contents
 */
ipcMain.handle('read-directory', async (event, dirPath) => {
  try {
    const items = await fs.readdir(dirPath, { withFileTypes: true });
    const result = await Promise.all(items.map(async (item) => {
      const fullPath = path.join(dirPath, item.name);
      const stats = await fs.stat(fullPath);
      
      return {
        name: item.name,
        path: fullPath,
        isDirectory: item.isDirectory(),
        isFile: item.isFile(),
        size: stats.size,
        modified: stats.mtime
      };
    }));
    
    return { success: true, items: result };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Read file content
 */
ipcMain.handle('read-file', async (event, filePath) => {
  try {
    const content = await fs.readFile(filePath, 'utf-8');
    return { success: true, content };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Write file content
 */
ipcMain.handle('write-file', async (event, filePath, content) => {
  try {
    await fs.writeFile(filePath, content, 'utf-8');
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Create directory
 */
ipcMain.handle('create-directory', async (event, dirPath) => {
  try {
    await fs.mkdir(dirPath, { recursive: true });
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Delete file or directory
 */
ipcMain.handle('delete-path', async (event, targetPath) => {
  try {
    const stats = await fs.stat(targetPath);
    if (stats.isDirectory()) {
      await fs.rmdir(targetPath, { recursive: true });
    } else {
      await fs.unlink(targetPath);
    }
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Rename/move file or directory
 */
ipcMain.handle('rename-path', async (event, oldPath, newPath) => {
  try {
    await fs.rename(oldPath, newPath);
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Check if path exists
 */
ipcMain.handle('path-exists', async (event, targetPath) => {
  try {
    await fs.access(targetPath);
    return { success: true, exists: true };
  } catch (error) {
    return { success: true, exists: false };
  }
});

/**
 * Get vault config path (in user data directory)
 */
function getVaultConfigPath() {
  return path.join(app.getPath('userData'), 'vaults.json');
}

/**
 * Load vaults configuration
 */
ipcMain.handle('load-vaults-config', async () => {
  try {
    const configPath = getVaultConfigPath();
    const exists = fsSync.existsSync(configPath);
    
    if (!exists) {
      return { success: true, vaults: [], currentVault: null };
    }
    
    const content = await fs.readFile(configPath, 'utf-8');
    const config = JSON.parse(content);
    return { success: true, ...config };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

/**
 * Save vaults configuration
 */
ipcMain.handle('save-vaults-config', async (event, config) => {
  try {
    const configPath = getVaultConfigPath();
    await fs.writeFile(configPath, JSON.stringify(config, null, 2), 'utf-8');
    return { success: true };
  } catch (error) {
    return { success: false, error: error.message };
  }
});

// ============================================================================
// App Lifecycle
// ============================================================================

app.whenReady().then(createWindow);

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

// Cleanup on quit
app.on('before-quit', () => {
  console.log('App shutting down...');
});
