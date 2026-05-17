function globals() 
container_width=2438
--container_width=1000
container_height=2591
--container_height=1000
--container_height=2895 --hc
container_long=6058
--container_long=1000
container_long=12192
--container_long=1829
fcompartment=450
ttmosaics=23
container_midle=container_width/2
   
crn_width=162
crn_height=118
crn_long=178
  
tunel_height=(container_height-120*2)/4 
 
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

CreateMat("scc_fresco","cyan", [[
*MATERIAL, NAME=SCC_FRESCO
*ELASTIC
1.e6, 0.499
*DENSITY
2400.
]])

end
function struct_xypex()
winstdx=1150
winstdy=850
winstdx=1700
winstdy=1100
winstdx=1700
winstdy=2300
winstdx=2100
winstdy=2200

wall_width=35
slab=30
lat_thick=50

tunel_height=tunel_height-20
Part "sketch_ext"
Rec(container_width,container_height)

Part "sketch_ext1"
Rec(container_width/2-wall_width-0,tunel_height)
--Pl "0,0 container_width/2-50-50/2,0 @0,tunel_height  @-(container_width/2-50-50/2),0 0,0"
--Offset(-50)

--Rec(100,50)
--Movel(50,tunel_height-50)
--Subtract()

--cunha rail
Pl "0,0 30,-40 @20,0 @-20,40 0,0"
Mirrorlx(90,1)
Join(1)
Movel(600,tunel_height)
Subtract()

--autorama
--Mloc()
--Rec(5,-30)
--Movel(600,tunel_height)
--Subtract()

--rodapé
Mloc()
Rec(8,200)
Subtract()

Movel(wall_width,30+160)
Mirrorlx(container_width/2-wall_width,1)
Fuse() --
--Join(1)
Arrayl(3,0,tunel_height+slab)

Part "sketch_dep"
Rec(container_width/2-50-50/2,130)
Movel(wall_width,30)
Mirrorlx(container_width/2-50,1)



Part ("multi_window")
Rec(winstdx,winstdy)
Extrude(6)

Part "help_window"
Rec(winstdx,winstdy)
Extrude(wall_width)

Part "hel_wolffia"
Rec(container_width-wall_width*2,80+120)
Movel(wall_width)



--Part "copilot"
--Pl("0,0 -10,-40 -30,-40 -20,0 0,0 150,0 160,-40 180,-40 170,0 150,0 ")


Part "extrusion,steel"
Clone(sketch_ext)
Clone(sketch_ext1)
--Common()
Subtract()
Clone(sketch_dep)
Subtract()
Extrude(-container_long)

Rec(container_width-50*2,container_long-50*2)
Rotatelx(-90)
Movel(50,container_height,-50)
Extrude(-35)
Subtract()

Clone(help_window)
Rotately(90)
Movel(0,300,-200)
Arrayl(8,0,0,-winstdx-100)
--Arrayl(1,0,winstdy+100)
Mirrorlx(container_width/2,1)
Join(1)
Subtract()






end
globals()
struct_xypex()