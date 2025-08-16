function inocente()
Part ("aa2")
pl("0,0 100,100 @0,100 @-190,90  @0,-30" )
offset(3)
--extrude(-200)

Part ("a2")
pl("0,0 100,100 @0,100 @-140,0 " )
offset(3)
extrude(-200)

--aa2:extrude(-270)

Part("a3")
clone(aa2)
extrude(600)
a3:translate(100)


--Part benfica


end
inocente()




function hiinocent()
--aa2 = luadraw_new("aa2" )
Part ("aa2")
 aa2:pl "0,6 10,10 @0,10 @-10,0 "
 aa2:offset(1)
 aa2:extrude(-20)
 
 --aa2:rotatex(90)
--aa2:create_wire( {  {0,0} , {10,0}, {0,20 }, {-20,0 } },false)
--aa2:create_wire( { (0,0) , (10,10) }),false)

--aa2:create_wire({ Vec2.new(0,0), Vec2.new(11,6),Vec2.new(7,6) }, false)



end
--hiinocent()
function hi()
aa = luadraw_new("aa" )
--aa:dofromstart(0)
aa:create_wire({ Vec2.new(0,0), Vec2.new(11,0), Vec2.new(10,10) , Vec2.new(3,10),Vec2.new(0,0) }, true)


--aa:create_wire({ Vec2.new(0,0), Vec2.new(15,0), Vec2.new(10,10) , Vec2.new(5,12) }, true)

aa:translate(100,1,0)
aa:offset(2)
--aa:translate(10,10,10,1, 110,10,30)

--aa:translate(10)
--aa:extrude(10)

bb = luadraw_new("bb" )
bb:clone(aa)
bb:translate(10)
bb:extrude(10)
--aa:redisplay()

cc = luadraw_new("cc" )
cc:clone(aa)
cc:copy_placement(aa)
cc:translate(0,20)
cc:offset(-2)
cc:extrude(10)


aa1 = luadraw_new("aa1" )
aa1:create_wire({ Vec2.new(0,0), Vec2.new(11,0), Vec2.new(10,10) , Vec2.new(3,10)  }, false)

aa1:copy_placement(cc)
aa1:translate(-20,0,0)
aa1:offset(-2)
aa1:extrude(30)



end
--hi()
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
