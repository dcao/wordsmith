const std = @import("std");
const rule = @import("rule.zig");
const Sink = @import("sink.zig").Sink;

pub const RegexLinter = @import("lint/regex.zig").RegexLinter;

pub const LintError = error {};

pub const Lint = struct {
    text: []u8,
    line: usize,
    col: usize,
    rule: *rule.Rule,
};

pub const Linter = struct {
    reportFn: fn(self: *Linter, rules: []const rule.Rule, prose: []const u8, sink: Sink) LintError!void,

    pub fn report(self: *Linter, rules: []const rule.Rule, prose: []const u8, sink: Sink) LintError!void {
        return self.reportFn(self, rules, prose, sink);
    }
};
