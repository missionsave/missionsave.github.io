// test.cpp

#pragma region TopLevelRef
void functionA() {
    // some code
}

class MyClass {
#pragma region ClassMembers
private:
    int member1;
    void method1() {
        #pragma region MethodInternal
        // ...
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

#pragma endregion TopLevelRegion

void functionB() {
    // ...
}