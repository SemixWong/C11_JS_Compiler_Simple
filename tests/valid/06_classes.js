// 类定义测试
class Animal {
    constructor(name) {
        this.name = name;
    }
    
    speak() {
        console.log(this.name + " makes a sound");
    }
}

// 继承
class Dog extends Animal {
    constructor(name, breed) {
        super(name);
        this.breed = breed;
    }
    
    speak() {
        console.log(this.name + " barks");
    }
}

// 静态方法
class MathUtils {
    static add(a, b) {
        return a + b;
    }
    
    static multiply(a, b) {
        return a * b;
    }
}

// Getter和Setter
class Rectangle {
    constructor(width, height) {
        this._width = width;
        this._height = height;
    }
    
    get area() {
        return this._width * this._height;
    }
    
    set width(value) {
        this._width = value;
    }
}

const dog = new Dog("Buddy", "Golden Retriever");
dog.speak();
