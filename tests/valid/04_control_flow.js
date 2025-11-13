// 控制流语句测试
if (true) {
    console.log("if");
} else {
    console.log("else");
}

// while循环
let i = 0;
while (i < 10) {
    i++;
}

// do-while循环
let j = 0;
do {
    j++;
} while (j < 5);

// for循环
for (let k = 0; k < 10; k++) {
    console.log(k);
}

// for-in循环
const obj = {a: 1, b: 2};
for (let key in obj) {
    console.log(key);
}

// for-of循环
const arr = [1, 2, 3];
for (let val of arr) {
    console.log(val);
}

// switch语句
const x = 2;
switch (x) {
    case 1:
        console.log("one");
        break;
    case 2:
        console.log("two");
        break;
    default:
        console.log("other");
}

// try-catch-finally
try {
    throw new Error("test");
} catch (e) {
    console.log(e);
} finally {
    console.log("cleanup");
}
