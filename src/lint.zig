const rule = @import("rule.zig");
const Sink = @import("sink.zig").Sink;

pub const RegexLinter = @import("lint/regex.zig").RegexLinter;

pub const Lint = struct {
    text: []u8,
    line: usize,
    col: usize,
    rule: *rule.Rule,
};

// TODO: Parameterize error type?
pub const Linter = struct {
    reportFn: fn(self: *Linter, rules: []const rule.Rule, prose: []const u8, sink: Sink) anyerror!void,

    pub fn report(self: *Linter, rules: []const rule.Rule, prose: []const u8, sink: Sink) anyerror!void {
        return self.reportFn(self, rules, prose, sink);
    }
};
