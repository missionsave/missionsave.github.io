function hi()



end

vt={"test1","test2"}

--Part("titletest",{"test1","test2","test2","test2","test2","test2","test2","test2"})




function hi1()
	a = luadraw_new("a")
	
	a.visible_hardcoded = false
	print("a.name =", a.name)
	a:dofromstart(900)
	a:extrude(200)
	a:redisplay()

	b = luadraw_new("b")
	print("b.name =", b.name)
	b:translate(10,0,20)
	--b:rotate(45)
	b:dofromstart(-10)
	b:extrude(-200)
	b.visible_hardcoded = false
	b:redisplay()
	
	c = luadraw_new("t2")
	c:fuse(a,b)
	c:rotatez(45)
	c:rotatey(90)
	c:redisplay()
	
	
	b = luadraw_new("b")

end
hi1()
function hi2()












































end
 
function hi3()



end
