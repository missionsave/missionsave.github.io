Part sketch
Pl 0,0 200,0 @0,100 @-300,0 @100,-50 0,0
Offset 10

Part p1
Clone sketch
Extrude 800
p1:rotatey(-10)
p1:translate(100,9)

Part p3

Pl 0,0 @30,0 @0,60 @-30,0 0,0
--Connect("p1 4 -1,-0,-0 0,50,300 60000")
Offset(3)
Connect(p1 ,3)
p3:translate(0,-60)
Extrude -50

--Extrude 500
--p3:copy_placement(p1)






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