gfwidth=1000
gfdepth=600
gftheight=200
acrthickness=3
qtracks=2

struct_width=20

Part "base1"  
Rec(gfwidth,gfdepth) 
Rotatelx(-90) 
Extrude(acrthickness)
Arrayl(qtracks,0,gftheight)

Part "sketch_struct"
Rec(struct_width)
Offset(1.5)
Rotatelx(-90)

Part "struct_vertical" 
Clone(sketch_struct)
Extrude(gftheight*qtracks)
Movel(gfwidth)
Mloc(0,0,-gfdepth/2,0,-90)
Mirror(0,1)
Join()
Mloc(gfwidth/2)
Mirror(0,1)

Part "struct_longitudinal" 
Clone(sketch_struct)
Extrude(gfdepth)
Rotatelx(-90)
Arrayl(qtracks,0,gftheight)
Mloc(gfwidth/2)
Mirror(0,1)

Part "struct_transversal" 
Clone(sketch_struct)
Extrude(gfwidth)
Rotatelz(-90)
Mloc(0,0,-gfdepth/2,0,-90)
Mirror(0,1)
Dup()
Movel(0,0,gfdepth/2)
Join()
Arrayl(qtracks,0,gftheight)

Part "help_wall"
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
