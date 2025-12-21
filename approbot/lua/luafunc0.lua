function foo(x)
print("lua test 2")
do return end
print("x is", x)
end

foo(-9)
print(_VERSION)
error()




common=require "common"
for i = 100,1,-1 do
	common.test()
end
close()

--posa 0:170 1:100 2:0 3:100 4:90

json=require "json"

print (json.teste)


 js={}
zamov=200
za=-600
movz(-700)
print(collectgarbage("count"))--kb
for i = 100,1,-1 do
	posa ( 20 , i-10  ,20 , 120, 90)
	update_axl()
	print(axl[5].x)
	goto continue
	for o, v in ipairs(axl) do
		print(v.x, v.y, v.z)
	end 
	::continue::
end
print(collectgarbage("count"))
close()

js["side"]=0
js["hi"]=260
za=-160
for i = 100,1,-1 do
	func( 12 ,za ,json.stringify(js)) 
	za=za-zamov
	--lsleep(1000)
	pressure()
	print(za)
	if za<-600 then break end
end
wait() 
--print "return"
--goto exit 

posa(180,m,m,m,m)

js["side"]=300
js["hi"]=160
za=-400
for i = 100,1,-1  do
func( 12 , za ,json.stringify(js)) 
za=za+zamov
if za>0 then break end
end

js["side"]=600
js["hi"]=260
za=0
for i = 100,1,-1  do
--func(12 ,za, json.stringify(js))  
za=za-zamov
if za<-600 then break end
end
func(6,-100)
goto exit
--func( 10 ,-300)  
--func( 6 ,-100) 
::exit::