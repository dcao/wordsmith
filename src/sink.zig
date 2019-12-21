const Lint = @import("lint.zig").Lint;

const std = @import("std");
const rule = @import("rule.zig");

pub const Sink = struct {
    handleFn: fn(s: *Sink, lint: Lint) anyerror!void,

    pub fn handle(self: *Sink, lint: Lint) anyerror!void {
        return self.handleFn(self, lint);
    }
};

// An example Sink that does nothing with Lints:
pub const NoopSink = struct {
    sink: Sink = Sink{.handleFn = handle},
    
    pub fn handle(s: *Sink, l: Lint) anyerror!void {}
};

// An example Sink that just prints them out to stderr
pub const StderrSink = struct {
    sink: Sink = Sink{.handleFn = handle},
    
    pub fn handle(s: *Sink, l: Lint) anyerror!void {
        std.debug.warn("{}\n", l);
    }
};

// An example Sink which collects everything in an ArrayList:
pub const ALSink = struct {
    alloc: *std.mem.Allocator,
    al: std.ArrayList(Lint),
    sink: Sink = Sink{.handleFn = handle},

    pub fn init(alloc: *std.mem.Allocator) ALSink {
        return ALSink {
            .alloc = alloc,
            .al = std.ArrayList(Lint).init(alloc)
        };
    }

    fn handle(s: *Sink, l: Lint) anyerror!void {
        const self = @fieldParentPtr(ALSink, "sink", s);
        try self.al.append(l);
    }

    pub fn deinit(self: ALSink) void {
        self.al.deinit();
    }
};
    
test "ALSink works" {
    var f: []const u8 = "";
    const rs = try rule.buildRules(std.heap.direct_allocator, f);
    defer rs.deinit();

    rule.printRules(rs);

    const prose = "";

    var sink = ALSink.init(std.heap.direct_allocator);
    defer sink.deinit();
    
    var rl = lint.RegexLinter.init(std.heap.direct_allocator);
    defer rl.deinit();
    const lints = rl.linter.report(rs.toSlice(), prose, sink.sink);
}
