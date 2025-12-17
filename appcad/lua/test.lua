
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
end
--test8()
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


Part sketch_upn40x20
Pl 0,0 0,40 @20,0 @0,-5 @-1,-1 @-14,-2 @0,-24 @14,-2 @1,-1 @0,-5 0,0




Part sketch_profile
Pl 50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20
Offset -2




function robot1()
Origin(container_width-800,600,-fcompartment)
--Origin(0,0,0)

Part sketch_body
Pl 0,0 220,0 @0,170 @-220,0 0,0

Part robot_body
Clone sketch_body
Offset 6
Extrude -100
Clone sketch_body
Extrude -6
Fuse()


Part sketch_bucket
Pl 0,0, 220,0 @0,100 @-220,0 0,0
--Visible(0)

Part robot_bucket
--Visible(0)
Clone sketch_bucket
Offset 5.2
Extrude 170-50
Rotatelx(90)
Movel(0,170-50,1)
Clone sketch_bucket
Extrude -6
Rotatelx(90)
Movel(0,0,1)
Fuse()

Part robot_arm
Pl 0,0 70,0 @0,70 @-70,0
Offset 6
Extrude 70
Rotatelx(90)
Movel(-71,170,-70-15)
--Fuse()
--FilletToAllEdges(2)


Part sketch_arm_1
Pl 0,0 280,0 @0,6 @-280,0 0,0
Visible(0)

Origin(0,0,0)
Part robot_arm_1
Pl 0,0 70,66 @70,-66 @70,66 @70,-80
Offset 7
Extrude (56) 
Movel(-285,170-7,-100)
error()
--FilletToAllEdges(2)
Rotatelx(90)
--
--Movel(-285,170-7,-100)
Pl 0,0 280,0 @0,6 @-280,0 0,0
Extrude 56
Clone sketch_arm_1
Extrude 56
--FilletToAllEdges(2)
Rotatelx(90)
Movel(-285,170-7,-100)
--Fuse()
Clone sketch_arm_1
Extrude 56 
Rotatelx(90)
Movel(0,0,70)
Movel(-285,170-7,-100)
Fuse()

--Part test
--Pl 0,0 100,100 @200,100 @0,100 0,0
--Extrude 20
--Pl 0,0 100,1000 @100,100 0,0
--Extrude 600
--Fuse()


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
Extrude(200)
Rotatelx(-90)

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
--mosaics()

function struct()

Origin(0,0,0)

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
Movel(container_width/3 , tunel_height+120*2+20,-container_long+50)

Part rail_right
Clone(rail,1)
Movel(container_width/3)

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


 struct()

--robot1()

function elevator()
Part elv
Clone sketch_profile
Extrude(container_height)
Rotatelx(-90)
Rotately(-90)
Movel(container_midle-120/2,0,-fcompartment)


end
--elevator()











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