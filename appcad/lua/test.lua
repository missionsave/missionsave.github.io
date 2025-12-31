--function foo(x)
--print("lua test 2")

----Part robot_ar
----Pl 0,0 70,0 @0,70 @-70,0
----Offset 6

--Part robot_a
--Pl 0,40 @50,0
--Offset -6
--Extrude 7

 --

--Part sketch_profile1
--Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
--Offset (-2)


--do return end
--print("x is", x)
--end

--foo(-9)
--do return end


function test8()
	--error()
	--a=9
	--Part test
	--Pl 0,0 100,100 @200,100 0,0
	--Extrude 20
	----Movel(30)
	--Pl 0,0 100,1000 @100,100 0,0
	--Extrude(60)
	----Movel(150)
	----compound()
Origin (0,0,0)
Part t1
Pl 0,0 10,0 @0,10 @-10,0 0,0
Offset (-3)
Extrude 10

--Part t2
Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
Offset (-3)
--Visible(0)
Extrude (10)
Fuse()
	
	
end
--test8()
--error()
--test31()


function globals() 
container_width=2438
--container_width=1000
container_height=2591
--container_height=1000
--container_height=2895 --hc
container_long=6058
--container_long=1000
--container_long=12192
fcompartment=1000
ttmosaics=23
container_midle=container_width/2

crn_width=162
crn_height=118
crn_long=178

tunel_height=(container_height-120*3)/4

compartments=2
floors=4

compart_len=(container_long-120*(compartments+1))/compartments
end
--testg()

function c40()
container_long=12192
compartments=4
ttmosaics=53
compart_len=(container_long-120*(compartments+1))/compartments
end 

globals()
--c40()







function robot1()
Origin(container_width/4*1-220/2,580,-fcompartment)
--Origin(0,0,0)


Part sketch_body
Pl 0,0 220,0 @0,170 @-220,0 0,0




Part robot_body
Clone sketch_body
Offset -6
Extrude -100
Clone sketch_body
Extrude -6
Fuse()


DebugShape("Robot Origin")

Part sketch_bucket
Pl 0,0, 220,0 @0,100 @-220,0 0,0
--Visible(0)

Part robot_bucket
--Visible(0)
Clone sketch_bucket
Offset -5.2
Extrude 170-50
Rotatelx(90)
Movel(0,170-50,1)
Clone sketch_bucket
Extrude -6
Rotatelx(90)
Movel(0,0,1)
Fuse()



Part servo70
Pl 0,0 66,0 @0,40 @8,0 @0,4 @-8,0 @0,6 @-12,7 @-43,0 @-11,-5 @0,-8 @-8,0 @0,-4 @8,0 0,0
Extrude 30
Circle(4)
--to implement tube with offset
--Offset 2
Extrude (10)
Rotatelx(-90)
Movel(15,57,15) 
Fuse()
Movel(220/2-15,170-57-6,-30-6)




Part servo70_arm
Clone (servo70)
Rotately(180)
Rotatelz(90)
Movel(57+6,170-6-8,-6)




Part robot_arm0
Pl 0,0 90,0 @0,70 @-90,0
Offset(6)
Extrude(30+6)
Pl 0,0 90,0 @0,70 @-90,0 0,0
Extrude(6)
Fuse()
Circle(10)
Movel(90-8,70/2,36/2)
Rotately(90)
Extrude(15)
--Rotatelz(-90)
--Rotatelz(90)
Fuse()
Rotatelx(90)
Movel(-70-10,170,-70-15)
--Movel(-70-0,170-70,-70-15)
--Fuse()
--FilletToAllEdges(2)
--Rotatelx(45)
local gap=2
Movel(-10-gap,-14,29)



Part servo70_arm0
Clone (servo70,0)
Rotately(-90)
Rotatelz(90)
Rotately(-90)
Rotatelz(180)
Movel(-66-8-6-gap,170-6-14-30,14-6)



Part robot_arm1
Pl 0,0 80,0 @20,-6 @180,0
Offset (6)
--Pl 0,12 280,0 @0,6 @-280,0 0,12
Rotatelx(-90)
--Movel(-280-30,170-36-21/2,14+6)
Movel(-280-30,170-40-10,14+6-6)
Extrude(36)
Dup()
Rotatelx(180)
Movel(0,36,-70)
Fuse()
--Movel(-280-30,170-40-10,14+6-6)
--Circle(10)
--Movel(-290,170-14-36/2,-70)
--Extrude 100


Part servo70_arm1
Clone (servo70_arm0,1) 
--Rotatelz(-90)
Movel(-220)




Part robot_arm2
Pl 0,0 240,0 @0,6 @-240,0 0,0
--DebugShape("ds1")
Movel(-280-7-36/2,-(0-156),20-6*0)
--DebugShape("ds2")
Rotatelz(-90)
Rotately(90)
--DebugShape("ds2b")    -- <--- add this
Extrude(36)
--DebugShape("ds3")
Dup()
--DebugShape("ds4")
Movel(0,0,-70-6)
--Rotately(-90)
--DebugShape("ds5")


Part robot_wrist
Pl 0,0 36,0 @0,-60 @-36,0 0,0
Movel(-305,-84,20)
--Circle(10)
Extrude(-82)
Circle(20)
--Pl 0,0 100,0 @0,20 @-100,0 0,0
Movel(-305,-84,20)
Rotatelx(90)
Movel(36/2,0,-82/2)
Extrude(2)
Subtract()




Part sk5
Circle(19)
--Pl 0,0 240,0 @0,6 @-240,0 0,0
Circle(10)
Movel(-18)
--DebugShapes()
Subtract()
--DebugShapes()
--Fuse()
Extrude(5)

--Part robot_arm2
--Pl 0,0 240,0 @0,6 @-240,0 0,0
--DebugShape("ds1")
--Movel(-280-30,-(0-156),20-6*0)
--DebugShape("ds2")
--Rotatelz(-90)
--Rotately(90)
--Extrude(36)
--DebugShape("ds3")
--Dup()
--DebugShape("ds4")
----Movel(0,0,0)
--Movel(0,0,-70-6)
--Rotately(90)
--DebugShape("ds5")

----Visible(0)
----do return end
----Origin(0,0,0)
--Part robot_arm_11
--Pl 0,0 70,66 @70,-66 @70,66 @70,-80
--Offset 6
--Extrude (56) 
--Movel(-285,170-7,-70)

do return end
----FilletToAllEdges(2)
--Rotatelx(90)
----error()
----Movel(-285,170-7,-100)
--Pl 0,0 280,0 @0,6 @-280,0 0,0
--Extrude 56
--Clone sketch_arm_1
--Extrude 56
----FilletToAllEdges(2)
--Rotatelx(90)
--Movel(-285,170-7,-100)
----Fuse()
--Clone sketch_arm_1
--Extrude 56 
--Rotatelx(90)
--Movel(0,0,70)
--Movel(-285,170-7,-100)
--Fuse()

----Part test
----Pl 0,0 100,100 @200,100 @0,100 0,0
----Extrude 20
----Pl 0,0 100,1000 @100,100 0,0
----Extrude 600
----Fuse()

--return
end
  --robot1()


function pmap()
suc_height=203
suc_jun=175.8

Part sketch_peanut
Plhex(suc_height)
Extrude 200
Rotatelx(90)
--Rotately(90)



Part suc
Clone(sketch_peanut)
Movel(0,suc_height)
Clone(sketch_peanut)
Movel(suc_jun,suc_height/2)

for i=1, 12 do
Part succ
Clone(suc,1)
Movel(suc_jun*i,suc_height/2*i)
end


end
--pmap()

function mosaics()
Part mosaic
Pl 0,0 203,0 @0,203 @-203,0 0,0
Extrude(150)
Rotatelx(-90)
Movel(0,240,0)

Part mosaic_clones
for i=0, ttmosaics do
Clone(mosaic,1)
Movel(0,0,-203*i-fcompartment)
end


for j=1, 6 do
Part mosaic_clones_adv
Clone(mosaic_clones,1)
Movel(203*(j-1)*2+203,0,-203/2)
if(j<6) then 
Clone(mosaic_clones,1)
Movel(203*j*2,0,0)
end
end

end

function struct()

Origin(0,0,0)
--Origin(0,0,-1000)

Part sketch_upn40x20
Pl 0,0 0,40 @20,0 @0,-5 @-1,-1 @-14,-2 @0,-24 @14,-2 @1,-1 @0,-5 0,0




Part sketch_profile
Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
Offset (-2)


Part corner
Pl 0,0 162,0 @0,118 @-162,0 0,0
Extrude(crn_long)
Movel(0,0,-crn_long)

Part corner_clones
Clone corner 
Movel(container_width-crn_width,0,-crn_long)
Clone corner 
Movel(0,0,-container_long)
Clone corner
Movel(container_width-crn_width,0,-container_long)

Part corner_clones_up
Clone(corner_clones,1)
Movel(0,container_height-crn_height,0)
Clone(corner,1)
Movel(0,container_height-crn_height,0)



--Part sketch_upn40x20
--Pl 0,0 0,40 @20,0 @0,-5 @-1,-1 @-14,-2 @0,-24 @14,-2 @1,-1 @0,-5 0,0




--Part sketch_profile
--Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
--Offset -2

Part frame
Clone sketch_profile
Extrude(-(container_long-crn_long*2))
Movel(0,0,-crn_long)

Part frame_top
Clone(frame,1) 
Movel(0,container_height-crn_height)

Part frame_right
Clone frame
Rotately(180)
Movel(container_width,0,-(container_long-crn_long*1))

Part frame_right_top
Clone (frame_right,1)
Movel(0,container_height-crn_height)


Part framev
Clone sketch_profile
Extrude((container_height-crn_height*2))
Rotatelx(-90)
Movel(0,crn_height)

Part framevref
Clone(framev,1)
Rotately(90)
Movel(120+50)

Part framevref_right
Clone(framevref,1)
Movel(container_width-120-50*2)


Part cupnref
Clone sketch_upn40x20
Extrude(400)
Rotatelx(-90)
Movel(2+1,crn_height+10,-120+80+2+10)

Part cupnref1
Clone(cupnref,1)
Movel(0,0,-40-5)
Clone(cupnref,1)
Rotately(90)
Movel(110,0,40-2*2)

--error()

 
Part longitudinal
Clone sketch_profile
Extrude(-compart_len)
Movel(0,120,-120)

for i = 0, compartments - 1 do
	Part frameva
	Clone(framev,1)
	Movel(0,0,(-compart_len-120)*i)
	
	Part longc
	Clone(longitudinal,1)
	Movel(0,0,(-compart_len-120)*i)

	Part longi_clones
	for j = 0, floors - 1 do
	Clone(longitudinal,1)
	Movel(0,tunel_height*j+120,(-compart_len-120)*i)
	Clone(longitudinal,1)
	Rotatelz(180)
	Movel(container_width,tunel_height*j+120*2,((-compart_len-120)*i)) 
	end
--Clone(longitudinal,1)
--Movel(0,tunel_height+120*1)
--Clone(longitudinal,1)
--Movel(0,tunel_height*2+120*1)
--Clone(longitudinal,1)
--Movel(0,tunel_height*3+120*1)

--Part longi_right_clones
--Clone(longitudinal_clones,1)
--Rotately(180)
--Movel(container_width,0.01,-container_long)
--Clone(longitudinal,0)
--Rotately(180)
--Movel(container_width,120,-container_long+120)
end 


Part framev_back
Clone(framev,1)
Movel(0,0,-container_long+120)

Part framev_right
Clone(framev,1)
Rotately(180)
Movel(container_width,0,-120)

Part framev_right_back
Clone(framev_right,1)
Movel(0,0,-container_long+120)


Part framet
Clone sketch_profile
Extrude((container_width-crn_width*2))
Rotately(90)
Movel(crn_width)

Part framet_top
Clone(framet,1)
Movel(0,container_height-120)

Part framet_back
Clone(framet,1)
Movel(container_width-crn_width*2,0,-container_long)
Rotately(-180)

Part framet_back_top
Clone(framet_back,1)
Movel(0,container_height-120)


Part upn_ref
Clone sketch_upn40x20
Extrude -200
Copy_placement(framev_back)
Movel(40-40+2,container_height-crn_height*2,-40)

Part upn_ref_lat
Clone(upn_ref,1)
Rotately(-90)
Movel(0,0,-120+40+2)

Part upn_ref_right
Clone(upn_ref,0)
Copy_placement(framev_right_back)

--Part longitudinal
--Clone sketch_profile
--Extrude(-container_long+120*2)
--Movel(0,120,-120)

--Part longitudinal_clones
--Clone(longitudinal,1)
--Movel(0,120*1) 
--Clone(longitudinal,1)
--Movel(0,tunel_height+120*1)
--Clone(longitudinal,1)
--Movel(0,tunel_height*2+120*1)
--Clone(longitudinal,1)
--Movel(0,tunel_height*3+120*1)

--Part longi_right_clones
--Clone(longitudinal_clones,1)
--Rotately(180)
--Movel(container_width,0.01,-container_long)
--Clone(longitudinal,0)
--Rotately(180)
--Movel(container_width,120,-container_long+120)



Part transversal
Clone sketch_profile
Extrude(container_width-50*2)
Rotately(90)
Movel(50,120,0)

Part transversal_back
Clone(transversal,1)
Rotately(180)
Movel(container_width-50*2,0,-container_long)


Part trans_back_clones
Clone(transversal_back,1)
Movel(0,120*1)
Clone(transversal_back,1)
Movel(0,tunel_height+120*1)
Clone(transversal_back,1)
Movel(0,tunel_height*2+120*1)
Clone(transversal_back,1)
Movel(0,tunel_height*3+120*1)


--Clone sketch_profile
--Extrude(container_width-50*2)
--Rotately(-90)
--Movel(container_width-50,tunel_height+120*2,-container_long)


Part pillar
Clone sketch_profile
Extrude(tunel_height-120-90)
Copy_placement(framev)
Rotately(90)
Movel(50+120,120*2)

Part pillar_right
Clone(pillar,1)
Movel(container_width-120-50*2)

Part pillar_back
Clone(pillar,1)
Rotately(180)
Movel(-120,0,-container_long)

--...


Part rail
Clone sketch_profile
Extrude(container_long-fcompartment-50)
Rotatelz(-90)
Movel(container_width/4-120/2 , tunel_height+120*2+20,-container_long+50)

Part rail_right
Clone(rail,1)
Movel(container_width/4*2)

Part rail_clones
Clone(rail,1)
Movel(0,tunel_height)
Clone(rail_right,1)
Movel(0,tunel_height)

Part rail_clones_up
Clone(rail_clones,1)
Movel(0,tunel_height)
Clone(rail_clones,1)
Movel(0,tunel_height*2)





end



function elevator()
--Origin(0,0,0)
Part elv
Clone sketch_profile
Extrude(container_height)
Rotatelx(-90)
Rotately(-90)
Movel(container_midle-120/2,0,-fcompartment)


end












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

function inocent2(o)
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



function circletest()
Part ctest
Circle(5)
Movel(5,5,0)
Extrude(10)
--Rotatelx(90)


end
--circletest()


--struct()

--elevator()

--mosaics()

robot1()

