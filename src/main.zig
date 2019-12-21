const alloc = @import("std").heap.c_allocator;
const lint = @import("lint.zig");
const rule = @import("rule.zig");
const sink = @import("sink.zig");

pub fn main() anyerror!void {
    var f: []const u8 = "this is a test;regex;wrong thing;xyz";
    const rs = try rule.buildRules(alloc, f);
    defer rule.freeRules(alloc, rs);

    rule.printRules(rs);

    const text = "xyzyahxyz";
    const prose = text[0..];

    var rl = lint.RegexLinter.init(alloc);
    defer rl.deinit();
    const s = sink.StderrSink{};
    const lints = try rl.linter.report(rs.toSlice(), prose, s.sink);
}
