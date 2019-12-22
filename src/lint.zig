const rule = @import("rule.zig");
const Prose = @import("prose.zig").Prose;
const Sink = @import("sink.zig").Sink;

pub const RegexLinter = @import("lint/regex.zig").RegexLinter;

pub const Lint = struct {
    offset: u64,
    line: u32,
    col: u32,
    prose: Prose,
    rule: *const rule.Rule,
};

// TODO: Parameterize error type?
pub const Linter = struct {
    reportFn: fn(self: *Linter, rules: []const rule.Rule, prose: Prose, sink: Sink) anyerror!void,

    pub fn report(self: *Linter, rules: []const rule.Rule, prose: Prose, sink: Sink) anyerror!void {
        return self.reportFn(self, rules, prose, sink);
    }
};
