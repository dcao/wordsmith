const c = @cImport({
    @cDefine("_NO_CRT_STDIO_INLINE", "1");
    @cInclude("hs.h");
});
const std = @import("std");
const Lint = @import("../lint.zig").Lint;
const Linter = @import("../lint.zig").Linter;
const Prose = @import("../prose.zig").Prose;
const Rule = @import("../rule.zig").Rule;
const Sink = @import("../sink.zig").Sink;

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
    prose: Prose = undefined,
    rules: []const Rule = undefined,
    linums: []Linum = undefined,
    err: anyerror = undefined,

    const Linum = struct {
        line: u32,
        col: u32,
    };

    pub fn init(alloc: *std.mem.Allocator) RegexLinter {
        return RegexLinter{ .alloc = alloc };
    }

    extern fn eventHandler(id: c_uint, from: c_ulonglong, to: c_ulonglong, flags: c_uint, ctx: ?*c_void) c_int {
        const self = @ptrCast(*RegexLinter, @alignCast(8, ctx));
        const l = Lint{
            .offset = to,
            .line = self.linums[to].line,
            .col = self.linums[to].col,
            .prose = self.prose,
            .rule = &self.rules[id],
        };
        self.sink.handle(l) catch |e| {
            self.err = e;
            return 1;
        };
        return 0;
    }

    /// Turning a byte offset into a line and column number can be massively
    /// unperformant; thus, we store all of the line and column numbers for
    /// every possible offset into our string in a slice.
    ///
    /// Given input prose size n, our memory complexity is O(n) but our
    /// time complexity to retrieve the line/col of a match is O(1).
    /// Since memory is relatively cheap compared to time, we use this
    /// solution.
    ///
    /// Alternatively, the linums slice could be simply a list of byte
    /// offsets corresponding to newlines; this would mean O(log n)
    /// memory and time complexity.
    ///
    /// In the future this process could likely be further sped up via
    /// SIMD instructions; however, this is unsupported by Zig at this
    /// time.
    fn buildLinums(self: *RegexLinter) !void {
        self.linums = try self.alloc.alloc(Linum, self.prose.text.len);
        var cur: u64 = 0;
        var line: u32 = 1;
        var col: u32 = 0;
        while (cur < self.prose.text.len) {
            if (self.prose.text[cur] == '\n') {
                line += 1;
                col = 1;
            } else {
                col += 1;
            }
            self.linums[cur] = Linum{ .line = line, .col = col };

            cur += 1;
        }
    }

    fn reportLint(l: *Linter, rules: []const Rule, prose: Prose, sink: Sink) anyerror!void {
        const self = @fieldParentPtr(RegexLinter, "linter", l);
        self.rules = rules;
        // Compile our regexes
        var rlens = std.ArrayList(usize).init(self.alloc);
        defer rlens.deinit();
        var regexes = std.ArrayList([*]const u8).init(self.alloc);
        defer self.deinitRegexes(regexes, rlens.toSliceConst());
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
                try rlens.append(new_str.len);
                try regexes.append(@ptrCast([*]const u8, new_str.ptr));
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
            if (comp_err == c.HS_COMPILER_ERROR) {
                std.debug.warn("{}", err);
            }
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

        try self.buildLinums();

        const scan_err = c.hs_scan(db, @ptrCast([*c]const u8, prose.text.ptr), @intCast(c_uint, prose.text.len), 0, scratch, eventHandler, self);
        if (scan_err == c.HS_SCAN_TERMINATED) {
            return self.err;
        } else if (scan_err != c.HS_SUCCESS) {
            return hsConvertErr(scan_err);
        }
    }

    fn deinitRegexes(self: *RegexLinter, rs: std.ArrayList([*]const u8), rlens: []const usize) void {
        var cur: usize = 0;
        while (cur < rs.len) {
            const r = rs.at(cur);
            self.alloc.free(r[0..rlens[cur]]);
            cur += 1;
        }
        rs.deinit();
    }

    fn deinit(self: *RegexLinter) void {
        self.sink = undefined;
        self.prose = undefined;
        self.rules = undefined;

        self.alloc.free(self.linums);
        self.linums = undefined;
    }
};
