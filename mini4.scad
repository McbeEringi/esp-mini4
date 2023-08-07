$fs=.1;$fa=6;
module joiner(v=1,h=5){
	cube([7.8,2.2+(v-1)*12.1,1]);
	translate([3.4,1.1,1])cube([1,(v-1)*12.1,h]);
	for(i=[1:v])
	translate([1.1,1.1+(i-1)*12.1,1]){
		cylinder(d=2.2,h=h);
		translate([5.6,0,0])cylinder(d=2.2,h=h);
		translate([0,-.5,0])cube([5.6,1,h]);
	}
}
module band(v=1,t=1){
	t=t*2;
	translate([(22.5+t)/2,(12.1*v+t)/2,5])difference(){
		cube([22.5+t,12.1*v+t,10],center=true);
		cube([22.5,12.1*v,11],center=true);
	}
}
module leg()difference(){
	hull(){
		cylinder(d=8,h=2.5);
		translate([40,0,2.5/2])scale([5,5,2.5])sphere(d=1);
	}
	cylinder(d=4.76,h=6,center=true);
	translate([0,-.5,-.1])cube([10,1,3]);
}


v=1;
//for(i=[0:1])translate([30*i,5,0])band(v);
//for(i=[0:1])translate([10*i,-5,0])scale([1,-1,1])joiner(v);
for(i=[0:0])rotate(90)translate([-17,20*i+10,0])leg();
