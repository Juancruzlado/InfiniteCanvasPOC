/**
 * Renderer Process (UI Logic)
 * Handles user interactions with the Electron UI
 */

// ============================================================================
// DOM Elements
// ============================================================================

const openCanvasBtn = document.getElementById('openCanvasBtn');
const saveBtn = document.getElementById('saveBtn');
const loadBtn = document.getElementById('loadBtn');
const clearBtn = document.getElementById('clearBtn');
const statusDiv = document.getElementById('status');
const notesList = document.getElementById('notesList');
const editor = document.getElementById('editor');
const newNoteBtn = document.getElementById('newNoteBtn');

// ============================================================================
// Helper Functions
// ============================================================================

function showStatus(message, type = 'info') {
  statusDiv.textContent = message;
  statusDiv.className = `status ${type}`;
  
  // Auto-hide after 3 seconds
  setTimeout(() => {
    statusDiv.className = 'status hidden';
  }, 3000);
}

function createNoteItem(title, date) {
  const item = document.createElement('div');
  item.className = 'note-item';
  item.innerHTML = `
    <div class="note-title">${title}</div>
    <div class="note-date">${date}</div>
  `;
  item.addEventListener('click', () => {
    document.querySelectorAll('.note-item').forEach(el => el.classList.remove('active'));
    item.classList.add('active');
    editor.value = `# ${title}\n\nEdit your note here...`;
  });
  return item;
}

// ============================================================================
// Canvas Integration
// ============================================================================

openCanvasBtn.addEventListener('click', async () => {
  try {
    openCanvasBtn.disabled = true;
    openCanvasBtn.textContent = '⏳ Opening...';
    
    const result = await window.canvasAPI.openCanvas();
    
    if (result.success) {
      showStatus('✅ Canvas window opened! Draw away!', 'success');
      openCanvasBtn.textContent = '🎨 Canvas Open';
      
      // Poll to check if window is still open
      const checkInterval = setInterval(async () => {
        const isOpen = await window.canvasAPI.isCanvasOpen();
        if (!isOpen) {
          openCanvasBtn.disabled = false;
          openCanvasBtn.textContent = '🎨 Open Canvas';
          showStatus('Canvas window closed', 'info');
          clearInterval(checkInterval);
        }
      }, 1000);
    } else {
      showStatus(`❌ Failed: ${result.error || 'Unknown error'}`, 'error');
      openCanvasBtn.disabled = false;
      openCanvasBtn.textContent = '🎨 Open Canvas';
    }
  } catch (error) {
    showStatus(`❌ Error: ${error.message}`, 'error');
    openCanvasBtn.disabled = false;
    openCanvasBtn.textContent = '🎨 Open Canvas';
  }
});

saveBtn.addEventListener('click', async () => {
  try {
    saveBtn.disabled = true;
    const result = await window.canvasAPI.saveDrawing();
    
    if (result.success) {
      showStatus(`✅ Saved: ${result.filePath}`, 'success');
    } else if (result.canceled) {
      showStatus('Save canceled', 'info');
    } else {
      showStatus(`❌ ${result.message || 'Save failed'}`, 'error');
    }
  } catch (error) {
    showStatus(`❌ Error: ${error.message}`, 'error');
  } finally {
    saveBtn.disabled = false;
  }
});

loadBtn.addEventListener('click', async () => {
  try {
    loadBtn.disabled = true;
    const result = await window.canvasAPI.loadDrawing();
    
    if (result.success) {
      showStatus(`✅ Loaded: ${result.filePath}`, 'success');
    } else if (result.canceled) {
      showStatus('Load canceled', 'info');
    } else {
      showStatus(`❌ ${result.message || 'Load failed'}`, 'error');
    }
  } catch (error) {
    showStatus(`❌ Error: ${error.message}`, 'error');
  } finally {
    loadBtn.disabled = false;
  }
});

clearBtn.addEventListener('click', async () => {
  if (!confirm('Clear the entire canvas? This cannot be undone.')) {
    return;
  }
  
  try {
    clearBtn.disabled = true;
    const result = await window.canvasAPI.clearCanvas();
    
    if (result.success) {
      showStatus('✅ Canvas cleared', 'success');
    } else {
      showStatus(`❌ ${result.message || 'Clear failed'}`, 'error');
    }
  } catch (error) {
    showStatus(`❌ Error: ${error.message}`, 'error');
  } finally {
    clearBtn.disabled = false;
  }
});

// ============================================================================
// Notes Management (Demo)
// ============================================================================

newNoteBtn.addEventListener('click', () => {
  const title = prompt('Note title:');
  if (!title) return;
  
  const date = new Date().toLocaleDateString();
  const noteItem = createNoteItem(title, date);
  notesList.appendChild(noteItem);
  
  showStatus(`✅ Note "${title}" created`, 'success');
});

// Initialize with sample notes
function initSampleNotes() {
  const samples = [
    { title: 'Welcome', date: new Date().toLocaleDateString() },
    { title: 'Project Ideas', date: new Date(Date.now() - 86400000).toLocaleDateString() },
    { title: 'Meeting Notes', date: new Date(Date.now() - 172800000).toLocaleDateString() }
  ];
  
  samples.forEach(note => {
    notesList.appendChild(createNoteItem(note.title, note.date));
  });
  
  // Select first note
  notesList.querySelector('.note-item').click();
}

// ============================================================================
// Initialization
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
  initSampleNotes();
  showStatus('Ready! Click "Open Canvas" to start drawing', 'info');
});

// Keyboard shortcuts
document.addEventListener('keydown', (e) => {
  if (e.ctrlKey || e.metaKey) {
    if (e.key === 's') {
      e.preventDefault();
      saveBtn.click();
    } else if (e.key === 'o') {
      e.preventDefault();
      loadBtn.click();
    } else if (e.key === 'n') {
      e.preventDefault();
      newNoteBtn.click();
    }
  }
});
