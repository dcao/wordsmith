const Lint = @import("lint.zig").Lint;

const std = @import("std");
const rule = @import("rule.zig");
const lint = @import("lint.zig");

pub const SinkError = error {
    AllocError,
};

pub const Sink = struct {
    handle: fn(self: *Sink, lint: Lint) SinkError!void
};

// An example Sink that does nothing with Lints:
pub const NoopSink = struct {
    sink: Sink = Sink{.handle = handle},
    
    pub fn handle(s: *Sink, l: Lint) SinkError!void {}
};

// An example Sink which collects everything in an ArrayList:
pub const ALSink = struct {
    // Assume a third party is responsible for allocation
    al: std.ArrayList(Lint),
    sink: Sink = Sink{.handle = handle},

    pub fn handle(s: *Sink, l: Lint) SinkError!void {
        const self = @fieldParentPtr(ALSink, "sink", s);
        _ = self.al.append(l) catch return SinkError.AllocError;
    }
};
    
test "ALSink works" {
    var f: []const u8 = "";
    const rs = try rule.build_rules(std.heap.direct_allocator, f);
    defer rs.deinit();

    rule.print_rules(rs);

    var al = std.ArrayList(Lint).init(std.heap.direct_allocator);
    const rl = lint.regex.RegexLinter;
    var sink = ALSink{.al = al};
    const lints = rl.report(rs, sink.sink);
}
