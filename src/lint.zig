const std = @import("std");
const rule = @import("rule.zig");
const Sink = @import("sink.zig").Sink;

pub const regex = @import("lint/regex.zig");

pub const LintError = error {};

pub const Lint = struct {
    text: []u8,
    rule: *rule.Rule,
};

pub const Linter = struct {
    report: fn(rules: std.ArrayList(rule.Rule), sink: Sink) LintError!void,
};
