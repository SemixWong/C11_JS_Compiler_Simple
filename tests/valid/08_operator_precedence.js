// 测试运算符优先级和结合性
let a = 1 + 2 * 3 // 应该是 1 + (2 * 3) = 7
let b = 10 - 5 - 2 // 应该是 (10 - 5) - 2 = 3
let c = 2 ** 3 ** 2 // 应该是 2 ** (3 ** 2) = 512
let d = true && false || true // 应该是 (true && false) || true = true
let e = 1 < 2 && 3 > 2 // 应该是 (1 < 2) && (3 > 2) = true
let f = a + b * c - d / e // 复合表达式
