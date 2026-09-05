radious=(3670)/2

-- Part "test"
-- Rec(100)
-- Offset(10)
-- Extrude(100)

Part "test"
Pl "0,0 -100,0 @50,50 @-50,50;r80 @0,200 @100,0"
--Rotatelx(-90)
--Pl "0,300 0,0"
Mirrorlx(100,1)
--Fuse()
--Join()
Offset(10)
Extrude(100)
--Rec(50)
-- error()
--do return end
Part "casco"
--Circle(10)
--Rec(10)
--Offset(2)



--Pl "0,0 30,-40 @20,0 @-20,40 "
--Circle(radious)
Pl "-100,0 @0,400 @2438+200,0;ir-radious @0,-400"
Offset(-100)
Extrude(-12000)
Movel(0,-310)

--Part "tampa"
--Mloc(1119,1471.59,0)
--Circle(radious-10)
--Extrude(-100)
--Fuse()

--Mloc(0,400)
--Pl "0,0 -2438/2,radious;rradious"


--Circle(radious)
--Subtract()
--Movel(2438/2,2591/2+300+200)
 --Extrude(-12000)
--Pl "0,0 60,60"
--Pl "0,0 @10,0.0;r-60 @0,70  "
--Offset(100)
--Extrude(10)
--Pl "0,0 0,radious"
--Mloc(100)
--Join()

--Circle (50)
--Movel(50,0)
--Mloc(1219,1295.5,0)
--Mloc(0)
--Rotatelz(88)

Part "cabine"
Rec(2438,2591)
Extrude(-12000)

Part "clat"
Clone(casco)
Part "clat"
Mloc(1000,10)
Rec(400)
Clone(casco)
Mloc(0,0)
--Movel(-2438)
--Movl()
Rotatelz(88)