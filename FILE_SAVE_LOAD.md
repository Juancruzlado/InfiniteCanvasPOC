# Sistema de Guardado y Carga de Archivos .mm

## Descripción General
Sistema completo de persistencia para guardar y cargar dibujos vectoriales en archivos con extensión `.mm` (Mind Map). Los archivos preservan todos los trazos con su información completa: puntos, colores, grosores y datos de presión.

---

## Formato de Archivo .mm

### Estructura Binaria

El formato `.mm` es un formato binario personalizado que garantiza guardado y carga rápidos:

```
┌─────────────────────────────────────┐
│ HEADER                              │
├─────────────────────────────────────┤
│ Magic Number: "MMVS" (4 bytes)     │ ← Mind Map Vector Sketch
│ Version: uint32 (4 bytes)          │ ← Versión del formato (actualmente 1)
├─────────────────────────────────────┤
│ Num Strokes: uint32 (4 bytes)      │ ← Cantidad de trazos
├─────────────────────────────────────┤
│ STROKE 1                            │
│   Color RGB: 3 floats (12 bytes)   │
│   Base Width: float (4 bytes)      │
│   Num Points: uint32 (4 bytes)     │
│   ┌─────────────────────────────┐  │
│   │ POINT 1                     │  │
│   │   Position X,Y: 2 floats    │  │
│   │   Pressure: float           │  │
│   │   Tilt X,Y: 2 floats        │  │
│   │   Timestamp: float          │  │
│   └─────────────────────────────┘  │
│   ... (más puntos)                  │
├─────────────────────────────────────┤
│ STROKE 2                            │
│   ... (igual estructura)            │
├─────────────────────────────────────┤
│ ... (más trazos)                    │
└─────────────────────────────────────┘
```

### Ventajas del Formato

1. **Compacto**: Binario es más pequeño que texto (JSON/XML)
2. **Rápido**: Lectura/escritura directa sin parsing
3. **Completo**: Preserva toda la información del trazo
4. **Versionado**: Sistema de versiones para compatibilidad futura
5. **Validación**: Magic number previene archivos corruptos

---

## Uso del Sistema

### Guardar Archivo (Ctrl+S)

```
1. Presiona Ctrl+S
2. Se abre el EXPLORADOR DE ARCHIVOS NATIVO de Ubuntu GNOME (GTK)
3. Navega por tu sistema de archivos usando el diálogo familiar de GNOME
4. Elige el directorio donde quieres guardar
5. Escribe el nombre del archivo
6. Presiona "Guardar"
```

**Características del Diálogo de Guardado:**
- ✅ **Diálogo nativo de Ubuntu GNOME** (no una UI personalizada)
- ✅ Filtros de archivo: Muestra solo archivos `.mm` por defecto
- ✅ Navegación completa por todo el sistema de archivos
- ✅ Confirmación automática al sobrescribir archivos existentes
- ✅ Auto-agrega extensión `.mm` si no está presente
- ✅ Acceso directo a favoritos, ubicaciones recientes, etc.

### Abrir Archivo (Ctrl+O)

```
1. Presiona Ctrl+O
2. Se abre el EXPLORADOR DE ARCHIVOS NATIVO de Ubuntu GNOME (GTK)
3. Navega por tu sistema de archivos
4. Selecciona un archivo .mm
5. Presiona "Abrir"
```

**Características del Diálogo de Apertura:**
- ✅ **Diálogo nativo de Ubuntu GNOME** (GTK file chooser)
- ✅ Filtros: Muestra archivos `.mm` y opción "Todos los archivos"
- ✅ Navegación completa del sistema
- ✅ Vista previa de miniaturas (si el sistema la soporta)
- ✅ Acceso a ubicaciones del sistema (Documentos, Descargas, etc.)

---

## Detalles Técnicos

### Datos Guardados por Trazo

Cada trazo incluye:

```cpp
struct SavedStroke {
    // Propiedades del trazo
    glm::vec3 color;      // RGB (3 floats)
    float baseWidth;       // Grosor base
    uint32_t numPoints;    // Cantidad de puntos
    
    // Cada punto contiene
    struct Point {
        glm::vec2 position;   // X, Y
        float pressure;       // 0.0 - 1.0
        float tiltX;          // -1.0 - 1.0
        float tiltY;          // -1.0 - 1.0
        float timestamp;      // Tiempo en segundos
    };
};
```

### Validaciones al Cargar

El sistema verifica:
1. ✅ **Magic Number**: Confirma que es un archivo `.mm` válido
2. ✅ **Versión**: Solo carga versión 1 (expandible en el futuro)
3. ✅ **Integridad**: Maneja excepciones de lectura
4. ✅ **Tipos de datos**: Lectura binaria directa con validación

### Manejo de Errores

```cpp
// Errores comunes y sus mensajes
"Failed to open file for writing"     → No se puede crear el archivo
"Failed to open file for reading"     → Archivo no existe o sin permisos
"Invalid file format (bad magic)"     → No es un archivo .mm válido
"Unsupported file version"            → Versión no compatible
"Error saving/loading file"           → Error general durante operación
```

---

## Integración con el Sistema

### Historial Undo/Redo

Después de cargar un archivo:
- Se **limpia el historial** actual
- Se **inicializa un nuevo historial** con el estado cargado
- El historial comienza desde ese punto
- Los cambios nuevos se pueden deshacer normalmente

### Vista del Canvas

Al cargar un archivo:
- Los trazos se cargan en las **posiciones originales** (coordenadas de mundo)
- La **vista NO se ajusta** automáticamente
- Puedes usar `R` para resetear la vista si es necesario
- Usa pan/zoom para navegar al contenido cargado

### Compatibilidad con Herramientas

El sistema guarda y carga:
- ✅ Trazos de **brush** (con su color)
- ✅ Trazos de **eraser** (color blanco)
- ✅ Todos los **grosores** (0.01 - 200 pts)
- ✅ Datos de **presión y tilt** de cada punto
- ✅ Información de **timestamp** para cada punto

---

## Flujo de Trabajo Típico

### Sesión de Trabajo

```
1. Abre la aplicación
2. Dibuja tu contenido
3. Ctrl+S → Guarda como "proyecto1.mm"
4. Continúa dibujando
5. Ctrl+S → Guarda (sobrescribe "proyecto1.mm")
6. Cierra la aplicación

7. (Más tarde) Abre la aplicación
8. Ctrl+O → Carga "proyecto1.mm"
9. Continúa trabajando desde donde lo dejaste
```

### Múltiples Archivos

```
Sketch A:
  Ctrl+S → "boceto_inicial.mm"
  
Sketch B (nuevo):
  C (limpiar canvas)
  Dibuja nuevo contenido
  Ctrl+S → "boceto_final.mm"
  
Volver a Sketch A:
  Ctrl+O → Selecciona "boceto_inicial.mm"
  Continúa editando
  Ctrl+S → Sobrescribe "boceto_inicial.mm"
```

---

## Atajos de Teclado

| Atajo | Acción |
|-------|--------|
| **Ctrl+S** | Abrir diálogo de guardado |
| **Ctrl+O** | Abrir diálogo de carga |

---

## Consideraciones Importantes

### ⚠️ Advertencias

1. **Sin confirmación al cargar**: Cargar un archivo reemplaza el dibujo actual sin preguntar
2. **Sin auto-guardado**: Debes guardar manualmente con Ctrl+S
3. **Sobrescritura directa**: Guardar con el mismo nombre sobrescribe sin avisar
4. **Sin sistema de backups**: Un solo archivo por dibujo

### 💡 Recomendaciones

1. **Guarda frecuentemente**: Usa Ctrl+S regularmente
2. **Nombres descriptivos**: Usa nombres que identifiquen el contenido
3. **Varias versiones**: Guarda versiones distintas ("v1.mm", "v2.mm", etc.)
4. **Organiza por carpetas**: Navega a carpetas específicas antes de guardar

---

## Implementación Interna

### Canvas.h / Canvas.cpp

```cpp
// Métodos públicos
bool saveToFile(const std::string& filepath);
bool loadFromFile(const std::string& filepath);
```

**Guardado:**
1. Abre archivo en modo binario
2. Escribe header (magic + version)
3. Escribe número de trazos
4. Para cada trazo:
   - Escribe color, ancho, num puntos
   - Para cada punto: escribe todos sus datos
5. Cierra archivo

**Carga:**
1. Abre archivo en modo binario
2. Lee y valida header
3. Lee número de trazos
4. Limpia canvas actual
5. Para cada trazo:
   - Lee color, ancho, num puntos
   - Para cada punto: lee todos sus datos
   - Agrega trazo reconstruido al canvas
6. Inicializa historial

### main.cpp

**Sistema de Diálogos Nativos con Zenity:**

```cpp
// Variables globales
bool showSaveDialog = false;
bool showOpenDialog = false;

// Función para abrir diálogo nativo
std::string openNativeFileDialog(bool isSave) {
    std::string command;
    
    if (isSave) {
        command = "zenity --file-selection --save --confirm-overwrite "
                  "--title=\"Guardar Dibujo\" "
                  "--filename=\"drawing.mm\" "
                  "--file-filter=\"Mind Map Files (*.mm) | *.mm\" ";
    } else {
        command = "zenity --file-selection "
                  "--title=\"Abrir Dibujo\" "
                  "--file-filter=\"Mind Map Files (*.mm) | *.mm\" ";
    }
    
    // Ejecuta zenity y captura el resultado
    FILE* pipe = popen(command.c_str(), "r");
    // ... lee la ruta del archivo ...
    pclose(pipe);
    return filepath;
}

// Procesamiento de diálogos
void processFileDialogs();

// Hotkeys en keyCallback
if (ctrlPressed && key == GLFW_KEY_S) showSaveDialog = true;
if (ctrlPressed && key == GLFW_KEY_O) showOpenDialog = true;
```

**¿Qué es Zenity?**

`zenity` es una herramienta de línea de comandos que viene preinstalada en Ubuntu GNOME. Permite crear diálogos GTK nativos desde cualquier aplicación. Ventajas:

- ✅ **Nativo**: Usa el mismo diálogo que todas las apps de GNOME
- ✅ **Familiar**: El usuario ve su explorador de archivos habitual
- ✅ **Integrado**: Acceso a marcadores, ubicaciones recientes, etc.
- ✅ **Sin dependencias**: Ya está instalado en Ubuntu GNOME
- ✅ **Liviano**: No agrega peso a la aplicación

**Requisito:** `zenity` debe estar instalado (viene por defecto en Ubuntu GNOME). Si no está disponible, aparecerá un mensaje de error en la consola.

---

## Futuras Mejoras

### Corto Plazo
- [ ] Confirmación antes de cargar (para evitar pérdida de cambios)
- [ ] Confirmación antes de sobrescribir
- [ ] Mensaje de éxito visible en UI (no solo consola)
- [ ] Mostrar nombre del archivo actual en título de ventana

### Mediano Plazo
- [ ] Auto-guardado cada X minutos
- [ ] Sistema de backups automáticos
- [ ] Vista previa de thumbnails en el diálogo
- [ ] Búsqueda/filtrado de archivos

### Largo Plazo
- [ ] Exportar a SVG/PDF
- [ ] Importar imágenes de referencia
- [ ] Sistema de capas guardable
- [ ] Metadatos (fecha creación, modificación, autor)

---

## Ejemplo de Uso

### Código de Usuario
```cpp
// En tu aplicación
Canvas canvas;

// Guardar
if (canvas.saveToFile("/home/user/drawings/sketch.mm")) {
    std::cout << "Guardado exitoso" << std::endl;
}

// Cargar
if (canvas.loadFromFile("/home/user/drawings/sketch.mm")) {
    std::cout << "Cargado exitoso" << std::endl;
}
```

### Desde la Aplicación
```
$ ./build/VectorSketch

# Dibuja algo
[Mouse drawing...]

# Guarda
[Ctrl+S]
→ Navega a /home/user/dibujos/
→ Escribe "mi_obra"
→ Click "Guardar"
→ Se crea: /home/user/dibujos/mi_obra.mm

# Continúa dibujando
[Mouse drawing...]

# Guarda cambios
[Ctrl+S]
→ Ya en el directorio correcto
→ Nombre ya está: "mi_obra"
→ Click "Guardar" (sobrescribe)

# Cierra aplicación
[ESC]

# Más tarde...
$ ./build/VectorSketch

# Carga el dibujo
[Ctrl+O]
→ Navega a /home/user/dibujos/
→ Click en "mi_obra.mm"
→ ¡Se carga al instante!
```

---

## Troubleshooting

### Problema: "Failed to open file for writing"
**Solución**: 
- Verifica permisos de escritura en el directorio
- Asegúrate de que el directorio existe
- No uses caracteres especiales en el nombre

### Problema: "Invalid file format"
**Solución**:
- El archivo no es un .mm válido
- Puede estar corrupto
- Fue creado con otra aplicación

### Problema: El archivo se guardó pero está vacío
**Solución**:
- Verifica que haya trazos dibujados antes de guardar
- El canvas puede estar vacío

### Problema: Al cargar no veo nada
**Solución**:
- Los trazos pueden estar fuera de la vista actual
- Presiona `R` para resetear la vista
- Usa pan/zoom para buscar el contenido

---

## Conclusión

El sistema de guardado/carga `.mm` proporciona:

✅ **Persistencia completa** de dibujos vectoriales  
✅ **Formato eficiente** y rápido  
✅ **Interfaz intuitiva** con explorador de archivos  
✅ **Hotkeys estándar** (Ctrl+S, Ctrl+O)  
✅ **Validación robusta** de archivos  
✅ **Integración perfecta** con undo/redo  

¡Guarda tu trabajo y continúa más tarde sin perder nada! 🎨💾
