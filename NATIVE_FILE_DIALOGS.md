# Diálogos de Archivo Nativos del Sistema

## ✅ Implementación Completada

Se implementó el sistema de guardado y carga usando **diálogos nativos de Ubuntu GNOME** mediante `zenity`.

---

## 🎯 Lo Que Pediste

> "I need my file system explorer from my system in my case ubuntu gnome, to open up, not the app's gui"

**✅ SOLUCIONADO:** Ahora se usa el **explorador de archivos nativo de Ubuntu GNOME** (GTK file chooser).

---

## 🚀 Cómo Funciona Ahora

### Guardar (Ctrl+S)
1. Presionas `Ctrl+S`
2. Se abre el **diálogo nativo de GNOME** (el mismo que ves en Nautilus, gedit, etc.)
3. Puedes navegar por **TODO tu sistema de archivos**
4. Ves tus **ubicaciones favoritas** (Documentos, Descargas, etc.)
5. Filtros automáticos para archivos `.mm`
6. Confirmación si vas a sobrescribir un archivo existente
7. Guardas donde quieras

### Abrir (Ctrl+O)
1. Presionas `Ctrl+O`
2. Se abre el **diálogo nativo de GNOME**
3. Navegas por todo tu sistema
4. Seleccionas un archivo `.mm`
5. Se carga inmediatamente

---

## 🔧 Implementación Técnica

### Herramienta Usada: Zenity

```bash
# Comando ejecutado internamente para guardar:
zenity --file-selection --save --confirm-overwrite \
       --title="Guardar Dibujo" \
       --filename="drawing.mm" \
       --file-filter="Mind Map Files (*.mm) | *.mm"

# Comando ejecutado internamente para abrir:
zenity --file-selection \
       --title="Abrir Dibujo" \
       --file-filter="Mind Map Files (*.mm) | *.mm"
```

### Código C++

```cpp
std::string openNativeFileDialog(bool isSave) {
    // Construye el comando zenity apropiado
    std::string command = isSave ? 
        "zenity --file-selection --save ..." :
        "zenity --file-selection ...";
    
    // Ejecuta zenity y captura la ruta seleccionada
    FILE* pipe = popen(command.c_str(), "r");
    char buffer[512];
    std::string filepath = "";
    
    if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        filepath = buffer;
        // Elimina newline al final
        if (!filepath.empty() && filepath.back() == '\n') {
            filepath.pop_back();
        }
    }
    
    pclose(pipe);
    return filepath; // Retorna la ruta o string vacío si canceló
}
```

---

## ✨ Ventajas de Esta Solución

### Para el Usuario
- ✅ **Familiar**: Es el mismo diálogo que usas en todas tus apps de GNOME
- ✅ **Completo**: Acceso a todo el sistema de archivos
- ✅ **Integrado**: Tus ubicaciones favoritas, recientes, montajes, etc.
- ✅ **Consistente**: Look & feel nativo de Ubuntu
- ✅ **Potente**: Todas las características del file chooser de GTK

### Para el Desarrollo
- ✅ **Sin dependencias extra**: `zenity` viene preinstalado en Ubuntu GNOME
- ✅ **Simple**: Solo ~50 líneas de código
- ✅ **Portable**: Fácil de adaptar a otros sistemas (kdialog para KDE, etc.)
- ✅ **Liviano**: No agrega peso a la aplicación
- ✅ **Mantenible**: No hay UI personalizada que mantener

---

## 📋 Requisitos

### Sistema Operativo
- **Ubuntu GNOME** (o cualquier distribución con GNOME)
- **Zenity** instalado (viene por defecto)

### Verificar Instalación de Zenity
```bash
which zenity
# Debería mostrar: /usr/bin/zenity

zenity --version
# Debería mostrar algo como: 3.44.0
```

### Si Zenity No Está Instalado
```bash
sudo apt install zenity
```

---

## 🎮 Uso

### Atajos de Teclado
- `Ctrl+S` → Abrir diálogo de **Guardar**
- `Ctrl+O` → Abrir diálogo de **Abrir**

### Comportamiento
1. **Guardar**:
   - Se abre el diálogo nativo de GNOME
   - Puedes navegar a cualquier carpeta
   - Escribe el nombre del archivo (sin extensión)
   - La extensión `.mm` se agrega automáticamente si no la pones
   - Si el archivo existe, te pregunta si quieres sobrescribir

2. **Abrir**:
   - Se abre el diálogo nativo de GNOME
   - Solo muestra archivos `.mm` (puedes cambiar a "Todos los archivos")
   - Selecciona y abre
   - El dibujo actual se reemplaza (sin advertencia)

3. **Cancelar**:
   - En ambos casos, puedes presionar "Cancelar" o ESC
   - No pasa nada, vuelves a la aplicación

---

## 🗂️ Ejemplos de Uso

### Guardar en Documentos
```
1. Ctrl+S
2. [Diálogo nativo se abre]
3. Click en "Documentos" en la barra lateral
4. Escribe "mi_dibujo"
5. Click "Guardar"
→ Se guarda en: /home/tu_usuario/Documentos/mi_dibujo.mm
```

### Guardar en Carpeta Personalizada
```
1. Ctrl+S
2. [Diálogo nativo se abre]
3. Navega a /home/tu_usuario/Proyectos/Arte/
4. Click "Crear carpeta" si quieres una nueva
5. Escribe "boceto_final"
6. Click "Guardar"
→ Se guarda en: /home/tu_usuario/Proyectos/Arte/boceto_final.mm
```

### Abrir Desde Cualquier Lugar
```
1. Ctrl+O
2. [Diálogo nativo se abre]
3. Navega a donde sea: USB, red, home, etc.
4. Selecciona tu archivo .mm
5. Click "Abrir"
→ Se carga el archivo
```

---

## 🔄 Comparación: Antes vs Ahora

### ❌ Antes (ImGui Custom Dialog)
- Diálogo personalizado dentro de la app
- Solo podías navegar directorios uno por uno
- Sin acceso a favoritos del sistema
- UI diferente al resto del sistema
- Más código para mantener

### ✅ Ahora (Zenity Native Dialog)
- Diálogo nativo de GNOME
- Navegación completa e instantánea
- Acceso a todas las ubicaciones del sistema
- UI consistente con el resto de Ubuntu
- Código simple y mantenible

---

## 🐛 Troubleshooting

### Problema: "Error: No se pudo abrir zenity"
**Causa:** Zenity no está instalado  
**Solución:**
```bash
sudo apt install zenity
```

### Problema: El diálogo no aparece
**Causa:** La aplicación está en background  
**Solución:** El diálogo puede estar detrás de otras ventanas, búscalo en el switcher (Alt+Tab)

### Problema: No veo mis archivos .mm
**Causa:** Estás en el directorio incorrecto o el filtro está activo  
**Solución:** 
1. Navega al directorio correcto
2. O cambia el filtro a "Todos los archivos"

### Problema: No funciona en otras distribuciones
**Solución:** 
- En KDE: Cambiar `zenity` por `kdialog`
- En otras: Instalar zenity o usar alternativa similar

---

## 📝 Código Relevante

### Archivos Modificados
- `src/main.cpp`: Implementación de diálogos nativos
- `src/Canvas.cpp`: Funciones de save/load (sin cambios)
- `include/Canvas.h`: Declaraciones save/load (sin cambios)

### Funciones Clave
```cpp
// main.cpp
std::string openNativeFileDialog(bool isSave);
void processFileDialogs();
void keyCallback(...); // Detecta Ctrl+S y Ctrl+O

// Canvas.cpp
bool Canvas::saveToFile(const std::string& filepath);
bool Canvas::loadFromFile(const std::string& filepath);
```

---

## 🚀 Testing

### Compilar y Ejecutar
```bash
cd /home/jcl/CascadeProjects/C++
./build.sh
./build/VectorSketch
```

### Probar Guardar
```
1. Dibuja algo
2. Ctrl+S
3. Verifica que se abre el diálogo nativo de GNOME
4. Navega a un directorio de tu elección
5. Guarda con un nombre
6. Verifica en el terminal: "✓ Archivo guardado: /ruta/archivo.mm"
```

### Probar Abrir
```
1. Ctrl+O
2. Verifica que se abre el diálogo nativo de GNOME
3. Navega al archivo que guardaste
4. Ábrelo
5. Verifica en el terminal: "✓ Archivo cargado: /ruta/archivo.mm"
6. Verifica que tu dibujo aparece en el canvas
```

---

## ✅ Resumen

### Lo Que Se Logró
1. ✅ Diálogos **nativos de Ubuntu GNOME** (GTK)
2. ✅ Navegación **completa** del sistema de archivos
3. ✅ Integración con **ubicaciones del sistema**
4. ✅ Filtros de archivo **automáticos** para `.mm`
5. ✅ Confirmación de **sobrescritura**
6. ✅ **Sin dependencias extra** (usa zenity preinstalado)
7. ✅ Código **simple y mantenible**
8. ✅ Experiencia **consistente** con el SO

### Funcionalidad Completa
- Guardar: `Ctrl+S` → Diálogo nativo → Guarda donde quieras
- Abrir: `Ctrl+O` → Diálogo nativo → Abre desde donde quieras
- Formato: `.mm` binario con toda la información del dibujo
- Persistencia: Colores, grosores, presión, tilt, timestamps

---

## 🎉 ¡Listo Para Usar!

Tu aplicación ahora usa el **explorador de archivos de tu sistema operativo**, exactamente como pediste. Es la misma experiencia que tienes en cualquier otra aplicación de GNOME.

```bash
./build/VectorSketch
# Dibuja → Ctrl+S → ¡Diálogo nativo de GNOME! 🎨
```
