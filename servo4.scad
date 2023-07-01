$fs=.5;
module hold(){
	translate([0,0,-2])difference(){
		cube([32.4,22.4,2]);
		translate([1,-1,-1])cube([30.4,11,5]);
	}
	translate([4.9,0,0])translate([.1,.1,0]){
		translate([0,15.7,0]){
			rotate(180)union(){
				cube([3.7,3,7.15]);
				translate([2.6,0,6.05])rotate([90,0,0])difference(){
					cylinder(d=1.9,h=2,center=true);
					translate([-1.1,1.1,0])rotate([70,0,0])cube([2.2,2.2,5]);
				}
			}
			translate([22.5,0,0])rotate(-90)union(){
				cube([3,3.7,7.15]);
				translate([0,2.6,6.05])rotate([0,90,0])difference(){
					cylinder(d=1.9,h=2,center=true);
					translate([-1.1,-1.1,0])rotate([0,110,0])cube([2.2,2.2,5]);
				}
			}
			translate([0,2.5,0]){
				rotate(90)cube([4.2,5,7.15]);
				translate([22.5,0,0])rotate(0)cube([4.9,4.2,7.15]);
			}
			
		}
	}
}

hold();scale([1,-1,1])hold();scale([-1,1,1])hold();scale([-1,-1,1])hold();
