local common={}

function f12(z,side,hi)
	movz(z)
	local a=0
	local b=0
	for i=2 ,1,-1 do
		posaik(side+450+a,190-b, z,1,1,0,0,1 )
		posaik(side+300+a,80-b, z,1,1,0,0,1 )
		--if pressure()>12 then
		--cancel_all() 
		--end
		posaik(side+300+a,hi-b, z ,1,1,0,0,1);
		a=a+50
		b=b+20
	end
end

function common.test()
	zamov=200
	za=-1200
	posa ( 0 , 0  ,20 , 120, 90)
	side=0
	hi=260
	za=-160
	for i = 100,1,-1 do
		f12(za,side,hi) 
		za=za-zamov  
		if za<-600 then break end
	end
	posa(180,m,o,m,m)
	side=300
	hi=160
	za=-400
	for i = 100,1,-1  do
		f12(za,side,hi) 
		za=za+zamov
		if za>0 then break end
	end
end
function common.stand_pose()
	posa ( 0 , -70  ,0 , 30, 0)
	posa ( 0 , -70  ,40 , 30, 0)
end
function common.center()
	common.stand_pose()
	movz(-1000-203/2-203*1)
	posa(m,m,0,m,m)
	posa(m,0,m,m,m)
	posa(m,0,m,0,m)
	for i=1,3 do
	posa(m,m,i*10,i*-10,m)
	end
	do return end
	common.stand_pose()
	posa(180,m,m,m,m)
	for i=0,10 do
	posa ( 0 , 0  ,40-i*2 , 30-i*2, 0)
	end
	--posa ( 0 , 0  ,-60 , -30, 0)
end

function common.test2()
	local x=2438/12*11
	local z=-1000-203*3-203/2
	common.stand_pose()
	movz(z)
	posa(180,0,m,m,m)
	posaik(x,450,z)
	--posa(90,0,m,m,m)

	--for i = 100,1,-1  do
		--f12(za,side,hi) 
		--za=za+zamov
		--if za>0 then break end
	--end
	



end



common.center()
--common.test2()

return common