const alloc = @import("std").heap.c_allocator;
const copy = @import("std").mem.copy;
const io = @import("std").io;
const fs = @import("std").fs;
const process = @import("std").process;
const lint = @import("lint.zig");
const Prose = @import("prose.zig").Prose;
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

    const name_stdin = "stdin";
    
    var text: []const u8 = undefined;
    var name: []u8 = &[_]u8{};
    defer alloc.free(text);

    const BufInStream = io.BufferedInStream(fs.File.InStream.Error);
    if (args.file) |path| {
        const f = try fs.cwd().openFile(path, .{});
        defer f.close();

        const file_limit = 2000000000;
        const inst = &BufInStream.init(&f.inStream().stream).stream;
        text = try inst.readAllAlloc(alloc, file_limit);
        name = try alloc.alloc(u8, path.len);
        copy(u8, name, path);
    } else {
        const stdin = &BufInStream.init(&(io.getStdIn()).inStream().stream).stream;

        const stdin_limit = 1000000000;
        text = try stdin.readAllAlloc(alloc, stdin_limit);
        // TODO: This prolly isn't necessary but...
        name = try alloc.alloc(u8, 5);
        copy(u8, name, name_stdin);
    }

    const prose = Prose{.text = text, .name = name};

    var rules: []u8 = &[_]u8{};
    defer alloc.free(rules);
    // TODO: Room for improving efficiency here; reusing scratch space? streaming? something
    for (args.rules) |rf| {
        const f = try fs.cwd().openFile(rf, .{});
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
