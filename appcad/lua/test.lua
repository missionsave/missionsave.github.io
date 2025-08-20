function robot1()

Part sketch_body
Pl 0,0 220,0 @0,170 @-220,0 0,0

Part robot_body
Clone sketch_body
Offset 4
Extrude -100
Clone sketch_body
Extrude -4 
Pl 0,0 50,0 @0,50 @-50,0 0,0
Offset 4 
Extrude -89
Rotatexl(-90)
Rotatexl(45)
Movel(220,170-50,0)
Movel(50)

Part robot_bucket
Pl 0,0, 220,0 @0,100 @-220,0 0,0
Offset 4
Extrude 170-50
Rotatexl(90)
Movel(0,170-50,0)

Part robot_arm
Pl 0,0 70,0 @0,70 @-70,0
Offset 6
Extrude 70
Movel(-70,170-70,-70-15)


end
robot1()



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
--robot()
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