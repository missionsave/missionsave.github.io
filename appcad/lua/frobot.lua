function robot()
--Origin(container_width/4*1-220/2,580,-fcompartment)
--Origin(0,0,0"

if (1) then
Part "sketch_body"
Pl "0,0 220,0 @0,170 @-220,0 0,0"




Part "robot_body"
Clone (sketch_body)
Offset (-6)
Extrude (-100)
Clone (sketch_body)
Extrude (-6)
Fuse()


-- DebugShape()Robot Origin))

Part "sketch_bucket"
Pl "0,0 220,0 @0,100 @-220,0 0,0"
--Visible(0)
--do return end

Part "robot_bucket"
--Visible(0)
Clone (sketch_bucket,0)
Offset (-5.2)
Extrude (170-50)
Rotatelx(90)
Movel(0,170-50,1)
Clone (sketch_bucket,0)
Extrude (-6)
Rotatelx(90)
Movel(0,0,1)
Fuse()
--FilletToAllEdges(1)
end

Part "help_servo70"
Movel(0,0,0)
Pl "0,0 66,0 @0,40 @8,0 @0,4 @-8,0 @0,6 @-12,7 @-43,0 @-11,-5 @0,-8 @-8,0 @0,-4 @8,0 0,0"
Extrude (30)
Circle(4)
--to implement tube with offset
--Offset 2
Extrude (10)
Rotatelx(-90)
Movel(15,57,15) 
Fuse()
--Subtract()

Part "servo70_center"
Clone (help_servo70)
--Circle(5)
--Extrude(50)
--Fuse()
--Rotatelx(90)
Movel(220/2-15,170-57-6,-30-6)




Part "servo70_arm"
Clone (help_servo70)
Mloc()
--do return end
Rotately(180)
Rotatelz(90)
Movel(57+6,170-6-8,-6)

--do return end

Part "robot_arm0"
Pl "0,0 90,0 @0,70 @-90,0"
Offset(6)
Extrude(30+6)
Pl "0,0 90,0 @0,70 @-90,0 0,0"
Extrude(6)
Fuse()
Mloc(0,0,0,0,0,0)
Circle(10)
Movel(90-8,70/2,36/2)
Rotately(90)
Extrude(15)
--Rotatelz(-90)
--Rotatelz(90)
--Common()
Subtract()
Mloc(0,0,0,0,0,0)
--Fuse()
Rotatelx(90)
Movel(-70-10,170,-70-15)
--Movel(-70-0,170-70,-70-15)
--Fuse()
--FilletToAllEdges(2)
--Rotatelx(45)
local gap=2
Movel(-10-gap,-14,29)




--do return end

Part "servo70_arm0"
Clone (help_servo70)
Mloc()
Rotately(-90)
Rotatelz(90)
Rotately(-90)
Rotatelz(180)
Movel(-66-8-6-gap,170-6-14-30,14-6)



Part "robot_arm1"
Pl "0,0 80,0 @20,-6 @180,0"
local thick=6
Offset (thick) 
Rotatelx(-90) 
Movel(-280-30,170-40-10,14)
Extrude(36) 
Mirrorlz(-70/2-thick,1) 


Part "servo70_arm1"
Clone (servo70_arm0) 
--Rotatelz(-90)
Movel(-220)



arm2len=240
Part "robot_arm2"
Pl "0,0 arm2len,0 @0,6 @-arm2len,0 0,0"
--DebugShape()ds1))
Movel(-280-7-36/2,-(0-156),20-6*0)
--DebugShape()ds2))
Rotatelz(-90)
Rotately(90)
--DebugShape()ds2b))    -- <--- add this
Extrude(36)
Mirrorlz(-(70+6*2)/2,1)
--DebugShape()ds3))
--Dup()
--DebugShape()ds4))
--Movel(0,0,-70-6)
--Rotately(-90)
--DebugShape()ds5))


Part "robot_wrist"
 Rec(70,30)
 Rotately(-90)
 Movel(-305,-84,-56)
Extrude(-10)
 --Rotately(-90)
 --Movel(-305,-84,-56)
--Extrude(-82)
--Circle(20)
Rec(20)
--Pl "0,0 100,0 @0,20 @-100,0 0,0
Movel(0,0,70/2)
--Rotatelz(90) 
Extrude(-20)
--Join()
Subtract()
--do return end
--Fuse()
--Pl "0,0 100,0 @0,20 @-100,0 0,0"
Circle(6)
--Movel(0,70,0)
--Rotately(-90)
Circle(4)  
Subtract()

Extrude(20)
Movel(0,60,0)
--Rotately(-90)
--Rotatelx(90)
--Move(0,0,1)
----Rec(6)

do return end

Part "sk5"
Pl "0,0 77,0 @0,34 @-77,0 0,0"
Circle(15.8/2)
Movel(18,20)
--Fuse()
--Subtract()
--DebugShapes()
Subtract()

Circle(4.4/2)
Movel(5,15)

--DebugShapes()
----Fuse()
Subtract()
--
--DebugShapes()

Circle(120/2)
Movel(20,55)
--Rotatelz(30)
Common()


Movel(-305,-84,20)
Rotatelz(-90)
Extrude(10)

do return end
end

robot()