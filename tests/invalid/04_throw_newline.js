// 错误4: 缺少分号且ASI无法插入
// throw后必须有表达式，且不能换行
function test() {
    throw
    new Error("test");
}
