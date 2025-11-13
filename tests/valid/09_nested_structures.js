// 测试深层嵌套的结构
function outer() {
    function inner() {
        function deepest() {
            let x = {
                a: {
                    b: {
                        c: [1, [2, [3, [4, 5]]]]
                    }
                }
            }
            return x
        }
        return deepest()
    }
    return inner()
}

// 嵌套的控制流
for (let i = 0; i < 10; i++) {
    while (true) {
        if (i > 5) {
            break
        } else {
            continue
        }
    }
}
