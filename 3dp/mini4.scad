$fs=.1;$fa=6;
module joiner(d,icon,c=2.2,h=4.5){
	translate([-2.5,-5,0])cube([5+d,10,1]);
	translate([0,0,1]){
		cylinder(d=c,h=h);
		translate([d,0,0])cylinder(d=c,h=h);
		translate([0,-.5,0])cube([d,1,h]);
		
		intersection(){
			for(i=[1:1])translate([d*i/2,0,-.5])rotate([45,0,0])
				cube([1,5.5*sqrt(2),5.5*sqrt(2)],center=true);
			translate([-2.5,-5,0])cube([5+d,10,h]);
		}
	}
}

module band(d,t=1,b=3){
	t=t*2;
	sw=22.5+.1;sh=12.1+.1;sd=27.7-sw;
	linear_extrude(9)translate([0,(sh+t)/2+b])difference(){
		translate([0,-b/2])square([sw*2+sd+t+d,sh+t+b],center=true);
		for(i=[-1,1])translate([(sw+sd+d)/2*i,0])
			square([sw,sh],center=true);
		w=(d+sd-t)/2;h=sh+t/2;r=sqrt((w*w+h*h)/4*(1+w*w/h/h));
		translate([0,max(h,r)-sh/2])circle(r=r<h?(d+sd-t)/2:r);
	}
}

module leg(l){
	difference(){
		hull(){
			cylinder(d=8,h=3);
			translate([l,0,0])cylinder(d=5,h=3);
		}
		cylinder(d=4.76,h=8,center=true);
		translate([0,-.5,-.1])cube([10,1,4]);
	}
	//translate([l,0,0])cylinder(d=5,h=10);
}

d=9;
l=47.2;
translate([0,12,0])band(d);
for(i=[-1,1])scale([i,1,1])translate([3.5,6,0])joiner(d);
for(i=[0:3])translate([-35,-5-10*i,0])leg(l);
