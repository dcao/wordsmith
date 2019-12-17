const c = @cImport({
    @cInclude("hs.h");
});
const std = @import("std");
const lint = @import("lint.zig");
const rule = @import("rule.zig");
const sink = @import("sink.zig");

pub fn main() anyerror!void {
    var f: []const u8 = "yeet;yah;yar;yeet";
    const rs = try rule.buildRules(std.heap.direct_allocator, f);
    defer rs.deinit();

    rule.printRules(rs);

    const prose = "";
    
    var rl = lint.RegexLinter.init(std.heap.direct_allocator);
    defer rl.deinit();
    const s = sink.NoopSink{};
    const lints = rl.linter.report(rs.toSlice(), prose, s.sink);
}
