function hi()



end

vt={"test1","test2"}

--Part("titletest",{"test1","test2","test2","test2","test2","test2","test2","test2"})
	local incr=10
	a = luadraw_new("a" .. incr)
	
	a.visible_hardcoded = false
	--print("a.name =", a.name)
	a:dofromstart(600)
	a:extrude(200+incr)
	a:redisplay()

	b = luadraw_new("b" .. incr)
	--print("b.name =", b.name)
	b:translate(10,0,20)
	--b:rotate(45)
	b:dofromstart(-10)
	b:extrude(-200)
	b.visible_hardcoded = false
	b:redisplay()


	c= luadraw_new( "c") 
	c:fuse(a,b) 
	--c.visible_hardcoded=false
	c:redisplay()
function hi1(incr)

	d= luadraw_new( "d") 
	d:clone(c)
	d:rotatez(45 + incr*5)
	d:rotatey(90+incr*5)
	d:redisplay()
	
	
	--b = luadraw_new("b")

end

function hi2()
	for i = 0, 400 do  
		hi1(i)
	end
end
hi2()
 
function hi3()



end
