const ArrayList = @import("std").ArrayList;
const Builder = @import("std").build.Builder;
const mem = @import("std").mem;
const fs = @import("std").fs;
const warn = @import("std").debug.warn;

pub fn build(b: *Builder) anyerror!void {
    const mode = b.standardReleaseOptions();
    const exe = b.addExecutable("ws", "src/main.zig");
    exe.setBuildMode(mode);

    exe.linkSystemLibrary("c");
    const ctx = try getContext(b);
    try linkStdCpp(b, exe, ctx);
    exe.linkSystemLibrary("libhs");

    exe.install();

    const run_cmd = exe.run();
    run_cmd.step.dependOn(b.getInstallStep());

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}

// To get linking against libstdc++ working, we need the following code
// Stolen from the zig compiler's build.zig script
// Tracking issue: #3936
const LibraryDep = struct {
    prefix: []const u8,
    libdirs: ArrayList([]const u8),
    libs: ArrayList([]const u8),
    system_libs: ArrayList([]const u8),
    includes: ArrayList([]const u8),
};

const Context = struct {
    cmake_binary_dir: []const u8,
    cxx_compiler: []const u8,
    llvm_config_exe: []const u8,
    lld_include_dir: []const u8,
    lld_libraries: []const u8,
    dia_guids_lib: []const u8,
    llvm: LibraryDep,
};

fn findLLVM(b: *Builder, llvm_config_exe: []const u8) !LibraryDep {
    const shared_mode = try b.exec(&[_][]const u8{ llvm_config_exe, "--shared-mode" });
    const is_static = mem.startsWith(u8, shared_mode, "static");
    const libs_output = if (is_static)
        try b.exec(&[_][]const u8{
            llvm_config_exe,
            "--libfiles",
            "--system-libs",
        })
    else
        try b.exec(&[_][]const u8{
            llvm_config_exe,
            "--libs",
        });
    const includes_output = try b.exec(&[_][]const u8{ llvm_config_exe, "--includedir" });
    const libdir_output = try b.exec(&[_][]const u8{ llvm_config_exe, "--libdir" });
    const prefix_output = try b.exec(&[_][]const u8{ llvm_config_exe, "--prefix" });

    var result = LibraryDep{
        .prefix = mem.tokenize(prefix_output, " \r\n").next().?,
        .libs = ArrayList([]const u8).init(b.allocator),
        .system_libs = ArrayList([]const u8).init(b.allocator),
        .includes = ArrayList([]const u8).init(b.allocator),
        .libdirs = ArrayList([]const u8).init(b.allocator),
    };
    {
        var it = mem.tokenize(libs_output, " \r\n");
        while (it.next()) |lib_arg| {
            if (mem.startsWith(u8, lib_arg, "-l")) {
                try result.system_libs.append(lib_arg[2..]);
            } else {
                if (fs.path.isAbsolute(lib_arg)) {
                    try result.libs.append(lib_arg);
                } else {
                    if (mem.endsWith(u8, lib_arg, ".lib")) {
                        lib_arg = lib_arg[0 .. lib_arg.len - 4];
                    }
                    try result.system_libs.append(lib_arg);
                }
            }
        }
    }
    {
        var it = mem.tokenize(includes_output, " \r\n");
        while (it.next()) |include_arg| {
            if (mem.startsWith(u8, include_arg, "-I")) {
                try result.includes.append(include_arg[2..]);
            } else {
                try result.includes.append(include_arg);
            }
        }
    }
    {
        var it = mem.tokenize(libdir_output, " \r\n");
        while (it.next()) |libdir| {
            if (mem.startsWith(u8, libdir, "-L")) {
                try result.libdirs.append(libdir[2..]);
            } else {
                try result.libdirs.append(libdir);
            }
        }
    }
    return result;
}

fn nextValue(index: *usize, build_info: []const u8) []const u8 {
    const start = index.*;
    while (true) : (index.* += 1) {
        switch (build_info[index.*]) {
            '\n' => {
                const result = build_info[start..index.*];
                index.* += 1;
                return result;
            },
            '\r' => {
                const result = build_info[start..index.*];
                index.* += 2;
                return result;
            },
            else => continue,
        }
    }
}

fn getContext(b: *Builder) !Context {
    // find the stage0 build artifacts because we're going to re-use config.h and zig_cpp library
    const build_info = try b.exec(&[_][]const u8{
        b.zig_exe,
        "BUILD_INFO",
    });
    var index: usize = 0;
    var ctx = Context{
        .cmake_binary_dir = nextValue(&index, build_info),
        .cxx_compiler = nextValue(&index, build_info),
        .llvm_config_exe = nextValue(&index, build_info),
        .lld_include_dir = nextValue(&index, build_info),
        .lld_libraries = nextValue(&index, build_info),
        .dia_guids_lib = nextValue(&index, build_info),
        .llvm = undefined,
    };
    ctx.llvm = try findLLVM(b, ctx.llvm_config_exe);

    return ctx;
}

fn linkStdCpp(b: *Builder, exe: var, ctx: Context) !void {
    if (exe.target.getOs() == .linux) {
        try addCxxKnownPath(b, ctx, exe, "libstdc++.a",
            \\Unable to determine path to libstdc++.a
            \\On Fedora, install libstdc++-static and try again.
        );

        exe.linkSystemLibrary("pthread");
    } else if (exe.target.isFreeBSD()) {
        try addCxxKnownPath(b, ctx, exe, "libc++.a", null);
        exe.linkSystemLibrary("pthread");
    } else if (exe.target.isDarwin()) {
        if (addCxxKnownPath(b, ctx, exe, "libgcc_eh.a", "")) {
            // Compiler is GCC.
            try addCxxKnownPath(b, ctx, exe, "libstdc++.a", null);
            exe.linkSystemLibrary("pthread");
            // TODO LLD cannot perform this link.
            // See https://github.com/ziglang/zig/issues/1535
            exe.enableSystemLinkerHack();
        } else |err| switch (err) {
            error.RequiredLibraryNotFound => {
                // System compiler, not gcc.
                exe.linkSystemLibrary("c++");
            },
            else => return err,
        }
    }

    if (ctx.dia_guids_lib.len != 0) {
        exe.addObjectFile(ctx.dia_guids_lib);
    }

    exe.linkSystemLibrary("c");
}

fn addCxxKnownPath(
    b: *Builder,
    ctx: Context,
    exe: var,
    objname: []const u8,
    errtxt: ?[]const u8,
) !void {
    const path_padded = try b.exec(&[_][]const u8{
        ctx.cxx_compiler,
        b.fmt("-print-file-name={}", objname),
    });
    const path_unpadded = mem.tokenize(path_padded, "\r\n").next().?;
    if (mem.eql(u8, path_unpadded, objname)) {
        if (errtxt) |msg| {
            warn("{}", msg);
        } else {
            warn("Unable to determine path to {}\n", objname);
        }
        return error.RequiredLibraryNotFound;
    }
    exe.addObjectFile(path_unpadded);
}
