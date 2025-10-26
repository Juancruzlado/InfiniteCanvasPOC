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
