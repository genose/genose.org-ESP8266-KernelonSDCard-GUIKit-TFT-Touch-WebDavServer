/**
 * GUIKit Web Editor - State Management
 */

const EditorState = {
    gui: {
        version: "1.0",
        name: "Untitled GUI",
        size: { width: 320, height: 240 },
        background: "#000000",
        widgets: []
    },
    currentFile: null,
    isSaved: true,
    zoom: 1.0,
    gridEnabled: true,
    snapToGrid: true,
    gridSize: 10,
    selectedWidget: null,
    multiSelect: false,
    selectedWidgets: new Set(),
    isDragging: false,
    dragWidget: null,
    dragStartX: 0,
    dragStartY: 0,
    dragOffsetX: 0,
    dragOffsetY: 0,
    isResizing: false,
    resizeHandle: null,
    resizeStartX: 0,
    resizeStartY: 0,
    resizeStartWidth: 0,
    resizeStartHeight: 0,
    isBoxSelecting: false,
    history: [],
    historyIndex: -1,
    maxHistory: 50
};

let guiSchemaValidator = null;
let canvasElement = null;
let canvasCtx = null;
let boxSelectStartX = 0;
let boxSelectStartY = 0;
let copiedWidget = null;
let lastTouch = null;
let touchTimeout = null;

// Widget types
const WIDGET_TYPES = [
    'view', 'button', 'label', 'text_input', 'text_editor',
    'slider', 'progress_bar', 'image', 'canvas',
    'checkbox', 'radio_button', 'dropdown'
];

// Widget icons (Unicode)
const WIDGET_ICONS = {
    view: '\uD83D\uDC66',
    button: '\uD83D\uDC18',
    label: '\uD83D\uDDD2',
    text_input: '\uD83D\uDCAC',
    text_editor: '\uD83D\uDC40',
    slider: '\uD83C\uDF9A',
    progress_bar: '\uD83D\uDCCA',
    image: '\uD83D\uDDBC',
    canvas: '\uD83C\uDFA8',
    checkbox: '\u2705',
    radio_button: '\uD83D\uDC18',
    dropdown: '\uD83D\uDCC3'
};

// Canvas font mapping
const CANVAS_FONTS = {
    default: 'Arial, sans-serif',
    small: 'Arial, sans-serif',
    medium: 'Arial, sans-serif',
    large: 'Arial, sans-serif',
    monospace: 'Courier New, monospace'
};

// Get canvas font
function getCanvasFont(font) {
    return CANVAS_FONTS[font] || CANVAS_FONTS.default;
}

// Get widget icon
function getWidgetIcon(type) {
    return WIDGET_ICONS[type] || '\u2753';
}

// Get widget types
function getWidgetTypes() {
    return [...WIDGET_TYPES];
}

// Check if widget type is valid
function isValidWidgetType(type) {
    return WIDGET_TYPES.includes(type);
}
