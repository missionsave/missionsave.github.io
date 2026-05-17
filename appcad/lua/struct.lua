function globals() 
container_width=2438
--container_width=1000
container_height=2591
--container_height=1000
--container_height=2895 --hc
container_long=6058
--container_long=1000
--container_long=12192
fcompartment=450
ttmosaics=23
container_midle=container_width/2
   
crn_width=162
crn_height=118
crn_long=178
  
tunel_height=(container_height-120*3)/4 
 
compartments=2
floors=4

compart_len=(container_long-120*(compartments+1))/compartments

CreateMat("steel", "blue", [[
*MATERIAL, NAME=ST_S355
*ELASTIC
210000., 0.3
*DENSITY
7850.
*PLASTIC
355., 0.
450., 0.02
*EXPANSION
1.2e-5
*CONDUCTIVITY
45.
*SPECIFIC HEAT
470.
]])
end
function c40()
container_long=12192
compartments=4
ttmosaics=53
compart_len=(container_long-120*(compartments+1))/compartments
end

function c10()
container_long=2991
compartments=1
ttmosaics=11
compart_len=(container_long-120*(compartments+1))/compartments
end 

function c6()
container_long=1829
compartments=1
ttmosaics=7
compart_len=(container_long-120*(compartments+1))/compartments
end 


function struct()

--Origin(0,0,0)
--Origin(0,0,-1000)

Part "sketch_upn40x20"
Pl "0,0 0,40 @20,0 @0,-5 @-1,-1 @-14,-2 @0,-24 @14,-2 @1,-1 @0,-5 0,0"
 
Part "sketch_profile"
Pl "50,20 @0,-20 @-50,0 @0,120 @50,0 @0,-20"
Offset (-2)

Part "multi_screw_M8"
Circle(8/2)
Extrude(10)

Part "multi_rei200x200x8"
Pl "0,0 200,0 @-200,-200 0,0"
Extrude(8)


 
Part "corners"
Pl "0,0 162,0 @0,118 @-162,0 0,0"
Extrude(-crn_long) 
Dup()
Movel(container_width-crn_width)
Join()
Dup()
Movel(0,0,-container_long+crn_long)
Join()
Dup()
Movel(0,container_height-crn_height,0)
--FilletToAllEdges(2)

--do return end
--region Frame edge horizontal
Part "frame,steel"
Clone (sketch_profile)
local extrudelen=(-compart_len-120+crn_long)
if (compartments==1) then
	extrudelen=-container_long+crn_long*2
end
Extrude(extrudelen)
--Circle(10)
Clone(multi_screw_M8)
Movel(0,120/2,-60)
Rotately(90)
Movel(0,0,-60)
--Extrude(2)
Subtract()
--Mloc()
Movel(0,0,-crn_long)

--do return end

Part "frame_front,steel"
Clone(frame) 
Mirrorly(container_height/2)
Mirrorlx(container_width/2,1)
Mirrorly(-container_height/2,1)

if (compartments>1) then
Part "frame_back"
local len=container_long/2-crn_long
Clone(frame_front)
Mirrorlz(-len)
Clone(frame)
Mirrorlz(-len)
end
--Mirrory(frame,container_height/2-120/2)
--Inverty(container_height/2-120/2)
--Mirrory(container_height/2-120/2)
--Circle(40)

--Part "frame_top1"
--Clone(frame_top,1) 
--Movel(0,container_height-120)
--Circle(80)
----Extrude(-1000)
--Rotatelx(-90)
----Movel(0,900,0"
----do return end
--Part "frame_right"
--Mirror(frame_top,container_width/2-60,1,0,0)
----Clone(frame_top,1)
----Rotately(180)
----Movel(container_width)
----Circle(40)
----Extrude(-1000)
----Rotatelx(-90)
----Subtract()
----do return end
--Part "frame_right_top"
--Mirror(frame_right,-container_height/2+120/2,0,1,0)
--Circle(70)
----Rotately(90)
----Movel(0,70,-170)
----Extrude(10000)
----Subtract()
----Circle(49)
----Movel(400)
--do return end
--Part "test"
--Mirror(frame_right_top,-container_height/2,1,0,0)

--Part "test2"
--Mirror(test,200,1,0,0)
--do return end
--Part (frame_right
--Clone frame
--Rotately(180)
--Movel(container_width,0,-(container_long-crn_long*1))

--Part (frame_right_top
--Clone (frame_right,1)
--Movel(0,container_height-crn_height)


Part "framev"
Clone (sketch_profile)
Extrude((container_height-crn_height*2))
Rotatelx(-90)
Movel(0,crn_height,0)
Dup()
Rotately(90)
Movel(120+50)

Part "framev_clones"
Clone(framev)
Mirrorlx(container_width/2) 
Dup()
Mirrorlz(-container_long/2)
Dup()
Mirrorlx(-container_width/2)

Part "cupnref"
Clone (sketch_upn40x20)
Extrude(-400)
Rotatelx(-90)
Movel(2+1,crn_height+0,-120+80+2+10)

Part "cupnref1"
--Clone(cupnref,1)
--Movel(0,0,-40-5)
Clone(cupnref,1)
Rotately(90)
Movel(110,0,20)
--do return end
--error()

Part "help_longitudinal"
Clone (sketch_profile)
Extrude(-compart_len)

function asmb_longi(l,notop,len)
label = "longitudinal" .. tostring(l)
Part (label  .. ",steel")
Clone (help_longitudinal)
if(notop==0) then
Movel(0,0,-len)
Dup()
len=0
end
Movel(0,120,-len)
Dup()
Movel(0,120)
for j = 1, floors - notop do
Dup()
Movel(0,tunel_height)
end

Part "longitudinal_right"
Clone (_G[label])
Mirrorlx(container_width/2)

return label
end
asmb_longi(0,1,120)


if(compartments>1) then
local len=container_long-compart_len-120*2
Part "compback,steel"
Clone (longitudinal0)
Movel(0,0,-len)
Clone (longitudinal_right)
Movel(0,0,-len)
end

if(compartments>2) then
for i = 1, compartments - 2 do
local len=(compart_len+120)*i+120
local label=asmb_longi(i,0,len)
end
end

if(compartments>1) then
Part "framevmid,steel"
local len=container_long/compartments-(120/compartments)


--transversal
Clone(sketch_profile)
Extrude(container_width-2*2)
Rotately(90)
Movel(2,120*2+20,-len-120/2+50/2)

Clone(multi_rei200x200x8)
Movel(2,120*2+20,-len-120/2+8)
Mirrorlx(container_width/2-2,1)


Join()

for i=1, compartments-2 do

Dup()
Movel(0,0,-len)
end
Join()
for vi=1, floors-0 do

Dup()
Movel(0,tunel_height)
end

Part "lo,steel"
Clone(sketch_profile)
Extrude(container_height)
--Mloc()
Rotatelx(-90)
Movel(0,0,-len)

Mirrorlx(container_width/2,1)
Join()
for i=1, compartments-2 do
Dup()
Movel(0,0,-len)
end
--Part "framevmid_right"
--Clone(framevmid)
--Mirrorlx(container_width/2)

--Part "tranversalmid"
--Clone(sketch_profile)
--Extrude(container_width-2*2)
--Rotately(90)
--Movel(2,tunel_height,-len-120/2+50/2)


end





--do return end
 
Part "framet"
Clone (sketch_profile)
Extrude((container_width-crn_width*2))
Rotately(90)
Movel(crn_width)

Part "framet_clones" 
Clone(framet)
Movel(0,container_height-120)
Mirrorlz(-container_long/2,1)
Mirrorly(-container_height/2,1)


Part "rail"
Clone (sketch_profile)
--Extrude(-100)

local fcompartmentlen=container_long/compartments-fcompartment-50
local len=fcompartmentlen
Extrude(len)
Rotatelz(-90)

Movel(container_width/4-120/2 , tunel_height+120*2+20,-fcompartmentlen-fcompartment-50)




Dup()
Movel(container_width/4*2)

 
Part "Mosaics"
Rec(1200,600)
Rotatelx(-90)
Extrude(10)
Movel(30,120*2+tunel_height+20,-fcompartment-50)
Rotatelz(-(100/360*2))
Mirrorlx(container_width/2-30/2,1)

do return end

local len=400
local xplane=150+20
Part "front_left"
Clone(sketch_profile)
Extrude(len)
Rotately(90)
Movel(xplane,tunel_height,-50)
Circle(4)
Movel(10,120/2,0)
Rotately(90)
Extrude(10)
Subtract()



Part "front_right"
Clone(front_left,1)
Mirrorlx(container_width/2-xplane)
 --do return end

Part "transversal"
Clone (sketch_profile)
Extrude(container_width-50*2)
Rotately(90)
Movel(50,120,0)

Part "transversal_back"
Clone(transversal,1)
Rotately(180)
Movel(container_width-50*2,0,-container_long)


Part "trans_back_clones"
Clone(transversal_back,1)
Movel(0,120*1)
Clone(transversal_back,1)
Movel(0,tunel_height+120*1)
Clone(transversal_back,1)
Movel(0,tunel_height*2+120*1)
Clone(transversal_back,1)
Movel(0,tunel_height*3+120*1)



Part "pillar"
Clone (sketch_profile)
Extrude(tunel_height-120-90)
--Copy_placement(framev)
Rotately(90)
Movel(50+120,120*2)

Part "pillar_right"
Clone(pillar,1)
Movel(container_width-120-50*2)

Part "pillar_back"
Clone(pillar,1)
Rotately(180)
Movel(-120,0,-container_long)

--...


Part "rail"
Clone (sketch_profile)
--Extrude(-100)
Extrude((container_long-fcompartment-50))
Rotatelz(-90)
Movel(container_width/4-120/2 , tunel_height+120*2+20,-container_long+50)

Dup()
Movel(container_width/4*2)


Part "rail_clones"
Clone(rail,1)
Movel(0,tunel_height)
Clone(rail_right,1)
Movel(0,tunel_height)
--do return end

Part "rail_clones_up"
Clone(rail_clones,1)
Movel(0,tunel_height)
Clone(rail_clones,1)
Movel(0,tunel_height*2)





end
globals()
--c6()
--c10()
c40()
struct()