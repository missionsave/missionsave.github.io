 #pragma region  Includes
 #pragma endregion  Includes

struct top: public Fl_Window {
 
#pragma region view_rotate
struct sbts {
    string label;
    std::function<void()> func;
    bool is_setview=0;
    struct vs{
		Standard_Real dx=0, dy=0, dz=0, ux=0, uy=0, uz=0;
	}v;

}
void test(){}
#pragma endregion view_rotate

};   



// #pragma region TopLevelRef
void functionA() {
    // some code
}




class MyClass {
	#pragma region ClassMembers
	private:
		int member1;
		void method1() {
			#pragma region MethodInternal
			int oi;
			#pragma endregion MethodInternal
		}
	public:
		void method2();
#pragma endregion ClassMembers
};

struct MyStruct {
    #pragma region StructMembers
    int data;
    void helper() {
        // ...
    }
    #pragma endregion StructMembers
};

// #pragma endregion TopLevelRegion

void functionB() {
    // ...
}