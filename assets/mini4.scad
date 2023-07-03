$fs=.2;
module joiner(h=5,g=0)translate([1.1,1.1,0]){
	cylinder(d=2.2,h=h);
	translate([4.6+g,0,0])cylinder(d=2.2,h=h);
	translate([0,-.5,0])cube([4.6+g,1,h]);
}
module joiner4(h=5){
	cube([7.8,14.3,1]);
	translate([3.4,1.1,1])cube([1,12.1,5]);
	translate([0,0,1])joiner(h,1);
	translate([0,12.1,1])joiner(h,1);
}
module band()translate([0,0,5])difference(){
cube([25,27,10],center=true);
cube([22.5,12.1*2,20],center=true);
}
module leg(){
	difference(){
		hull(){
			cylinder(d=5,h=2);
			translate([50,0,0])cylinder(d=5,h=2);
			translate([80,0,0])cylinder(d=1,h=2);
		}
		hull(){
			cylinder(d=1,h=5,center=true);
			translate([45,0,0])cylinder(d=1,h=5,center=true);
		}
		translate([50,0,0])cylinder(d=1,h=5,center=true);
	}
}


//band();
//for(i=[2:4])translate([10*i,0,0])joiner4();
leg();



/*
module hold(hole){
	translate([0,0,-2])difference(){
		cube([32.4,22.4,2]);
		if(hole)translate([-1,-1,-1])cube([6,11,5]);
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
module x4(x){hold(x);scale([1,-1,1])hold(x);scale([-1,1,1])hold(x);scale([-1,-1,1])hold(x);}

x4(1);//translate([0,50,0])x4(1);
*/