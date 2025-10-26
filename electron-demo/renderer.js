/**
 * Renderer Process - File Tree with Context Menu
 * Using CodeMirror 6 for markdown editing
 */

console.log('🚀 renderer.js loaded!');

// ============================================================================
// DOM Elements (will be initialized after DOM loads)
// ============================================================================

let openCanvasBtn, statusDiv, fileTree, filePanel, editorContainer, editor, newNoteBtn, newFolderBtn;
let inputModal, modalTitle, modalInput, modalCancel, modalConfirm;
let vaultFooter, currentVaultName, vaultSelectorModal, vaultList, manageVaultsBtn;
let vaultManagerModal, managerVaultList, createVaultBtn, openVaultBtn, closeManagerBtn;
let tabsContainer, newTabBtn;

// ============================================================================
// Vault System
// ============================================================================

let vaultsConfig = {
  vaults: [],
  currentVault: null
};

let currentVaultPath = null;

// ============================================================================
// Custom Prompt (replaces browser prompt which is disabled in Electron)
// ============================================================================

function showPrompt(title) {
  return new Promise((resolve) => {
    modalTitle.textContent = title;
    modalInput.value = '';
    inputModal.classList.remove('hidden');
    inputModal.classList.add('flex');
    modalInput.focus();
    
    const handleConfirm = () => {
      const value = modalInput.value.trim();
      cleanup();
      resolve(value || null);
    };
    
    const handleCancel = () => {
      cleanup();
      resolve(null);
    };
    
    const handleKeyDown = (e) => {
      if (e.key === 'Enter') {
        e.preventDefault();
        handleConfirm();
      } else if (e.key === 'Escape') {
        e.preventDefault();
        handleCancel();
      }
    };
    
    const cleanup = () => {
      inputModal.classList.add('hidden');
      inputModal.classList.remove('flex');
      modalConfirm.removeEventListener('click', handleConfirm);
      modalCancel.removeEventListener('click', handleCancel);
      modalInput.removeEventListener('keydown', handleKeyDown);
    };
    
    modalConfirm.addEventListener('click', handleConfirm);
    modalCancel.addEventListener('click', handleCancel);
    modalInput.addEventListener('keydown', handleKeyDown);
  });
}

// ============================================================================
// Vault Management Functions
// ============================================================================

async function loadVaultsConfig() {
  console.log('Loading vaults config...');
  const result = await window.vaultAPI.loadVaultsConfig();
  
  if (result.success) {
    vaultsConfig = {
      vaults: result.vaults || [],
      currentVault: result.currentVault || null
    };
    
    console.log('Vaults loaded:', vaultsConfig);
    
    // If no vaults, show manager
    if (vaultsConfig.vaults.length === 0) {
      showVaultManager();
      return false;
    }
    
    // If has vaults but no current, set first as current
    if (!vaultsConfig.currentVault && vaultsConfig.vaults.length > 0) {
      vaultsConfig.currentVault = vaultsConfig.vaults[0].id;
      await saveVaultsConfig();
    }
    
    // Load current vault
    if (vaultsConfig.currentVault) {
      const vault = vaultsConfig.vaults.find(v => v.id === vaultsConfig.currentVault);
      if (vault) {
        await switchToVault(vault.id);
        return true;
      }
    }
  }
  
  return false;
}

async function saveVaultsConfig() {
  await window.vaultAPI.saveVaultsConfig(vaultsConfig);
}

async function createNewVault() {
  const name = await showPrompt('Vault name:');
  if (!name) return;
  
  const result = await window.vaultAPI.selectFolder();
  if (!result.success || result.canceled) return;
  
  const vault = {
    id: generateId(),
    name: name,
    path: result.path,
    created: new Date().toISOString()
  };
  
  vaultsConfig.vaults.push(vault);
  vaultsConfig.currentVault = vault.id;
  
  await saveVaultsConfig();
  await switchToVault(vault.id);
  
  showStatus(`✅ Vault "${name}" created`, 'success');
  closeVaultManager();
}

async function openExistingVault() {
  const result = await window.vaultAPI.selectFolder();
  if (!result.success || result.canceled) return;
  
  // Get folder name as vault name
  const pathParts = result.path.split('/');
  const folderName = pathParts[pathParts.length - 1] || 'Vault';
  
  const name = await showPrompt(`Vault name (folder: ${folderName}):`);
  if (!name) return;
  
  const vault = {
    id: generateId(),
    name: name,
    path: result.path,
    created: new Date().toISOString()
  };
  
  vaultsConfig.vaults.push(vault);
  vaultsConfig.currentVault = vault.id;
  
  await saveVaultsConfig();
  await switchToVault(vault.id);
  
  showStatus(`✅ Vault "${name}" opened`, 'success');
  closeVaultManager();
}

async function switchToVault(vaultId) {
  const vault = vaultsConfig.vaults.find(v => v.id === vaultId);
  if (!vault) return;
  
  console.log('Switching to vault:', vault);
  
  vaultsConfig.currentVault = vaultId;
  currentVaultPath = vault.path;
  
  await saveVaultsConfig();
  
  // Update UI
  currentVaultName.textContent = vault.name;
  
  // Load filesystem from vault
  await loadVaultFileSystem();
  
  // Close vault selector if open
  closeVaultSelector();
}

async function loadVaultFileSystem(preserveTabs = false) {
  if (!currentVaultPath) return;
  
  console.log('Loading filesystem from:', currentVaultPath);
  
  try {
    fileSystem = await readDirectoryRecursive(currentVaultPath, null);
    renderFileTree();
    
    // If preserveTabs is false and no tabs are open, don't auto-open anything
    // User can manually open files or use the + button
  } catch (error) {
    console.error('Failed to load vault filesystem:', error);
    showStatus('❌ Failed to load vault', 'error');
  }
}

async function readDirectoryRecursive(dirPath, parentPath) {
  const result = await window.vaultAPI.readDirectory(dirPath);
  
  if (!result.success) {
    return { type: 'root', children: [] };
  }
  
  const children = [];
  
  for (const item of result.items) {
    // Skip hidden files
    if (item.name.startsWith('.')) continue;
    
    if (item.isDirectory) {
      const folder = {
        id: generateId(),
        type: 'folder',
        name: item.name,
        path: item.path,
        expanded: false,
        children: await readDirectoryRecursive(item.path, item.path)
      };
      children.push(folder);
    } else if (item.isFile) {
      // Only include .md, .mm, and image files
      const ext = item.name.split('.').pop().toLowerCase();
      if (['md', 'mm', 'png', 'jpg', 'jpeg', 'gif', 'svg'].includes(ext)) {
        const file = {
          id: generateId(),
          type: 'file',
          fileType: ext === 'mm' ? 'canvas' : (ext === 'md' ? 'note' : 'image'),
          name: item.name,
          path: item.path,
          created: item.modified
        };
        children.push(file);
      }
    }
  }
  
  if (parentPath === null) {
    return { type: 'root', children };
  }
  
  return children;
}

// ============================================================================
// File System Data Structure
// ============================================================================

let fileSystem = {
  type: 'root',
  children: []
};

let currentFile = null;
let selectedItem = null;
let openTabs = []; // Array of open file objects
let activeTabIndex = -1;

// ============================================================================
// Context Menu
// ============================================================================

let contextMenu = null;

function createContextMenu(x, y, targetItem, menuType = 'create') {
  // Remove existing menu
  removeContextMenu();
  
  // Create menu element
  contextMenu = document.createElement('div');
  contextMenu.className = 'context-menu';
  contextMenu.style.left = x + 'px';
  contextMenu.style.top = y + 'px';
  
  let menuItems = [];
  
  if (menuType === 'item') {
    // Context menu for files/folders
    menuItems = [
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M8 7v8a2 2 0 002 2h6M8 7V5a2 2 0 012-2h4.586a1 1 0 01.707.293l4.414 4.414a1 1 0 01.293.707V15a2 2 0 01-2 2h-2M8 7H6a2 2 0 00-2 2v10a2 2 0 002 2h8a2 2 0 002-2v-2"></path>',
        label: 'Make a copy',
        action: () => makeFileCopy(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path>',
        label: 'Show in system explorer',
        action: () => showInExplorer(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z"></path>',
        label: 'Rename...',
        action: () => renameItem(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 7l-.867 12.142A2 2 0 0116.138 21H7.862a2 2 0 01-1.995-1.858L5 7m5 4v6m4-6v6m1-10V4a1 1 0 00-1-1h-4a1 1 0 00-1 1v3M4 7h16"></path>',
        label: 'Delete',
        color: 'text-red-400',
        action: () => deleteItem(targetItem)
      }
    ];
  } else {
    // Context menu for empty space (create new)
    menuItems = [
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M11 5H6a2 2 0 00-2 2v11a2 2 0 002 2h11a2 2 0 002-2v-5m-1.414-9.414a2 2 0 112.828 2.828L11.828 15H9v-2.828l8.586-8.586z"></path>',
        label: 'New note',
        action: () => promptNewNote(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path>',
        label: 'New folder',
        action: () => promptNewFolder(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z"></path>',
        label: 'New canvas',
        action: () => promptNewCanvas(targetItem)
      },
      {
        icon: '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"></path>',
        label: 'New base',
        action: () => promptNewBase(targetItem)
      }
    ];
  }
  
  menuItems.forEach(item => {
    const menuItem = document.createElement('div');
    menuItem.className = `context-menu-item ${item.color || ''}`;
    menuItem.innerHTML = `
      <svg fill="none" stroke="currentColor" viewBox="0 0 24 24">
        ${item.icon}
      </svg>
      <span>${item.label}</span>
    `;
    menuItem.addEventListener('click', async () => {
      removeContextMenu();
      await item.action();
    });
    contextMenu.appendChild(menuItem);
  });
  
  document.body.appendChild(contextMenu);
  
  // Close menu on click outside
  setTimeout(() => {
    document.addEventListener('click', removeContextMenu);
  }, 0);
}

function removeContextMenu() {
  if (contextMenu) {
    contextMenu.remove();
    contextMenu = null;
    document.removeEventListener('click', removeContextMenu);
  }
}

// ============================================================================
// File Operations
// ============================================================================

function generateId() {
  return 'id_' + Math.random().toString(36).substr(2, 9);
}

async function promptNewNote(parentItem) {
  console.log('promptNewNote called, parentItem:', parentItem);
  const name = await showPrompt('Note name:');
  console.log('User entered name:', name);
  if (!name) return;
  await createNewFile(name, parentItem ? parentItem.id : null, 'note');
  showStatus(`✅ Note "${name}" created`, 'success');
}

async function createNewNoteAndOpen() {
  const name = await showPrompt('Note name:');
  if (!name) return;
  await createNewFile(name, null, 'note', true);
  showStatus(`✅ Note "${name}" created`, 'success');
}

async function promptNewFolder(parentItem) {
  console.log('promptNewFolder called, parentItem:', parentItem);
  const name = await showPrompt('Folder name:');
  console.log('User entered name:', name);
  if (!name) return;
  createNewFolder(name, parentItem ? parentItem.id : null);
  showStatus(`✅ Folder "${name}" created`, 'success');
}

async function promptNewCanvas(parentItem) {
  const name = await showPrompt('Canvas name:');
  if (!name) return;
  createNewFile(name, parentItem ? parentItem.id : null, 'canvas');
  showStatus(`✅ Canvas "${name}" created`, 'success');
}

async function promptNewBase(parentItem) {
  const name = await showPrompt('Base file name:');
  if (!name) return;
  createNewFile(name, parentItem ? parentItem.id : null, 'base');
  showStatus(`✅ Base "${name}" created`, 'success');
}

// ============================================================================
// File Management Functions
// ============================================================================

async function makeFileCopy(item) {
  console.log('Making copy of:', item);
  if (!item || !item.path) return;
  
  const copyName = await showPrompt(`Copy of "${item.name}":`);
  if (!copyName) return;
  
  const pathParts = item.path.split('/');
  pathParts[pathParts.length - 1] = copyName;
  const copyPath = pathParts.join('/');
  
  if (item.type === 'file') {
    // Read original file
    const readResult = await window.vaultAPI.readFile(item.path);
    if (!readResult.success) {
      showStatus('❌ Failed to read original file', 'error');
      return;
    }
    
    // Write copy
    const writeResult = await window.vaultAPI.writeFile(copyPath, readResult.content);
    if (writeResult.success) {
      await loadVaultFileSystem(true);
      showStatus(`✅ Copy created: ${copyName}`, 'success');
    } else {
      showStatus('❌ Failed to create copy', 'error');
    }
  } else {
    showStatus('❌ Copying folders not yet implemented', 'error');
  }
}

function showInExplorer(item) {
  console.log('Show in explorer:', item);
  showStatus(`ℹ️ Would open: ${item.name} in system explorer (not implemented in browser)`, 'info');
  // In a real app, you'd use Electron's shell.showItemInFolder()
}

async function renameItem(item) {
  console.log('Renaming:', item);
  if (!item || !item.path) return;
  
  const newName = await showPrompt(`Rename "${item.name}" to:`);
  if (!newName || newName === item.name) return;
  
  const pathParts = item.path.split('/');
  pathParts[pathParts.length - 1] = newName;
  const newPath = pathParts.join('/');
  
  const result = await window.vaultAPI.renamePath(item.path, newPath);
  
  if (result.success) {
    // Update tab name if file is open
    const tabIndex = openTabs.findIndex(tab => tab.id === item.id);
    if (tabIndex !== -1) {
      openTabs[tabIndex].name = newName;
      openTabs[tabIndex].path = newPath;
      renderTabs();
    }
    
    await loadVaultFileSystem(true);
    showStatus(`✅ Renamed to: ${newName}`, 'success');
  } else {
    showStatus('❌ Failed to rename', 'error');
  }
}

async function deleteItem(item) {
  console.log('Deleting:', item);
  if (!item || !item.path) return;
  
  const confirmMsg = item.type === 'folder' 
    ? `Delete folder "${item.name}" and all its contents?`
    : `Delete "${item.name}"?`;
  
  if (!confirm(confirmMsg)) return;
  
  const result = await window.vaultAPI.deletePath(item.path);
  
  if (result.success) {
    // Close tab if the deleted item is open
    const tabIndex = openTabs.findIndex(tab => tab.id === item.id);
    if (tabIndex !== -1) {
      closeTab(tabIndex);
    }
    
    await loadVaultFileSystem(true);
    showStatus(`✅ Deleted: ${item.name}`, 'success');
  } else {
    showStatus('❌ Failed to delete', 'error');
  }
}

function findItemById(id, tree = fileSystem) {
  if (tree.id === id) return tree;
  if (tree.children) {
    for (const child of tree.children) {
      const found = findItemById(id, child);
      if (found) return found;
    }
  }
  return null;
}

async function createNewFile(name, parentId, fileType = 'note', autoOpen = false) {
  if (!currentVaultPath) {
    showStatus('❌ No vault selected', 'error');
    return;
  }
  
  let extension = '.md';
  let defaultContent = `# ${name}\n\nStart writing...`;
  
  if (fileType === 'canvas') {
    extension = '.mm';
    defaultContent = `<!-- Canvas file: ${name} -->`;
  } else if (fileType === 'base') {
    extension = '.txt';
    defaultContent = `Base file: ${name}`;
  }
  
  const fileName = name.endsWith(extension) ? name : name + extension;
  
  // Determine parent path
  let parentPath = currentVaultPath;
  if (parentId) {
    const parent = findItemById(parentId);
    if (parent && parent.path) {
      parentPath = parent.path;
    }
  }
  
  const filePath = `${parentPath}/${fileName}`;
  
  // Write file to filesystem
  const result = await window.vaultAPI.writeFile(filePath, defaultContent);
  
  if (result.success) {
    // Reload filesystem to reflect changes
    await loadVaultFileSystem(true);
    
    // Find and open the new file
    const newFile = findFileByPath(filePath, fileSystem);
    if (newFile) {
      await openFile(newFile);
    }
  } else {
    showStatus('❌ Failed to create file', 'error');
  }
}

async function createNewFolder(name, parentId) {
  if (!currentVaultPath) {
    showStatus('❌ No vault selected', 'error');
    return;
  }
  
  // Determine parent path
  let parentPath = currentVaultPath;
  if (parentId) {
    const parent = findItemById(parentId);
    if (parent && parent.path) {
      parentPath = parent.path;
    }
  }
  
  const folderPath = `${parentPath}/${name}`;
  
  // Create directory in filesystem
  const result = await window.vaultAPI.createDirectory(folderPath);
  
  if (result.success) {
    // Reload filesystem to reflect changes
    await loadVaultFileSystem(true);
  } else {
    showStatus('❌ Failed to create folder', 'error');
  }
}

async function openFile(file) {
  // Check if file is already open in a tab
  const existingTabIndex = openTabs.findIndex(tab => tab.id === file.id);
  
  if (existingTabIndex !== -1) {
    // Switch to existing tab
    await switchToTab(existingTabIndex);
  } else {
    // Add new tab
    openTabs.push(file);
    activeTabIndex = openTabs.length - 1;
    await loadFileContent(file);
    renderTabs();
  }
  
  renderFileTree();
}

async function loadFileContent(file) {
  currentFile = file;
  
  if (file.path) {
    // Read from filesystem
    const result = await window.vaultAPI.readFile(file.path);
    if (result.success) {
      setEditorContent(result.content);
    } else {
      setEditorContent(`# ${file.name}\n\nError loading file...`);
      showStatus('❌ Failed to load file', 'error');
    }
  } else {
    setEditorContent('');
  }
}

// Helper functions for CodeMirror 6
function setEditorContent(content) {
  editor.dispatch({
    changes: {
      from: 0,
      to: editor.state.doc.length,
      insert: content
    }
  });
}

function getEditorContent() {
  return editor.state.doc.toString();
}

async function switchToTab(index) {
  if (index < 0 || index >= openTabs.length) return;
  
  // Save current file before switching
  if (currentFile && currentFile.path) {
    await saveCurrentFile();
  }
  
  activeTabIndex = index;
  await loadFileContent(openTabs[index]);
  renderTabs();
  renderFileTree();
}

function closeTab(index) {
  if (index < 0 || index >= openTabs.length) return;
  
  openTabs.splice(index, 1);
  
  if (openTabs.length === 0) {
    // No tabs left
    activeTabIndex = -1;
    currentFile = null;
    setEditorContent('');
  } else {
    // Adjust active tab index
    if (activeTabIndex >= openTabs.length) {
      activeTabIndex = openTabs.length - 1;
    }
    if (activeTabIndex >= 0) {
      loadFileContent(openTabs[activeTabIndex]);
    }
  }
  
  renderTabs();
  renderFileTree();
}

function renderTabs() {
  const tabsContainer = document.getElementById('tabsContainer');
  if (!tabsContainer) return;
  
  tabsContainer.innerHTML = '';
  
  openTabs.forEach((file, index) => {
    const tab = document.createElement('div');
    tab.className = `tab ${index === activeTabIndex ? 'active' : ''}`;
    tab.draggable = true;
    tab.dataset.index = index;
    
    tab.innerHTML = `
      <span class="tab-name">${file.name}</span>
      <button class="tab-close" data-index="${index}">
        <svg class="w-4 h-4" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M6 18L18 6M6 6l12 12"></path>
        </svg>
      </button>
    `;
    
    // Click to switch tab
    tab.addEventListener('click', (e) => {
      if (!e.target.closest('.tab-close')) {
        switchToTab(index);
      }
    });
    
    // Close button
    const closeBtn = tab.querySelector('.tab-close');
    closeBtn.addEventListener('click', (e) => {
      e.stopPropagation();
      closeTab(index);
    });
    
    // Drag and drop for reordering
    tab.addEventListener('dragstart', (e) => {
      e.dataTransfer.effectAllowed = 'move';
      e.dataTransfer.setData('text/plain', index.toString());
      setTimeout(() => {
        tab.classList.add('dragging');
      }, 0);
    });
    
    tab.addEventListener('dragend', (e) => {
      tab.classList.remove('dragging');
      // Remove drag-over class from all tabs
      document.querySelectorAll('.tab').forEach(t => t.classList.remove('drag-over'));
    });
    
    tab.addEventListener('dragenter', (e) => {
      e.preventDefault();
      if (!tab.classList.contains('dragging')) {
        tab.classList.add('drag-over');
      }
    });
    
    tab.addEventListener('dragleave', (e) => {
      e.preventDefault();
      tab.classList.remove('drag-over');
    });
    
    tab.addEventListener('dragover', (e) => {
      e.preventDefault();
      e.dataTransfer.dropEffect = 'move';
    });
    
    tab.addEventListener('drop', (e) => {
      e.preventDefault();
      e.stopPropagation();
      
      tab.classList.remove('drag-over');
      
      const fromIndex = parseInt(e.dataTransfer.getData('text/plain'));
      const toIndex = parseInt(tab.dataset.index);
      
      if (fromIndex !== toIndex && !isNaN(fromIndex) && !isNaN(toIndex)) {
        // Reorder tabs
        const [movedTab] = openTabs.splice(fromIndex, 1);
        openTabs.splice(toIndex, 0, movedTab);
        
        // Update active index
        if (activeTabIndex === fromIndex) {
          activeTabIndex = toIndex;
        } else if (fromIndex < activeTabIndex && toIndex >= activeTabIndex) {
          activeTabIndex--;
        } else if (fromIndex > activeTabIndex && toIndex <= activeTabIndex) {
          activeTabIndex++;
        }
        
        renderTabs();
      }
    });
    
    tabsContainer.appendChild(tab);
  });
}

async function saveCurrentFile() {
  if (currentFile && currentFile.path) {
    const content = getEditorContent();
    const result = await window.vaultAPI.writeFile(currentFile.path, content);
    
    if (!result.success) {
      showStatus('❌ Failed to save file', 'error');
    }
  }
}

function findFileByPath(path, tree) {
  if (tree.path === path) return tree;
  
  if (tree.children) {
    for (const child of tree.children) {
      const found = findFileByPath(path, child);
      if (found) return found;
    }
  }
  
  return null;
}

let saveTimeout;

// ============================================================================
// File Tree Rendering
// ============================================================================

function createFileElement(item, level = 0) {
  const container = document.createElement('div');
  
  if (item.type === 'folder') {
    const folderDiv = document.createElement('div');
    folderDiv.className = 'tree-item tree-folder';
    folderDiv.dataset.id = item.id;
    folderDiv.dataset.type = 'folder';
    folderDiv.style.paddingLeft = `${level * 12}px`;
    
    folderDiv.innerHTML = `
      <svg class="tree-chevron ${item.expanded ? 'expanded' : ''}" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 5l7 7-7 7"></path>
      </svg>
      <svg class="tree-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path>
      </svg>
      <span>${item.name}</span>
    `;
    
    folderDiv.addEventListener('click', (e) => {
      e.stopPropagation();
      item.expanded = !item.expanded;
      selectedItem = item;
      renderFileTree();
    });
    
    folderDiv.addEventListener('contextmenu', (e) => {
      e.preventDefault();
      e.stopPropagation();
      createContextMenu(e.clientX, e.clientY, item, 'item');
    });
    
    container.appendChild(folderDiv);
    
    if (item.children && item.children.length > 0) {
      const childrenDiv = document.createElement('div');
      childrenDiv.className = `tree-children ${item.expanded ? '' : 'collapsed'}`;
      
      item.children.forEach(child => {
        childrenDiv.appendChild(createFileElement(child, level + 1));
      });
      
      container.appendChild(childrenDiv);
    }
    
  } else if (item.type === 'file') {
    const fileDiv = document.createElement('div');
    fileDiv.className = `tree-item tree-file ${currentFile && currentFile.id === item.id ? 'active' : ''}`;
    fileDiv.dataset.id = item.id;
    fileDiv.dataset.type = 'file';
    fileDiv.style.paddingLeft = `${level * 12 + 12}px`;
    
    const icon = item.fileType === 'canvas' 
      ? '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M4 16l4.586-4.586a2 2 0 012.828 0L16 16m-2-2l1.586-1.586a2 2 0 012.828 0L20 14m-6-6h.01M6 20h12a2 2 0 002-2V6a2 2 0 00-2-2H6a2 2 0 00-2 2v12a2 2 0 002 2z"></path>'
      : '<path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z"></path>';
    
    fileDiv.innerHTML = `
      <svg class="tree-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24">
        ${icon}
      </svg>
      <span>${item.name}</span>
    `;
    
    fileDiv.addEventListener('click', (e) => {
      e.stopPropagation();
      openFile(item);
    });
    
    fileDiv.addEventListener('contextmenu', (e) => {
      e.preventDefault();
      e.stopPropagation();
      createContextMenu(e.clientX, e.clientY, item, 'item');
    });
    
    container.appendChild(fileDiv);
  }
  
  return container;
}

function findParentById(id, tree = fileSystem) {
  if (tree.children) {
    for (const child of tree.children) {
      if (child.id === id) return tree;
      const found = findParentById(id, child);
      if (found) return found;
    }
  }
  return null;
}

function renderFileTree() {
  fileTree.innerHTML = '';
  
  if (fileSystem.children.length === 0) {
    fileTree.innerHTML = '<div class="text-gray-500 text-xs p-2">Right-click to create files!</div>';
    return;
  }
  
  fileSystem.children.forEach(item => {
    fileTree.appendChild(createFileElement(item));
  });
}

// LocalStorage persistence removed - now using real filesystem via vaultAPI

// ============================================================================
// Button Event Handlers (initialized in DOMContentLoaded)
// ============================================================================

function initButtonHandlers() {
  console.log('=== Initializing button handlers ===');
  console.log('newNoteBtn:', newNoteBtn);
  console.log('newFolderBtn:', newFolderBtn);
  
  if (!newNoteBtn) {
    console.error('ERROR: newNoteBtn not found!');
    return;
  }
  
  if (!newFolderBtn) {
    console.error('ERROR: newFolderBtn not found!');
    return;
  }
  
  newNoteBtn.addEventListener('click', async (e) => {
    console.log('>>> NEW NOTE BUTTON CLICKED! <<<');
    e.preventDefault();
    e.stopPropagation();
    await promptNewNote(selectedItem);
  });

  newFolderBtn.addEventListener('click', async (e) => {
    console.log('>>> NEW FOLDER BUTTON CLICKED! <<<');
    e.preventDefault();
    e.stopPropagation();
    await promptNewFolder(selectedItem);
  });
  
  console.log('✅ Button handlers initialized successfully');
}

// ============================================================================
// Canvas Integration
// ============================================================================

function showStatus(message, type = 'info') {
  statusDiv.textContent = message;
  statusDiv.className = `status ${type}`;
  setTimeout(() => {
    statusDiv.className = 'status hidden';
  }, 3000);
}

async function openCanvas() {
  try {
    openCanvasBtn.disabled = true;
    const originalHTML = openCanvasBtn.innerHTML;
    openCanvasBtn.innerHTML = '<span class="text-xl">⏳</span><span>Opening...</span>';
    
    const result = await window.canvasAPI.openCanvas();
    
    if (result.success) {
      showStatus('✅ Canvas window opened!', 'success');
      openCanvasBtn.innerHTML = '<span class="text-xl">🎨</span><span>Canvas Open</span>';
      
      const checkInterval = setInterval(async () => {
        const isOpen = await window.canvasAPI.isCanvasOpen();
        if (!isOpen) {
          openCanvasBtn.disabled = false;
          openCanvasBtn.innerHTML = originalHTML;
          showStatus('Canvas window closed', 'info');
          clearInterval(checkInterval);
        }
      }, 1000);
    } else {
      showStatus(`❌ Failed: ${result.error || 'Unknown error'}`, 'error');
      openCanvasBtn.disabled = false;
      openCanvasBtn.innerHTML = originalHTML;
    }
  } catch (error) {
    showStatus(`❌ Error: ${error.message}`, 'error');
    openCanvasBtn.disabled = false;
  }
}

function initCanvasHandlers() {
  openCanvasBtn.addEventListener('click', openCanvas);
}

// ============================================================================
// Keyboard Shortcuts
// ============================================================================

document.addEventListener('DOMContentLoaded', () => {
  console.log('DOM fully loaded, starting app initialization...');
  initializeApp().catch(error => {
    console.error('Failed to initialize app:', error);
    showStatus(`❌ Initialization failed: ${error.message}`, 'error');
  });
});

document.addEventListener('keydown', (e) => {
  if (e.ctrlKey || e.metaKey) {
    if (e.key === 's') {
      e.preventDefault();
      saveCurrentFile();
      showStatus('✅ File saved', 'success');
    } else if (e.key === 'p') {
      e.preventDefault();
      openCanvas();
    } else if (e.key === 'n') {
      e.preventDefault();
      newNoteBtn.click();
    }
  }
});

// ============================================================================
// Vault UI Functions
// ============================================================================

function showVaultSelector() {
  renderVaultList();
  vaultSelectorModal.classList.remove('hidden');
  vaultSelectorModal.classList.add('flex');
}

function closeVaultSelector() {
  vaultSelectorModal.classList.add('hidden');
  vaultSelectorModal.classList.remove('flex');
}

function renderVaultList() {
  vaultList.innerHTML = '';
  
  vaultsConfig.vaults.forEach(vault => {
    const item = document.createElement('div');
    item.className = `vault-item ${vault.id === vaultsConfig.currentVault ? 'active' : ''}`;
    
    item.innerHTML = `
      <div class="flex items-center gap-2">
        <svg class="w-4 h-4 text-purple-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M3 7v10a2 2 0 002 2h14a2 2 0 002-2V9a2 2 0 00-2-2h-6l-2-2H5a2 2 0 00-2 2z"></path>
        </svg>
        <div>
          <div class="font-medium text-white">${vault.name}</div>
          <div class="text-xs text-gray-500">${vault.path}</div>
        </div>
      </div>
      ${vault.id === vaultsConfig.currentVault ? `
        <svg class="w-5 h-5 text-blue-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M5 13l4 4L19 7"></path>
        </svg>
      ` : ''}
    `;
    
    item.addEventListener('click', async () => {
      await switchToVault(vault.id);
    });
    
    vaultList.appendChild(item);
  });
}

function showVaultManager() {
  renderManagerVaultList();
  vaultManagerModal.classList.remove('hidden');
  vaultManagerModal.classList.add('flex');
}

function closeVaultManager() {
  vaultManagerModal.classList.add('hidden');
  vaultManagerModal.classList.remove('flex');
}

function renderManagerVaultList() {
  managerVaultList.innerHTML = '';
  
  if (vaultsConfig.vaults.length === 0) {
    managerVaultList.innerHTML = '<div class="p-4 text-gray-500 text-center text-sm">No vaults yet</div>';
    return;
  }
  
  vaultsConfig.vaults.forEach(vault => {
    const item = document.createElement('div');
    item.className = `vault-manager-item ${vault.id === vaultsConfig.currentVault ? 'selected' : ''}`;
    
    item.innerHTML = `
      <div class="font-medium text-white">${vault.name}</div>
      <div class="text-xs text-gray-500 mt-1">${vault.path}</div>
    `;
    
    item.addEventListener('click', () => {
      // Select for future actions (e.g., delete)
      managerVaultList.querySelectorAll('.vault-manager-item').forEach(el => {
        el.classList.remove('selected');
      });
      item.classList.add('selected');
    });
    
    managerVaultList.appendChild(item);
  });
}

// ============================================================================
// Initialization
// ============================================================================

// Enhanced debug logging
const debug = {
  log: (...args) => console.log('[DEBUG]', new Date().toISOString(), ...args),
  error: (...args) => console.error('[ERROR]', new Date().toISOString(), ...args),
  warn: (...args) => console.warn('[WARN]', new Date().toISOString(), ...args),
  dom: (element) => {
    if (!element) return 'null';
    return {
      id: element.id,
      tag: element.tagName,
      class: element.className,
      parent: element.parentNode ? element.parentNode.tagName : 'no-parent',
      display: window.getComputedStyle(element).display,
      visibility: window.getComputedStyle(element).visibility,
      dimensions: element.getBoundingClientRect()
    };
  }
};

// Wait for both DOM and CodeMirror to be ready
async function initializeApp() {
  debug.log('Initializing application...');
  debug.log('Window object:', {
    codeMirrorReady: !!window.codeMirrorReady,
    CodeMirrorEditor: !!window.CodeMirrorEditor,
    documentReadyState: document.readyState
  });
  
  try {
    debug.log('Waiting for CodeMirror to be ready...');
    const CodeMirrorEditor = await window.codeMirrorReady;
    debug.log('CodeMirror ready, initializing editor...', {
      CodeMirrorEditor: !!CodeMirrorEditor,
      createEditor: !!CodeMirrorEditor?.createEditor
    });
    
    if (!CodeMirrorEditor || typeof CodeMirrorEditor.createEditor !== 'function') {
      throw new Error('CodeMirror editor factory function not found');
    }
    
    // Initialize DOM elements with debug logging
    const elements = {
      openCanvasBtn: document.getElementById('openCanvasBtn'),
      statusDiv: document.getElementById('status'),
      fileTree: document.getElementById('fileTree'),
      filePanel: document.getElementById('filePanel'),
      editorContainer: document.getElementById('editor'),
      newNoteBtn: document.getElementById('newNoteBtn'),
      newFolderBtn: document.getElementById('newFolderBtn'),
      tabsContainer: document.getElementById('tabsContainer'),
      newTabBtn: document.getElementById('newTabBtn')
    };
    
    // Assign to global variables
    Object.assign(window, elements);
    
    debug.log('DOM elements initialized:', {
      editorContainer: debug.dom(editorContainer),
      filePanel: debug.dom(filePanel),
      tabsContainer: debug.dom(tabsContainer),
      newTabBtn: debug.dom(newTabBtn)
    });
    
    if (!editorContainer) {
      throw new Error('Editor container not found in DOM');
    }
    
    // Debug container visibility
    console.log('Editor container:', {
      element: editorContainer,
      isConnected: editorContainer?.isConnected,
      parent: editorContainer?.parentElement,
      display: editorContainer ? window.getComputedStyle(editorContainer).display : 'N/A',
      visibility: editorContainer ? window.getComputedStyle(editorContainer).visibility : 'N/A',
      dimensions: editorContainer?.getBoundingClientRect()
    });
    
    // Ensure container is visible and has dimensions
    if (editorContainer) {
      editorContainer.style.display = 'block';
      editorContainer.style.height = '100%';
      editorContainer.style.width = '100%';
      editorContainer.style.overflow = 'auto';
    } else {
      console.error('Editor container not found!');
      showStatus('❌ Error: Editor container not found', 'error');
      return;
    }
    
    // Initialize CodeMirror 6 Editor
    debug.log('Creating CodeMirror editor instance...');
    
    try {
      debug.log('Calling CodeMirrorEditor.createEditor...');
      editor = CodeMirrorEditor.createEditor(editorContainer, '# Hello World\n\nThis is a test note.', (content) => {
        debug.log('Editor content changed:', content?.substring(0, 50) + (content?.length > 50 ? '...' : ''));
        clearTimeout(saveTimeout);
        saveTimeout = setTimeout(saveCurrentFile, 1000);
      });
      
      debug.log('Editor created successfully:', {
        editor: !!editor,
        getValue: typeof editor?.getValue === 'function',
        setValue: typeof editor?.setValue === 'function'
      });
      
      // Show success status
      showStatus('Editor initialized successfully', 'success');
      
    } catch (error) {
      debug.error('Failed to create editor:', error);
      showStatus(`❌ Editor error: ${error.message}`, 'error');
      
      // Create a visible error message
      const errorDiv = document.createElement('div');
      errorDiv.style.padding = '1rem';
      errorDiv.style.margin = '1rem';
      errorDiv.style.border = '1px solid #f87171';
      errorDiv.style.borderRadius = '0.5rem';
      errorDiv.style.backgroundColor = '#7f1d1d';
      errorDiv.style.color = '#fecaca';
      errorDiv.innerHTML = `
        <h3 style="font-weight: bold; margin-bottom: 0.5rem;">Editor Initialization Failed</h3>
        <p>${error.message}</p>
        <pre style="margin-top: 0.5rem; font-size: 0.8em; opacity: 0.8; overflow: auto; max-height: 200px;">${error.stack || 'No stack trace available'}</pre>
      `;
      
      editorContainer.appendChild(errorDiv);
      return;
    }
    
    // Initialize modal elements
    inputModal = document.getElementById('inputModal');
    modalTitle = document.getElementById('modalTitle');
    modalInput = document.getElementById('modalInput');
    modalCancel = document.getElementById('modalCancel');
    modalConfirm = document.getElementById('modalConfirm');
    
    // Initialize vault elements
    vaultFooter = document.getElementById('vaultFooter');
    currentVaultName = document.getElementById('currentVaultName');
    vaultSelectorModal = document.getElementById('vaultSelectorModal');
    vaultList = document.getElementById('vaultList');
    manageVaultsBtn = document.getElementById('manageVaultsBtn');
    vaultManagerModal = document.getElementById('vaultManagerModal');
    managerVaultList = document.getElementById('managerVaultList');
    createVaultBtn = document.getElementById('createVaultBtn');
    openVaultBtn = document.getElementById('openVaultBtn');
    closeManagerBtn = document.getElementById('closeManagerBtn');
  });
  
  closeManagerBtn.addEventListener('click', () => {
    closeVaultManager();
  });
  
  // Close vault selector on click outside
  vaultSelectorModal.addEventListener('click', (e) => {
    if (e.target === vaultSelectorModal) {
      closeVaultSelector();
    }
  });
  
  // Close vault manager on click outside
  vaultManagerModal.addEventListener('click', (e) => {
    if (e.target === vaultManagerModal) {
      closeVaultManager();
    }
  });
  
  // New tab button handler
  newTabBtn.addEventListener('click', async () => {
    await createNewNoteAndOpen();
  });
  
  // Auto-save is now handled in editor initialization above
  
  // Right-click on file panel area (includes header and file tree)
  filePanel.addEventListener('contextmenu', (e) => {
    // Check if we're clicking on a file/folder item
    const isTreeItem = e.target.closest('.tree-item');
    
    // If not clicking on a tree item, show create menu
    if (!isTreeItem) {
      e.preventDefault();
      createContextMenu(e.clientX, e.clientY, null, 'create');
    }
  });
  
  // Load vaults configuration and initialize
  await loadVaultsConfig();
  
  showStatus('Right-click to create files!', 'info');
  console.log('Initialization complete!');
});

function findFirstFile(tree) {
  if (tree.type === 'file') return tree;
  if (tree.children) {
    for (const child of tree.children) {
      const file = findFirstFile(child);
      if (file) return file;
    }
  }
  return null;
}
