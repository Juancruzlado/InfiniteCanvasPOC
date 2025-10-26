/**
 * Electron Main Process
 * Handles native canvas integration and file I/O
 */

const { app, BrowserWindow, ipcMain, dialog } = require('electron');
const path = require('path');

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
