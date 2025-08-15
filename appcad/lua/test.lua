function hiinocent()
aa = luadraw_new("aa" )
 
aa:create_wire( {  {0,0} , {10,0}, {0,20 }, {-20,0 } },true)

end
--hiinocent()
function hi()
aa = luadraw_new("aa" )
--aa:dofromstart(0)
--aa:create_wire({ Vec2.new(0,0), Vec2.new(10,0), Vec2.new(10,10) , Vec2.new(7,10) }, false)

aa:create_wire({ Vec2.new(0,0), Vec2.new(10,0), Vec2.new(10,10) , Vec2.new(5,12) }, true)

--aa:translate(10,1,0)
aa:offset(2,true)
--aa:translate(10,10,10,1, 110,10,30)

--aa:translate(10)
aa:extrude(20)

bb = luadraw_new("bb" )
bb:clone(aa)
bb:translate(40)

--aa:redisplay()
end
hi()
function oioi()
exit()
bb = luadraw_new("bb" )
bb:clone(aa)
--bb:copy_placement(aa)

bb:translate(10)
hi()


exit()

vt={"test1","test2"}

--Part("titletest",{"test1","test2","test2","test2","test2","test2","test2","test2"})
	local incr=10
	a = luadraw_new("a" )
	a.name="test"
	
	a.visible_hardcoded = false
	--print("a.name =", a.name)
	a:dofromstart(600)
	a:translate(10,0,20)
	--a:redisplay()
	a:extrude(200+incr)
	a:redisplay()

end
function bb()
	b = luadraw_new("b" .. incr)
	--print("b.name =", b.name)
	b:translate(100,10,200)
	--b:rotate(45)
	b:dofromstart(-10)
	b:extrude(-200)
	b.visible_hardcoded = false
	--b:redisplay()
	
	
	
	c= luadraw_new( "c") 
	c:fuse(a,b) 
	--c.visible_hardcoded=false
	--c:redisplay()
end
--bb()
function hi1(incr)

	d= luadraw_new( "d") 
	d:clone(c)
	d:rotatez(45 + incr*5)
	d:rotatey(90+incr*5)
	--d:redisplay()
	
	
	--b = luadraw_new("b")

end

function hi2()
	for i = 0, 40 do  
		hi1(i)
	end
	print("benfica.name ") 
end
--hi2()
 
function hi3()



end
