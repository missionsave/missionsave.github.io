gfwidth=1000
gfdepth=600
gftheight=200
acrthickness=3
qtracks=2

debug=500

rbewidth=15

Part "base1" 
Rec(gfwidth,debug)
--Rec(gfwidth,gfdepth) 
Rotatelx(-90) 
Extrude(acrthickness)
Arrayl(qtracks,0,gftheight)

Part "relevator"
Mloc(gfwidth)
Rec(rbewidth)
Offset(1.5)
Rotatelx(-90)
Extrude(gftheight*qtracks)


Part "study_wall"
--Rec(10,2591)
--Rec(45,2100)
--Movel(-35,10)
Rec(10)
Extrude(10)
Mloc(10,5,0,-90,-90)
Pl "0,0 @5,0 @0,5"

Part "help_hsp"
Rec(1000,600)
Mloc(1000)
Rec(490,50)
Rotatelz(45*3)
Dup()
Mloc(653.52,346.48,0)
Rotatelz(-45*3)
