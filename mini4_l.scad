$fs=.1;$fa=6;
module joiner(d=30.5,c=2.2,h=4.5){
	translate([-2.5,-5,0])cube([5+d,10,1]);
	translate([0,0,1]){
		cylinder(d=c,h=h);
		translate([d,0,0])cylinder(d=c,h=h);
		translate([0,-.5,0])cube([d,1,h]);
		
		intersection(){
		translate([d/2,0,-.5])rotate([45,0,0])
			cube([1,5.5*sqrt(2),5.5*sqrt(2)],center=true);
		translate([-2.5,-5,0])cube([5+d,10,h]);
		}
	}
}

module band(d=30.5){
	t=2;
	translate([0,(12.1+t)/2,5])difference(){
		cube([22.5*2+5.2+t+d,12.1+t,10],center=true);
		for(i=[-1,1])translate([(22.5+5.2+d)/2*i,0,0])
			cube([22.5,12.1,11],center=true);
		w=(d+5.2-t)/2;h=12.1+t/2;r=sqrt((w*w+h*h)/4*(1+w*w/h/h));
		translate([0,r-12.1/2,0])cylinder(r=r,h=11,center=true);
	}
}

module leg()difference(){
	hull(){
		cylinder(d=8,h=2.5);
		translate([44,0,0])cylinder(d=6,h=2.5);
		translate([66,0,2.5/2])scale([5,5,2.5])sphere(d=1);
	}
	cylinder(d=4.76,h=6,center=true);
	translate([0,-.5,-.1])cube([10,1,3]);
}

band();
for(i=[-1,1])scale([i,1,1])translate([3.5,21,0])joiner();
for(i=[0:3])translate([-35,-5-10*i,0])leg();
