# Changelog - Herramienta de Goma de Borrar

## Fecha: Octubre 25, 2025

### ✨ Nueva Característica: Goma de Borrar

Se agregó una herramienta de goma de borrar al tool wheel que funciona dibujando con color blanco (del fondo) y usa los mismos controles de grosor que el pincel.

---

## Cambios Realizados

### 1. **ToolWheel.h** - Definición de herramienta
- ✅ Agregado `ToolType::ERASER` al enum de herramientas
- ✅ Agregado método `getEffectiveColor()` que retorna:
  - Blanco (1.0, 1.0, 1.0) cuando la goma está seleccionada
  - Color del usuario cuando el pincel está seleccionado

```cpp
enum class ToolType {
    BRUSH,
    ERASER
};

glm::vec3 getEffectiveColor() const { 
    return (currentTool == ToolType::ERASER) ? 
           glm::vec3(1.0f, 1.0f, 1.0f) : 
           currentColor; 
}
```

### 2. **ToolWheel.cpp** - Interfaz visual
- ✅ Agregado **segmento inferior** del anillo externo para la goma
- ✅ Implementada **detección de clicks** con cálculo de ángulo del mouse
- ✅ Agregados **colores visuales** para indicar herramienta activa:
  - **Brush seleccionado**: Negro oscuro (40, 40, 40)
  - **Brush no seleccionado**: Gris medio (100, 100, 100)
  - **Eraser seleccionado**: Rojo claro (240, 100, 100)
  - **Eraser no seleccionado**: Gris claro (150, 150, 150)
- ✅ Agregados **iconos distintivos**:
  - Brush: Círculo blanco pequeño (arriba)
  - Eraser: Cuadrado blanco (abajo)

### 3. **main.cpp** - Lógica de dibujo
- ✅ Cambiado `getCurrentColor()` por `getEffectiveColor()`
- ✅ Ahora usa color blanco automáticamente cuando la goma está activa
- ✅ Mantiene compatibilidad total con el sistema existente

```cpp
// Antes:
glm::vec3 color = toolWheel.getCurrentColor();

// Ahora:
glm::vec3 color = toolWheel.getEffectiveColor();
```

### 4. **Documentación**
- ✅ Creado `ERASER_TOOL.md` con guía completa de la herramienta
- ✅ Actualizado `README.md` con:
  - Nueva característica en la lista de features
  - Controles del tool wheel en la sección de controles
  - Instrucciones detalladas de uso
  - Características implementadas marcadas con ✅

---

## Funcionalidad

### Ubicación en el Tool Wheel
```
       [BRUSH] ← Arriba
         🔴
    ┌─────────┐
    │ ANILLO  │
    │ EXTERNO │
    │         │
    │  WIDTH  │ ← Anillo medio
    │         │
    │  COLOR  │ ← Centro
    └─────────┘
         🟦
      [ERASER] ← Abajo
```

### Cómo Seleccionar Herramientas
1. **Brush**: Click en segmento **superior** (arriba)
2. **Eraser**: Click en segmento **inferior** (abajo)
3. La herramienta activa se resalta visualmente

### Comportamiento de la Goma
- Dibuja con **color blanco** (RGB: 1.0, 1.0, 1.0)
- Usa el **mismo grosor** configurado para el pincel (0.01-200 pts)
- Usa la **misma suavización** de Bézier que el pincel
- Se puede **deshacer/rehacer** igual que trazos normales
- Escala correctamente con el **zoom** del canvas

### Ventajas del Diseño
1. **Simplicidad**: No requiere algoritmos de intersección complejos
2. **Consistencia**: Comportamiento idéntico al pincel
3. **Rendimiento**: Sin costo adicional de procesamiento
4. **Compatibilidad**: Funciona con todo el sistema existente (undo/redo, zoom, etc.)

---

## Testing Realizado

### Compilación
```bash
./build.sh
# ✅ Build exitoso sin errores
# ⚠️  Solo warnings de parámetros no usados (no críticos)
```

### Funcionalidad Verificada
- ✅ Selección de herramienta Brush
- ✅ Selección de herramienta Eraser
- ✅ Indicadores visuales funcionan correctamente
- ✅ Grosor compartido entre herramientas
- ✅ Color efectivo correcto (blanco para goma)
- ✅ Iconos distintivos visibles

---

## Arquitectura Técnica

### Patrón de Diseño
La implementación usa el **patrón Strategy** implícito:
- La misma interfaz de dibujo para ambas herramientas
- El comportamiento cambia según el estado de `currentTool`
- El método `getEffectiveColor()` actúa como selector de estrategia

### Flujo de Datos
```
Usuario Click → Detectar Ángulo → Cambiar currentTool
                                        ↓
Inicio Dibujo → getEffectiveColor() → beginStroke(color)
                                        ↓
                    currentTool == ERASER ? Blanco : currentColor
```

### Integración
- **Sin cambios** en Canvas, Stroke, BezierSmoother, VectorRenderer
- **Mínimos cambios** en ToolWheel y main.cpp
- **Máxima reutilización** de código existente

---

## Limitaciones Conocidas

1. **Solo fondo blanco**: La goma solo funciona sobre fondo blanco
2. **Sin capas**: No puede "revelar" contenido debajo
3. **Sin atajos de teclado**: Solo selección por click en UI
4. **Sin cursor visual**: El cursor no indica la herramienta actual

---

## Posibles Mejoras Futuras

### Corto Plazo
- [ ] Añadir atajo de teclado (`E` para Eraser, `B` para Brush)
- [ ] Cambiar cursor según herramienta seleccionada
- [ ] Añadir tooltip al pasar mouse sobre segmentos
- [ ] Animación de transición al cambiar de herramienta

### Mediano Plazo
- [ ] Modo de goma que elimina geometría real (no solo dibuja blanco)
- [ ] Goma inteligente que detecta y elimina trazos completos
- [ ] Soporte para fondos de diferentes colores
- [ ] Modo de goma con transparencia variable

### Largo Plazo
- [ ] Sistema de capas con borrado por capa
- [ ] Goma con blend modes (multiplicar, sobreponer, etc.)
- [ ] Historial separado para brush vs eraser
- [ ] Exportación que distingue trazos de borrado

---

## Compatibilidad

### Mantenida
- ✅ Todo el código existente funciona sin cambios
- ✅ Undo/Redo funciona automáticamente
- ✅ Sistema de zoom y pan no se ve afectado
- ✅ Archivos guardados (si se implementa persistencia) serán compatibles

### Versiones
- **OpenGL**: 3.3+ (sin cambios)
- **C++**: 17 (sin cambios)
- **Dependencias**: GLFW, GLEW, GLM (sin cambios)

---

## Conclusión

La herramienta de goma de borrar se integró exitosamente al proyecto con:
- ✅ **Implementación mínima** (< 100 líneas de código)
- ✅ **Máxima reutilización** de sistemas existentes
- ✅ **Cero regresiones** en funcionalidad existente
- ✅ **Interfaz intuitiva** en el tool wheel
- ✅ **Documentación completa** incluida

La solución es simple, eficiente y fácil de mantener. 🎉
