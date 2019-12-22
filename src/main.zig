const alloc = @import("std").heap.c_allocator;
const copy = @import("std").mem.copy;
const io = @import("std").io;
const fs = @import("std").fs;
const process = @import("std").process;
const lint = @import("lint.zig");
const rule = @import("rule.zig");
const sink = @import("sink.zig");
const argparse = @import("cli/argparse.zig").parseArgs;
const warn = @import("std").debug.warn;

const Args = struct {
    rules: [][]const u8,
    // TODO: Multiple files
    file: ?[]const u8,
};

pub fn main() anyerror!u8 {
    const args = try argparse(Args);
    defer alloc.free(args.rules);

    var prose: []u8 = undefined;
    defer alloc.free(prose);

    const BufInStream = io.BufferedInStream(fs.File.InStream.Error);
    if (args.file) |path| {
        const cwd = try process.getCwdAlloc(alloc);
        defer alloc.free(cwd);
        var dir = try fs.Dir.open(alloc, cwd);
        defer dir.close();
        const f = try dir.openRead(path);
        defer f.close();

        const file_limit = 2000000000;
        const inst = &BufInStream.init(&f.inStream().stream).stream;
        prose = try inst.readAllAlloc(alloc, file_limit);
    } else {
        const stdin = &BufInStream.init(&(try io.getStdIn()).inStream().stream).stream;

        const stdin_limit = 1000000000;
        prose = try stdin.readAllAlloc(alloc, stdin_limit);
    }

    var rules: []u8 = [_]u8{};
    defer alloc.free(rules);
    // TODO: Room for improving efficiency here; reusing scratch space? streaming? something
    for (args.rules) |rf| {
        const cwd = try process.getCwdAlloc(alloc);
        defer alloc.free(cwd);
        var dir = try fs.Dir.open(alloc, cwd);
        defer dir.close();
        const f = try dir.openRead(rf);
        defer f.close();

        const file_limit = 2000000000;
        const inst = &BufInStream.init(&f.inStream().stream).stream;
        const rt = try inst.readAllAlloc(alloc, file_limit);
        defer alloc.free(rt);
        const old_len = rules.len;
        rules = try alloc.realloc(rules, rules.len + rt.len);
        copy(u8, rules[old_len..], rt);
    }

    const rs = try rule.buildRules(alloc, rules);
    defer rule.freeRules(alloc, rs);

    var rl = lint.RegexLinter.init(alloc);
    defer rl.deinit();
    const s = sink.StderrSink{};
    const lints = try rl.linter.report(rs.toSlice(), prose, s.sink);

    return 0;
}
