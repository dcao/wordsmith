const c = @cImport({
    @cInclude("hs.h");
});
const std = @import("std");
const lint = @import("lint.zig");
const rule = @import("rule.zig");
const sink = @import("sink.zig");

pub fn main() anyerror!void {
    var f: []const u8 = "yeet;yah;yar;yeet";
    const rs = try rule.build_rules(std.heap.direct_allocator, f);
    defer rs.deinit();

    rule.print_rules(rs);

    const rl = lint.regex.RegexLinter;
    const s = sink.NoopSink{};
    const lints = rl.report(rs, s.sink);
}
