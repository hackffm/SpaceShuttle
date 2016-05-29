$fn = 32;
teeth = 11; // 64,68mm
motor_shaft = 1;
pulley_t_ht = 16;

include <Pulley_T-MXL-XL-HTD-GT2_N-tooth.scad>;

shaft_radius = 5.4/2;


difference() {
	union() {
		pulley ( "XL" , XL_pulley_dia , 1.27, 3.051 );
		hull() {
			cylinder(r=15/2,h=3.5);
			cylinder(r=18/2,h=3);
		}
	}

	difference() {
		translate([0,0,-1]) cylinder(r=shaft_radius,h=20);
		translate([5+1.6,0,10+8]) cube([10,10,20],center=true);	
	}	
}	



