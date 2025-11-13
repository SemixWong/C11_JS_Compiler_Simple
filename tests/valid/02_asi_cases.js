// ASI测试 - return后换行自动插入分号
function test1() {
    return
    42
}

// ASI测试 - 变量声明换行
let a = 1
let b = 2
let c = 3

// ASI测试 - 表达式语句换行
console.log("Hello")
console.log("World")

// ASI测试 - break后换行
function test2() {
    while (true) {
        if (true)
            break
    }
}

// ASI测试 - continue后换行
function test3() {
    for (let i = 0; i < 10; i++) {
        if (i % 2 === 0)
            continue
        console.log(i)
    }
}

// ASI测试 - 对象字面量前不插入分号（正确情况）
let obj = {
    x: 1,
    y: 2
}

// ASI测试 - 文件结束自动插入分号
let last = 100
