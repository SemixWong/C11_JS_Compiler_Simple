// 表达式测试
const arithmetic = 1 + 2 * 3 - 4 / 2;
const exponent = 2 ** 3;
const modulo = 10 % 3;

// 比较运算符
const eq = 1 == "1";
const strict_eq = 1 === 1;
const ne = 1 != 2;
const strict_ne = 1 !== "1";
const lt = 1 < 2;
const le = 1 <= 2;
const gt = 2 > 1;
const ge = 2 >= 1;

// 逻辑运算符
const and = true && false;
const or = true || false;
const not = !true;
const nullish = null ?? "default";

// 位运算符
const bitwise_and = 5 & 3;
const bitwise_or = 5 | 3;
const bitwise_xor = 5 ^ 3;
const bitwise_not = ~5;
const lshift = 5 << 1;
const rshift = 5 >> 1;
const urshift = -5 >>> 1;

// 赋值运算符
let x = 10;
x += 5;
x -= 2;
x *= 3;
x /= 2;
x %= 3;
x **= 2;
x <<= 1;
x >>= 1;
x >>>= 1;
x &= 3;
x |= 3;
x ^= 3;

// 递增递减
x++;
++x;
x--;
--x;

// 三元运算符
const result = x > 5 ? "big" : "small";

// 成员访问
const obj = {a: {b: {c: 1}}};
const val1 = obj.a.b.c;
const val2 = obj["a"]["b"]["c"];

// 可选链
const val3 = obj?.a?.b?.c;

// 函数调用
function test(a, b, c) {
    return a + b + c;
}
const sum = test(1, 2, 3);

// 展开运算符 - 简化版本
const arr1 = [1, 2, 3];
const arr2 = [4, 5];
// const arr3 = [...arr1, ...arr2];  // 暂不支持完整的展开语法
