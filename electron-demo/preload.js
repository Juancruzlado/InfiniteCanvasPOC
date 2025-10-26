/**
 * Electron Preload Script
 * Secure bridge between main process and renderer
 */

const { contextBridge, ipcRenderer } = require('electron');

// Expose protected methods to renderer process
contextBridge.exposeInMainWorld('canvasAPI', {
  // Canvas operations
  openCanvas: () => ipcRenderer.invoke('open-canvas'),
  isCanvasOpen: () => ipcRenderer.invoke('is-canvas-open'),
  saveDrawing: () => ipcRenderer.invoke('save-drawing'),
  loadDrawing: () => ipcRenderer.invoke('load-drawing'),
  clearCanvas: () => ipcRenderer.invoke('clear-canvas')
});

// Expose filesystem and vault APIs
contextBridge.exposeInMainWorld('vaultAPI', {
  // Folder selection
  selectFolder: () => ipcRenderer.invoke('select-folder'),
  
  // File operations
  readFile: (filePath) => ipcRenderer.invoke('read-file', filePath),
  writeFile: (filePath, content) => ipcRenderer.invoke('write-file', filePath, content),
  
  // Directory operations
  readDirectory: (dirPath) => ipcRenderer.invoke('read-directory', dirPath),
  createDirectory: (dirPath) => ipcRenderer.invoke('create-directory', dirPath),
  
  // Path operations
  deletePath: (targetPath) => ipcRenderer.invoke('delete-path', targetPath),
  renamePath: (oldPath, newPath) => ipcRenderer.invoke('rename-path', oldPath, newPath),
  pathExists: (targetPath) => ipcRenderer.invoke('path-exists', targetPath),
  
  // Vault configuration
  loadVaultsConfig: () => ipcRenderer.invoke('load-vaults-config'),
  saveVaultsConfig: (config) => ipcRenderer.invoke('save-vaults-config', config)
});
