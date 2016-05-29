$fn = 6;
teeth = 85; // 64,68mm
motor_shaft = 42;




// additional_tooth_width = 0;


// T2_5_pulley_dia = tooth_spaceing_curvefit (0.7467,0.796,1.026); // 0.7467,0.796,1.026

// pulley ( "T2.5" , 67.15 , 0.7 , 1.678 ); //0.7 , 1.678

include <Pulley_T-MXL-XL-HTD-GT2_N-tooth.scad>

union() {

	import("sensor_pulley_base.stl");
	
	difference() {
		pulley ( "T2.5" , T2_5_pulley_dia , 0.7 , 1.678 );

		translate([0,0,-1]) cylinder(r=(60/2)+0.2,h=30,$fn=64);
	    
	}

}
