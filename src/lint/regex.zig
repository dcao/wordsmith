const c = @cImport({
    @cDefine("_NO_CRT_STDIO_INLINE", "1");
    @cInclude("hs.h");
});
const std = @import("std");
const Lint = @import("../lint.zig").Lint;
const Linter = @import("../lint.zig").Linter;
const Sink = @import("../sink.zig").Sink;
const Rule = @import("../rule.zig").Rule;

pub const RegexLinterError = error{
    Invalid,
    NoMem,
    ScanTerminated,
    CompilerError,
    DbVersionError,
    DbPlatformError,
    DbModeError,
    BadAlign,
    BadAlloc,
    ScratchInUse,
    ArchError,
    InsufficientSpace,
    UnknownError,
};

fn hsConvertErr(err: c_int) RegexLinterError {
    return switch (err) {
        c.HS_INVALID => RegexLinterError.Invalid,
        c.HS_NOMEM => RegexLinterError.NoMem,
        c.HS_SCAN_TERMINATED => RegexLinterError.ScanTerminated,
        c.HS_COMPILER_ERROR => RegexLinterError.CompilerError,
        c.HS_DB_VERSION_ERROR => RegexLinterError.DbVersionError,
        c.HS_DB_PLATFORM_ERROR => RegexLinterError.DbPlatformError,
        c.HS_DB_MODE_ERROR => RegexLinterError.DbModeError,
        c.HS_BAD_ALIGN => RegexLinterError.BadAlign,
        c.HS_BAD_ALLOC => RegexLinterError.BadAlloc,
        c.HS_SCRATCH_IN_USE => RegexLinterError.ScratchInUse,
        c.HS_ARCH_ERROR => RegexLinterError.ArchError,
        c.HS_INSUFFICIENT_SPACE => RegexLinterError.InsufficientSpace,
        else => RegexLinterError.UnknownError,
    };
}

pub const RegexLinter = struct {
    alloc: *std.mem.Allocator,
    linter: Linter = Linter{ .reportFn = reportLint },
    sink: Sink = undefined,
    prose: []const u8 = undefined,
    rules: []const Rule = undefined,
    err: anyerror = undefined,

    pub fn init(alloc: *std.mem.Allocator) RegexLinter {
        return RegexLinter{ .alloc = alloc };
    }

    extern fn eventHandler(id: c_uint, from: c_ulonglong, to: c_ulonglong, flags: c_uint, ctx: ?*c_void) c_int {
        const self = @ptrCast(*RegexLinter, @alignCast(8, ctx));
        var line: u64 = 1;
        var column: u64 = 0;
        var cur: u64 = 0;
        while (cur < to) {
            if (self.prose[cur] == '\n') {
                line += 1;
                column = 1;
            } else {
                column += 1;
            }
            cur += 1;
        }
        const l = Lint{ .prose = self.prose[from..to], .line = line, .col = column, .rule = &self.rules[id] };
        self.sink.handle(l) catch |e| {
            self.err = e;
            return 1;
        };
        return 0;
    }

    fn reportLint(l: *Linter, rules: []const Rule, prose: []const u8, sink: Sink) anyerror!void {
        const self = @fieldParentPtr(RegexLinter, "linter", l);
        self.rules = rules;
        // Compile our regexes
        var regexes = std.ArrayList([]const u8).init(self.alloc);
        defer self.deinitRegexes(regexes);
        try regexes.ensureCapacity(rules.len);
        var ids = std.ArrayList(u32).init(self.alloc);
        defer ids.deinit();

        var cur: u32 = 0;
        for (rules) |r| {
            if (std.mem.eql(u8, r.lint, "regex")) {
                const new_str = try self.alloc.alloc(u8, r.payl.len + 1);
                std.mem.copy(u8, new_str, r.payl);
                new_str[r.payl.len] = 0;
                try ids.append(cur);
                try regexes.append(new_str);
            }

            cur += 1;
        }

        var db: ?*c.hs_database = null;
        var err: ?*c.hs_compile_error = null;
        const err_ptr = @ptrCast([*c][*c]c.hs_compile_error, &err);
        const rs = @ptrCast([*c]const [*c]const u8, regexes.toSliceConst().ptr);
        const nrs = @intCast(c_uint, regexes.len);
        const ids_ptr = @ptrCast([*c]const c_uint, ids.toSliceConst().ptr);
        const comp_err = c.hs_compile_multi(rs, null, ids_ptr, nrs, c.HS_MODE_BLOCK, null, &db, err_ptr);
        if (comp_err != c.HS_SUCCESS) {
            _ = c.hs_free_compile_error(err);
            return hsConvertErr(comp_err);
        }
        defer _ = c.hs_free_database(db);

        var scratch: ?*c.hs_scratch = null;
        const scr_err = c.hs_alloc_scratch(db, &scratch);
        if (scr_err != c.HS_SUCCESS) {
            return hsConvertErr(scr_err);
        }
        defer _ = c.hs_free_scratch(scratch);

        self.sink = sink;
        self.prose = prose;

        const scan_err = c.hs_scan(db, @ptrCast([*c]const u8, prose.ptr), @intCast(c_uint, prose.len), 0, scratch, eventHandler, self);
        if (scan_err == c.HS_SCAN_TERMINATED) {
            return self.err;
        } else if (scan_err != c.HS_SUCCESS) {
            return hsConvertErr(scan_err);
        }
    }

    fn deinitRegexes(self: *RegexLinter, rs: std.ArrayList([]const u8)) void {
        for (rs.toSlice()) |r| {
            self.alloc.free(r);
        }
        rs.deinit();
    }

    fn deinit(self: *RegexLinter) void {
        self.sink = undefined;
        self.prose = undefined;
        self.rules = undefined;
    }
};
