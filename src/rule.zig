const std = @import("std");
const assert = std.debug.assert;

pub const Rule = struct {
    name: []u8,
    lint: []u8,
    mesg: []u8,
    payl: []u8,
};

const RulesError = error{
    EmptyField,
    MissingFields,
    ExtraFields,
};

pub fn fieldSize(txt: []const u8, ptr: *usize) usize {
    var src = txt;
    var len = txt.len;
    var valid: usize = 0;
    while (ptr.* < len and src[ptr.*] != ';' and src[ptr.*] != '\n') {
        if (src[ptr.*] == 92 and (src[ptr.* + 1] == 92 or src[ptr.* + 1] == ';' or src[ptr.* + 1] == '\n')) {
            ptr.* += 1;
        }
        valid += 1;
        ptr.* += 1;
    }

    return valid;
}

pub fn strEsc(alloc: *std.mem.Allocator, txt: []const u8, size: usize) ![]u8 {
    var valid: usize = 0;
    var new = try alloc.alloc(u8, size);
    var cur: usize = 0;
    const len = txt.len;
    while (cur < len) {
        if (txt[cur] == 92 and (txt[cur + 1] == 92 or txt[cur + 1] == ';' or txt[cur + 1] == '\n')) {
            cur += 1;
        }
        new[valid] = txt[cur];
        valid += 1;
        cur += 1;
    }

    return new;
}

pub fn buildRules(alloc: *std.mem.Allocator, rules_txt: []const u8) !std.ArrayList(Rule) {
    var rules = std.ArrayList(Rule).init(alloc);
    errdefer rules.deinit();

    var cur: usize = 0;
    var end: usize = cur;
    const len = rules_txt.len;
    while (cur < len) {
        var rule = Rule{
            .name = undefined,
            .lint = undefined,
            .mesg = undefined,
            .payl = undefined,
        };

        // Attempt to parse name
        const name_size = fieldSize(rules_txt, &end);
        if (name_size == 0) {
            return RulesError.EmptyField;
        } else if (end >= len or rules_txt[end] != ';') {
            return RulesError.MissingFields;
        }
        rule.name = try strEsc(alloc, rules_txt[cur..end], name_size);
        errdefer alloc.free(rule.name);
        cur = end + 1;
        end = cur;

        // Attempt to parse rule
        const rule_size = fieldSize(rules_txt, &end);
        if (name_size == 0) {
            return RulesError.EmptyField;
        } else if (end >= len or rules_txt[end] != ';') {
            return RulesError.MissingFields;
        }
        rule.lint = try strEsc(alloc, rules_txt[cur..end], rule_size);
        errdefer alloc.free(rule.lint);
        cur = end + 1;
        end = cur;

        // Attempt to parse mesg
        const mesg_size = fieldSize(rules_txt, &end);
        if (name_size == 0) {
            return RulesError.EmptyField;
        } else if (end >= len or rules_txt[end] != ';') {
            return RulesError.MissingFields;
        }
        rule.mesg = try strEsc(alloc, rules_txt[cur..end], mesg_size);
        errdefer alloc.free(rule.mesg);
        cur = end + 1;
        end = cur;

        // Attempt to parse payl
        const payl_size = fieldSize(rules_txt, &end);
        if (payl_size == 0) {
            return RulesError.EmptyField;
        } else if (end < len and rules_txt[end] == ';') {
            return RulesError.ExtraFields;
        }
        rule.payl = try strEsc(alloc, rules_txt[cur..end], payl_size);
        errdefer alloc.free(rule.payl);
        cur = end + 1;
        end = cur;

        try rules.append(rule);
        break;
    }

    return rules;
}

pub fn printRules(rules: std.ArrayList(Rule)) void {
    var cur: usize = 0;
    while (cur < rules.len) {
        const r = rules.at(cur);
        std.debug.warn("{} | {} | {} | {}\n", r.name, r.lint, r.mesg, r.payl);
        cur += 1;
    }
}

test "strEsc" {
    var s = "hello\\; world";
    var c: []const u8 = "hello; world";
    var offset: usize = 0;
    var o = strEsc(std.debug.global_allocator, s, 12) catch unreachable;

    assert(std.mem.eql(u8, c, o));
}

test "fieldSize" {
    var s = "hello\\; world";
    var offset: usize = 0;
    var o = fieldSize(s, &offset);
    const c = 12;

    assert(o == c);
    assert(offset == c + 1);
}
