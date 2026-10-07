stationated=1
pwidth=80
plen=200
rdim=60
rwwidth=56


Part "wheel"
Circle(rdim/2)
Extrude(30)
--Rec(rdim)
--Movel(-30,-30,30)
--Extrude(7)
Mloc()


Part "foot"
Rec(pwidth,plen)
Extrude(10)
Rotatelx(-90)

Part "wheels_rear"
--https://www.alibaba.com/product-detail/70mm-Hub-In-Wheel-Brushless-Motor_1600097018225.html
--https://www.alibaba.com/product-detail/24V-36V-70mm-Hub-Brushless-Permanent_1601213265598.html
--Clone(wheel)
----Movel(-rdim/2)
--Movel(rdim/2)
--Rotately(-90)
--Movel(0,-rdim/2)

Circle(70/2)
Extrude(rwwidth)
Rotately(-90)
Movel(pwidth/2+rwwidth/2,-rdim/2)

Mloc() 

Movel(0,-8,-plen+rdim/2+20)

if stationated==1 then
Mloc(0,0,-plen)
Rotatelx(180)
end

--Rotatelz(-180)
--Mloc(pwidth/2)
--Mirror(0,1)

Part "wheels"
Clone(wheel)
--Movel(-rdim/2)
Movel(rdim/2)
Rotately(-90)
Movel(0,-rdim/2)

Mloc()
--Rotatelz(-180)

Movel(0,-8,-rdim/2-20)
Mloc()
--Rotatelx(-180)


if stationated==1 then
Mloc(0,20,-50)
do return end
Rotatelx(-180)
end

--Rotatelz(-180)
Mloc(pwidth/2)
Mirror(0,1)

