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


winstdx=1150
winstdy=850
winstdx=1700
winstdy=1100
winstdx=1700
winstdy=2300
winstdx=2100
winstdy=2200

wall_width=55
slab=35 
baseboard=22

tunel_height=tunel_height
  
tunel_height=(container_height-190-slab)/4 -slab-0
 
compartments=2
floors=4

compart_len=(container_long-120*(compartments+1))/compartments

CreateMat("steel", "blue", [[
*MATERIAL, NAME=ST_S355
*ELASTIC
210000., 0.3
*DENSITY
7.85e-9
*PLASTIC
355., 0.
450., 0.02
*EXPANSION
1.2e-5
*CONDUCTIVITY
45.
*SPECIFIC_HEAT
470.
]])
CreateMat("scc","red", [[
*MATERIAL, NAME=SCC_FIBRAS_COMPLETO
*ELASTIC
32000., 0.3
*DENSITY
2.45e-9
*SPECIFIC HEAT
40.0

]])

CreateMat("biochar","blue", [[
*MATERIAL, NAME=BIOCHAR_HUMIDO
*ELASTIC
30., 0.25
*DENSITY
0.65e-9
*EXPANSION
2.5e-5
*CONDUCTIVITY
0.35
*SPECIFIC HEAT
2.2e9
]])




CreateMat("scct","red", [[
*MATERIAL, NAME=SCCT
*ELASTIC
32000., 0.2
*DENSITY
7.45e-9

*EXPANSION
1.1e-5
*CONDUCTIVITY
1.8
*SPECIFIC_HEAT
1.0e9
]])

--CreateMat("scc","red", [[
--*MATERIAL, NAME=SCC_FIBRAS_COMPLETO
--*ELASTIC
--32000., 0.2
--*DENSITY
--2.45e-6
--*PLASTIC
--30., 0.
--45., 0.006
--*EXPANSION
--1.1e-5
--*CONDUCTIVITY
--1.8e-3
--*SPECIFIC_HEAT
--900000.
--]])
--CreateMat("scc","red", [[
--*MATERIAL, NAME=SCC_FIBRAS_COMPLETO
--*ELASTIC
--3000000., 0.2
--*DENSITY
--2.5e-6
--]])

CreateMat("scc_fresco","cyan", [[
*MATERIAL, NAME=SCC_FRESCO
*ELASTIC
1000., 0.499
*DENSITY
2.4e-6
]])

CcxStep([[
*STEP, NLGEOM=YES
*STATIC

*BOUNDARY
CHAO_FIXO,1,3,0.

*ELSET,ELSET=ALL,GENERATE
1,1000,1

*DLOAD
ALL, GRAV, 9810., 0., -1., 0.

*EL FILE
S, E

*END STEP
]])

--CcxStep([[
--*STEP, NLGEOM=YES
--*STATIC
--*BOUNDARY
--CHAO_FIXO, 1, 3, 0.
--*ELSET, ELSET=ALL, GENERATE
--1, 999999
--*DLOAD
--ALL, GRAV, 9810., 0,-1,0
--*CONTROLS, PARAMETERS=TIME INCREMENTATION
--,, 0.1, 1., 10., 1.
--*RESTART, WRITE, FREQUENCY=1
--*EL PRINT, ELSET=ALL
--S, E
--*NODE FILE
--U
--*EL FILE
--S, E
--*END STEP
--]])

end
function struct_xypex()
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
Rec(baseboard,200)
Subtract()

Movel(wall_width,30+160)
Mirrorlx(container_width/2-wall_width,1)
Fuse() --
--Join(1)
Arrayl(3,0,tunel_height+slab)

Part "sketch_dep"
Rec(container_width-wall_width*2,130)
Movel(wall_width,30)
--Mirrorlx(container_width/2-50,1)



Part ("multi_window,scc")
Rec(winstdx,winstdy)
Extrude(6)

Part "multi_windowsf,scc"
Rec(winstdx,winstdy)
Extrude(6)
Movel(0,300,20)
Part "multi_windowsf2,scc_fresco"
Rec(winstdx,winstdy)
Extrude(6)
Movel(0,0,200)

Part "help_window"
Rec(winstdx,winstdy)
Extrude(wall_width)

Part "floor,scc"
Rec(container_width,180)
Extrude(-container_long)
Rec(container_width-35*2,180-30*2)
Movel(35,30,-35)
Extrude(-container_long+35*2)
Subtract()

Part "wall_lat"
Rec(35,container_height)
Extrude(-container_long)

Part "mold_base"
Rec(container_width,container_long)
Rotatelx(-90)
Extrude(-10)



Part "hel_wolffia"
Rec(container_width-wall_width*2,80+120)
Movel(wall_width)



--Part "copilot"
--Pl("0,0 -10,-40 -30,-40 -20,0 0,0 150,0 160,-40 180,-40 170,0 150,0 ")

--Part "

Part "bioc,biochar"
Rec(container_width-wall_width*2-baseboard*2,180)
Extrude(-container_long)
Movel(wall_width+baseboard,781.25)
--Arrayl(2,0,tunel_height+slab)

Part "bioc1,biochar"
Clone(bioc)
Movel(0,tunel_height+slab)
Part "bioc11,biochar"
Clone(bioc1)
Movel(0,tunel_height+slab)

Part "extrusion,scc"
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
Extrude(-10)
Subtract()

Clone(help_window)
Rotately(90)
Movel(0,300,-300)
Arrayl(4,0,0,-winstdx-280)
--Clone(help_window)--
--Rotately(90)
--Movel(0,300,-container_long+1091.9)
--Join(1)
--Arrayl(1,0,winstdy+100)
Mirrorlx(container_width/2,1)
Join(1)

Subtract()



Part "testc,scc"
Rec(2000,100)
Movel(43,800.25,0)
Extrude(-1000)


Part "test,scc"
Rec(300)
Rotatelx(-90)
Extrude(2000*2)
Mirrorlz(-15000/2,1)
--Join()
Mloc(0,1000*2,0)
Rec(400)
Extrude(-15000)
Mloc(0,2000*2,0)
Rec(400)
Extrude(-15000)
Fuse()

Part "test1,biochar"
--Mloc(0,2410,-300)
Rec(300)
Extrude(-1000)
Movel(0,2400,-3000)
--Arrayl(4,0,0,-1500)
Dup()
Movel(0,0,-3000)
--Dup()
--Movel(0,2000,-3000)

Part "test11,biochar"
Rec(300)
Extrude(-1000)
Movel(0,2400,-3000)
--Dup()
--Movel(0,0,-3000)
--Dup()
--Movel(0,2000,-3000)

Part "test11,biochar"
Rec(300)
Extrude(-1000)
Movel(0,2400,-6000)

Part "test111,biochar"
Rec(300)
Extrude(-1000)
Movel(0,4400,-9000)


width=20000
height=6000
Part "testb,biochar"
Rec(width,10)
Extrude(-width)
Movel(0,height)
Mloc()
Rec(40,height)
Extrude(-width)
Mirrorlx(width/2,1)
--Fuse()

----Part "tedtd,scc1"
--Mloc(width/2,height)
--Rec(2000-80*2,20000)
--Extrude(-20000)
----Movel(width/2,height)
Fuse()

Part "t1,biochar"
Rec(1000)
Rotatelx(90)
Extrude(1000)




end
globals()
struct_xypex()