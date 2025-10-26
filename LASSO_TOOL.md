# Herramienta Lasso - Selección y Transformación

## ✅ Implementación Completada

La **herramienta Lasso** permite seleccionar partes del dibujo y moverlas libremente, similar a la función "cortar y mover" en aplicaciones de dibujo tradicionales.

---

## 🎯 ¿Qué es el Lasso Tool?

El Lasso es una herramienta de **selección libre** que te permite:
1. **Dibujar un contorno** alrededor de los trazos que quieres seleccionar
2. **Seleccionar automáticamente** todos los trazos que estén dentro del contorno
3. **Mover los trazos seleccionados** como un grupo
4. **No elimina ni corta** - solo separa y permite mover

### Concepto

```
  ANTES                LASSO ACTIVO           DESPUÉS DEL MOVIMIENTO
    🎨                    🎨                         🎨
   /|\                  /|\                         
  / | \                / | \                      /|\  
    |      →   [Lasso] |     →    Mover →        / | \ ← Movido!
   / \                / \                           |
  /   \              /   \                         / \
```

**Es como** usar la herramienta de lazo en Photoshop, Paint, o cualquier editor gráfico.

---

## 🎨 Ubicación en el Tool Wheel

```
       [BRUSH] ← Arriba
         🔴
    ┌─────────┐
 [LASSO]      │  WIDTH  │   
    🔵        │         │  
    ↖️        │  COLOR  │
    └─────────┘
         🟦
      [ERASER] ← Abajo
```

- **Brush**: Segmento superior (arriba)
- **Eraser**: Segmento inferior (abajo)
- **Lasso**: Segmento izquierdo (lado izquierdo) ← **NUEVO!**

---

## 🚀 Cómo Usar el Lasso

### Paso 1: Seleccionar la Herramienta

**Opción A:** Clic en el Tool Wheel
```
1. Haz clic en el segmento IZQUIERDO del anillo externo
2. El segmento se vuelve AZUL CLARO cuando está activo
3. El ícono es un círculo punteado
```

### Paso 2: Dibujar el Lasso

```
1. Con la herramienta Lasso activa
2. Haz clic y MANTÉN presionado el botón izquierdo del mouse
3. Dibuja un contorno alrededor de los trazos que quieres seleccionar
4. Suelta el mouse para completar la selección
```

**Mientras dibujas:**
- Verás una **línea azul** que sigue tu cursor
- La línea se cierra automáticamente al soltar el mouse
- Puedes hacer contornos irregulares, no tiene que ser circular

### Paso 3: Ver la Selección

Después de soltar:
```
✓ Los trazos dentro del lasso quedan SELECCIONADOS
✓ Aparece un texto: "Selection active - Click and drag to move"
✓ La consola muestra: "Selected X stroke(s)"
```

### Paso 4: Mover la Selección

```
1. Con trazos seleccionados
2. Haz clic y ARRASTRA en cualquier parte del canvas
3. Los trazos seleccionados se mueven juntos
4. Suelta para finalizar el movimiento
```

### Paso 5: Deseleccionar

**Opción A:** Presiona `ESC`
**Opción B:** Haz clic derecho
**Opción C:** Cambia a otra herramienta (Brush/Eraser)

---

## 🎮 Flujo de Trabajo Completo

### Ejemplo: Mover una Firma

```
Escenario: Dibujaste tu firma, pero está en el lugar incorrecto

1. Haz clic en el segmento LASSO del tool wheel
2. Dibuja un círculo alrededor de tu firma
3. Suelta el mouse
   → Console: "Selected 15 stroke(s)"
   → Pantalla: "Selection active - Click and drag to move"
4. Haz clic y arrastra la firma al nuevo lugar
5. Suelta el mouse
6. Presiona ESC para deseleccionar
7. Cambia de vuelta a BRUSH para seguir dibujando
```

### Ejemplo: Reorganizar un Diagrama

```
Escenario: Hiciste un diagrama pero necesitas reorganizar partes

1. Selecciona LASSO
2. Rodea el primer elemento con el lasso
3. Muévelo a su nueva posición
4. ESC para deseleccionar
5. Rodea el segundo elemento
6. Muévelo
7. Repite según necesites
8. Vuelve a BRUSH para agregar conexiones
```

---

## 🎯 Algoritmo de Selección

### Point-in-Polygon (Ray Casting)

El lasso usa el **algoritmo de ray casting** para determinar qué está dentro:

```cpp
bool pointInPolygon(point, polygon) {
    // Traza un rayo desde el punto hacia el infinito
    // Cuenta cuántas veces cruza los bordes del polígono
    // Si cruza un número IMPAR de veces → DENTRO
    // Si cruza un número PAR de veces → FUERA
    return inside;
}
```

**Cómo funciona:**
1. Para cada trazo en el canvas
2. Revisa cada punto del trazo
3. Si **AL MENOS UN PUNTO** está dentro del lasso
4. **TODO EL TRAZO** queda seleccionado

**Ventajas:**
- ✅ Funciona con polígonos irregulares
- ✅ Rápido (O(n) por punto)
- ✅ Preciso matemáticamente
- ✅ No requiere formas especiales

---

## 🔧 Detalles Técnicos

### Arquitectura

**Archivos Modificados:**
1. `include/ToolWheel.h` - Agregado `ToolType::LASSO`
2. `src/ToolWheel.cpp` - Segmento visual del lasso
3. `include/Canvas.h` - Sistema de selección
4. `src/Canvas.cpp` - Lógica de selección y movimiento
5. `src/main.cpp` - Manejo de eventos y renderizado

### Canvas - Sistema de Selección

```cpp
class Canvas {
public:
    // Seleccionar trazos dentro de un polígono (lasso)
    void selectStrokesInPolygon(const std::vector<glm::vec2>& lassoPoints);
    
    // Limpiar selección
    void clearSelection();
    
    // Mover trazos seleccionados
    void moveSelectedStrokes(const glm::vec2& delta);
    
    // Verificar si hay selección
    bool hasSelection() const;
    
private:
    std::set<size_t> selectedStrokes; // Índices de trazos seleccionados
    
    // Algoritmo point-in-polygon
    bool pointInPolygon(const glm::vec2& point, 
                        const std::vector<glm::vec2>& polygon) const;
};
```

### Estados del Sistema

```cpp
// En main.cpp
std::vector<glm::vec2> lassoPoints;  // Puntos del contorno del lasso
bool isDrawingLasso = false;          // Dibujando el lasso
bool isMovingSelection = false;       // Moviendo selección
glm::vec2 moveStartPos;                // Posición inicial del movimiento
```

### Flujo de Eventos

**1. Click con Lasso (Sin selección):**
```
Mouse Down → isDrawingLasso = true
           → lassoPoints.clear()
           → Agregar primer punto
```

**2. Movimiento del Mouse (Dibujando Lasso):**
```
Mouse Move → if (isDrawingLasso)
           → Agregar punto cada 3 píxeles
           → Renderizar línea azul en tiempo real
```

**3. Soltar Mouse (Completar Lasso):**
```
Mouse Up → Convertir lassoPoints a coordenadas del mundo
         → canvas.selectStrokesInPolygon(worldPoints)
         → isDrawingLasso = false
         → lassoPoints.clear()
```

**4. Click con Lasso (Con selección):**
```
Mouse Down → if (canvas.hasSelection())
           → isMovingSelection = true
           → moveStartPos = mouse position
```

**5. Movimiento con Selección:**
```
Mouse Move → if (isMovingSelection)
           → calcular delta = newPos - moveStartPos
           → canvas.moveSelectedStrokes(delta)
           → moveStartPos = newPos
```

**6. Soltar después de Mover:**
```
Mouse Up → isMovingSelection = false
         → Selección permanece activa
```

---

## 🎨 Feedback Visual

### Lasso Dibujándose
- **Color**: Azul claro (RGB: 100, 200, 255)
- **Grosor**: 2 píxeles
- **Estilo**: Línea continua
- **Cierre**: Línea semi-transparente entre último y primer punto

### Selección Activa
- **Texto en pantalla**: "Selection active - Click and drag to move"
- **Color del texto**: Azul claro
- **Posición**: Esquina inferior izquierda
- **Console**: "Selected X stroke(s)"

### Tool Wheel - Segmento Lasso
- **Activo**: Azul claro brillante (RGB: 100, 200, 255)
- **Inactivo**: Gris (RGB: 130, 130, 130)
- **Ícono**: Círculo punteado (8 segmentos)
- **Ubicación**: Lado izquierdo del wheel (ángulo π)

---

## ⌨️ Controles y Atajos

| Acción | Control |
|--------|---------|
| **Seleccionar Lasso** | Click en segmento izquierdo del tool wheel |
| **Dibujar Lasso** | Click y arrastra (con Lasso activo, sin selección) |
| **Mover Selección** | Click y arrastra (con Lasso activo, con selección) |
| **Deseleccionar** | `ESC` o click derecho |
| **Deseleccionar automático** | Cambiar a Brush/Eraser |

---

## 💡 Casos de Uso

### 1. Reorganizar Composición
```
Problema: El dibujo está bien pero mal ubicado
Solución: 
- Usa Lasso para seleccionar partes
- Muévelas a mejor posición
- Mantén proporciones y relaciones
```

### 2. Ajustar Diagrama
```
Problema: Diagrama de flujo con elementos desalineados
Solución:
- Selecciona cada caja individualmente
- Muévela a la posición correcta
- Repite para todas las cajas
```

### 3. Duplicar Elementos (Futuro)
```
Idea: Seleccionar + Copiar + Pegar
Estado: No implementado aún
Workaround: Redibujar manualmente
```

### 4. Corrección de Errores
```
Problema: Dibujaste algo en el lugar equivocado
Solución:
- Selecciona con Lasso
- Mueve al lugar correcto
- Más rápido que Undo + Redibujar
```

---

## 🔍 Limitaciones Actuales

### ❌ No Implementado

1. **Copiar/Pegar**: No puedes duplicar la selección
2. **Rotar**: No hay rotación de selección
3. **Escalar**: No hay redimensionamiento
4. **Highlight visual**: No se dibuja borde alrededor de seleccionados
5. **Selección múltiple**: No puedes agregar a la selección
6. **Deselección parcial**: No puedes quitar individual del grupo

### ⚠️ Comportamiento Actual

1. **Selección completa del trazo**: Si un punto está dentro, todo el trazo se selecciona
2. **Movimiento permanente**: No hay preview antes de soltar
3. **Sin Undo específico**: Undo/Redo general, no de selección
4. **Coords del mundo**: La selección es en coordenadas de mundo (escala con zoom)

---

## 🚀 Futuras Mejoras

### Corto Plazo
- [ ] Highlight visual de trazos seleccionados (borde azul)
- [ ] Copiar/Pegar selección (`Ctrl+C` / `Ctrl+V`)
- [ ] Selección por rectángulo (alternativa al lasso)
- [ ] Feedback visual mejorado (bounding box)

### Mediano Plazo
- [ ] Transformaciones: Rotar, Escalar
- [ ] Selección múltiple aditiva (`Shift+Click`)
- [ ] Deselección parcial (`Alt+Click`)
- [ ] Preview de transformación (ghost/outline)

### Largo Plazo
- [ ] Agrupación de selecciones
- [ ] Layers con selección por capa
- [ ] Transformación con handles visuales
- [ ] Snap to grid/guides durante movimiento

---

## 🐛 Troubleshooting

### Problema: No puedo seleccionar nada
**Causa**: El lasso tool no está activo  
**Solución**: Click en el segmento IZQUIERDO del tool wheel (debe volverse azul)

### Problema: El lasso no aparece mientras dibujo
**Causa**: Posible problema de renderizado  
**Solución**: Verifica que ImGui esté renderizando correctamente

### Problema: Los trazos no se mueven
**Causa**: No hay selección activa  
**Solución**: 
1. Dibuja el lasso completamente
2. Suelta el mouse
3. Verifica el mensaje "Selected X stroke(s)" en console
4. Luego intenta mover

### Problema: Selección desaparece inmediatamente
**Causa**: Cambiaste de herramienta o presionaste ESC  
**Solución**: La selección se limpia automáticamente al cambiar de tool o presionar ESC

### Problema: Solo se seleccionan algunos trazos
**Causa**: El algoritmo selecciona basado en puntos individuales  
**Solución**: Asegúrate de que el lasso cubra una porción significativa del trazo

---

## 📊 Rendimiento

### Complejidad Algorítmica

**Selección:**
```
O(N × M × P)
donde:
  N = número de trazos en el canvas
  M = número promedio de puntos por trazo
  P = número de puntos en el lasso
```

**Movimiento:**
```
O(S × M)
donde:
  S = número de trazos seleccionados
  M = número promedio de puntos por trazo
```

### Optimización

El sistema está optimizado para:
- ✅ Selecciones pequeñas (< 100 trazos)
- ✅ Canvas medianos (< 1000 trazos totales)
- ✅ Lazos simples (< 100 puntos de contorno)

Para canvas muy grandes:
- La selección puede tomar ~100-200ms
- El movimiento es en tiempo real (< 16ms)
- Considera agregar spatial indexing (quadtree) en el futuro

---

## 🎓 Ejemplo de Código

### Usar la API desde Código

```cpp
// En tu aplicación
Canvas canvas;

// 1. Crear contorno de lasso (coordenadas del mundo)
std::vector<glm::vec2> lassoContour = {
    glm::vec2(100, 100),
    glm::vec2(200, 100),
    glm::vec2(200, 200),
    glm::vec2(100, 200)
};

// 2. Seleccionar trazos dentro del contorno
canvas.selectStrokesInPolygon(lassoContour);

// 3. Verificar cuántos se seleccionaron
if (canvas.hasSelection()) {
    const auto& selected = canvas.getSelectedStrokes();
    std::cout << "Seleccionados: " << selected.size() << std::endl;
    
    // 4. Mover la selección
    glm::vec2 delta(50, -30); // 50 px derecha, 30 px arriba
    canvas.moveSelectedStrokes(delta);
    
    // 5. Limpiar selección
    canvas.clearSelection();
}
```

---

## ✅ Resumen

### Lo Implementado
- ✅ Tool type LASSO en el ToolWheel
- ✅ Segmento visual en el tool wheel (izquierdo)
- ✅ Sistema de selección con point-in-polygon
- ✅ Dibujo del lasso en tiempo real
- ✅ Movimiento de trazos seleccionados
- ✅ Feedback visual (texto en pantalla)
- ✅ Deselección con ESC y click derecho
- ✅ Limpieza automática al cambiar de herramienta

### Funcionalidad Completa
1. **Seleccionar**: Dibuja contorno → auto-selecciona
2. **Mover**: Click y arrastra → mueve grupo
3. **Deseleccionar**: ESC / click derecho / cambiar tool
4. **Visual**: Lasso azul + texto de instrucciones

---

## 🎉 ¡Listo para Usar!

```bash
./build/VectorSketch

# 1. Dibuja algo con Brush
# 2. Click en segmento IZQUIERDO del tool wheel
# 3. Dibuja un lasso alrededor de parte del dibujo
# 4. Suelta el mouse → Selección hecha!
# 5. Click y arrastra para mover
# 6. ESC para deseleccionar
```

**¡Exactamente como en Paint, Photoshop, y otros editores!** 🎨✨
