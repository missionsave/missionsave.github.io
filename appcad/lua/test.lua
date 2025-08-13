function hi()



end

vt={"test1","test2"}

--Part("titletest",{"test1","test2","test2","test2","test2","test2","test2","test2"})




function hi1(incr)
	a = luadraw_new("a" .. incr)
	
	a.visible_hardcoded = false
	--print("a.name =", a.name)
	a:dofromstart(900)
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
	
	c = luadraw_new("t2" .. incr)
	c:fuse(a,b)
	c:rotatez(45 + incr)
	c:rotatey(90)
	c:redisplay()
	
	
	b = luadraw_new("b")

end
for i = 0, 20 do  
    hi1(i)
end
function hi2()












































end
 
function hi3()



end
