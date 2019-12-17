const c = @cImport({
    @cInclude("hs.h");
});
const std = @import("std");
const Lint = @import("../lint.zig").Lint;
const Linter = @import("../lint.zig").Linter;
const LintError = @import("../lint.zig").LintError;
const Sink = @import("../sink.zig").Sink;
const Rule = @import("../rule.zig").Rule;

fn report_lint(rules: std.ArrayList(Rule), sink: Sink) LintError!void {
    // Nothing
}

pub const RegexLinter = Linter{ .report = report_lint };
