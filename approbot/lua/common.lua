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
	za=-600
	posa ( 0 , 90  ,20 , 120, 90)
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

--common.test()

return common