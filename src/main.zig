const alloc = @import("std").heap.c_allocator;
const lint = @import("lint.zig");
const rule = @import("rule.zig");
const sink = @import("sink.zig");

pub fn main() anyerror!void {
    var f: []const u8 = "yeet;yah;yar;yeet";
    const rs = try rule.buildRules(alloc, f);
    defer rule.freeRules(alloc, rs);

    rule.printRules(rs);

    const prose = "";
    
    var rl = lint.RegexLinter.init(alloc);
    defer rl.deinit();
    const s = sink.NoopSink{};
    const lints = rl.linter.report(rs.toSlice(), prose, s.sink);
}
