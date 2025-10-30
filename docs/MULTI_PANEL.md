# Multi-Panel Layouts

Complete guide to chaining multiple HUB75 panels for larger displays.

## Table of Contents

- [Overview](#overview)
- [Row-Major Chaining](#row-major-chaining)
- [Layout Types](#layout-types)
- [Coordinate Remapping](#coordinate-remapping)
- [Physical Wiring](#physical-wiring)
- [Configuration](#configuration)
- [Troubleshooting](#troubleshooting)

---

## Overview

### Key Concepts

**Panel**: Single physical RGB LED matrix module (e.g., 64×64 pixels)
**Display**: Virtual display combining multiple panels (e.g., 128×128 from four 64×64 panels)
**Chain**: Series of panels connected via ribbon cables
**Layout**: Pattern describing how panels are arranged and wired

### Row-Major Traversal

**Critical**: Panels chain **HORIZONTALLY** across rows (not vertically down columns).

```
2×3 Layout (6 panels):
┌───┬───┬───┐
│ 0 │ 1 │ 2 │  ← Row 0: Panels 0, 1, 2
├───┼───┼───┤
│ 3 │ 4 │ 5 │  ← Row 1: Panels 3, 4, 5
└───┴───┴───┘

DMA Chain Order: [0][1][2][3][4][5]
```

**Compatibility**: Matches ESP32-HUB75-MatrixPanel-DMA reference library for easy migration.

---

## Row-Major Chaining

### Panel Numbering

Panels are numbered left-to-right, top-to-bottom (like reading text):

```
┌─────┬─────┬─────┐
│  0  │  1  │  2  │
├─────┼─────┼─────┤
│  3  │  4  │  5  │
├─────┼─────┼─────┤
│  6  │  7  │  8  │
└─────┴─────┴─────┘
```

**Total panels** = `layout_rows × layout_cols`

### Virtual Display Space

Applications draw in virtual coordinates (0,0) to (width-1, height-1):
- **Width** = `panel_width × layout_cols`
- **Height** = `panel_height × layout_rows`

**Example** (2×3 layout, 64×64 panels):
- Virtual display: 192×128 pixels (64×3 = 192 width, 64×2 = 128 height)

**Driver automatically remaps** virtual coords → physical panel chain.

---

## Layout Types

### HORIZONTAL

**Use case**: Single row of panels (simplest)

**Requirements**: `layout_rows = 1`

**Panel orientation**: All upright

**Wiring**: Left-to-right chain

```
┌─────┬─────┬─────┐
│  0→ │ →1→ │ →2  │  All panels upright
└─────┴─────┴─────┘
   ↑     ↑     ↑
  ESP   Cable Cable
```

**Coordinate mapping**: Direct (no transformation)

---

### Serpentine Layouts (Non-ZIGZAG)

**Concept**: Alternate rows are physically mounted **upside down** (rotated 180°) to save cable length.

**Requirements**: `layout_rows > 1`

**Trade-offs**:
- ✅ Shorter cables (saves money)
- ✅ Cleaner physical installation
- ⚠️ Must physically rotate alternate rows

#### TOP_LEFT_DOWN (Serpentine)

**Start**: Top-left corner
**Row 0**: Left→right, panels upright
**Row 1**: Right→left, panels **upside down**
**Row 2**: Left→right, panels upright
...

```
Physical Mounting:
┌─────┬─────┐
│  0→ │ ←1  │  ← Row 0: 0 upright, 1 FLIPPED
├─────┼─────┤
│  2→ │ ←3  │  ← Row 1: 2 upright, 3 FLIPPED
└─────┴─────┘

Wiring (saves cable):
0 OUT → 1 IN (short cable)
      1 OUT ↓ (short drop)
      2 IN ←
2 OUT → 3 IN (short cable)
```

**Y-coordinate inversion**: Driver inverts Y for odd rows (panels 1, 3).

#### TOP_RIGHT_DOWN (Serpentine)

**Start**: Top-right corner
**Row 0**: Right→left, upright
**Row 1**: Left→right, upside down

```
┌─────┬─────┐
│  ←1 │  0← │  ← Row 0: 0 upright, 1 FLIPPED
├─────┼─────┤
│  2→ │  →3 │  ← Row 1: 2 FLIPPED, 3 upright
└─────┴─────┘
```

#### BOTTOM_LEFT_UP / BOTTOM_RIGHT_UP

Same concept but starting from bottom row.

---

### Zigzag Layouts (_ZIGZAG suffix)

**Concept**: All panels mounted **upright** (no physical rotation), but cables snake back.

**Requirements**: `layout_rows > 1 AND layout_cols > 1` (must be actual grid)

**Trade-offs**:
- ✅ All panels same orientation (easier physical install)
- ⚠️ Longer cables required

#### TOP_LEFT_DOWN_ZIGZAG

**Row 0**: Left→right, all upright
**Row 1**: Right→left, all upright (cable routes back)
**Row 2**: Left→right, all upright

```
Physical Mounting (all upright):
┌─────┬─────┐
│  0→ │  →1 │  ← All upright
├─────┼─────┤
│  2→ │  →3 │  ← All upright
└─────┴─────┘

Wiring (longer cables):
0 OUT → 1 IN
      1 OUT ═══════╗ (LONG cable back)
                   ↓
      2 IN ←═══════╝
2 OUT → 3 IN
```

**No Y-inversion**: Driver only reverses X direction for odd rows.

---

## Coordinate Remapping

### Virtual → Physical Transformation

**Pipeline**: Virtual (x,y) → Layout remap → Scan pattern remap → Physical DMA buffer

### HORIZONTAL (No remapping)

```cpp
// Virtual coords map directly to physical chain
virtual_x = 0-191 (for 3 panels × 64 width)
physical_panel = virtual_x / 64
physical_x = virtual_x % 64
```

### Serpentine (TOP_LEFT_DOWN)

```cpp
int row = y / panel_h;           // Which horizontal row of panels
int col = x / panel_w;           // Which column in grid
int local_x = x % panel_w;       // X within panel
int local_y = y % panel_h;       // Y within panel

if ((row & 1) == 1) {  // Odd rows: upside down
  // Reverse X direction (right-to-left in chain)
  x = dma_width - x - 1 - (row * virtual_res_x);

  // Invert Y (panel is physically flipped)
  y = panel_h - 1 - local_y;
} else {  // Even rows: right-side up
  x = ((rows - (row + 1)) * virtual_res_x) + x;
  y = local_y;
}
```

**Result**: Drawing at (100, 70) in virtual space automatically maps to correct physical panel and inverts Y if needed.

### Zigzag (TOP_LEFT_DOWN_ZIGZAG)

```cpp
if ((row & 1) == 1) {  // Odd rows: reversed X
  x = dma_width - x - 1 - (row * virtual_res_x);
} else {  // Even rows: normal X
  x = ((rows - (row + 1)) * virtual_res_x) + x;
}
y = local_y;  // No Y inversion!
```

**Implementation**: `src/panels/panel_layout.h` - `PanelLayoutRemap::remap()`

---

## Physical Wiring

### Connectors

HUB75 panels have two connectors:
- **INPUT** (usually labeled IN) - Receives data
- **OUTPUT** (usually labeled OUT) - Passes data to next panel

### Chain Wiring Examples

#### Two Horizontal Panels

```
     ESP32
       ↓
┌──────────┐      ┌──────────┐
│ Panel 0  │  ══▶ │ Panel 1  │
│   IN OUT │      │ IN   OUT │
└──────────┘      └──────────┘
                         ↓
                     (unused)
```

#### 2×2 Serpentine (TOP_LEFT_DOWN)

```
     ESP32
       ↓
┌──────────┐      ┌──────────┐
│ Panel 0  │  ══▶ │ Panel 1  │  ← Row 0
│  UPRIGHT │      │ FLIPPED  │
│   IN OUT │      │OUT    IN │
└──────────┘      └─────┬────┘
                        │ Short cable
                        ↓
┌──────────┐      ┌──────────┐
│ Panel 2  │  ◀══ │ Panel 3  │  ← Row 1
│  UPRIGHT │      │ FLIPPED  │
│ IN   OUT │      │IN    OUT │
└──────────┘      └──────────┘
```

**Panel 1 and 3**: Physically rotate 180° (text/arrow indicators upside down).

#### 2×2 Zigzag (TOP_LEFT_DOWN_ZIGZAG)

```
     ESP32
       ↓
┌──────────┐      ┌──────────┐
│ Panel 0  │  ══▶ │ Panel 1  │  ← Row 0 (all upright)
│  UPRIGHT │      │ UPRIGHT  │
│   IN OUT │      │IN    OUT │
└──────────┘      └─────┬────┘
                        │ LONG cable back
                        ╰════════╗
                                 ↓
┌──────────┐      ┌──────────┐
│ Panel 2  │  ══▶ │ Panel 3  │  ← Row 1 (all upright)
│  UPRIGHT │      │ UPRIGHT  │
│IN    OUT │      │IN    OUT │
└──────────┘      └──────────┘
```

All panels upright, but cable from Panel 1→2 is longer.

### Power Wiring

**Critical**: ALL panels need 5V power!

**Options**:
1. **Daisy-chain**: Connect 5V/GND across panels via power connectors
2. **Star distribution**: Separate 5V wire from PSU to each panel

**Power Requirements**:
- Single 64×64 panel: ~3-5A @ 5V (full white brightness)
- Four 64×64 panels: ~12-20A @ 5V
- Use adequate power supply!

---

## Configuration

### Menuconfig Settings

**Menu**: `HUB75 Display Configuration → Multi-Panel Layout`

**Example** (2×3 layout, serpentine):
```
CONFIG_HUB75_PANEL_WIDTH=64
CONFIG_HUB75_PANEL_HEIGHT=64
CONFIG_HUB75_LAYOUT_ROWS=2
CONFIG_HUB75_LAYOUT_COLS=3
CONFIG_HUB75_LAYOUT_TOP_LEFT_DOWN=y  # Serpentine
```

**Virtual display**: 192×128 (64×3 width, 64×2 height)

### Code Configuration

```cpp
Hub75Config config = {};
config.panel_width = 64;
config.panel_height = 64;
config.layout_rows = 2;
config.layout_cols = 3;
config.layout = PanelLayout::TOP_LEFT_DOWN;  // Serpentine

Hub75Driver driver(config);
driver.begin();

// Draw at virtual coordinates
driver.set_pixel(100, 70, 255, 0, 0);  // Driver remaps automatically
```

### Validation

Driver validates configuration:
- `layout_rows = 1` → Must use HORIZONTAL
- Serpentine → Requires `layout_rows > 1`
- Zigzag → Requires `layout_rows > 1 AND layout_cols > 1`

---

## Troubleshooting

### All Panels Show Same Content

**Cause**: `layout_cols = 1` (single panel mode)
**Fix**: Set `layout_cols` to actual number of horizontal panels

### Second/Third Panel is Black

**Causes**:
- Ribbon cable not connected (Panel N OUT → Panel N+1 IN)
- Power not connected to Panel N+1
- Cable too long or damaged

**Debug**:
1. Verify physical cable connections
2. Check 5V power to ALL panels
3. Try shorter/higher quality ribbon cables

### Scrambled Display Across Panels

**Causes**:
- Wrong layout type (e.g., HORIZONTAL when serpentine wiring)
- Wrong scan_wiring pattern

**Fix**:
1. Verify layout type matches physical wiring
2. Check panel orientation (serpentine = alternate rows upside down)
3. Try different scan_wiring patterns if needed

### Colors Correct on Panel 0, Wrong on Others

**Causes**:
- Mixed panel types (different shift drivers)
- One panel needs FM6126A init, others don't

**Fix**:
- Use panels of same model/manufacturer
- Try FM6126A shift driver (works for most modern panels)

### Panels in Wrong Order

**Cause**: Mismatched physical wiring and layout configuration
**Fix**: Either:
1. Rewire panels to match layout, OR
2. Change layout type to match wiring

### Panel N Shows Content from Wrong Position

**Causes**:
- Wrong layout_rows or layout_cols
- Wrong layout type

**Debug**:
1. Count physical panels horizontally → `layout_cols`
2. Count physical panel rows vertically → `layout_rows`
3. Check if alternate rows are upside down → serpentine, else HORIZONTAL/zigzag

---

## Examples

See working examples:
- `examples/02_multi_panel/two_horizontal/` - 2×1 HORIZONTAL
- `examples/02_multi_panel/three_horizontal/` - 3×1 HORIZONTAL
- `examples/02_multi_panel/serpentine/` - 2×1 TOP_LEFT_DOWN (serpentine)

---

## Design Recommendations

### Cable Management

**Serpentine**:
- ✅ Use when panels can be rotated
- ✅ Clean installations (walls, frames)
- Saves ~50% cable length vs zigzag

**Zigzag**:
- ✅ Use when panels must stay upright
- ✅ Easier to troubleshoot (all panels same orientation)
- Requires longer cables

**HORIZONTAL**:
- ✅ Use for single-row installations (simplest)
- ✅ No coordinate remapping overhead

### Scalability

**Tested configurations**:
- 2×1 (128×64)
- 3×1 (192×64)
- 2×2 (128×128)

**Theoretical maximum**: 8×8 (limited by menuconfig range)

**Practical limits**:
- **Power**: PSU must supply adequate amperage
- **Signal integrity**: Long chains may need buffering
- **Memory**: GDMA limited by internal SRAM, PARLIO by PSRAM
- **Refresh rate**: More panels = more data = potentially lower refresh

---

## Related Documentation

- **[MENUCONFIG.md](MENUCONFIG.md)** - Layout configuration options
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - How coordinate remapping works
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Multi-panel debug tips

---

**For implementation details**, see:
- `src/panels/panel_layout.h` - Coordinate remapping logic
- `examples/02_multi_panel/` - Working examples
