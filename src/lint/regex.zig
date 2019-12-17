const c = @cImport({
    @cDefine("_NO_CRT_STDIO_INLINE", "1");
    @cInclude("hs.h");
});
const std = @import("std");
const Lint = @import("../lint.zig").Lint;
const Linter = @import("../lint.zig").Linter;
const LintError = @import("../lint.zig").LintError;
const Sink = @import("../sink.zig").Sink;
const Rule = @import("../rule.zig").Rule;

pub const RegexLinter = struct {
    alloc: *std.mem.Allocator,
    linter: Linter = Linter{ .reportFn = report_lint },

    pub fn init(alloc: *std.mem.Allocator) RegexLinter {
        return RegexLinter{.alloc = alloc};
    }

    fn report_lint(l: *Linter, rules: []const Rule, prose: []const u8, sink: Sink) LintError!void {
        // Compile our regexes
        // const regexes = [0][*]const u8{};
        _ = c.hs_compile_multi;
    }

    fn deinit(self: RegexLinter) void {}
};
