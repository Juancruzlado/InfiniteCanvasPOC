# Herramienta de Goma de Borrar

## Descripción General
Se agregó una herramienta de goma de borrar al tool wheel que permite borrar trazos dibujando con color blanco (el color del fondo).

## Ubicación en el Tool Wheel

```
       [BRUSH]
         🔴
    ┌─────────┐
    │         │
    │  WIDTH  │  ← Anillo medio (ancho)
    │         │
    └─────────┘
         🟦
      [ERASER]
```

- **Brush (Arriba)**: Segmento superior del anillo externo
- **Eraser (Abajo)**: Segmento inferior del anillo externo

## Cómo Usar

### Seleccionar la Goma
1. Haz clic en el segmento **inferior** (abajo) del anillo externo del tool wheel
2. El segmento se volverá de color **rojo claro** cuando esté seleccionado
3. El ícono es un **cuadrado blanco**

### Seleccionar el Pincel
1. Haz clic en el segmento **superior** (arriba) del anillo externo
2. El segmento se volverá de color **negro oscuro** cuando esté seleccionado
3. El ícono es un **círculo blanco**

### Ajustar el Grosor
- El grosor de la goma se controla con el **mismo slider** que el pincel
- Haz clic en el anillo medio para abrir el slider de tamaño
- Usa los mismos presets (0.05 - 200 pts)
- **La goma usa exactamente el mismo rango de grosores que el pincel**

## Funcionamiento Técnico

### Método de Borrado
La goma **no elimina geometría**, sino que dibuja con **color blanco** (RGB: 1.0, 1.0, 1.0), que coincide con el color de fondo del canvas.

### Ventajas de Este Método
1. **Simplicidad**: Usa el mismo sistema de trazos que el pincel
2. **Consistencia**: El grosor y la suavidad son idénticos al pincel
3. **Rendimiento**: No requiere cálculos de intersección o eliminación de geometría
4. **Undo/Redo**: Funciona automáticamente con el sistema de historial

### Implementación
```cpp
// En ToolWheel.h
glm::vec3 getEffectiveColor() const { 
    return (currentTool == ToolType::ERASER) ? 
           glm::vec3(1.0f, 1.0f, 1.0f) :  // Blanco para goma
           currentColor;                    // Color seleccionado para pincel
}
```

## Indicadores Visuales

### Brush Seleccionado
- Color del segmento: **Negro oscuro** (RGB: 40, 40, 40)
- Ícono: Círculo blanco pequeño

### Eraser Seleccionado
- Color del segmento: **Rojo claro** (RGB: 240, 100, 100)
- Ícono: Cuadrado blanco

### No Seleccionado
- Brush: Gris medio (RGB: 100, 100, 100)
- Eraser: Gris claro (RGB: 150, 150, 150)

## Casos de Uso

### Borrado Fino
- Grosor: 0.05 - 1 pts
- Ideal para: Correcciones precisas, eliminar detalles pequeños

### Borrado Normal
- Grosor: 5 - 40 pts
- Ideal para: Borrado general, correcciones medianas

### Borrado Grande
- Grosor: 60 - 200 pts
- Ideal para: Limpiar áreas grandes, borrado rápido

## Atajos de Teclado
Actualmente no hay atajos de teclado para cambiar entre herramientas. Para cambiar de pincel a goma o viceversa, haz clic en el segmento correspondiente del tool wheel.

## Notas Importantes

1. **El color del pincel se mantiene**: Cuando cambias de goma a pincel, el color seleccionado previamente se mantiene
2. **El grosor es compartido**: Ambas herramientas usan el mismo valor de grosor
3. **Undo/Redo funciona**: Los trazos de borrado se pueden deshacer igual que los trazos de dibujo
4. **Zoom automático**: El grosor de la goma escala correctamente con el zoom, igual que el pincel

## Limitaciones

- La goma solo funciona sobre fondo blanco
- No es posible "revelar" capas inferiores (solo dibuja blanco encima)
- Si el fondo cambia de color, será necesario ajustar el color de la goma
