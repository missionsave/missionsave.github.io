radious=(3670)/2
length=11000

 --Part "test"
 --Rec(100)
 --Offset(10)
-- Extrude(100)

Part "test"
Mloc(900,0,0)
Pl "0,0 -100,0 @50,50 @-50,50;r-50 @0,200 @100,0 0,0"
--Rotatelx(-90)
--Pl "0,300 0,100"
--Pl "0,0 0,100"
--Fuse()
Mirrorlx(100,1)
Fuse()


--Circle(100)
--Rec(-50)


--Part "test"
--Clone(test)
----Movel(-30,-30)
----Subtract()
----Fuse()
----Join()
Offset(-6)
Extrude(100)
----Fuse()
----Rec(50)
----  error()
do return end

Part "test"
--Mloc(150,0,0,0,-45)
--Pl "0,0 40,0 @0,40 @-40,0 0,0"
--Rec(40)
Rotatelx(-90)
-- Extrude(90)
--Fuse()


--Clone(test)
--Offset(2)
Extrude(9)
--Fuse()
do return end

Part "ctest"
Mloc(-110,-20)
Circle(220,360)
--Rec(220,360)
Extrude(100)
--Clone(test)
--Subtract()


Part "casco"
--Circle(10)
--Rec(10)
--Offset(2)




--Pl "0,0 30,-40 @20,0 @-20,40 "
--Circle(radious)
Pl "-100,0 @0,400 @2438+200,0;ir-radious @0,-400"
Offset(-100)
Extrude(-length)
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
 --Extrude(-length)
--Pl "0,0 60,60"
--Pl "0,0 @10,0.0;r-60 @0,70  "
--Offset(100)
--Extrude(10)
--Pl "0,0 0,radious"
--Mloc(100)
--Join()

Part "circle"
Circle(radious)
Extrude(length)
--Circle (50)
--Movel(50,0)
--Mloc(1219,1295.5,0)
--Mloc(0)
--Rotatelz(88)

Part "latlastro"
Pl "-100,90 -100,2591;rradious -100,90"
Extrude(length)

Part "cabine"
Rec(2438,2591)
Extrude(-length)

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