function robot1()

Part sketch_body
Pl 0,0 220,0 @0,170 @-220,0 0,0

Part robot_body
Clone sketch_body
Offset 6
Extrude -100
Clone sketch_body
Extrude -6
Pl 0,0 50,0 @0,50 @-50,0 0,0
Offset 6

--Rotatexl(-90)
--Rotatexl(45)
--Movel(220,170-50,0)

Extrude -89

Rotatexl(-90)
Rotatexl(45)
Movel(220,170-50,0)
--FilletToAllEdges(2)

--Movel(50)


Part sketch_bucket
Pl 0,0, 220,0 @0,100 @-220,0 0,0
--Visible(0)

Part robot_bucket
--Visible(0)
Clone sketch_bucket
Offset 5.2
Extrude 170-50
Rotatexl(90)
Movel(0,170-50,1)
Clone sketch_bucket
Extrude -6

Rotatexl(90)
Movel(0,0,1)

Part robot_arm
Pl 0,0 70,0 @0,70 @-70,0
Offset 6
Extrude 70
Rotatexl(90)
Movel(-70,170,-70-15)
--FilletToAllEdges(2)


Part skech_arm_1
Pl 0,0 280,0 @0,6 @-280,0 0,0

Part robot_arm_1
Pl 0,0 70,66 @70,-66 @70,66 @70,-66
Offset 7
Extrude (56) 
--FilletToAllEdges(2)
Rotatexl(90)
Movel(-285,170-7,-100)
--Movel(-285,170-7,-100)

--Pl 0,0 280,0 @0,6 @-280,0 0,0
--Extrude 56
Clone skech_arm_1
Extrude 56
--FilletToAllEdges(2)
Rotatexl(90)
Movel(-285,170-7,-100)
Clone skech_arm_1
Extrude 56
--FilletToAllEdges(2)
Movel(0,70)
Rotatexl(90)
Movel(-285,170-7,-100)


end
robot1()

function struct()
container_width=2438
container_height=2591
container_long=6058
--container_long=12000
container_midle=container_width/2

crn_width=162
crn_height=118
crn_long=178

Part corner
Pl 0,0 162,0 @0,118 @-162,0 0,0
Extrude(crn_long)
Movel(0,0,-crn_long)


Part corner_clones
Clone corner
--Extrude(crn_long) 
--Movel(0,0,-crn_long)
--Clone corner
--Extrude(crn_long) 
Movel(container_width-crn_width,0,-crn_long)
Clone corner
--Extrude(crn_long) 
Movel(container_width-crn_width,0,-container_long)
Clone corner
--Extrude(crn_long) 
Movel(0,0,-container_long)

Part sketch_profile
Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
Offset -3
Extrude 12000


end
struct()

















function robot()
Part sketch_body
Pl 0,0 220,0 @0,170 @-220,0 0,0

Part helper_body
Clone sketch_body
Offset 4
Extrude -100

Part body_robotoioioioioioioi
Clone sketch_body
Extrude -4
Fuse helper_body
Pl 0,0 50,0 @0,50 @-50,0 0,0
Extrude -80
--compound()
--body_robot:translate(100,0)

end
--robot() --old
function inocent3()
Part sketch
Pl 0,0 200,0 @0,100 @-300,0 @0,-20 @100,-50 
Offset 10

Part p1
Clone sketch
Extrude 800
p1:rotatez(20)
p1:translate(80)

Part p2
Clone sketch
Extrude 400
Connect(p1,2)
end
--inocent3()

function inocent2()
Part sketch
Pl 0,0 200,0 @0,100 @-300,0 @100,-50 0,0
Offset 10

Part p1
Clone sketch
Extrude 800
--p1:rotatez(-10)
--p1:translate(100,9)

Part p3

Pl 0,0 @50,0 @0,60 @-30,0 0,0
--Connect("p1 4 -1,-0,-0 0,50,300 60000")
Offset(3)
--p3:rotatex(180)
Connect(p1 ,0)
--p3:rotatex(180)
--p3:translate(0,-60)
--p3:translate(0,0,60,1)
Extrude 50

--Extrude 500
--p3:copy_placement(p1)

end
--inocent2()



--to_local(p1)
--to_local(p3)
--to_world(p2)
--Add_placement(p1)
--p3:rotatex(-90)
--Placeg(p1)
--p3:translate(-300)

--to_local(p1)
--to_local(p3)
--to_world(p3)

--Add_placement(p2)
--p3:copy_placement(p2)
--Pl 0,0 40,0 @0,60 @-40,0
--Connect("p2 1 -0,1,0 50,-100,300 60000")
--p3:copy_placement(p2)
--Connect("p2 2 -1,0,0 100,50,300 60000")
--Connect("p2 1 -0,1,0 50,-100,300 60000")
--Revolution p1 90