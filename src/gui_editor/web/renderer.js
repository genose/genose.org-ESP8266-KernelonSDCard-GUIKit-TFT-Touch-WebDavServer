/**
 * GUIKit Web Editor - Canvas Renderer
 * 
 * Renders GUI widgets on the HTML5 canvas based on JSON definitions
 * This is the visual representation layer for the web editor
 */

class GUIRenderer {
    constructor(canvas, guiDefinition) {
        this.canvas = canvas;
        this.ctx = canvas.getContext('2d');
        this.gui = guiDefinition || this.getDefaultGUI();
        this.widgetElements = {};  // Maps widget IDs to DOM elements
        this.selectedWidget = null;
        this.hoveredWidget = null;
        this.zoom = 1.0;
        this.offset = { x: 0, y: 0 };
        this.gridSize = 20;
        this.showGrid = true;
        this.snapToGrid = true;
        
        this.init();
    }
    
    init() {
        // Set canvas dimensions
        this.canvas.width = this.gui.size.width;
        this.canvas.height = this.gui.size.height;
        
        // Set up event listeners
        this.setupEvents();
        
        // Initial render
        this.render();
    }
    
    getDefaultGUI() {
        return {
            version: "1.0",
            name: "Untitled GUI",
            size: { width: 320, height: 240 },
            background: "#000000",
            widgets: []
        };
    }
    
    setupEvents() {
        // Mouse events for canvas interaction
        this.canvas.addEventListener('mousedown', this.handleMouseDown.bind(this));
        this.canvas.addEventListener('mousemove', this.handleMouseMove.bind(this));
        this.canvas.addEventListener('mouseup', this.handleMouseUp.bind(this));
        this.canvas.addEventListener('mouseleave', this.handleMouseLeave.bind(this));
        this.canvas.addEventListener('wheel', this.handleWheel.bind(this));
        this.canvas.addEventListener('dblclick', this.handleDoubleClick.bind(this));
        
        // Touch events for mobile
        this.canvas.addEventListener('touchstart', this.handleTouchStart.bind(this));
        this.canvas.addEventListener('touchmove', this.handleTouchMove.bind(this));
        this.canvas.addEventListener('touchend', this.handleTouchEnd.bind(this));
    }
    
    /**
     * Main render function - draws the entire GUI
     */
    render() {
        // Clear canvas
        this.clear();
        
        // Draw background
        this.drawBackground();
        
        // Draw grid
        if (this.showGrid) {
            this.drawGrid();
        }
        
        // Draw all widgets
        this.gui.widgets.forEach(widget => {
            this.drawWidget(widget);
        });
        
        // Draw selection highlight if any
        if (this.selectedWidget) {
            this.drawSelection(this.selectedWidget);
        }
    }
    
    clear() {
        this.ctx.fillStyle = this.hexToRgba(this.gui.background || '#000000');
        this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
    }
    
    drawBackground() {
        const bg = this.gui.background;
        if (bg && bg !== 'transparent') {
            this.ctx.fillStyle = this.hexToRgba(bg);
            this.ctx.fillRect(0, 0, this.canvas.width, this.canvas.height);
        }
    }
    
    drawGrid() {
        const ctx = this.ctx;
        const width = this.canvas.width;
        const height = this.canvas.height;
        
        // Fine grid
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.1)';
        ctx.lineWidth = 1;
        
        for (let x = 0; x <= width; x += this.gridSize / 2) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
            ctx.stroke();
        }
        
        for (let y = 0; y <= height; y += this.gridSize / 2) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
            ctx.stroke();
        }
        
        // Strong grid lines
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.2)';
        ctx.lineWidth = 1;
        
        for (let x = 0; x <= width; x += this.gridSize) {
            ctx.beginPath();
            ctx.moveTo(x, 0);
            ctx.lineTo(x, height);
            ctx.stroke();
        }
        
        for (let y = 0; y <= height; y += this.gridSize) {
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(width, y);
            ctx.stroke();
        }
    }
    
    /**
     * Draw a widget on the canvas
     */
    drawWidget(widget) {
        if (!widget || !widget.visible) return;
        
        const type = widget.type;
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 50;
        
        // Save context state
        this.ctx.save();
        
        // Apply clipping for rounded corners if needed
        if (style.border_radius && style.border_radius > 0) {
            this.ctx.beginPath();
            this.roundedRect(x, y, width, height, style.border_radius);
            this.ctx.clip();
        }
        
        // Draw based on widget type
        switch (type) {
            case 'view':
                this.drawView(widget);
                break;
            case 'button':
                this.drawButton(widget);
                break;
            case 'label':
                this.drawLabel(widget);
                break;
            case 'text_input':
                this.drawTextInput(widget);
                break;
            case 'text_editor':
                this.drawTextEditor(widget);
                break;
            case 'slider':
                this.drawSlider(widget);
                break;
            case 'progress_bar':
                this.drawProgressBar(widget);
                break;
            case 'image':
                this.drawImage(widget);
                break;
            case 'canvas':
                this.drawCanvasWidget(widget);
                break;
            case 'checkbox':
                this.drawCheckbox(widget);
                break;
            case 'radio_button':
                this.drawRadioButton(widget);
                break;
            case 'dropdown':
                this.drawDropdown(widget);
                break;
            default:
                this.drawUnknownWidget(widget);
        }
        
        // Draw border
        if (style.border_width && style.border_width > 0) {
            this.ctx.strokeStyle = this.hexToRgba(style.border_color || '#808080');
            this.ctx.lineWidth = style.border_width;
            if (style.border_radius && style.border_radius > 0) {
                this.ctx.beginPath();
                this.roundedRect(x, y, width, height, style.border_radius);
                this.ctx.stroke();
            } else {
                this.ctx.strokeRect(x, y, width, height);
            }
        }
        
        // Restore context
        this.ctx.restore();
    }
    
    /**
     * Draw View (container) widget
     */
    drawView(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 50;
        
        // Draw background
        if (style.bg_color) {
            this.ctx.fillStyle = this.hexToRgba(style.bg_color);
            if (style.border_radius && style.border_radius > 0) {
                this.ctx.beginPath();
                this.roundedRect(x, y, width, height, style.border_radius);
                this.ctx.fill();
            } else {
                this.ctx.fillRect(x, y, width, height);
            }
        }
        
        // Draw children
        if (widget.children) {
            widget.children.forEach(child => {
                this.drawWidget(child);
            });
        }
    }
    
    /**
     * Draw Button widget
     */
    drawButton(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 40;
        const text = widget.text || 'Button';
        const pressed = widget.pressed || false;
        
        // Draw background (slightly darker when pressed)
        let bgColor = style.bg_color || '#3E3E3E';
        if (pressed) {
            bgColor = this.adjustColor(bgColor, -20);
        }
        
        this.ctx.fillStyle = this.hexToRgba(bgColor);
        if (style.border_radius && style.border_radius > 0) {
            this.ctx.beginPath();
            this.roundedRect(x, y, width, height, style.border_radius);
            this.ctx.fill();
        } else {
            this.ctx.fillRect(x, y, width, height);
        }
        
        // Draw text
        this.drawText(text, x + width / 2, y + height / 2, {
            align: 'center',
            baseline: 'middle',
            color: style.fg_color || '#FFFFFF',
            fontSize: style.font_size || 14,
            font: style.font || 'default'
        });
    }
    
    /**
     * Draw Label widget
     */
    drawLabel(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 20;
        const text = widget.text || 'Label';
        const textAlign = style.text_align || 'left';
        
        // Draw background if specified
        if (style.bg_color) {
            this.ctx.fillStyle = this.hexToRgba(style.bg_color);
            this.ctx.fillRect(x, y, width, height);
        }
        
        // Draw text
        const padding = style.padding || 4;
        const textX = textAlign === 'center' ? x + width / 2 : 
                      textAlign === 'right' ? x + width - padding : x + padding;
        const textY = y + height / 2;
        
        this.drawText(text, textX, textY, {
            align: textAlign,
            baseline: 'middle',
            color: style.fg_color || '#FFFFFF',
            fontSize: style.font_size || 14,
            font: style.font || 'default'
        });
    }
    
    /**
     * Draw Text Input widget
     */
    drawTextInput(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 200;
        const height = widget.height || 30;
        const text = widget.text || widget.placeholder || '';
        const placeholder = widget.placeholder || '';
        const isPlaceholder = !widget.text && placeholder;
        
        // Draw background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.fillRect(x, y, width, height);
        
        // Draw text (dimmed for placeholder)
        this.drawText(text, x + 8, y + height / 2, {
            align: 'left',
            baseline: 'middle',
            color: isPlaceholder ? '#666666' : (style.fg_color || '#FFFFFF'),
            fontSize: style.font_size || 14,
            font: style.font || 'default'
        });
        
        // Draw cursor if focused (simplified - in real editor this would blink)
        if (widget.focused) {
            this.ctx.fillStyle = '#FFFFFF';
            this.ctx.fillRect(x + 8 + this.ctx.measureText(text).width, y + 4, 2, height - 8);
        }
    }
    
    /**
     * Draw Text Editor widget
     */
    drawTextEditor(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 200;
        const height = widget.height || 100;
        const text = widget.text || '';
        const lineNumbers = widget.show_line_numbers || false;
        
        // Draw background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#1E1E1E');
        this.ctx.fillRect(x, y, width, height);
        
        // Draw line numbers if enabled
        if (lineNumbers) {
            const gutterWidth = 40;
            this.ctx.fillStyle = this.hexToRgba('#2D2D2D');
            this.ctx.fillRect(x, y, gutterWidth, height);
            
            const lines = text.split('\n');
            this.ctx.fillStyle = this.hexToRgba('#808080');
            this.ctx.font = `${style.font_size || 12}px ${style.font || 'monospace'}`;
            this.ctx.textAlign = 'right';
            
            lines.forEach((line, i) => {
                const lineY = y + (i + 1) * (this.ctx.measureText('M').width * 1.5);
                this.ctx.fillText(i + 1, x + gutterWidth - 4, lineY);
            });
            
            this.ctx.textAlign = 'left';
        }
        
        // Draw text lines
        const lines = text.split('\n');
        this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#D4D4D4');
        this.ctx.font = `${style.font_size || 12}px ${style.font || 'monospace'}`;
        
        const lineHeight = this.ctx.measureText('M').width * 1.5;
        const textX = lineNumbers ? x + 44 : x + 4;
        
        lines.forEach((line, i) => {
            const lineY = y + (i + 1) * lineHeight - this.ctx.measureText('M').width * 0.25;
            this.ctx.fillText(line, textX, lineY);
        });
    }
    
    /**
     * Draw Slider widget
     */
    drawSlider(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 200;
        const height = widget.height || 20;
        const vertical = widget.vertical || false;
        const min = widget.min_value || 0;
        const max = widget.max_value || 100;
        const value = widget.value || 50;
        
        const trackColor = style.bg_color || '#3E3E3E';
        const thumbColor = style.fg_color || '#0078D4';
        const trackHeight = vertical ? height : 4;
        const trackWidth = vertical ? 4 : width;
        
        if (vertical) {
            // Vertical slider
            const trackX = x + width / 2 - trackWidth / 2;
            this.ctx.fillStyle = this.hexToRgba(trackColor);
            this.ctx.fillRect(trackX, y, trackWidth, height);
            
            // Draw fill
            const fillHeight = (value - min) / (max - min) * height;
            this.ctx.fillStyle = this.hexToRgba(thumbColor);
            this.ctx.fillRect(trackX, y + height - fillHeight, trackWidth, fillHeight);
            
            // Draw thumb
            const thumbY = y + height - fillHeight - 8;
            this.ctx.beginPath();
            this.ctx.arc(trackX + trackWidth / 2, thumbY, 8, 0, Math.PI * 2);
            this.ctx.fillStyle = thumbColor;
            this.ctx.fill();
        } else {
            // Horizontal slider
            const trackY = y + height / 2 - trackHeight / 2;
            this.ctx.fillStyle = this.hexToRgba(trackColor);
            this.ctx.fillRect(x, trackY, trackWidth, trackHeight);
            
            // Draw fill
            const fillWidth = (value - min) / (max - min) * width;
            this.ctx.fillStyle = this.hexToRgba(thumbColor);
            this.ctx.fillRect(x, trackY, fillWidth, trackHeight);
            
            // Draw thumb
            const thumbX = x + fillWidth - 8;
            this.ctx.beginPath();
            this.ctx.arc(thumbX, trackY + trackHeight / 2, 8, 0, Math.PI * 2);
            this.ctx.fillStyle = thumbColor;
            this.ctx.fill();
        }
    }
    
    /**
     * Draw Progress Bar widget
     */
    drawProgressBar(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 200;
        const height = widget.height || 20;
        const value = widget.value || 0;
        const max = widget.max || 100;
        const showText = widget.show_text || false;
        
        const progress = Math.min(1, Math.max(0, value / max));
        const fillWidth = progress * width;
        
        // Draw background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.fillRect(x, y, width, height);
        
        // Draw progress fill
        this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#0078D4');
        this.ctx.fillRect(x, y, fillWidth, height);
        
        // Draw text if enabled
        if (showText) {
            const text = `${Math.round(progress * 100)}%`;
            this.drawText(text, x + width / 2, y + height / 2, {
                align: 'center',
                baseline: 'middle',
                color: '#FFFFFF',
                fontSize: 12
            });
        }
    }
    
    /**
     * Draw Image widget (placeholder - actual image loading would be async)
     */
    drawImage(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 100;
        const src = widget.src || '';
        
        // Draw placeholder
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.fillRect(x, y, width, height);
        
        // Draw image icon or name
        this.ctx.fillStyle = this.hexToRgba('#808080');
        this.ctx.font = '12px sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText(src ? src.split('/').pop() : 'Image', x + width / 2, y + height / 2);
        this.ctx.textAlign = 'left';
        this.ctx.textBaseline = 'alphabetic';
    }
    
    /**
     * Draw Canvas widget (placeholder for custom drawing)
     */
    drawCanvasWidget(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 100;
        
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#1E1E1E');
        this.ctx.fillRect(x, y, width, height);
        
        this.ctx.fillStyle = this.hexToRgba('#808080');
        this.ctx.font = '12px sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText('Canvas', x + width / 2, y + height / 2);
        this.ctx.textAlign = 'left';
        this.ctx.textBaseline = 'alphabetic';
    }
    
    /**
     * Draw Checkbox widget
     */
    drawCheckbox(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 20;
        const text = widget.text || '';
        const checked = widget.checked || false;
        
        const boxSize = height * 0.8;
        const boxX = x;
        const boxY = y + height / 2 - boxSize / 2;
        
        // Draw checkbox background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.fillRect(boxX, boxY, boxSize, boxSize);
        
        // Draw checkbox border
        this.ctx.strokeStyle = this.hexToRgba(checked ? style.fg_color || '#0078D4' : '#808080');
        this.ctx.lineWidth = 2;
        this.ctx.strokeRect(boxX, boxY, boxSize, boxSize);
        
        // Draw checkmark if checked
        if (checked) {
            this.ctx.strokeStyle = this.hexToRgba(style.fg_color || '#0078D4');
            this.ctx.lineWidth = 2;
            this.ctx.beginPath();
            this.ctx.moveTo(boxX + boxSize * 0.2, boxY + boxSize / 2);
            this.ctx.lineTo(boxX + boxSize * 0.4, boxY + boxSize * 0.7);
            this.ctx.lineTo(boxX + boxSize * 0.8, boxY + boxSize * 0.3);
            this.ctx.stroke();
        }
        
        // Draw text if any
        if (text) {
            this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#D4D4D4');
            this.ctx.font = `${style.font_size || 12}px ${style.font || 'sans-serif'}`;
            this.ctx.textAlign = 'left';
            this.ctx.textBaseline = 'middle';
            this.ctx.fillText(text, x + boxSize + 8, y + height / 2);
        }
    }
    
    /**
     * Draw Radio Button widget
     */
    drawRadioButton(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 20;
        const text = widget.text || '';
        const checked = widget.checked || false;
        
        const dotSize = height * 0.6;
        const dotX = x + dotSize * 0.5;
        const dotY = y + height / 2;
        
        // Draw radio circle background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.beginPath();
        this.ctx.arc(dotX, dotY, dotSize / 2, 0, Math.PI * 2);
        this.ctx.fill();
        
        // Draw radio circle border
        this.ctx.strokeStyle = this.hexToRgba(checked ? style.fg_color || '#0078D4' : '#808080');
        this.ctx.lineWidth = 2;
        this.ctx.beginPath();
        this.ctx.arc(dotX, dotY, dotSize / 2, 0, Math.PI * 2);
        this.ctx.stroke();
        
        // Draw dot if checked
        if (checked) {
            this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#0078D4');
            this.ctx.beginPath();
            this.ctx.arc(dotX, dotY, dotSize / 6, 0, Math.PI * 2);
            this.ctx.fill();
        }
        
        // Draw text if any
        if (text) {
            this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#D4D4D4');
            this.ctx.font = `${style.font_size || 12}px ${style.font || 'sans-serif'}`;
            this.ctx.textAlign = 'left';
            this.ctx.textBaseline = 'middle';
            this.ctx.fillText(text, x + dotSize + 8, dotY);
        }
    }
    
    /**
     * Draw Dropdown widget
     */
    drawDropdown(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 150;
        const height = widget.height || 30;
        const items = widget.items || [];
        const selectedIndex = widget.selected_index || 0;
        
        // Draw background
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#252526');
        this.ctx.fillRect(x, y, width, height);
        
        // Draw border
        this.ctx.strokeStyle = this.hexToRgba(style.border_color || '#808080');
        this.ctx.lineWidth = 1;
        this.ctx.strokeRect(x, y, width, height);
        
        // Draw selected text
        const selectedText = items[selectedIndex] || (items.length > 0 ? items[0] : 'Select...');
        this.ctx.fillStyle = this.hexToRgba(style.fg_color || '#D4D4D4');
        this.ctx.font = `${style.font_size || 12}px ${style.font || 'sans-serif'}`;
        this.ctx.textAlign = 'left';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText(selectedText, x + 8, y + height / 2);
        
        // Draw dropdown arrow
        const arrowX = x + width - 20;
        const arrowY = y + height / 2;
        this.ctx.fillStyle = this.hexToRgba('#808080');
        this.ctx.beginPath();
        this.ctx.moveTo(arrowX, arrowY - 4);
        this.ctx.lineTo(arrowX + 8, arrowY);
        this.ctx.lineTo(arrowX, arrowY + 4);
        this.ctx.closePath();
        this.ctx.fill();
    }
    
    /**
     * Draw unknown widget type
     */
    drawUnknownWidget(widget) {
        const style = widget.style || {};
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 50;
        
        this.ctx.fillStyle = this.hexToRgba(style.bg_color || '#800080');
        this.ctx.fillRect(x, y, width, height);
        
        this.ctx.fillStyle = this.hexToRgba('#FFFFFF');
        this.ctx.font = '12px sans-serif';
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        this.ctx.fillText(`Unknown: ${widget.type}`, x + width / 2, y + height / 2);
        this.ctx.textAlign = 'left';
        this.ctx.textBaseline = 'alphabetic';
    }
    
    /**
     * Draw selection highlight around a widget
     */
    drawSelection(widget) {
        const x = widget.x || 0;
        const y = widget.y || 0;
        const width = widget.width || 100;
        const height = widget.height || 50;
        
        this.ctx.save();
        this.ctx.strokeStyle = '#0078D4';
        this.ctx.lineWidth = 2;
        this.ctx.setLineDash([5, 5]);
        this.ctx.strokeRect(x - 1, y - 1, width + 2, height + 2);
        this.ctx.restore();
        
        // Draw resize handles
        this.drawResizeHandles(x, y, width, height);
    }
    
    /**
     * Draw resize handles for selected widget
     */
    drawResizeHandles(x, y, width, height) {
        const handleSize = 8;
        const handles = [
            { nx: 0, ny: 0, cx: 'nw' },    // Top-left
            { nx: 0.5, ny: 0, cx: 'n' },   // Top
            { nx: 1, ny: 0, cx: 'ne' },    // Top-right
            { nx: 1, ny: 0.5, cx: 'e' },   // Right
            { nx: 1, ny: 1, cx: 'se' },     // Bottom-right
            { nx: 0.5, ny: 1, cx: 's' },   // Bottom
            { nx: 0, ny: 1, cx: 'sw' },     // Bottom-left
            { nx: 0, ny: 0.5, cx: 'w' }    // Left
        ];
        
        this.ctx.fillStyle = '#0078D4';
        handles.forEach(handle => {
            const hx = x + handle.nx * width - handleSize / 2;
            const hy = y + handle.ny * height - handleSize / 2;
            this.ctx.fillRect(hx, hy, handleSize, handleSize);
        });
    }
    
    /**
     * Draw text with formatting options
     */
    drawText(text, x, y, options = {}) {
        const ctx = this.ctx;
        const align = options.align || 'left';
        const baseline = options.baseline || 'alphabetic';
        const color = options.color || '#FFFFFF';
        const fontSize = options.fontSize || 14;
        const fontFamily = this.getFontFamily(options.font || 'default');
        
        ctx.save();
        ctx.fillStyle = this.hexToRgba(color);
        ctx.font = `${fontSize}px ${fontFamily}`;
        ctx.textAlign = align;
        ctx.textBaseline = baseline;
        ctx.fillText(text, x, y);
        ctx.restore();
    }
    
    /**
     * Get font family based on font name
     */
    getFontFamily(font) {
        const fonts = {
            'default': 'sans-serif',
            'small': 'sans-serif',
            'medium': 'sans-serif',
            'large': 'sans-serif',
            'monospace': 'monospace'
        };
        return fonts[font] || fonts.default;
    }
    
    /**
     * Draw rounded rectangle path
     */
    roundedRect(x, y, width, height, radius) {
        const ctx = this.ctx;
        const r = Math.min(radius, width / 2, height / 2);
        
        ctx.beginPath();
        ctx.moveTo(x + r, y);
        ctx.arcTo(x + width, y, x + width, y + r, r);
        ctx.arcTo(x + width, y + height, x + width - r, y + height, r);
        ctx.arcTo(x, y + height, x, y + height - r, r);
        ctx.arcTo(x, y, x + r, y, r);
        ctx.closePath();
    }
    
    /**
     * Convert hex color to rgba
     */
    hexToRgba(hex, alpha = 1) {
        if (!hex || hex === 'transparent') {
            return 'rgba(0, 0, 0, 0)';
        }
        
        // Remove # if present
        hex = hex.replace('#', '');
        
        // Parse hex
        let r, g, b;
        if (hex.length === 3) {
            r = parseInt(hex[0] + hex[0], 16);
            g = parseInt(hex[1] + hex[1], 16);
            b = parseInt(hex[2] + hex[2], 16);
        } else if (hex.length === 6) {
            r = parseInt(hex.substring(0, 2), 16);
            g = parseInt(hex.substring(2, 4), 16);
            b = parseInt(hex.substring(4, 6), 16);
        } else {
            return '#FFFFFF';
        }
        
        return `rgba(${r}, ${g}, ${b}, ${alpha})`;
    }
    
    /**
     * Adjust color brightness
     */
    adjustColor(hex, amount) {
        hex = hex.replace('#', '');
        let r = parseInt(hex.substring(0, 2), 16);
        let g = parseInt(hex.substring(2, 4), 16);
        let b = parseInt(hex.substring(4, 6), 16);
        
        r = Math.min(255, Math.max(0, r + amount));
        g = Math.min(255, Math.max(0, g + amount));
        b = Math.min(255, Math.max(0, b + amount));
        
        return `#${r.toString(16).padStart(2, '0')}${g.toString(16).padStart(2, '0')}${b.toString(16).padStart(2, '0')}`;
    }
    
    /**
     * Set GUI definition and re-render
     */
    setGUI(gui) {
        this.gui = gui;
        this.canvas.width = gui.size.width;
        this.canvas.height = gui.size.height;
        this.render();
    }
    
    /**
     * Update a widget and re-render
     */
    updateWidget(widget) {
        const index = this.gui.widgets.findIndex(w => w.id === widget.id);
        if (index !== -1) {
            this.gui.widgets[index] = { ...this.gui.widgets[index], ...widget };
            this.render();
        }
    }
    
    /**
     * Add a widget to the GUI
     */
    addWidget(widget) {
        this.gui.widgets.push(widget);
        this.render();
    }
    
    /**
     * Remove a widget from the GUI
     */
    removeWidget(widgetId) {
        this.gui.widgets = this.gui.widgets.filter(w => w.id !== widgetId);
        if (this.selectedWidget && this.selectedWidget.id === widgetId) {
            this.selectedWidget = null;
        }
        this.render();
    }
    
    /**
     * Select a widget
     */
    selectWidget(widget) {
        this.selectedWidget = widget;
        this.render();
    }
    
    /**
     * Clear selection
     */
    clearSelection() {
        this.selectedWidget = null;
        this.render();
    }
    
    /**
     * Set zoom level
     */
    setZoom(zoom) {
        this.zoom = Math.min(4, Math.max(0.25, zoom));
        this.render();
    }
    
    /**
     * Set offset (panning)
     */
    setOffset(x, y) {
        this.offset.x = x;
        this.offset.y = y;
        this.render();
    }
    
    /**
     * Toggle grid visibility
     */
    toggleGrid() {
        this.showGrid = !this.showGrid;
        this.render();
    }
    
    /**
     * Toggle snap to grid
     */
    toggleSnap() {
        this.snapToGrid = !this.snapToGrid;
    }
    
    /**
     * Snap coordinates to grid
     */
    snapToGrid(x, y) {
        if (!this.snapToGrid) return { x, y };
        return {
            x: Math.round(x / this.gridSize) * this.gridSize,
            y: Math.round(y / this.gridSize) * this.gridSize
        };
    }
    
    /**
     * Mouse event handlers
     */
    handleMouseDown(e) {
        const rect = this.canvas.getBoundingClientRect();
        const x = (e.clientX - rect.left) / this.zoom + this.offset.x;
        const y = (e.clientY - rect.top) / this.zoom + this.offset.y;
        
        // Check if we clicked on a widget
        const widget = this.getWidgetAt(x, y);
        if (widget) {
            this.selectedWidget = widget;
            this.render();
        } else {
            this.clearSelection();
        }
        
        this.dragStart = { x, y };
        this.isDragging = false;
    }
    
    handleMouseMove(e) {
        const rect = this.canvas.getBoundingClientRect();
        const x = (e.clientX - rect.left) / this.zoom + this.offset.x;
        const y = (e.clientY - rect.top) / this.zoom + this.offset.y;
        
        if (this.dragStart && this.selectedWidget) {
            const dx = x - this.dragStart.x;
            const dy = y - this.dragStart.y;
            
            if (Math.abs(dx) > 2 || Math.abs(dy) > 2) {
                this.isDragging = true;
                this.moveWidget(this.selectedWidget, dx, dy);
                this.dragStart = { x, y };
            }
        }
    }
    
    handleMouseUp() {
        if (this.isDragging) {
            this.isDragging = false;
            // Snap to grid after dragging
            if (this.snapToGrid && this.selectedWidget) {
                this.selectedWidget.x = Math.round(this.selectedWidget.x / this.gridSize) * this.gridSize;
                this.selectedWidget.y = Math.round(this.selectedWidget.y / this.gridSize) * this.gridSize;
                this.render();
            }
        }
        this.dragStart = null;
    }
    
    handleMouseLeave() {
        this.dragStart = null;
        this.isDragging = false;
    }
    
    handleWheel(e) {
        e.preventDefault();
        if (e.ctrlKey) {
            // Zoom
            const zoomChange = e.deltaY > 0 ? -0.1 : 0.1;
            this.setZoom(this.zoom + zoomChange);
        } else {
            // Pan
            this.setOffset(
                this.offset.x - e.deltaX / this.zoom,
                this.offset.y - e.deltaY / this.zoom
            );
        }
    }
    
    handleDoubleClick(e) {
        const rect = this.canvas.getBoundingClientRect();
        const x = (e.clientX - rect.left) / this.zoom + this.offset.x;
        const y = (e.clientY - rect.top) / this.zoom + this.offset.y;
        
        const widget = this.getWidgetAt(x, y);
        if (widget) {
            // Double-click on widget - trigger edit action
            this.fireEvent('widget-dblclick', { widget });
        }
    }
    
    handleTouchStart(e) {
        e.preventDefault();
        const touch = e.touches[0];
        const rect = this.canvas.getBoundingClientRect();
        const x = (touch.clientX - rect.left) / this.zoom + this.offset.x;
        const y = (touch.clientY - rect.top) / this.zoom + this.offset.y;
        
        const widget = this.getWidgetAt(x, y);
        if (widget) {
            this.selectedWidget = widget;
        } else {
            this.clearSelection();
        }
        
        this.dragStart = { x, y };
    }
    
    handleTouchMove(e) {
        e.preventDefault();
        const touch = e.touches[0];
        const rect = this.canvas.getBoundingClientRect();
        const x = (touch.clientX - rect.left) / this.zoom + this.offset.x;
        const y = (touch.clientY - rect.top) / this.zoom + this.offset.y;
        
        if (this.dragStart && this.selectedWidget) {
            const dx = x - this.dragStart.x;
            const dy = y - this.dragStart.y;
            this.moveWidget(this.selectedWidget, dx, dy);
            this.dragStart = { x, y };
        }
    }
    
    handleTouchEnd() {
        if (this.snapToGrid && this.selectedWidget) {
            this.selectedWidget.x = Math.round(this.selectedWidget.x / this.gridSize) * this.gridSize;
            this.selectedWidget.y = Math.round(this.selectedWidget.y / this.gridSize) * this.gridSize;
            this.render();
        }
        this.dragStart = null;
    }
    
    /**
     * Get widget at specific coordinates
     */
    getWidgetAt(x, y) {
        // Check widgets in reverse order (topmost first)
        for (let i = this.gui.widgets.length - 1; i >= 0; i--) {
            const widget = this.gui.widgets[i];
            if (this.isPointInWidget(widget, x, y)) {
                return widget;
            }
        }
        return null;
    }
    
    /**
     * Check if point is inside widget
     */
    isPointInWidget(widget, x, y) {
        if (!widget || !widget.visible) return false;
        return x >= widget.x && x <= widget.x + (widget.width || 100) &&
               y >= widget.y && y <= widget.y + (widget.height || 50);
    }
    
    /**
     * Move a widget
     */
    moveWidget(widget, dx, dy) {
        widget.x = (widget.x || 0) + dx;
        widget.y = (widget.y || 0) + dy;
        this.render();
        this.fireEvent('widget-moved', { widget, dx, dy });
    }
    
    /**
     * Resize a widget
     */
    resizeWidget(widget, dw, dh) {
        widget.width = Math.max(10, (widget.width || 100) + dw);
        widget.height = Math.max(10, (widget.height || 50) + dh);
        this.render();
        this.fireEvent('widget-resized', { widget, dw, dh });
    }
    
    /**
     * Event system
     */
    on(event, callback) {
        if (!this.events) this.events = {};
        if (!this.events[event]) this.events[event] = [];
        this.events[event].push(callback);
    }
    
    fireEvent(event, data) {
        if (this.events && this.events[event]) {
            this.events[event].forEach(callback => callback(data));
        }
    }
    
    /**
     * Get current GUI definition
     */
    getGUI() {
        return this.gui;
    }
    
    /**
     * Get selected widget
     */
    getSelectedWidget() {
        return this.selectedWidget;
    }
    
    /**
     * Generate unique widget ID
     */
    generateWidgetId() {
        return `widget_${Date.now()}_${Math.random().toString(36).substr(2, 4)}`;
    }
}

// Export for use in other modules
if (typeof module !== 'undefined' && module.exports) {
    module.exports = GUIRenderer;
}
