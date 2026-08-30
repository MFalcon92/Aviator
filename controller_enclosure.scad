// ============================================================
//  THE AVIATOR — Handheld Controller Enclosure
//  Arduino Nano + nRF24L01 + joystick + 5 buttons + laser
//  Print in PETG, 40% infill, 0.2mm layer height
// ============================================================

W     = 140;
H     = 100;
D     = 48;
WALL  = 2.8;
R     = 5;
$fn   = 48;

JOY_HOLE_D   = 14;
BTN_HOLE_D   = 16;
LASER_PORT_D = 8;
USB_W        = 10;
USB_H        = 4.5;
SCREW_D      = 3.2;
SCREW_HEAD_D = 6.0;

// Button positions on front face
JOY_X   = W/2;      JOY_Y   = H/2 + 8;
PWR_X   = 18;       PWR_Y   = H - 18;
ARM_X   = 38;       ARM_Y   = H/2 - 14;
LSR_X   = W - 38;   LSR_Y   = H/2 - 14;
GRB_X   = 28;       GRB_Y   = 18;
REL_X   = W - 28;   REL_Y   = 18;

module rbox(w, h, d, r=R) {
    hull()
        for (x=[r, w-r]) for (y=[r, h-r])
            translate([x, y, 0]) cylinder(r=r, h=d);
}

module front_holes() {
    for (pos=[[JOY_X,JOY_Y,JOY_HOLE_D],
              [PWR_X,PWR_Y,BTN_HOLE_D],
              [ARM_X,ARM_Y,BTN_HOLE_D],
              [LSR_X,LSR_Y,BTN_HOLE_D],
              [GRB_X,GRB_Y,BTN_HOLE_D],
              [REL_X,REL_Y,BTN_HOLE_D]])
        translate([pos[0], pos[1], -1])
            cylinder(d=pos[2], h=WALL+2);
}

module side_holes() {
    // Laser exit port on right side
    translate([W+1, H/2, D/2])
        rotate([0,-90,0]) cylinder(d=LASER_PORT_D, h=WALL+2);
    // USB port on left side for Arduino flashing
    translate([-1, H*0.22, D/2 - USB_W/2])
        cube([WALL+2, USB_H, USB_W]);
}

module bosses() {
    inset = WALL + SCREW_HEAD_D * 0.5;
    for (x=[inset, W-inset]) for (y=[inset, H-inset])
        translate([x, y, WALL])
            difference() {
                cylinder(d=SCREW_HEAD_D * 0.8, h=7);
                translate([0,0,-1]) cylinder(d=SCREW_D, h=9);
            }
}

module pcb_standoffs() {
    // Arduino Nano footprint 18x43mm, centred horizontally
    for (x=[W/2-9, W/2+9]) for (y=[H*0.35, H*0.35+43])
        translate([x, y, WALL])
            difference() {
                cylinder(d=5, h=5);
                translate([0,0,-1]) cylinder(d=2.2, h=7);
            }
}

module battery_door_slot() {
    translate([W*0.2, -1, WALL])
        cube([W*0.6, WALL+2, D*0.38]);
}

module top_shell() {
    difference() {
        rbox(W, H, D, R);
        // hollow interior
        translate([WALL, WALL, WALL])
            rbox(W-WALL*2, H-WALL*2, D-WALL+1, R-WALL);
        front_holes();
        side_holes();
        battery_door_slot();
        // label embossing slots (leave as recesses for stickers or paint)
        translate([PWR_X-8, PWR_Y-9, -0.5]) cube([16, 5, 1.2]);
        translate([GRB_X-8, GRB_Y-9, -0.5]) cube([16, 5, 1.2]);
        translate([REL_X-8, REL_Y-9, -0.5]) cube([16, 5, 1.2]);
        translate([ARM_X-10, ARM_Y-9, -0.5]) cube([20, 5, 1.2]);
        translate([LSR_X-8, LSR_Y-9, -0.5]) cube([16, 5, 1.2]);
    }
    bosses();
    pcb_standoffs();
}

module battery_lid() {
    lw = W * 0.6 - WALL*2;
    lh = D * 0.38 - WALL;
    translate([W*0.2 + WALL, 0, 0]) {
        difference() {
            cube([lw, WALL*1.5, lh + WALL]);
            translate([WALL, -1, WALL]) cube([lw-WALL*2, WALL*2+2, lh]);
            translate([lw/2, -1, lh/2]) rotate([-90,0,0]) cylinder(d=9, h=WALL*2+2);
        }
    }
}

// Preview both parts
top_shell();
translate([0, -(D*0.4 + 12), 0]) battery_lid();
