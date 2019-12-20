const c = @cImport({
    @cDefine("_NO_CRT_STDIO_INLINE", "1");
    @cInclude("hs.h");
});
const std = @import("std");
const Lint = @import("../lint.zig").Lint;
const Linter = @import("../lint.zig").Linter;
const Sink = @import("../sink.zig").Sink;
const Rule = @import("../rule.zig").Rule;

pub const RegexLinterError = error {
    HyperscanCompile
};

pub const RegexLinter = struct {
    alloc: *std.mem.Allocator,
    linter: Linter = Linter{ .reportFn = reportLint },

    pub fn init(alloc: *std.mem.Allocator) RegexLinter {
        return RegexLinter{.alloc = alloc};
    }

    fn reportLint(l: *Linter, rules: []const Rule, prose: []const u8, sink: Sink) anyerror!void {
        const self = @fieldParentPtr(RegexLinter, "linter", l);
        // Compile our regexes
        var regexes = std.ArrayList([]const u8).init(self.alloc);
        try regexes.ensureCapacity(rules.len);
        defer self.deinitRegexes(regexes);

        for (rules) |r| {
            if (std.mem.eql(u8, r.lint, "regex")) {
                const new_str = try self.alloc.alloc(u8, r.payl.len + 1);
                std.mem.copy(u8, new_str, r.payl);
                new_str[r.payl.len] = 0;
                try regexes.append(new_str);
            }
        }

        var db: *c.hs_database = undefined;
        const db_ptr = @ptrCast([*c]?*c.hs_database, &db);
        var err: *c.hs_compile_error = undefined;
        const err_ptr = @ptrCast([*c][*c]c.hs_compile_error, &err);
        const rs = @ptrCast([*c]const [*c]const u8, &regexes.toSliceConst());
        const nrs = @intCast(c_uint, regexes.len);
        const is_err = c.hs_compile_multi(rs, null, null, nrs, c.HS_MODE_BLOCK, null, db_ptr, err_ptr);
        if (is_err != c.HS_SUCCESS) {
            // TODO: Something more robust
            _ = c.hs_free_compile_error(err);
            return RegexLinterError.HyperscanCompile;
        }
        // TODO: do we ignore this? do/can we even use defer?
        defer _ = c.hs_free_database(@ptrCast(?*c.hs_database, db));

        // TODO: Actual logic
    }

    fn deinitRegexes(self: *RegexLinter, rs: std.ArrayList([]const u8)) void {
        for (rs.toSlice()) |r| {
            self.alloc.free(r);
        }
        rs.deinit();
    }

    fn deinit(self: RegexLinter) void {}
};
