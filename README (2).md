# cad/controller_enclosure

Handheld housing for the Arduino Nano controller, joystick, buttons, laser, and 9V battery.

## Layout

```
┌─────────────────────────────────────────┐
│  [PWR OFF]                              │
│                                         │
│         ┌───────────────┐               │
│         │  KY-023       │               │
│         │  Joystick     │               │
│         └───────────────┘               │
│                                         │
│  [ARM/LAND]           [LASER]           │
│                                         │
│  [GRAB]               [RELEASE]         │
│                          [Laser port]──►│
│                                         │
│  ══════════════════════════════════     │
│  Battery bay (9V) — rear slide door     │
└─────────────────────────────────────────┘
```

## Design Requirements

- Fits Arduino Nano + nRF24L01 module internally
- 9V battery bay accessible from rear or bottom without tools
- Joystick protrudes through top with full range of motion
- 5 button holes (16mm panel-mount diameter)
- Laser port or window on front face — nRF antenna unobstructed
- Comfortable two-hand grip or pistol-grip form factor

## Suggested Dimensions

| Dimension | Value |
|---|---|
| Width | 140mm |
| Height | 100mm |
| Depth | 45mm |
| Wall thickness | 2.5mm |
| Button hole diameter | 16mm |
| Joystick cutout | 14mm round |

## Files to Add

| Filename | Format | Description |
|---|---|---|
| `enclosure_top.stl` | STL | Top shell with button and joystick holes |
| `enclosure_bottom.stl` | STL | Bottom shell with battery bay |
| `enclosure_assembly.step` | STEP | Full assembly |
| `enclosure_assembly.f3d` | Fusion 360 | Editable source |
